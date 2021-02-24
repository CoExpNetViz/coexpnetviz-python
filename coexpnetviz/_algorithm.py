# Copyright (C) 2015 VIB/BEG/UGent - Tim Diels <tim@diels.me>
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

from itertools import product
from textwrap import dedent
import logging

from varbio import pearson_df
import numpy as np
import pandas as pd

from coexpnetviz._various import (
    Network, NodeType, distinct_colours, RGB
)


# TODO refactor: inplace does not necessarily improve performance. Remove
# unnecessary use of inplace.

def create_network(baits, expression_matrices, gene_families, percentile_ranks=(5, 95)):
    '''
    Create a comparative co-expression network.

    Parameters
    ----------
    baits : ~pandas.Series[str]
        Genes to which all genes are compared.
    expression_matrices : ~typing.Sequence[~coexpnetviz.ExpressionMatrix]
        Gene expression matrices containing at least one bait.
    gene_families : ~pandas.DataFrame
        Gene families (to make family nodes with) as data frame with columns:

        family
            `str` -- Family name.
        gene
            `str` -- A gene in the family.

        There should be no duplicate rows.

    percentile_ranks : ~pytil.numpy.ArrayLike[float]
        Lower and upper percentile ranks respectively to use as cutoff. For each
        matrix, 2 percentiles are derived. Genes are only considered
        co-expressed when their correlation value does not lie between the lower
        and upper percentile. The percentiles are derived by taking a sample of
        rows from the matrix, constructing a correlation matrix of all vs all
        genes in the sample and forming a distribution with all the correlations
        of the correlation matrix.

    Returns
    -------
    ~coexpnetviz._various.Network
        The created network. A bait node for each bait is included. However,
        family nodes are only included if at least one of their genes correlates
        with a bait. Similarly, only gene nodes which correlate with a bait are
        included.
    '''
    _validate_input(baits, expression_matrices, gene_families, percentile_ranks)

    corrs, samples, percentiles, corr_matrices = _correlate_matrices(
        expression_matrices, baits, percentile_ranks
    )
    nodes = _get_nodes(baits, corrs, gene_families)
    homology_edges = _get_homology_edges(nodes)
    correlation_edges = _get_correlation_edges(nodes, corrs)

    return Network(
        significant_correlations=corrs,
        samples=samples,
        percentiles=percentiles,
        correlation_matrices=corr_matrices,
        nodes=nodes,
        homology_edges=homology_edges,
        correlation_edges=correlation_edges,
    )

def _validate_input(baits, expression_matrices, gene_families, percentile_ranks):
    # Validate baits (more complex validate below)
    if baits.empty:
        raise ValueError('Must specify at least one bait, got: {}'.format(baits))

    # Validate percentile ranks
    percentile_ranks = np.array(percentile_ranks)
    out_of_bounds = (percentile_ranks < 0) | (percentile_ranks > 100)
    if out_of_bounds.any():
        raise ValueError(
            'Percentile ranks must be in range of [0, 100], got: {}'
            .format(percentile_ranks)
        )
    if percentile_ranks[0] > percentile_ranks[1]:
        raise ValueError(
            'Lower percentile rank must be less or equal to upper percentile rank, got: {}'
            .format(percentile_ranks)
        )

    # Validate expression_matrices
    if not expression_matrices:
        raise ValueError(
            'Must provide at least one expression matrix, got: {}'
            .format(expression_matrices)
        )
    names = [matrix.name for matrix in expression_matrices]
    if len(expression_matrices) != len(set(names)):
        raise ValueError(
            'Expression matrices must have unique name, got: {}'
            .format(sorted(names))
        )

    # Check each bait occurs in exactly one matrix
    bait_presence = np.array([
        bait in matrix.data.index
        for matrix, bait in product(expression_matrices, baits)
    ])
    bait_presence = bait_presence.reshape(len(expression_matrices), len(baits))
    missing_bait_matrix = pd.DataFrame(
        bait_presence,
        index=expression_matrices,
        columns=baits
    )
    missing_bait_matrix = missing_bait_matrix.loc[:,bait_presence.sum(axis=0) != 1]
    if not missing_bait_matrix.empty:
        missing_bait_matrix = missing_bait_matrix.applymap(lambda x: 'present' if x else 'absent')
        missing_bait_matrix.index = missing_bait_matrix.index.map(lambda matrix: matrix.name)
        missing_bait_matrix.index.name = 'Matrix name'
        missing_bait_matrix.columns.name = 'Gene name'
        raise ValueError(dedent('''\
            Each of the following baits is either missing from all or present in
            multiple expression matrices:
            
            {}
            
            Missing baits are columns with no "present" value, while baits in
            multiple matrices have multiple "present" values in a column.''')
            .format(missing_bait_matrix.to_string())
        )

    # and each matrix has at least one bait
    is_baitless = bait_presence.sum(axis=1) == 0
    if is_baitless.any():
        matrices = np.array(expression_matrices)[is_baitless]
        matrices = ', '.join(map(str, matrices))
        raise ValueError(
            'Some expression matrices have no baits: {}. '
            'Each expression matrix must contain at least one bait. '
            'Either drop the matrices or add some of their genes to the baits list.'
            .format(matrices)
        )

    # Check the matrices don't overlap (same gene in multiple matrices)
    all_genes = pd.Series(sum((list(matrix.data.index) for matrix in expression_matrices), []))
    overlapping_genes = all_genes[all_genes.duplicated()]
    if not overlapping_genes.empty:
        raise ValueError(
            'The following genes appear in multiple expression matrices: {}. '
            'CoExpNetViz does not support gene expression data from different matrices for the same gene. '
            'Please remove rows from the given matrices such that no gene appears in multiple matrices.'
            .format(', '.join(overlapping_genes))
        )

