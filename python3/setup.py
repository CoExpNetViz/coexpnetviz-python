"""
A setuptools based setup module.

See:
https://packaging.python.org/en/latest/distributing.html
https://github.com/pypa/sampleproject
"""

# Always prefer setuptools over distutils
from setuptools import setup, find_packages
# To use a consistent encoding
from codecs import open
from os import path
import pypandoc

here = path.abspath(path.dirname(__file__))

with open(here + '/src/deep_blue_genome/version.py') as f:
    code = compile(f.read(), here + 'src/deep_blue_genome/version.py', 'exec')
    exec(code)

long_description = pypandoc.convert('readme.md', 'rst')

setup(
    name='deep_blue_genome',

    version=__version__,

    description='Genome analysis platform',
    long_description=long_description,

    # The project's main homepage.
    url='https://bitbucket.org/deep_blue_genome/deep_blue_genome',

    # Author details
    author='VIB/BEG/UGent',
    author_email='tidie@psb.vib-ugent.be',

    # Choose your license
    license='LGPL3', #TODO

    # See https://pypi.python.org/pypi?%3Aaction=list_classifiers
    classifiers=[
        # How mature is this project? Common values are
        #   3 - Alpha
        #   4 - Beta
        #   5 - Production/Stable
        'Development Status :: 2 - Pre-Alpha',

        # Indicate who your project is intended for
        'Environment :: Console',
        'Intended Audience :: Science/Research',
        'Topic :: Software Development :: Build Tools',
        'Natural Language :: English',
        'Operating System :: POSIX', #TODO must add any sub things too

        # Pick your license as you wish (should match "license" above)
        'License :: OSI Approved :: GNU Lesser General Public License v3 (LGPLv3)',

        # Specify the Python versions you support here. In particular, ensure
        # that you indicate whether you support Python 2, Python 3 or both.
        'Programming Language :: Python',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.2',
        'Programming Language :: Python :: 3.3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Topic :: Scientific/Engineering :: Bio-Informatics',
        'Topic :: Scientific/Engineering :: Artificial Intelligence',
    ],

    # What does your project relate to?
    keywords='bioinformatics genome-analysis morph coexpnetviz',

    # You can just specify the packages manually here if your project is
    # simple. Or you can use find_packages().
    packages=find_packages(exclude=['contrib', 'docs', 'tests']),

    # List run-time dependencies here.  These will be installed by pip when
    # your project is installed. For an analysis of "install_requires" vs pip's
    # requirements files see:
    # https://packaging.python.org/en/latest/requirements.html
    install_requires='pandas numexpr bottleneck plumbum inflection sqlalchemy more_itertools memory_profiler psutil'.split(), #TODO mysql-connector

    # List additional groups of dependencies here (e.g. development
    # dependencies). You can install these using the following syntax,
    # for example:
    # $ pip install -e .[dev,test]
    extras_require={
        'dev': 'twine pypandoc '.split(),
        'test': ['pytest'],
    },

    # If there are data files included in your packages that need to be
    # installed, specify them here.  If using Python 2.6 or less, then these
    # have to be included in MANIFEST.in as well.
#     package_data={
#         'sample': ['package_data.dat'],
#     },

    # Although 'package_data' is the preferred approach, in some case you may
    # need to place data files outside of your packages. See:
    # http://docs.python.org/3.4/distutils/setupscript.html#installing-additional-files # noqa
    # In this case, 'data_file' will be installed into '<sys.prefix>/my_data'
#     data_files=[('my_data', ['data/data_file'])],

    # To provide executable scripts, use entry points in preference to the
    # "scripts" keyword. Entry points provide cross-platform support and allow
    # pip to create the appropriate form of executable for the target platform.
    entry_points={
        'console_scripts': [
            'dbg-dataprep=deep_blue_genome.data_preparation.main:main',
            'dbg-coexpnetviz=deep_blue_genome.coexpnetviz.main:main',
        ],
    },
)