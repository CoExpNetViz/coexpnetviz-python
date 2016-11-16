# Copyright (C) 2015 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
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

from enum import Enum
import numpy as np
from numpy.linalg import norm
from chicken_turtle_util.algorithms import spread_points_in_hypercube
import attr

_network_attrs = ('nodes', 'homology_edges', 'correlation_edges', 'significant_correlations', 'samples', 'percentiles', 'correlation_matrices')
Network = attr.make_class('Network', _network_attrs, frozen=True)
'''
Network (aka a graph) result of CoExpNetViz

Attributes
----------
nodes : pd.DataFrame
    The data frame's columns are:
    
    id : int
        Unique node id.
    label : str
        Short description of the node. Labels are unique (but often don't make
        legal identifiers, e.g. they may contain spaces).
    type : NodeType
        The type of the node.
    genes : frozenset({deep_genome.core.database.entities.Gene})
        If a bait node, the bait gene. If a family node, each gene of the family
        that correlates with a bait. If a gene node, the gene, which correlates
        with a bait.
    family : str or null
        If a bait node, family name which the bait gene is part of, if any. If a
        family node, the corresponding family name. Else, null (`np.nan` or ``None``).
    colour : deep_genome.core.util.RGB
        Colour of the node
    partition_id : int
        Partition id. Bait nodes form a partition. Other nodes are partitioned
        according to the subset of baits they correlate with.
        
    Each Gene is assigned to at most 1 node.
    
homology_edges : pd.DataFrame(dict(bait_node1=[node_id :: int], bait_node2=[node_id :: int]))
    Homology edges indicating homologous baits. There are no self edges
    (``bait_node1==bait_node2``), no synonymous edges (one edge being the same as another edge
    when swapping `bait_node1` and `bait_node2`) and no duplicates.
correlation_edges : pd.DataFrame(dict(bait_node=[node_id :: int], node=[node_id :: int], max_correlation=[float]))
    Correlation edges between bait nodes and between bait and family nodes.
    `max_correlation` is the the correlation of the gene of the family with
    ``max(correlation(any bait, gene))``. There are no self edges
    (``bait_node==node``), no synonymous edges (one edge being the same as another
    edge when swapping `bait_node` and `node`) and no duplicates.
significant_correlations : pd.DataFrame({'bait' => [Gene], 'gene' => [Gene], 'correlation' => [float]})
    Gene correlations after percentile-based cut-off. There are no self edges
    (``bait==gene``), no synonymous edges (one edge being the same as another edge
    when swapping `bait` and `gene`) and no duplicates.
samples : (pd.DataFrame([[correlation :: float]], index=[Gene], columns=[Gene]), ...)
    ``samples[i]`` is the correlation matrix derived from a sample of
    ``expression_matrices[i]`` and was used to generate ``percentiles[i]``. Its
    `columns` and `index` are a subset of `expression_matrices[i].index` and
    ``columns.equals(index)``.
percentiles : ((lower_percentile :: float, upper_percentile :: float), ...)
    ``percentiles[i]`` are the percentiles used as cutoff on
    ``correlation_matrices[i]``.
correlation_matrices : (pd.DataFrame([[correlation :: float]], columns=baits, index=expression_matrices[i].index), ...)
    ``correlation_matrices[i]`` is the correlation matrix derived from
    ``expression_matrices[i]`` before any values have been cut off.
    
See also
--------
coexpnetviz : Create a comparative coexpression network
'''
        
MutableNetwork = attr.make_class('MutableNetwork', _network_attrs)  # internal class

class NodeType(Enum):
    '''
    Type of a node in a Network.
    
    Bait nodes represent a bait gene. Family nodes represent the genes of the family
    that correlate with a bait. Gene nodes represent a non-bait gene that has no
    family but does correlate with a bait.
    '''
    bait = 'bait'
    family = 'family'
    gene = 'gene'

