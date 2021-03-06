# Copyright (C) 2015-2021 VIB/BEG/UGent - Tim Diels <tim@diels.me>
#
# This file is part of CoExpNetViz.
#
# CoExpNetViz is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# CoExpNetViz is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with CoExpNetViz.  If not, see <http://www.gnu.org/licenses/>.

import logging

from varbio import pearson_df, join_lines
import numpy as np
import pandas as pd

from coexpnetviz._various import (
    Network, ExpressionMatrixInfo, distinct_colours, RGB
)


# In hindsight it would have made sense to either map genes to nodes, with node
# ids, up front; or to do away with node ids entirely and always use the gene
# name. Probably the latter is a good option, assuming they are unique.

def create_network(baits, expression_matrices, gene_families, percentiles=(5, 95)):
    cors, matrix_infos = _correlate_matrices(
        expression_matrices, baits, percentiles
    )
    nodes = _create_nodes(baits, cors, gene_families)
    homology_edges = _create_homology_edges(nodes)
    cor_edges = _create_cor_edges(nodes, cors)

    return Network(
        significant_cors=cors,
        nodes=nodes,
        homology_edges=homology_edges,
        cor_edges=cor_edges,
        matrix_infos=matrix_infos,
    )

def _correlate_matrices(expression_matrices, baits, percentiles):
    results = tuple(
        _correlate_matrix(matrix, baits, percentiles)
        for matrix in expression_matrices
    )

    cors = pd.concat([result[0] for result in results])
    # Drop self comparisons and symmetrical ones. There are no duplicates
    # because baits do not appear in multiple matrices.
    cors = cors[cors['bait'] < cors['gene']]

    matrix_infos = tuple(result[1] for result in results)
    return cors, matrix_infos

def _correlate_matrix(matrix, baits, percentiles):
    matrix_df = matrix.data

    # Remove rows with no variance as correlation functions yield nan for it
    #
    # Note: we only drop the absolutely necessary so that the user can
    # choose how to clean the expression matrices instead of the algorithm
    # doing it for them
    tiny_stds = matrix_df.std(axis=1) < np.finfo(float).tiny
    rows_dropped = sum(tiny_stds)
    if rows_dropped:
        matrix_df = matrix_df[~tiny_stds]
        logging.warning(join_lines(
            f'''
            Dropped {rows_dropped} out of {len(matrix_df)+rows_dropped} rows
            from {matrix} due to having (near) 0 standard deviation. These rows
            have a NaN correlation with any other row.
            '''
        ))
        if matrix_df.empty:
            raise ValueError(join_lines(
                f'''
                After dropping rows with tiny standard deviation, {matrix} has
                no rows. Please check the expression matrix for errors or drop
                it from the input.
                '''
            ))

    # Get cutoffs
    sample, cutoffs = _estimate_cutoffs(matrix, percentiles)
    cutoffs = tuple(cutoffs)
    lower_cutoff, upper_cutoff = cutoffs

    # Correlation matrix
    present_baits = matrix_df.reindex(baits).dropna()
    cors = pearson_df(matrix_df, present_baits)
    cor_matrix = cors

    # Cutoff and reformat to relational (DB) format
    cors = cors[(cors <= lower_cutoff) | (cors >= upper_cutoff)]
    cors.index.name = None
    cors = cors.reset_index()
    cors = cors.rename(columns={'index': 'gene'})
    cors = pd.melt(cors, id_vars=['gene'], var_name='bait', value_name='correlation')
    cors = cors.dropna(subset=['correlation'])

    return cors, ExpressionMatrixInfo(matrix, sample, cutoffs, cor_matrix)

def _estimate_cutoffs(matrix, percentiles):
    '''
    Estimate upper and lower correlation cutoffs

    Takes the x-th and y-th percentile of a sample similarity matrix of
    `matrix`, returning these as the lower and upper cut-off respectively.

    Using a sample as calculating all correlations is n**2. Sample size is
    chosen to be easy enough to calculate; for a large matrix our estimate
    is less accurate than for a small matrix. We could report a confidence
    interval for the cut-off estimate if that bothers us or more likely delve
    into statistics to find a better cut-off. In practice the user just plays
    with the percentiles until the result is what they want; so this is good
    enough.
    '''
    matrix_df = matrix.data

    # Take a sample unless it's a tiny matrix
    if len(matrix_df) <= 800:
        sample = matrix_df
    else:
        sample_size = 800
        sample = np.random.choice(len(matrix_df), sample_size, replace=False)
        sample = matrix_df.iloc[sample]

    sample = sample.sort_index()  # for prettier output later
    cors = pearson_df(sample, sample)

    # Get the upper triangle as 1D array, excluding the diagonal.
    #
    # The diagonal is pearson(x, x) == 1, so we ignore that. The matrix is
    # symmetric as pearson(x, y) == pearson(y, x); so we only need to look at
    # the upper triangle.
    triu = cors.values[np.triu_indices(len(cors), 1)]

    # Warn if >10% NaN
    nan_count = np.isnan(triu).sum()
    if nan_count > triu.size * .1:
        logging.warning(join_lines(
            f'''
            Correlation sample of {matrix} contains more than 10% NaN values,
            specifically {nan_count}/{triu.size} correlations are NaN (only
            including non-diagonal upper triangle correlation matrix values).
            '''
        ))

    # Ignore NaN values when calculating percentiles
    triu = triu[~np.isnan(triu)]
    cutoffs = np.percentile(triu, percentiles)

    return cors, cutoffs

