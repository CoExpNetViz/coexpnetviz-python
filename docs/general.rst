Common concepts
===============
Below we introduce concepts common to all CoExpNetViz interfaces.

Input
-----
The algorithm takes the following input:

- A group of genes (from one or more species), called the bait genes or baits.
- One gene expression matrix per species.
- Optionally, a lower and upper percentile rank by which to determine whether 2
  genes are considered to be co-expressed.  When not specified, a default lower
  and upper percentile rank is used.
- Optionally, a set of gene families.
- Optionally, the correlation method to use. The default is Pearson's r.

Genes are referred to by name, the same name should be used across all input
as different names are assumed to refer to different genes.

Below are the expected input file formats.


.. _Baits file:

Baits file
''''''''''
A baits file contains gene names separated by whitespace and/or commas.

Examples::

    PGSC0003DMP400054926 PGSC0003DMP400018704 Solyc03g097500 Solyc02g014730 Solyc04g011600 AT5G41040 AT5G23190 AT3G11430

::

    PGSC0003DMP400054926	PGSC0003DMP400018704 Solyc03g097500 Solyc02g014730 Solyc04g011600 AT5G41040 AT5G23190 AT3G11430

::

    PGSC0003DMP400054926
    PGSC0003DMP400018704
    Solyc03g097500
    Solyc02g014730, Solyc04g011600			AT5G41040 AT5G23190	AT3G11430


.. _Expression matrix file:

Expression matrix file
''''''''''''''''''''''
See :ref:`varbio:Expression matrix file`.


.. _Gene families file:

Gene families file
''''''''''''''''''
A gene families file is a plain text file with one line per gene family. A line
is tab separated. The first field is the family name, the other fields are
genes in the family.

For example::

    ORTHO000001	PGSC0003DMP400018704	Solyc03g097500	AT5G41040
    ORTHO000002	Solyc02g014730	Solyc04g011600	AT5G23190
    ORTHO000005	AT3G11430

Algorithm
---------
First, the algorithm obtains, for each gene expression matrix, all
co-expression values (i.e. correlations) between the baits (present in the
matrix) and other genes (of the matrix).

Next, for each gene expression matrix, a sample of rows is taken and
co-expression is measured between all genes in the sample. The lower and upper
percentile (corresponding to the given percentile ranks) of the distribution of
co-expression values are taken. Genes are considered co-expressed if their
co-expression value is either less (or equal) to the lower percentile or
greater (or equal) to the upper percentile of the corresponding expression
matrix.

Having obtained the co-expression values between all baits and other genes, the
nodes and edges of the resulting network are built. There are 3 types of nodes:
bait, gene and family nodes. Respectively they represent a bait gene, a
non-bait gene and a gene family. Each gene corresponds to exactly one node.
When gene families are provided, non-bait genes are grouped into family nodes
and non-bait genes not appearing in any family become gene nodes. Gene and
family nodes which aren't co-expressed (correlated or anti-correlated) with any
bait are dropped from the network.

Next, edges are added between bait and gene nodes whose genes are
co-expressed, as well as between bait and family nodes where at least one of
the family's genes is co-expressed with the corresponding bait.

Then, gene and family nodes are grouped into partitions in such a way that all
nodes in a partition are co-expressed with the same set of baits.

Finally, nodes and edges are annotated with attributes such as which genes of a
family node are actually co-expressed.  In addition to the network,
intermediate data is made available as well; for example, the percentiles and
co-expression values (See the documentation of the interface you are using for
a full list).

Output
------
The following files are outputted by all interfaces:

coexpnetviz_style.xml, {network_name}.edge.attr, {network_name}.node.attr, {network_name}.sif
    Cytoscape files, see Network section below

The CLI and website additionally output:

{matrix_name}.correlation_matrix.txt
    Correlations between genes and baits of the corresponding expression matrix
    before any cutoffs have been applied.
{matrix_name}.sample_matrix.txt
    Correlations between sample of genes from the corresponding expression
    matrix. This sample was used to derive the percentile cutoffs.
{matrix_name}.sample_histogram.png
    Frequency of sample matrix values. The red vertical lines are the
    percentiles. Correlations ranging between the red lines are cut off.
{matrix_name}.sample_cdf.png
    Cumulative distribution function (discrete) of the sample matrix values.
    The red horizontal lines are the percentile ranks used, divided by 100. The
    points where the red lines first intersect with the function, roughly
    (depending on how jumpy the function is in that part) mark the cutoff
    thresholds (imagine a vertical line at the intersection and read the
    correlation value on the X-axis).
percentiles.txt
    Lower and upper percentile cutoffs used for each matrix. These
    percentiles were derived from ``*.sample_matrix.txt``.
significant_correlations.txt
    Correlations between genes and baits after cutoffs have been applied.
coexpnetviz.log
    Further details of the run: coexpnetviz version, warnings, ...

Network
'''''''
To open the outputted network in Cytoscape (the app does this for you):

1. Open Cytoscape 3
2. Menu: File > Import > Network: network.sif
3. Menu: File > Import > Table: network.node.attr, Import Data As Node Table
   Columns (= default)
4. Menu: File > Import > Table: network.edge.attr, Import Data As Edge Table
   Columns
5. Menu: File > Import > Style: coexpnetviz_style.xml
6. Control Panel: Click the 'Style' tab, select CoExpNetViz from the list of
   styles
7. Menu: Layout > Group Attributes Layout > partition_id

The outputted network has 3 node types:

bait
    represents a single bait gene. They are white and diamond shaped.
family
    represents the genes of a family which (anti-)correlate with a bait.
gene
    represents a gene which is neither a bait, nor appears in a family, but
    does (anti-)correlate with a bait.

Genes which do not (anti-)correlate with any bait are not represented by any
node. All other genes are represented by exactly one node in the network.

Nodes are organised in partitions, denoted by their color. Bait nodes form one
partition, other nodes are grouped/partitioned by the baits they
(anti-)correlate to. Each partition is layed out in a circle.

Nodes have the following attributes:

id
    Unique node id.
label
    Short description of the node. Labels are unique (but often don't make
    legal identifiers, e.g. they may contain spaces).
colour
    Node display colour.
type
    Node type: ``bait node``, ``family node`` or ``gene node``.
bait_gene
    If a bait node, the bait gene. Otherwise, this field is empty.
species
    The species of the bait or gene node, if available. Otherwise, this field
    is empty. CoExpNetViz does not currently support specifying the species, so
    this field is always empty.
families
    If bait node, the family the bait is part of, if any. Otherwise, this field
    is empty.
family
    Name of the family. For non-family nodes, this field is empty.
correlating_genes_in_family
    Genes in the family which sufficiently (anti-)correlate with a bait
    separated by ', '. For non-family nodes, this field is empty.
partition_id
    Id of the partition the node is part of.

Edges between a bait and non-bait node denotes the bait is sufficiently
(anti-)correlated to the gene expression of the gene(s) represented by the
non-bait node. A green edge denotes correlation, a red edge denotes
anti-correlation. The darker the color, the stronger the correlation.
In the edge attributes, you will find the r-value attribute, which is the
average of the Pearson correlation of the bait and each gene represented by the
non-bait node (i.e. excluding correlation values that were cut off by the
tresholds).

Edges between baits denote homology, i.e. they are of the same family.

There are no self (``x-x``), synonymous (``x-y`` and ``y-x``) or duplicate
(``x-y`` and ``x-y``) edges.


.. TODO a few cytoscape screenshots

.. _varbio's documentation: http://varbio.readthedocs.io/en/stable/file_formats.html#expression-matrix-file

