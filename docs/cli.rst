Command line interface documentation
====================================
Please first read the :doc:`general description of CoExpNetViz <general>`, then
we'll get into the specifics of this interface below.

To use the CLI, first install the CoExpNetViz PyPI package with `pip`_ by
running ``pip install --user coexpnetviz`` on the command line.

A full list of options can be obtained by running ``coexpnetviz --help``.

Let us run CoExpNetViz on the example data with the API. First `download the
example data <example data_>`_ and unpack it. Inside the unpacked
``coexpnetviz_example`` directory, run ``coexpnetviz --baits
baits_two_species.txt -e arabidopsis_dataset.txt -e tomato_dataset.txt``. This
creates a new directory ``output`` with all output.

.. _pip: https://pip.pypa.io/en/stable/quickstart/
.. _example data: http://bioinformatics.psb.ugent.be/webtools/coexpr/index.php?__controller=ui&__action=get_example_files