def _correlate_matrices(expression_matrices, baits, percentile_ranks):
    results = tuple(
        _correlate_matrix(expression_matrix, baits, percentile_ranks)
        for expression_matrix in expression_matrices
    )

    corrs = pd.concat([result[0] for result in results])
    # Drop self comparisons
    corrs = corrs[corrs['bait'] < corrs['gene']]
    corrs = corrs.reindex(columns=('bait', 'gene', 'correlation'))

    samples = tuple(result[1] for result in results)
    percentiles = tuple(result[2] for result in results)
    corr_matrices = tuple(result[3] for result in results)

    return corrs, samples, percentiles, corr_matrices

def _correlate_matrix(matrix, baits, percentile_ranks):
    matrix_data = matrix.data

    # Remove rows with no variance as correlation functions yield nan for it
    #
    # Note: we only drop the absolutely necessary so that the user can
    # choose how to clean the expression matrices instead of the algorithm
    # doing it for them
    tiny_stds = matrix_data.std(axis=1) < np.finfo(float).tiny
    rows_dropped = sum(tiny_stds)
    if rows_dropped:
        matrix_data = matrix_data[~tiny_stds]
        logging.warning(
            'Dropped {} out of {} rows from {} due to having (near) 0 standard deviation. '
            'These rows have a NaN correlation with any other row.'
            .format(rows_dropped, len(matrix_data)+rows_dropped, matrix)
        )
        if matrix_data.empty:
            raise ValueError(
                'After dropping rows with tiny standard deviation, {} has no rows. '
                'Please check the expression matrix for errors or drop it from the input.'
                .format(matrix)
            )

    # Get cutoffs
    sample, percentiles = _get_cutoffs(matrix, matrix_data, percentile_ranks)
    percentiles = tuple(percentiles)
    lower_cutoff, upper_cutoff = percentiles

    # Baits present in matrix
    baits_ = matrix_data.reindex(baits).dropna()

    # Correlation matrix
    corrs = pearson_df(matrix_data, baits_)
    corrs.index.name = None
    corrs.columns.name = None
    corr_matrix = corrs.copy()

    # Apply cutoff
    corrs = corrs[(corrs <= lower_cutoff) | (corrs >= upper_cutoff)]
    # TODO not sure if helps performance. If not, this is unnecessary
    corrs.dropna(how='all', inplace=True)

    # Reformat to relational (DB) format
    corrs.index.name = 'gene'
    corrs.reset_index(inplace=True)
    corrs = pd.melt(corrs, id_vars=['gene'], var_name='bait', value_name='correlation')
    corrs.dropna(subset=['correlation'], inplace=True)

    return corrs, sample, percentiles, corr_matrix

def _get_cutoffs(expression_matrix, expression_matrix_, percentile_ranks):
    '''
    Get upper and lower correlation cutoffs for coexpnetviz

    Takes the 5th and 95th percentile of a sample similarity matrix of
    `expression_matrix`, returning these as the lower and upper cut-off
    respectively.

    Parameters
    ----------
    expression_matrix : Expressionmatrix
    expression_matrix_
        exp mat data

    Returns
    -------
    sample : pd.DataFrame
    np.array((lower, upper))
        Cut-offs
    '''
    # TODO we took a sample of the population of correlations, so take into
    # account statistics when drawing conclusions from it... In fact, that's how
    # we should determine our sample size, probably.
    # Or simply ask a stats person to review the stats used in CoExpNetViz

    # TODO should also take into account the sample size, e.g. if in some freak
    # case we have only 2 rows in the matrix, we won't be able to tell much this
    # way

    sample_size = min(len(expression_matrix_), 800)
    sample = np.random.choice(len(expression_matrix_), sample_size, replace=False)
    sample = expression_matrix_.iloc[sample]
    sample = sample.sort_index()
    sample = pearson_df(sample, sample)
    sample_ = sample.values.copy()
    nan_count = np.isnan(sample_).sum()
    np.fill_diagonal(sample_, np.nan)
    sample_ = sample_[~np.isnan(sample_)].ravel()

    size = sample.size - len(sample)  # minus the diagonal, as it's not part of sample_
    if nan_count > .1 * size: # XXX 10% is arbitrary pick
        logging.warning(
            'Correlation sample of {} contains more than 10% NaN values, specifically {} values out of a sample matrix of {} values are NaN'
            .format(expression_matrix, nan_count, size)
        )

    # Return result
    return sample, np.percentile(sample_, percentile_ranks)

