Internal CLI used by the
[CoExpNetViz Cytoscape app](https://github.com/CoExpNetViz/CoExpNetViz).
You will want to use the app instead.

### Links
- [Conda](https://anaconda.org/CoExpNetViz/coexpnetviz)
- [GitHub](https://github.com/CoExpNetViz/coexpnetviz-python)
- [Old PyPI](https://pypi.python.org/pypi/coexpnetviz/), for older versions.

### Usage
`conda install -c coexpnetviz coexpnetviz`. Run with `coexpnetviz --help`.

### Development guide
Guide for developing coexpnetviz-python itself. It's analog to
[varbio's dev guide](https://github.com/timdiels/varbio#development-guide).

Versioning: The Cytoscape app pins to the major version of this lib, e.g.
`coexpnetviz==5.*` and autoupdates so long as its the same major version.
Whenever the json API changes in a way that could break the Cytoscape app
versions using it you must bump the major version (pytil explains where to bump
it) and remember to update the BACKEND_MAJOR_VERSION var in the Cytoscape app
as well. Other than that just bump as you normally would in semver.
