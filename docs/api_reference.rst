API reference
=============
The API (aplication programmer interface; i.e. a library) consists of a single
module, ``coexpnetviz``, documented below.

The API reference makes use of a `type language`_; for example, to
describe exactly what arguments can be passed to a function.  

.. currentmodule:: coexpnetviz
.. automodule:: coexpnetviz

.. autosummary::
   :nosignatures:

   create_network
   ExpressionMatrix
   Network
   NodeType
   RGB
   write_cytoscape

.. autofunction:: create_network
.. autoclass:: ExpressionMatrix
.. autoclass:: Network
.. autoclass:: NodeType
.. autoclass:: RGB
.. autofunction:: write_cytoscape

.. _type language: http://pytil.readthedocs.io/en/5.0.0/type_language.html