def _get_nodes(baits, correlations, gene_families):
    # bait nodes
    assert not baits.empty
    bait_nodes = baits.to_frame('genes')
    bait_nodes.reset_index(drop=True, inplace=True)
    bait_nodes['type'] = NodeType.bait
    bait_nodes = pd.merge(bait_nodes, gene_families, left_on='genes', right_on='gene', how='left')
    del bait_nodes['gene']
    bait_nodes['label'] = bait_nodes['genes']
    bait_nodes['genes'] = bait_nodes['genes'].apply(lambda gene: frozenset({gene}))
    bait_nodes['colour'] = [RGB((255, 255, 255))] * len(bait_nodes)
    bait_nodes['partition_id'] = hash(frozenset())
    assert not bait_nodes.empty

    # other nodes
    correlations = correlations[~correlations['gene'].isin(baits)].copy()  # no baits
    if not correlations.empty:
        # split into family and gene nodes
        correlations = pd.merge(correlations, gene_families, on='gene', how='left')
        orphans = correlations[correlations['family'].isnull()].copy()
        correlations.dropna(inplace=True)  # no orphans
        nodes = []

        # family nodes
        if not correlations.empty:
            # TODO is it necessary?
            # pylint: disable=unnecessary-lambda
            family_nodes = (
                correlations.groupby('family')[['bait','gene']]
                .agg(lambda x: frozenset(x))
                .reset_index()
            )
            family_nodes.columns = ('family', 'baits', 'genes')
            family_nodes['type'] = NodeType.family
            family_nodes['label'] = family_nodes['family']
            nodes.append(family_nodes)

        # gene nodes
        if not orphans.empty:
            # TODO is it necessary?
            # pylint: disable=unnecessary-lambda
            orphans = (
                orphans.groupby('gene')[['bait']]
                .agg(lambda x: frozenset(x))
                .reset_index()
            )
            orphans.columns = ('gene', 'baits')
            orphans['label'] = orphans['gene']
            orphans['type'] = NodeType.gene
            orphans['genes'] = orphans['gene'].apply(lambda gene: frozenset({gene}))
            orphans.drop('gene', axis=1, inplace=True)
            nodes.append(orphans)

        # concat gene and family nodes
        nodes = pd.concat(nodes, ignore_index=True)

        # partitions and colours
        nodes['partition_id'] = nodes['baits'].apply(hash)
        nodes.drop('baits', axis=1, inplace=True)
        partitions = nodes[['partition_id']].drop_duplicates()
        colours = distinct_colours(len(partitions))
        # shuffle the colours so distinct colours are less likely to be put
        # next to each other
        colours = np.random.permutation([RGB.from_float(x) for x in colours])
        partitions['colour'] = colours
        nodes = pd.merge(nodes, partitions, on='partition_id')

        # concat bait nodes to other nodes
        nodes = pd.concat((nodes, bait_nodes), ignore_index=True)
    else:
        nodes = bait_nodes

    # assign ids
    nodes.index.name = 'id'
    nodes.reset_index(inplace=True)
    columns = (
        'id', 'label', 'type', 'genes', 'family', 'colour', 'partition_id'
    )
    nodes = nodes.reindex(columns=columns)

    # replace all na with None
    #
    # TODO instead of replacing with None, let nan and None roam free until the
    # point where it actually makes a difference, at that point you can use
    # .replace(np.nan, 'whatever you need'). Will have to be careful to use
    # pd.isna/notna in if statements and in bool()
    nodes = nodes.where(pd.notna(nodes), None)

    return nodes

def _get_homology_edges(nodes):
    bait_nodes = nodes[nodes['type'] == NodeType.bait][['id', 'family']]
    bait_nodes = bait_nodes.dropna(subset=('family',)).rename(columns={'id': 'bait_node'})
    homology_edges = pd.merge(bait_nodes, bait_nodes, on='family', suffixes=('1', '2'))
    del homology_edges['family']
    bait1_lt_bait2 = homology_edges['bait_node1'] < homology_edges['bait_node2']
    homology_edges = homology_edges[bait1_lt_bait2].copy()
    return homology_edges

def _get_correlation_edges(nodes, correlations):
    if not correlations.empty:
        correlations = correlations.copy()
        nodes = nodes[['id', 'genes']].copy()
        nodes.update(nodes['genes'].apply(list))
        ids = nodes.explode('genes').set_index('genes')['id']
        correlations.update(correlations['gene'].map(ids))
        correlations.update(correlations['bait'].map(ids))
        correlations.rename(columns={'bait': 'bait_node', 'gene': 'node'}, inplace=True)

        # Summarise correlations per edge by taking the max (in the abs sense)
        # per nodes of an edge
        groups = correlations.groupby(['bait_node', 'node'])[['correlation']]
        correlations = groups.agg(lambda x: x.iloc[x.abs().argmax()])

        correlations.reset_index(inplace=True)
        correlations.rename(columns={'correlation': 'max_correlation'}, inplace=True)
        correlations = correlations.reindex(columns=('bait_node', 'node', 'max_correlation'))
        return correlations
    else:
        return pd.DataFrame(columns=('bait_node', 'node', 'max_correlation'))
