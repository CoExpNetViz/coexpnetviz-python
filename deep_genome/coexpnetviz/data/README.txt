To open the network in Cytoscape:

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

Output files:

coexpnetviz_style.xml, network.edge.attr, network.node.attr, network.sif
    Cytoscape files, see previous section
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
    percentiles were derived from `*.sample_matrix.txt`.
significant_correlations.txt
    Correlations between genes and baits after cutoffs have been applied.
coexpnetviz.log
    Further details of the run: coexpnetviz version, warnings, ...
input/baits/{baits_name}
    Bait genes input file that was provided
input/gene_families/{gene_families_name}
    Gene families input file that was provided, if any.
input/expression_matrices/
    Expression matrices that were provided as input
