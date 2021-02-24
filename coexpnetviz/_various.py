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

from enum import Enum
from math import floor, sqrt

from varbio import parse_yaml
import attr
import numpy as np
import pandas as pd


@attr.s(frozen=True, slots=True)
class Network:

    '''
    Network (aka a graph) result of CoExpNetViz.

    Parameters
    ----------
    nodes : ~pandas.DataFrame
        Nodes of the network as a data frame with the following columns:

        id
            `int` -- unique node id.
        label
            `str` -- short description of the node. Labels are unique (but often
            don't make legal identifiers, e.g. they may contain spaces).
        type
            `NodeType` -- the type of the node.
        genes
            :py:class:`~typing.FrozenSet` of `str` -- if a bait node, the bait
            gene. If a family node, each gene of the family that correlates
            with a bait. If a gene node, the gene, which correlates with a
            bait.
        family
            `str` or `None` -- if a bait node, family name which the bait
            gene is part of, if any. If a family node, the corresponding family
            name. Else, `None`.
        colour
            `RGB` -- colour of the node.
        partition_id
            `int` -- partition id. Bait nodes form a partition. Other nodes are
            partitioned according to the subset of baits they correlate with.

        Each gene is assigned to at most 1 node.

    homology_edges : ~pandas.DataFrame
        Homology edges indicating homologous baits. There are no self edges
        (``bait_node1==bait_node2``), no synonymous edges (one edge being the
        same as another edge when swapping ``bait_node1`` and ``bait_node2``) and no
        duplicates.

        The data frame has the following columns:

        bait_node1
            `int` -- Node id of the first bait of the edge.
        bait_node2
            `int` -- Node id of the other bait of the edge.

    correlation_edges : ~pandas.DataFrame
        Correlation edges between bait nodes and all nodes (including baits again).
        There are no self edges (``bait_node==node``), no synonymous edges (one
        edge being the same as another edge when swapping ``bait_node`` and
        ``node``) and no duplicates.

        The data frame has the following columns:

        bait_node
            `int` -- Node id of the bait of the edge.
        node
            `int` -- Node id of the other node (of any type) of the edge.
        max_correlation
            `float` -- If node is a family node, the highest of all correlations
            between bait and family genes (``max(correlation(bait, gene) for
            gene in family)``). Otherwise, simply the correlation between the
            bait gene and the gene of the other node.

    significant_correlations : ~pandas.DataFrame
        Gene correlations after percentile-based cut-off. There are no self edges
        (``bait==gene``), no synonymous edges (one edge being the same as another edge
        when swapping ``bait`` and ``gene``) and no duplicates.

        The data frame has the following columns:

        bait
            `str` -- Bait gene.
        gene
            `str` -- Correlating gene (can be a bait as well).
        correlation
            `float` -- Correlation between ``bait`` and ``gene``.

    samples : ~typing.Tuple[~pandas.DataFrame]
        ``samples[i]`` is the correlation matrix derived from a sample of
        ``expression_matrices[i]`` (referring to the argument given to
        `create_network`) and was used to generate ``percentiles[i]``. Its
        columns and index are a subset of ``expression_matrices[i].index`` and
        ``columns.equals(index)``. Each data frame's is a matrix of correlations
        of type `float`, its index and columns are genes of type `str`.

    percentiles : ~typing.Tuple[float, float]
        ``percentiles[i]`` are the lower and upper percentiles, respectively,
        used as cutoff on ``correlation_matrices[i]``.

    correlation_matrices : ~typing.Tuple[~pandas.DataFrame]
        ``correlation_matrices[i]`` is the correlation matrix derived from
        ``expression_matrices[i]`` before any values have been cut off. Each
        data frame's is a matrix of correlations of type `float`, its columns
        are bait genes of type `str`, its index are genes of type `str`.

    See also
    --------
    create_network : Create a comparative coexpression network.
    '''

    nodes = attr.ib()
    homology_edges = attr.ib()
    correlation_edges = attr.ib()
    significant_correlations = attr.ib()
    samples = attr.ib()
    percentiles = attr.ib()
    correlation_matrices = attr.ib()

class NodeType(Enum):

    '''
    Type of a node in a `Network`.

    Bait nodes represent a bait gene. Family nodes represent the genes of the family
    that correlate with a bait. Gene nodes represent a non-bait gene that has no
    family but does correlate with a bait.
    '''

    bait = 'bait'
    family = 'family'
    gene = 'gene'

class RGB:

    '''
    Colour as sequence of red, green, blue colour components.

    Each component is an integer between 0 and 255, inclusive. The class is
    immutable.

    Parameters
    ----------
    rgb : ~pytil.numpy.ArrayLike[int]

    Attributes
    ----------
    r : int
        Red colour component.
    g : int
        Green colour component.
    b : int
        Blue colour component.
    '''

    def __init__(self, rgb):
        self._rgb = np.array(rgb)
        if ((self._rgb < 0) | (self._rgb > 255)).any():
            raise ValueError('Invalid colour component value(s). Given rgb: {}'.format(self._rgb))
        if self._rgb.dtype != int:
            raise ValueError('Colour component value(s) must be int. Given values have type {}'.format(self._rgb.dtype))

    @staticmethod
    def from_float(rgb):
        '''
        Create RGB from float array-like.

        Parameters
        ----------
        rgb : ~pytil.numpy.ArrayLike[float]
            ``(red, green, blue)`` array with values between 0 and 1.
        '''
        rgb = np.array(rgb)
        if ((rgb < 0.0) | (rgb > 1.0)).any():
            raise ValueError(
                'Invalid component value(s), should be float in range of [0, 1]. Given rgb: {}'
                .format(rgb)
            )
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
        '''
        Hex formatted colour, i.e. ``#RRGGBB``.
        '''
        return '#{:02x}{:02x}{:02x}'.format(*self._rgb)

