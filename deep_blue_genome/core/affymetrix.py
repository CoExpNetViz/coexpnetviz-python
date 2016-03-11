import pandas
import os.path

'''
All things related to Affymetrix expression matrix files
'''

# Affymetrix files like to vary it up a bit in terms of column naming
_column_names = (
        ('Affymetrix:CHPSignal', 'Affymetrix:CHPDetection'),
        ('GEO:AFFYMETRIX_VALUE', 'GEO:AFFYMETRIX_ABS_CALL'),
        ('VALUE', 'ABS_CALL')
)

def _read(path, is_part):
    # TODO needed?: "$basedir/../any2unix.sh" "$@"
    header = 0 if is_part else [0,1]
    affy = pandas.read_table(str(path), index_col=0, header=header, engine='python')
    affy.index = affy.index.str.lower()
    return affy

def read_parts(*paths):
    '''
    Read unmerged affymetrix parts.

    Returns merged data frame with sorted columns index
    '''
    parts = [_read(path, True) for path in paths]
    condition_names = list(map(os.path.basename, paths))
    merged = pandas.concat(parts, axis=1, keys=condition_names, names=['Condition', 'Field'])
    merged.sortlevel(axis=1, inplace=True, sort_remaining=True)
    return merged

def read(path):
    '''
    Read full affymetrix file.

    Returns data frame with sorted columns index
    '''
    affy = _read(path, False)
    affy.sortlevel(axis=1, inplace=True, sort_remaining=True)
    return affy

def to_expression_matrix(affy, probe_to_gene):
    '''
    - affy: data frame as read
    - probe_to_gene: see `deep_blue_genome.reader.probe_gene_map` module

    Returns data frame in exp mat format
    '''

    # Ignore probes with any other suffix
    probe_suffix = 's1_at' # than this one
    affy = affy.loc[affy.index.str.endswith(probe_suffix)]

    # replace probe names with gene names
    # Note: not each probe maps to a gene. (Some probes serve as a scientific control)
    affy.index = affy.index.map(lambda x: probe_to_gene[x] if x in probe_to_gene else None)
    affy.drop(None)

    # Multiple probes can map to the same gene. When it does, we only use the
    # first encountered probe (TODO figure out a proper scheme of picking the
    # best probe; why are there multiple probes of the same gene in the first
    # place (gene variants perhaps?)?)
    affy = affy.groupby(level=0).first()

    # Determine the column naming used 
    for (signal_column, abs_call_column) in _column_names:
        if signal_column in affy.columns.get_level_values(1):
            break
    else:
        raise Exception('Unknown signal column name used in affymetrix file. Perhaps another variant of column naming should be added to the code?')

    # Filter to rows with favorable abs call for each condition  # TODO consider letting through those with a few failed abs calls. TODO take along the P-value if any, if doing proper bayesian calculations
    if abs_call_column in affy.columns.get_level_values(1):
        rows_with_all_present = affy.loc[:, (slice(None),abs_call_column)].apply(lambda x: x.str.match('(?i)p|present')).all(axis=1)
        affy = affy.loc[rows_with_all_present]

    # Transform to exp mat
    affy = affy.loc[:, (slice(None), signal_column)]
    affy.columns = affy.columns.droplevel(1)
    affy.columns = ['gene'] + affy.columns[1:].tolist()

    return affy

#def gup():
#    experiment_name="$1"
#
#    # For the unmerged ones
#    "$basedir/merge_data_files.py" "$@"
#    mv merged_data $experiment_name
#
#    #
#    file="$experiment_name"
#    stats_file="${file}.stats"
#    expr_file="${file}.expression_matrix"
#
#    write_stats():
#        wc -l "$file" >> "$stats_file"
#        # TODO switches to expr_file along the way
#
#    touch(stats_file)
#
#    affy = affymetrix.read(TODO) or read_parts(TODO, ...)
#    probe_to_gene = probe_gene_map.read(TODO, TODO)
#        conf.src_dir / 'rice/expression_matrices/affymetrix/probe_to_gene/probe_names gene_names'
#    affymetrix.to_expression_matrix(affy, probe_to_gene)
#
#
#    echo Quantile normalization
#    "$basedir/quantile_normalise.py" "$expr_file"
#
#    echo 'Filter rows with all(values < 10) or sd<0.1'
#    "$basedir/filter.py" '0.1' "$expr_file"
#    wc -l "$expr_file" >> "$stats_file"
#
#    #echo 'Filter rows with sd < 25'
#    #"$basedir/filter.py" 25 "$expr_file"
#    #wc -l "$expr_file" >> "$stats_file"
#    echo "NaN NA" >> "$stats_file"
#
#    #TODO output should go to `target`, no other file should be written
#
