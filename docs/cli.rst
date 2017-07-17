Command line interface guide
============================

If you have not installed the ``deep-genome-coexpnetviz`` package yet, please
follow the `installation instructions`_ first.

Further install/setup
---------------------
The CLI requires a MySQL database for gene information. To create the database,
either use phpMyAdmin (if available) or connect to your server using ``mysql
-h $DATABASE_HOST -u $DATABASE_USER -p``. Then run the
CREATE db, you can choose name
CREATE user (no create/grant if default user works fine)
GRANT permission to the user on db

Next, we configure CoExpNetViz to use the database we just created by writing a
configuration file ``~/?/TODO`` containing::

    contents

.. TODO Test this on our system with dev and final version

With this, CoExpNetViz is all set up.

For reference, a full list of options can be obtained by running ``coexpnetviz
--help``. We will now walk through an example of its usage.

TODO usage example

.. _installation instructions: installation.html
