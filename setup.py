from setuptools import setup, find_packages

name = 'coexpnetviz'
setup(
    version='5.1.1.dev',
    name=name,
    entry_points={'console_scripts': [
        'coexpnetviz = coexpnetviz.main:main'
    ]},

    # Only include {name}/, not e.g. tests/
    packages=find_packages(include=(name, name + '.*')),
)
