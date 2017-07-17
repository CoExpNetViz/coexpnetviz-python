Changelog
=========
Starting with 5.0.0, `semantic versioning <semver_>`_ is used. When depending
on this project, pin the major version, e.g. ``install_requires =
['coexpnetviz==1.*']``.

5.0.0 (not released yet)
------------------------
TODO

TODO See current output file and compare to what we have here to get an idea
+ see commits

output files (renamed and added)
  # output file names changed: ``*.sim_mat.txt`` -> ``*.correlations.txt``; ``*.corr_sample_histogram.png`` -> ``*.sample_histogram.png``; ``*.corr_sample_cdf.png`` -> ``*.sample_cdf.png``
  # files added: ``*.sample_matrix.txt``
  TODO didn't we lose a file?

log file added?

changed CLI:
# ``--baits-file`` -> ``--baits``; ``--correlation-method`` -> ``--correlation-function``; ``--lower-percentile-rank``, ``--upper-percentile-rank`` -> ``--percentile-ranks``

# ``--database-*`` removed
no database
no config file (it was only used for storing database credentials)

Added tests. Had none before

family node label is now family name instead of its correlating genes 

Older versions
--------------
No change log

.. _semantic versioning: http://semver.org/spec/v2.0.0.html
