# Copyright (C) 2015 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
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

from varbio import parse
from pytil import data_frame as df_
import pandas as pd

def baits(reader):
    '''
    Parse baits file

    TODO refer to file formats

    Parameters
    ----------
    reader : file object
        Text reader whose content is a baits file

    Returns
    -------
    pd.Series([gene :: str])
    '''
    return pd.Series(reader.read().replace(',', ' ').split())

def gene_families(reader, name_index=0):
    '''
    Parse gene families file

    TODO refer to file formats

    Parameters
    ----------
    reader : file object
        Text reader whose content is a gene families file, i.e. a clustering
        with families as clusters
    name_index : int
        The index of the gene family name field on each row. See `varbio.parse.clustering`

    Returns
    -------
    pd.DataFrame({'family' => [str], 'gene' => [str]})
    '''
    clustering = parse.clustering(reader, name_index)
    clustering = pd.DataFrame(list(clustering.items()), columns=('family', 'gene'))
    clustering['gene'] = clustering['gene'].apply(list)
    clustering = df_.split_array_like(clustering, 'gene')
    _validate_gene_families(clustering)
    return clustering

def _validate_gene_families(gene_families):
    '''
    Validate gene families

    Parameters
    ----------
    gene_families : pd.DataFrame({'family' => [str], 'gene' => [str]})
        Gene families to validate

    Raises
    ------
    ValueError
        If:

        - families overlap
        - family name is invalid
    '''
    if gene_families.empty:
        return gene_families

    gene_families = gene_families.copy()

    # Raise if invalid family name
    #
    # Family name must be unique, cannot be empty, ``nan``, ``None`` or contain
    # a null character
    def raise_if_invalid_name(is_invalid, reason):
        mask = gene_families['family'].apply(is_invalid)
        invalid_families = gene_families[mask]
        if not invalid_families.empty:
            invalid_families = invalid_families.applymap(repr)
            raise ValueError('{}. Got:\n{}'.format(reason, invalid_families.to_string(index=False)))
    raise_if_invalid_name(
        lambda family: not isinstance(family, str),
        'Gene family names must be strings'
    )
    raise_if_invalid_name(
        lambda family: not family,
        'Gene family names must not be empty'
    )
    raise_if_invalid_name(
        lambda family: '\0' in family,
        'Gene family names must not contain a null character (\\x00)'
    )

    # Raise if families overlap (i.e. if a gene is a member of multiple
    # families)
    duplicates = gene_families[gene_families['gene'].duplicated(keep=False)].copy()
    if not duplicates.empty:
        duplicates = duplicates.sort_values(list(duplicates.columns))
        raise ValueError('Gene families overlap:\n{}'.format(duplicates.to_string(index=False)))