# Convert HDTV Y'UV to RGB. See http://www.equasys.de/colorconversion.html
_yuv_to_rgb = np.matrix([
    [1, 0, 1.14],
    [1, -0.395, -0.581],
    [1, 2.032, 0]
]).T

def distinct_colours(n):
    '''
    Get n approximately most distinguishable colours as perceived by human vision.

    Colours close to white or black are excluded.

    Parameters
    ----------
    n : int
        Number of colours to return.

    Returns
    -------
    ~pytil.numpy.ArrayLike[float]
        ``(n, 3)`` shaped array of RGB colours as rows.

    Notes
    -----
    - Other approaches `<http://stackoverflow.com/a/30881059/1031434>`_.
    - YUV conversion info: http://www.equasys.de/colorconversion.html. When
      transforming RGB to YUV, the result is not a cube as the link seems to
      suggest, it's actually a parallelepiped.
    - Distance in YUV is perceptional distance in humans, making it ideal for
      finding a set of most visually distinct colours.
    - YUV = Y'UV
    - Y C_r C_b is not the same as YUV. It uses integers, shifted by an amount,
      while YUV are floats using the whole space.
    '''
    # YUV extrema in which we'll generate points
    yuv_extrema = np.array([
        # min max
        [.4, .8],  # y, limited range to eliminate too dark/bright colours
        [-.436, .436],  # u, limited to values for which a y,v exists that yields a valid RGB
        [-.615, .615]  # v, limited to values for which a y,u exists that yields a valid RGB
    ])

    # Volume of the YUV cube we generate points in
    yuv_volume = np.prod(yuv_extrema[:,1] - yuv_extrema[:,0])

    # Volume of the RGB cube mapped into YUV space, i.e. a parallelepiped
    mapped_rgb_volume = .2357

    # Estimate number of points to generate in YUV cube to have enough that map to RGB
    yuv_points = n / mapped_rgb_volume * yuv_volume

    # Derive initial number of points to place per side of the 3D YUV grid to
    # generate
    side = int(np.ceil(yuv_points**(1/3)))

    # Generate grids, increasing side each time, until its RGB mapping has at least
    # n valid points
    random = np.random.RandomState(seed=0)  # be deterministic
    y_side = max(2, floor(sqrt(n))-1)
    while True:
        # Generate YUV points
        y = np.linspace(*yuv_extrema[0], y_side)
        u = np.linspace(*yuv_extrema[1], side)
        v = np.linspace(*yuv_extrema[2], side)
        yuv = np.array(np.meshgrid(y, u, v)).reshape(3,-1).T

        # Map to RGB
        rgb = (yuv * _yuv_to_rgb).A

        # Drop invalid points
        valid_values = (rgb >= 0) & (rgb <= 1)
        valid_rows = valid_values.all(axis=1)
        rgb = rgb[valid_rows,:]

        # If enough points, drop extra points by returning a random selection
        if len(rgb) > n:
            indices = random.choice(len(rgb), n, replace=False)
            return rgb[indices]

        # Else, continue with increased side
        side += 1

def parse_gene_families(path):
    '''
    Parse gene families file.

    This is a yaml file like:

        'family1': ['gene1', 'gene2']
        'family2': ['gene3', 'gene4', 'gene5']

    I.e. it is a dict/mapping of families to lists of genes. The easiest way of
    creating such a file is to find a yaml library in your preferred language
    and let it format the yaml for you from a dict (or whatever your preferred
    language calls it). Caveat: family/gene names must be strings, so if your
    family names are numbers, cast them to string first in your dict.

    Parameters
    ----------
    path : ~pathlib.Path
        Gene families file

    Returns
    -------
    ~pandas.DataFrame
        Data frame with a ``family`` and ``gene`` `str` column.
    '''
    # We only support yaml as we're unsure what format is commonly used. yaml
    # is easy to work with, easy to read and tool agnostic (orthofinder vs
    # ...), so we offer that instead. We can add more formats later by popular
    # demand.
    families = parse_yaml(path)
    data = (
        (family, gene)
        for family, genes in families.items()
        for gene in genes
    )
    families = pd.DataFrame(data, columns=('family', 'gene'))
    _validate_gene_families(families)
    return families

def _validate_gene_families(gene_families):
    '''
    Validate gene families.

    Parameters
    ----------
    gene_families : ~pandas.DataFrame
        Gene families to validate. Must have ``family``, ``gene`` `str` columns.

    Raises
    ------
    ValueError
        If:

        - families overlap
        - family name is invalid
    '''
    if gene_families.empty:
        return

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