def _create_nodes(baits, cors, gene_families):
    '''
    Create DataFrame of nodes

    Columns (see node attrs in the docs):

    id : int
    label : str
    type : 'bait', 'family' or 'gene'
    genes : FrozenSet of str
    family : str or None
    colour : RGB
    partition_id : int
    '''
    bait_nodes = _create_bait_nodes(baits, gene_families)
    family_nodes, gene_nodes = _create_non_bait_nodes(baits, cors, gene_families)
    return _concat_nodes(bait_nodes, family_nodes, gene_nodes)

def _create_bait_nodes(baits, gene_families):
    nodes = baits.to_frame('genes')
    nodes = nodes.reset_index(drop=True)
    nodes['type'] = 'bait'
    nodes = pd.merge(
        nodes, gene_families, left_on='genes', right_on='gene', how='left'
    )
    del nodes['gene']
    nodes['label'] = nodes['genes']
    nodes['genes'] = nodes['genes'].apply(lambda gene: frozenset({gene}))
    nodes['colour'] = RGB((255, 255, 255))
    nodes['partition_id'] = hash(frozenset())
    return nodes

def _create_non_bait_nodes(baits, cors, gene_families):
    is_not_a_bait = ~cors['gene'].isin(baits)
    cors = cors[is_not_a_bait]
    family_nodes = pd.DataFrame(
        columns=('baits', 'label', 'type', 'genes', 'family')
    )
    gene_nodes = pd.DataFrame(
        columns=('baits', 'label', 'type', 'gene')
    )
    if not cors.empty:
        # Split into family and gene nodes
        cors = pd.merge(cors, gene_families, on='gene', how='left')
        gene_nodes = cors[cors['family'].isnull()]
        cors = cors.dropna()  # drop gene nodes

        if not cors.empty:
            family_nodes = (
                cors.groupby('family')[['bait', 'gene']]
                .agg(frozenset)
                .reset_index()
            )
            family_nodes.columns = ('family', 'baits', 'genes')
            family_nodes['type'] = 'family'
            family_nodes['label'] = family_nodes['family']

        if not gene_nodes.empty:
            gene_nodes = (
                gene_nodes.groupby('gene')[['bait']]
                .agg(frozenset)
                .reset_index()
            )
            gene_nodes.columns = ('gene', 'baits')
            gene_nodes['label'] = gene_nodes['gene']
            gene_nodes['type'] = 'gene'
            gene_nodes['genes'] = gene_nodes['gene'].apply(lambda gene: frozenset({gene}))
            del gene_nodes['gene']
    return family_nodes, gene_nodes

def _concat_nodes(bait_nodes, family_nodes, gene_nodes):
    nodes = pd.concat((family_nodes, gene_nodes), ignore_index=True)

    # Add partitions and colours to non-bait nodes
    if not nodes.empty:
        nodes['partition_id'] = nodes['baits'].apply(hash)
        del nodes['baits']
        partitions = nodes[['partition_id']].drop_duplicates()
        colours = list(distinct_colours(len(partitions)))
        partitions['colour'] = colours
        nodes = pd.merge(nodes, partitions, on='partition_id')

    # Assign ids to all nodes
    nodes = pd.concat((nodes, bait_nodes), ignore_index=True)
    nodes.index.name = 'id'
    nodes = nodes.reset_index()

    # Replace NaN with None (can't use replacena for that)
    nodes = nodes.where(pd.notna(nodes), None)

    return nodes

def _create_homology_edges(nodes):
    '''
    Create DataFrame of homology edges between baits with columns:

    bait_node1 : int
    bait_node2 : int
        Node id
    '''
    bait_nodes = nodes[nodes['type'] == 'bait'][['id', 'family']]
    bait_nodes = bait_nodes.dropna(subset=('family',)).rename(columns={'id': 'bait_node'})
    homology_edges = pd.merge(bait_nodes, bait_nodes, on='family', suffixes=('1', '2'))
    del homology_edges['family']
    # Avoid self/symmetrical edges (and there are no duplicate edges to begin
    # with)
    bait1_lt_bait2 = homology_edges['bait_node1'] < homology_edges['bait_node2']
    homology_edges = homology_edges[bait1_lt_bait2].copy()
    return homology_edges

def _create_cor_edges(nodes, cors):
    '''
    Create DataFrame of correlation edges between nodes.

    cors param has no self/symmetrical/duplicate edges.

    Columns:

    bait_node : int
    node : int
    max_correlation : float
    '''
    if cors.empty:
        return pd.DataFrame(columns=('bait_node', 'node', 'max_correlation'))

    # Map cors.bait and cors.gene to their node id.
    #
    # Do include bait-bait cors (coexpnetviz/coexpnetviz#13)
    cors = cors.copy()
    nodes = nodes[['id', 'genes', 'type']].copy()
    nodes = nodes.explode('genes').set_index('genes')
    cors.update(cors['bait'].map(nodes['id']))
    cors.update(cors['gene'].map(nodes['id']))
    cors = cors.rename(columns={'bait': 'bait_node', 'gene': 'node'})

    # Summarise correlations per edge by taking the max (in the abs sense)
    # per nodes of an edge
    groups = cors.groupby(['bait_node', 'node'])[['correlation']]
    cors = groups.agg(lambda x: x.iloc[x.abs().argmax()])

    cors = cors.reset_index()
    cors = cors.rename(columns={'correlation': 'max_correlation'})
    return cors
