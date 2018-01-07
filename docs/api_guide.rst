API guide
=========
Please first read the :doc:`general description of CoExpNetViz <general>`, then
we'll get into the specifics of this interface below.

To use the API, first install the CoExpNetViz PyPI package with `pip`_ by
running ``pip install --user coexpnetviz`` on the command line.

Let us run CoExpNetViz on the example data with the API. First `download the
example data <example data_>`_ and unpack it. Inside the
unpacked ``coexpnetviz_example`` directory, write ``main.py`` with the
following contents and run it with ``python3 main.py``::

    from varbio import clean, correlation
    from coexpnetviz import parse, create_network, write_cytoscape, ExpressionMatrix
    import pandas as pd
    import varbio.parse

    # Read baits file
    # Note: clean.plain_text is only necessary if your files are dirty
    # (incorrect line endings, nul characters, ...)
    with open('baits_two_species.txt') as f:
        baits = parse.baits(clean.plain_text(f))

    # Read expression matrices
    expression_matrices = []
    with open('arabidopsis_dataset.txt') as f:
        expression_matrices.append(ExpressionMatrix('arabidopsis', varbio.parse.expression_matrix(clean.plain_text(f))))
    with open('tomato_dataset.txt') as f:
        expression_matrices.append(ExpressionMatrix('tomato', varbio.parse.expression_matrix(clean.plain_text(f))))

    # Make empty gene families frame (i.e. not using any gene families)
    gene_families = pd.DataFrame(columns=('family', 'gene'))

    # If you do have a gene families file, you could load it like so
    #with open('gene_families.txt') as f:
    #    gene_families = parse.gene_families(clean.plain_text(f))

    # Run CoExpNetViz algorithm, returns a coexpnetviz.Network
    network = create_network(
            baits, expression_matrices, gene_families,
            correlation.pearson_df, percentile_ranks=[5.0, 95.0]
    )

    # Write network to files which can be loaded in Cytoscape
    write_cytoscape(network, 'my_network')

Unlike the other interfaces, the above only outputs ``my_network.sif``,
``my_network.node.attr``, ``my_network.edge.attr`` and
``coexpnetviz_style.xml``. Please use the CLI if you need the other files or
mail a feature request to `Oren Tzfadia`_.

For more info, see :doc:`CoExpNetViz' API reference <api_reference>` and
:doc:`varbio's API reference <varbio:api_reference>`.

.. _pip: https://pip.pypa.io/en/stable/quickstart/
.. _example data: http://bioinformatics.psb.ugent.be/webtools/coexpr/index.php?__controller=ui&__action=get_example_files
.. _oren tzfadia: http://bioinformatics.psb.ugent.be/people/profile/orentzfadia
