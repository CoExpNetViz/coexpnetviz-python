# Copyright (C) 2015 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
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

from deep_blue_genome.core.reader.various import read_mcl_clustering,\
    read_gene_families_file, read_mapping
# from deep_blue_genome.data_preparation.genes import load_gene_info
import pandas as pd
from deep_blue_genome.core.util import series_invert

'''
The main tool to prepare data for DBG tools
'''

def merge_plaza():
    # clusters: pd.DataFrame(index=(cluster_id : int), columns=[(family : str), (merged_family : str)])
    clusters = read_mcl_clustering('family_clusters')
    clusters.name = 'family'
    cluster_names = clusters.apply(lambda x: [x]).groupby(clusters.index).sum().apply(lambda x: '+'.join(sorted(x)))
    cluster_names.name = 'merged_family'
    clusters = pd.concat([clusters, cluster_names], axis=1)
    
    # families
    families = pd.concat([
        read_gene_families_file('dicot_families'),
        read_gene_families_file('monocot_families')
    ])
    
    # apply itag -> pgsc gene name mapping (not actually needed for merging, just another plaza prep step)
    pgsc_itag = read_mapping('pgsc_itag_mapping')
    pgsc_itag.columns = ['pgsc','itag']
    new_rows = pgsc_itag.join(series_invert(families), on='itag', how='inner')
    new_rows.rename(columns={'pgsc': 'gene'}, inplace=True)
    new_rows.set_index('family', inplace=True)
    families = families.append(new_rows['gene'])
    
    # merge
    merged = clusters.join(families, on='family')
    merged.set_index('merged_family', inplace=True)
    merged.index.name = 'family'
    merged = merged['gene']
    
    # write
    assert merged.notnull().all()
    merged.to_csv('merged_plaza.gene_fams.txt', sep='\t', index=True, header=False)
    
    # Show histogram of family counts
#     import matplotlib.pyplot as plt
#     import time
#     merged.groupby(merged.index).count().plot.hist()
#     plt.show()
#     #plt.savefig('f')
#     time.sleep(100)

