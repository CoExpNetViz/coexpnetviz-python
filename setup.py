from setuptools import setup, find_packages
from collections import defaultdict
from pathlib import Path
import os

setup_args = dict(
    version='5.0.0.dev1',
    name='coexpnetviz',
    description='Comparative co-expression network construction and visualization',
    long_description=Path('README.rst').read_text(),
    url='https://gitlab.psb.ugent.be/deep_genome/coexpnetviz.git',
    author='VIB/BEG/UGent',
    author_email='tim@diels.me',
    license='LGPL3',
    keywords='bioinformatics coexpression guilt-by-association',
    packages=find_packages(),
    install_requires=[
        'attrs>=17',
        'matplotlib>=1',
        'numpy>=1',
        'pandas>=0.19',
        'more_itertools>=3',
        'pytil[data_frame,series,numpy]==7.*,>7',
        'varbio==3.*',
    ],
    extras_require={
        'dev': [
            'numpydoc',
            'sphinx>=1',
            'sphinx-rtd-theme',
            'pytest>=3',
            'pytest-env',
            'pytil[test]==7.*,>7',
        ],
    },
    entry_points={'console_scripts': [
        'coexpnetviz = coexpnetviz.main:main'
    ]},
    # https://pypi.python.org/pypi?%3Aaction=list_classifiers
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Intended Audience :: Science/Research',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: GNU Lesser General Public License v3 (LGPLv3)',
        'Natural Language :: English',
        'Environment :: Console',
        'Operating System :: POSIX :: Linux',
        'Environment :: MacOS X',
        'Operating System :: Microsoft :: Windows',
        'Programming Language :: Python :: 3 :: Only',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: Implementation :: CPython',
        'Programming Language :: Python :: Implementation :: Stackless',
        'Topic :: Scientific/Engineering',
        'Topic :: Scientific/Engineering :: Bio-Informatics'
    ],
)

# Generate extras_require['all'], union of all extras
all_extra_dependencies = []
for dependencies in setup_args['extras_require'].values():
    all_extra_dependencies.extend(dependencies)
all_extra_dependencies = list(set(all_extra_dependencies))
setup_args['extras_require']['all'] = all_extra_dependencies

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
