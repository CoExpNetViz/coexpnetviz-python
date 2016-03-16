# Copyright (C) 2015, 2016 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
# 
# This file is part of Deep Blue Genome.
# 
# Deep Blue Genome is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Deep Blue Genome is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with Deep Blue Genome.  If not, see <http://www.gnu.org/licenses/>.


import pandas as pd
import plumbum as pb
import csv
from chicken_turtle_util.pandas import df_expand_iterable_values
from chicken_turtle_util.algorithms import merge_overlapping_named_sets
from itertools import chain
from io import TextIOWrapper

# TODO validation should be part of reading. We at least want to save a log of
# warnings. TODO input validation error handling + setting stuff as warning or
# error. Different users care about different things, in different use cases. By
# default there should be a strict config and a more lenient config. Sadly, the
# lenient config should be picked as default. In either case, errors and warnings
# will be logged. Though keep in mind that in the future one might even want to limit logging.

'''
File reading.

Duplicate gene names in input will generally not be removed in the output.
(Dropping duplicate gene names does not remove synonyms, thus the same gene
would still be duplicated in the list (instead use get_genes_by_name, then
drop_duplicates))

Input validation error handling can be configured to either:

- show and/or log an error and attempt to recover
- throw an exception

Some of the design principles used:

- Don't trust the user, sanitise frantically
'''

# XXX test promises in robustness for the various file formats

# XXX in the future this may be of interest https://pypi.python.org/pypi/python-string-utils/0.3.0  We could donate our own things to that library eventually...

# XXX switch from pb.Path to pathlib from the std. pb will still be used for writing shell-like scripts, but its pb.Path will be avoided and will not be exposed in one of our API
    
_sed = pb.local['sed']
_tr = pb.local['tr']

def _get_sanitised_plain_text_file(path):   # XXX need to change sanitise to stream output (= most flexible, but probably not necessary) or add destination file
    '''
    Get sanitised contents of plain text file.
    
    - Remove null characters
    - Fix newlines, drop empty lines
    - Replace multiple tabs by single tab.
    
    Parameters
    ----------
    path : plumbum.Path
        Path to file to sanitise
        
    Returns
    -------
    io.BufferedReader
        Stream of sanitised file contents
    '''
    cmd = _sed['-r', '-e', r's/[\x0]//g', '-e', r's/(\t)+/\t/g', path] | _tr['-s', r'\r', r'\n'] 
    return TextIOWrapper(cmd.popen().stdout, encoding='utf-8')

def read_expression_matrix_file(path, sanitise=True):
    '''
    Read expression matrix file.
    
    This file has 1 header line and 0 or more rows. The first column is the gene
    (or gene variant) name. The other columns contain the expression values of
    each condition.
    
    Lines are separated by one or more newline characters (any amount of \r or
    \n). Empty lines are ignored. Columns are separated by any whitespace other
    than newline characters. Empty fields are ignored.
    
    Parameters
    ----------
    path : hasattr(__str__)
        Path to expression matrix file to read
    sanitise : bool
        Sanitise file before reading.
    
    Returns
    -------
    pandas.DataFrame({condition_name : [float]}, index=('gene' : [str]))
    '''
    if sanitise:
        path = _get_sanitised_plain_text_file(path)
    mat = pd.read_table(path, index_col=0, header=0, engine='python').astype(float)
    mat = mat[mat.index.to_series().notnull()]  # TODO log warnings for dropped rows
    mat.index.name = 'gene'
    return mat

