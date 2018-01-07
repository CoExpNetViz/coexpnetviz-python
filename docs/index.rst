Welcome to CoExpNetViz's documentation!
=======================================
CoExpNetViz allows visualising, as a (Cytoscape) network, the co-expression
between a group of genes and all other genes of one or more species;
optionally, grouping nodes by gene families.

CoExpNetViz can be used through one of the following interfaces:

- `Website`_: Run CoExpNetViz on our server with a web form. If your data is
  confidential, consider using the command line interface or API instead.

- `Cytoscape app`_: Run CoExpNetViz on our server with a Cytoscape plugin which
  immediately opens the result in Cytoscape. If your data is confidential,
  consider using the command line interface or API instead.

- :doc:`Command line interface (CLI) <cli>`: Run CoExpNetViz locally on the
  command line.

- :doc:`Python API (application programmer interface) <api_guide>`: Run CoExpNetViz
  locally in your Python code using the `coexpnetviz` package.

When using CoExpNetViz in a paper, please cite `Tzfadia, O., Diels, T., De
Meyer, S., Vandepoele, K., Aharoni, A., & Van de Peer, Y. (2015). CoExpNetViz:
Comparative Co-Expression Networks Construction and Visualization Tool.
Frontiers in Plant Science, 6, 1194.  http://doi.org/10.3389/fpls.2015.01194
<paper_>`__

Contents
--------

.. toctree::
   :maxdepth: 2

   general
   cli
   api_guide
   api_reference
   developer_documentation
   changelog


Indices and tables
==================
* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

.. _paper: http://www.ncbi.nlm.nih.gov/pubmed/26779228
.. _website: http://bioinformatics.psb.ugent.be/webtools/coexpnetviz
.. _cytoscape app: http://apps.cytoscape.org/apps/coexpnetviz
