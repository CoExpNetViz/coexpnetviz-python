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

from collections import namedtuple

Network = namedtuple('Network', 'name bait_nodes family_nodes gene_families'.split())
'''
Network (aka a graph) result of CoExpNetViz

#TODO no nodes, just str
Attributes
----------
name : str
bait_nodes : pd.Series(BaitNode, index=(bait_name : str))
    Bait nodes
family_nodes : pd.Series(FamilyNode, index=(fam_name : str))
    Family nodes
correlations : pd.DataFrame(columns=[family : FamilyNode, family_gene : str, bait : BaitNode, correlation : float-like])
    Sufficient correlations between family and bait nodes. `family_gene` denotes the gene in the family that correlates with the bait.
partitions : pd.DataFrame(index=(partition_id : int), columns=[family : FamilyNode, bait : BaitNode]) #TODO rm
    Grouping of family nodes by subset of baits they correlate to.
gene_families
    Gene families of bait and family nodes. See `read_gene_families_file` for its type
'''
        