def read_clustering_file(path, name_index=0, merge_overlapping=False, sanitise=True):
    '''
    Read generic clustering.
    
    Expected format: csv format with tabs as separator (instead of ',').
    
    Each row is a cluster: `cluster_name item1 item2 ...`. (An item may occur
    in multiple clusters).
    
    Robustness notes:
    
    - removes duplicates (e.g. if an item is part of a cluster twice, it removes one occurence)
    - sanitises input file (e.g. newline errors)
    
    Clusters can also be split across multiple lines::
    
        cluster1 item1
        cluster1 item2
        cluster2 item5
        cluster1 item3
    
    Parameters
    ----------
    path : str
        Path to clustering file to read
    name_index : int
        If None, each line is an unnamed cluster, else column with index `name_index` refers to cluster names.
    merge_overlapping : bool
        If True, merge overlapping clusters.
    sanitise : bool
        Sanitise file before reading.
        
    Returns
    -------
    pandas.DataFrame(columns=[cluster_id, item : str]))
        Clustering in relational table format. If `merge_overlapping`,
        `cluster_id` is a set of cluster ids.
    '''
    # Read file
    if sanitise:
        f = _get_sanitised_plain_text_file(path)
    else:
        f = open(path, 'r', encoding='utf-8')
    with f:
        reader = csv.reader(f, delimiter='\t')
        if name_index is not None:
            df = pd.DataFrame(([row[name_index], row[0:name_index] + row[name_index+1:]] for row in reader), columns=['cluster_id', 'item'])
            df['cluster_id'] = df['cluster_id'].str.lower()
        else:
            df = pd.DataFrame(([row] for row in reader), columns=['item'])
            df.index.name = 'cluster_id'
            df.reset_index(inplace=True)
    
    # Merge overlapping, maybe
    if merge_overlapping:
        groups = df.groupby('cluster_id')['item'].apply(lambda x: set(chain(*x)))
        groups = merge_overlapping_named_sets(({name}, set_) for name, set_ in groups.items())
        df = pd.DataFrame(groups, columns=['cluster_id', 'item'])
    
    # Split lists
    df = df_expand_iterable_values(df, ['item'])
    
    # Finish up
    df.drop_duplicates(inplace=True)
    return df
 
def read_gene_families_file(path, sanitise=True):
    '''
    Read a gene families file.
    
    Expected format: csv format with tabs as separator (instead of ',').
    
    Each row is a gene family: `family_name gene1 gene2 ...`. (A gene may occur
    in multiple gene families).
    
    Families can also be split across multiple lines::
    
        fam1 gene1
        fam1 gene2
        fam2 gene5
        fam1 gene3
    
    Example return::
    
        family
        name1  AT5G41040
        name1  AT5G23190
        name2  AT3G11430
    
    Parameters
    ----------
    path : str
        path to gene families file to read
    sanitise : bool
        Sanitise file before reading.
    
    Returns
    -------
    pandas.DataFrame(columns=[family : str, gene : str])
        Gene families
    '''
    # XXX use read_clustering
    if sanitise:
        f = _get_sanitised_plain_text_file(path)
    else:
        f = open(path, 'r', encoding='utf-8')
    with f:
        reader = csv.reader(f, delimiter="\t")
        df = pd.DataFrame(([row[0], row[1:]] for row in reader), columns='family gene'.split())
    df = df_expand_iterable_values(df, ['gene'])
    df['family'] = df['family'].str.lower()
    return df

def read_genes_file(path, sanitise=True):
    '''
    Read a file of whitespace separated gene names.
    
    Example return::
    
        gene
        0  AT5G41040
        1  AT5G23190
        2  AT3G11430
        
    Parameters
    ----------
    path : str
        path to file to read
    sanitise : bool
        Sanitise file before reading.
    
    Returns
    -------
    pandas.Series(data=(gene : str))
        Series of genes
    '''
    if sanitise:
        f = _get_sanitised_plain_text_file(path)
    else:
        f = open(path, encoding='utf-8')
    with f:
        baits = pd.Series(f.read().split(), name='gene')
        return baits

# XXX Temporarily unused
# def read_mcl_clustering(path):
#     '''
#     Read MCL clustering outputted by an `--abc` run.
#     
#     No input sanitisation as output is expected to come directly from the MCL
#     algorithm (which is well behaved).
#         
#     Parameters
#     ----------
#     path : str
#         path to file to read
#     
#     Returns
#     -------
#     pandas.DataFrame(data=[cluster_id : int, item : str])
#         Clusters
#     '''
#     # XXX use read_clustering
#     df = pd.read_csv(path, names=['item']).applymap(lambda x: x.lower().split())
#     df.index.name = 'cluster_id'
#     df.reset_index(inplace=True)
#     df = df_expand_iterable_values(df, 'item')
#     return df

def read_gene_mapping_file(path, sanitise=True): 
    '''
    Read a gene mapping file
    
    A gene mapping is a non-transitive, symmetric, reflexive relation between gene names. 
    An example of this is the rice MSU-RAP mapping (http://www.thericejournal.com/content/6/1/4).
    
    For each line, gene in first column maps to the genes in the other columns.
    By symmetry, each item on the right hand side maps to the item in the first
    column.
    
    Returns
    -------
    pandas.DataFrame(columns=[left : str, right : str])
        Mapping
    sanitise : bool
        Sanitise file before reading.
    '''
    gene_mapping = read_clustering_file(path, name_index=0, merge_overlapping=False, sanitise=sanitise)
    gene_mapping.columns = ['left', 'right']
    return gene_mapping
