import pytest
from chicken_turtle_util.test import temp_dir_cwd
from deep_genome.coexpnetviz.main import Context
from deep_genome.coexpnetviz import initialise
from pathlib import Path

# http://stackoverflow.com/a/30091579/1031434
from signal import signal, SIGPIPE, SIG_DFL
signal(SIGPIPE, SIG_DFL) # Ignore SIGPIPE

initialise()

# TODO test_conf, cli_test_args, context fixture, session fixture, database (db) fixture are all recurring things across all the algos. Maybe add a core.test with util

@pytest.fixture()
def test_conf_path(pytestconfig):
    return Path(str(pytestconfig.rootdir / 'test.conf'))

@pytest.yield_fixture
def context(test_conf_path):
    context = Context(test_conf_path)
    yield context
    context.dispose()

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