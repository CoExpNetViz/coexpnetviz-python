Changelog
=========
Starting with 5.0.0, `semantic versioning`_ is used. When depending
on this project, pin the major version, e.g. ``install_requires =
['coexpnetviz==1.*']``.

3.0.0
-----
Backwards incompatible changes:

- CLI:

  - Rename ``dbg-coexpnetviz`` to ``coexpnetviz``
  - Rename ``--baits-file`` to ``--baits``.
  - Rename ``--correlation-method`` to ``--correlation-function``.
  - Combine ``--lower-percentile-rank`` and ``--upper-percentile-rank`` into ``--percentile-ranks``.
  - Remove ``--database-*`` options; no longer using a database.
  - ``-e`` now takes one expression matrix but can be specified multiple times. E.g. ``-e m1 -e m2`` instead of ``-e m1 -e m2``.
  - ``--gene-families`` no longer defaults to PLAZA gene families, but remains optional.

- Output:

  - Rename ``*.sim_mat.txt`` to ``*.correlation_matrix.txt``.
  - Rename ``*.corr_sample_histogram.png`` to ``*.sample_histogram.png``.
  - Rename ``*.corr_sample_cdf.png`` to ``*.sample_cdf.png``.
  - network.node.attr:

    - type: Add ``gene`` node type. Leave off `` node`` suffix on type values,
      e.g. ``family`` instead of ``family node``.
    - Replace ``bait_gene`` and ``correlating_genes_in_family`` with a ``genes`` column.
    - Remove ``species`` column.
    - Replace ``families`` column with ``family`` column.

- Complete API overhaul: Rename from deep_blue_genome.coexpnetviz to
  coexpnetviz; ...

- Compare genes by string comparison, no longer support mappings (E.g. rice LOC
  vs MSU) or synonyms. This greatly simplifies code.

Fixes:

- Random sample sometimes sampled rows multiple times (`np.random.choice`
  defaults to ``replace=True``).

Enhancements / additions:

- Output

  - Add ``*.sample_matrix.txt``
  - Add ``percentiles.txt``
  - Add ``significant_correlations.txt``

- Add ``--version``
- Add ``--output`` which defaults to current directory

Internal:

- Add tests. There were no automatic tests before.
- Use varbio and pytil.
- Use simple project structure 1.2.0.
- ...

2.0.0.dev2
----------
No change log. Package was named ``deep_blue_genome`` at this time.

Older versions
--------------
No change log

.. _semantic versioning: http://semver.org/spec/v2.0.0.html
