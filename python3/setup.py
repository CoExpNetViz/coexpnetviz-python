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
#import pypandoc

here = path.abspath(path.dirname(__file__))

with open(here + '/src/deep_blue_genome/version.py') as f:
    code = compile(f.read(), here + 'src/deep_blue_genome/version.py', 'exec')
    exec(code)

long_description = 'TODO' #pypandoc.convert('readme.md', 'rst', 'md')
src_root = 'src'
    
setup(
    name='deep_blue_genome',
    description='Genome analysis platform',
    long_description=long_description,
    version=__version__,
    author='VIB/BEG/UGent',
    author_email='tidie@psb.vib-ugent.be',

    url='https://bitbucket.org/deep_blue_genome/deep_blue_genome', # project homepage.
 
    license='LGPL3', #TODO
 
    classifiers=[ # https://pypi.python.org/pypi?%3Aaction=list_classifiers
        'Natural Language :: English',
        'Intended Audience :: Science/Research',
        
        'Environment :: Console',
        
        'Development Status :: 2 - Pre-Alpha',
        
        'Topic :: Scientific/Engineering :: Bio-Informatics',
        'Topic :: Scientific/Engineering :: Artificial Intelligence',
        
        'Operating System :: POSIX', #TODO must add any sub things too
 
        'License :: OSI Approved :: GNU Lesser General Public License v3 (LGPLv3)',
 
        'Programming Language :: Python',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.2',
        'Programming Language :: Python :: 3.3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
    ],
 
    # What does your project relate to?
    keywords='bioinformatics genome-analysis morph coexpnetviz',
 
    # List packages
    packages=find_packages(src_root),
    package_dir={'': src_root}, # tell setup where packages are
 
    # Required dependencies
    install_requires=[],#TODO tmp disabled deps 'matplotlib scikit-learn pandas numexpr bottleneck plumbum inflection more_itertools memory_profiler psutil numpy'.split(), #Not essential yet sqlalchemy. TODO mysql-connector
 
    # Optional dependencies
    extras_require={
        'dev': 'twine pypandoc '.split(),
        'test': ['pytest'],
    },
 
    # Auto generate entry points
    entry_points={
        'console_scripts': [
            'dbg-dataprep = deep_blue_genome.data_preparation.main:main',
            'dbg-coexpnetviz = deep_blue_genome.coexpnetviz.main:main',
        ],
    },
)