class RGB(object): #TODO somewhere someone must have written some color classes before in Python
    
    '''
    Color as red green blue sequence color components
    
    Each component is an integer in the range of [0, 255].
    '''
    def __init__(self, rgb):
        '''
        Parameters
        ----------
        rgb : array-like
        '''
        self._rgb = np.array(rgb)
        if ((self._rgb < 0) | (self._rgb > 255)).any():
            raise ValueError('Invalid color component value(s). Given rgb: {}'.format(self._rgb))
        if self._rgb.dtype != int:
            raise ValueError('Color component value(s) must be int. Given values have type {}'.format(self._rgb.dtype))
        
    @staticmethod
    def from_float(rgb):
        '''
        Parameters
        ----------
        rgb : array-like
        '''
        rgb = np.array(rgb)
        if ((rgb < 0.0) | (rgb > 1.0)).any():
            raise ValueError('Invalid component value(s), should be float in range of [0, 1]. Given rgb: {}', rgb)
        return RGB((rgb * 255).round().astype(int))
        
    @property
    def r(self):
        return self[0]
    
    @property
    def g(self):
        return self[1]
    
    @property
    def b(self):
        return self[2]
    
    def __equals__(self, other):
        return isinstance(other, RGB) and other._rgb == self._rgb
    
    def __get_item__(self, index):
        return self._rgb[index]
    
    def __repr__(self):
        return 'RGB({})'.format(self._rgb)
    
    def __str__(self):
        return repr(self)
    
    def to_hex(self):
        return '#{:02x}{:02x}{:02x}'.format(*self._rgb)

# # TODO get_distinct_colours based on YUV as to get the most visually distinct colours (instead of simply most distinct/orthogonal RGB values)
# # See https://en.wikipedia.org/wiki/YUV#HDTV_with_BT.601
# _yuv_to_rgb = np.matrix([
#     [1, 0, 1.28033],
#     [1, -0.21482, -0.38059],
#     [1, 2.12798, 0]
# ]).T
# 
# def yuv_to_rgb(yuv):
#     '''
#     HDTV-Y'UV point to RGB color point
#     
#     Note that not all YUV values between [0,0,0] and [1,1,1] map to valid rgb
#     values (i.e. some fall outside the [0,1] range) (see p30 http://www.compression.ru/download/articles/color_space/ch03.pdf)
#     
#     Parameters
#     ----------
#     yuv : array-like
#         An (n,3) shaped array. YUV point per row.
#     
#     Returns
#     -------
#     array-like
#         RGB point per row.
#     '''
#     return yuv * _yuv_to_rgb

def get_distinct_colours(n):
    '''
    Get `n` most distinguishably colours as perceived by human vision.
    
    No returned colour is entirely black, nor entirely white.
    
    Based on: http://stackoverflow.com/a/30881059/1031434
    
    Returns
    -------
    np.array(shape=(n, 3))
        n raw RGB float colours
    ''' 
    points = spread_points_in_hypercube(n+2, 3)
    lightest = norm(points, axis=1).argmax()
    darkest = norm(points - np.array([1,1,1]), axis=1).argmax()
    points = np.delete(points, np.array([lightest,darkest]), axis=0)
    return points
    # TODO use CIEDE2000 or the simpler CMC l:c.
    # https://en.wikipedia.org/wiki/Color_difference
    # The trick lies in
    # intersecting a regular space to the part of the color space that maps back
    # to valid rgb values and hoping you are left with enough points.
#     # TODO to avoid black or white, scale down the Y component to [0.1, 0.9]
#     return yuv_to_rgb(points)

#TODO name validation (see deleted code)
# here: non-empty str, no \0 chars
@attr.s(frozen=True, repr=False)
class ExpressionMatrix(object):
    
    '''
    Parameters
    ----------
    name : str
        Unique name of the matrix
    data : pd.DataFrame({condition_name => [gene_expression :: float]}, index=[Gene])
        Gene expression data
    '''
    
    name = attr.ib()
    data = attr.ib()
    
    def __repr__(self):
        return 'ExpressionMatrix({!r})'.format(self.name)
    
