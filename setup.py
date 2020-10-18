from setuptools import setup, find_packages
from collections import defaultdict
from pathlib import Path
import os

setup_args = dict(
    version='5.0.0.dev1',
    name='coexpnetviz',
    long_description=Path('README.rst').read_text(),
    packages=find_packages(),
    extras_require={
        'dev': [
            'numpydoc',
            'sphinx>=1',
            'sphinx-rtd-theme',
            'pytest>=3',
            'pytest-env',
            'pytil[data_frame,test]==8.*',
        ],
    },
    entry_points={'console_scripts': [
        'coexpnetviz = coexpnetviz.main:main'
    ]},
)

# Generate package data
#
# Anything placed underneath a directory named 'data' in a package, is added to
# the package_data of that package; i.e. included in the sdist/bdist and
# accessible via pkg_resources.resource_*
project_root = Path(__file__).parent
package_data = defaultdict(list)
for package in setup_args['packages']:
    package_dir = project_root / package.replace('.', '/')
    data_dir = package_dir / 'data'
    if data_dir.exists() and not (data_dir / '__init__.py').exists():
        # Found a data dir
        for parent, _, files in os.walk(str(data_dir)):
            package_data[package].extend(str((data_dir / parent / file).relative_to(package_dir)) for file in files)
setup_args['package_data'] = {k: sorted(v) for k,v in package_data.items()}  # sort to avoid unnecessary git diffs

# setup
setup(**setup_args)
