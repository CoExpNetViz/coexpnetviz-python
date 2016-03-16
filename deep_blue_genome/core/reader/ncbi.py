import pandas
import numpy as np
from chicken_turtle_util.list import is_sorted

def read_gene_info_file_chunked(path, chunk_size, usecols=None):
    '''
    Read NCBI gene info file
    
    E.g. `ftp://ftp.ncbi.nlm.nih.gov/gene/DATA/gene_info.gz` after unpacking.
    
    Parameters
    ----------
    path : str
        path to file to read
        
    chunk_size : int
        Chunk size in bytes
        
    usecols : array-like, default None
        Return a subset of the columns.
        Results in much faster parsing time and lower memory usage.
    
    Returns
    -------
    pandas.DataFrame
        Gene info as data frame with gene ids as index, a header line and NaN for empty fields 
    '''
    column_names = np.array('tax_id gene_id symbol locus_tag synonyms db_xrefs chromosome map_location description type_of_gene symbol_from_nomenclature_authority full_name_from_nomenclature_authority nomenclature_status other_designations modification_date'.split())
    assert is_sorted(usecols), 'Order in usecols is ignored in Pandas'
    if usecols:
        column_names = column_names[usecols]
        if 1 not in usecols:
            usecols.insert(0, 1)
            index_col = 0
        elif 0 not in usecols:
            index_col = 0
    rows_per_chunk = int(chunk_size / 113.79378692106279)  # = size / average characters per row 
    return pandas.read_table(path, index_col=index_col, header=None, names=column_names, skiprows=1, engine='python', na_values=['-','NEWENTRY'], usecols=usecols, chunksize=rows_per_chunk)