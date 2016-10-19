# Copyright (C) 2015, 2016 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
# 
# This file is part of Deep Genome.
# 
# Deep Genome is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Deep Genome is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with Deep Genome.  If not, see <http://www.gnu.org/licenses/>.

from chicken_turtle_util import data_frame as df_

def gene_families(session, gene_families):
    '''
    Validate and interpret low-level gene families representation
    
    Parameters
    ----------
    session : deep_genome.core.database.Session
    gene_families : pd.DataFrame({'family' => [str], 'gene' => [str]})
        Gene families as data frame with columns:
        
        family
            Unique family name or id. Cannot be empty, ``nan``, ``None`` or
            contain a nul character.
        gene
            Gene symbol of gene present in the family
            
        Families musn't overlap (no gene is a member of multiple families).
        
    Returns
    -------
    pd.DataFrame({'family' => [str], 'gene' => [Gene]})
        Higher-level gene families representation
    
    Raises
    ------
    ValueError
        If:
        
        - families overlap
        - family name is invalid
        
    Examples
    --------
    Clean, parse and interpret gene families file::
        
        from deep_genome.core import clean
        from . import _parse, _interpret
        gene_families = _interpret.gene_families(_parse.gene_families(clean.plain_text(f)))
    '''
    if gene_families.empty:
        return gene_families
        
    gene_families = gene_families.copy()
    
    def assert_overlap(mapped):
        '''
        Assert families do not overlap
        '''
        duplicates = gene_families[gene_families['gene'].duplicated(keep=False)].copy()
        if not duplicates.empty:
            if mapped:
                duplicates['gene'] = duplicates['gene'].apply(lambda x: x.canonical_name.name)
                postfix = ' after gene mapping'
            else:
                postfix = ''
            duplicates = duplicates.sort_values(list(duplicates.columns))
            raise ValueError('gene_families contains overlap{}:\n{}'.format(postfix, duplicates.to_string(index=False)))
        
    # Check family names
    mask = gene_families['family'].apply(lambda x: not isinstance(x, str))
    invalid_families = gene_families[mask]
    if not invalid_families.empty:
        invalid_families = invalid_families.applymap(repr)
        raise ValueError('Gene family names must be strings. Got:\n{}'.format(invalid_families.to_string(index=False)))
    
    mask = gene_families['family'].apply(lambda x: not x or '\0' in x)
    invalid_families = gene_families[mask]
    if not invalid_families.empty:
        invalid_families = invalid_families.applymap(repr)
        raise ValueError('Gene family names must not be empty or contain a null character (\\0). Got:\n{}'.format(invalid_families.to_string(index=False)))
    
    # Check for overlap (this is merely for user friendliness, the next check below would catch any overlap as well)
    assert_overlap(mapped=False)
    
    # Get genes
    gene_families['gene'] = session.get_genes_by_name(gene_families['gene'])
    gene_families['gene'] = gene_families['gene'].apply(list)
    gene_families = df_.split_array_like(gene_families, 'gene')
    
    # Remove duplicates (caused by mapping; harmless)
    gene_families = gene_families.drop_duplicates()
    
    # Check for overlap again (source genes can map to the same destination gene)
    assert_overlap(mapped=True)
    
    return gene_families
