import pytest
from configparser import ConfigParser
from chicken_turtle_util.test import temp_dir_cwd
from deep_genome.coexpnetviz.main import Context
from deep_genome.coexpnetviz import initialise
from click.testing import CliRunner
from pathlib import Path

# http://stackoverflow.com/a/30091579/1031434
from signal import signal, SIGPIPE, SIG_DFL
signal(SIGPIPE, SIG_DFL) # Ignore SIGPIPE

initialise()

# TODO test_conf, cli_test_args, context fixture, session fixture, database (db) fixture are all recurring things across all the algos. Maybe add a core.test with util

@pytest.fixture(scope='session')
def test_conf(pytestconfig):
    config = ConfigParser()
    config.read([str(pytestconfig.rootdir / 'test.conf')])  # machine specific testing conf goes here
    return config['main']

@pytest.fixture(scope='session')
def cli_test_args(test_conf):
    '''
    Arguments to prepend to any DG CLI invocation
    '''
    return test_conf['cli_args'].split()  # Note: offers no support for args with spaces

def _create_context(cli_test_args):
    _context = []
    
    @Context.command()
    def main(context):
        _context.append(context)
    
    CliRunner().invoke(main, cli_test_args, catch_exceptions=False)
    
    return _context[0]

@pytest.fixture
def context(cli_test_args, temp_dir_cwd, mocker):
    mocker.patch('xdg.BaseDirectory.xdg_data_home', str(Path('xdg_data_home').absolute()))
    mocker.patch('xdg.BaseDirectory.xdg_cache_home', str(Path('xdg_cache_home').absolute()))
    return _create_context(cli_test_args)

@pytest.yield_fixture
def session(db):
    with db.scoped_session() as session:
        yield session
        
@pytest.fixture
def db(context):
    db = context.database
    db.clear()
    db.create()
    return db