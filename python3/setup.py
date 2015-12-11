"""
A setuptools based setup module.

See:
https://packaging.python.org/en/latest/distributing.html
https://github.com/pypa/sampleproject
"""

from setuptools import setup, find_packages  # Always prefer setuptools over distutils
from codecs import open  # To use a consistent encoding
from os import path
import pypandoc
import plumbum as pb

# Automated template stuff (TODO extract this into reusable lib to include in our projects) 
def setup_(**args):
    here = pb.local.path(__file__).dirname
    
    # read some of args
    name = args['name']
    src_root = args['src_root']
    del args['src_root']
    
    # various tidbits
    pkg_root = src_root / name
    
    # version
    version_file = pkg_root / 'version.py'
    with version_file.open() as f:
        code = compile(f.read(), str(version_file), 'exec')
        locals = {}
        exec(code, None, locals)
        __version__ = locals['__version__']
        
    # data files
    data_files = [str(path - pkg_root) for path in (pkg_root / 'data').walk(filter=lambda x: not x.isdir())]
    
    # override
    relative_src_root = str(src_root - here)
    args.update(
        version=__version__,
        
        # List packages
        packages=find_packages(relative_src_root),
        package_dir={'': relative_src_root}, # tell setup where packages are
        
        # List data files
        package_data={name: data_files},
    )
    
    # setup
    setup(**args)

here = pb.local.path(__file__).dirname
    
# Debug stuff
setup_(name='deep_blue_genome', src_root=here / 'src')

# setup
setup_(
    # custom attrs
    src_root = here / 'src',
    
    # standard
    name='deep_blue_genome',
    long_description = pypandoc.convert('readme.md', 'rst'),
    
    description='Genome analysis platform',
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
 
    # Required dependencies
    setup_requires='pypandoc plumbum'.split(), # required to run setup.py
    install_requires=(
        'numpy matplotlib scipy scikit-learn pandas numexpr bottleneck' +
        'plumbum inflection more_itertools memory_profiler psutil pyxdg' +
        'sqlalchemy pymysql'
        
    ).split(),
 
    # Optional dependencies
    extras_require={
        'dev': 'twine'.split(),
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