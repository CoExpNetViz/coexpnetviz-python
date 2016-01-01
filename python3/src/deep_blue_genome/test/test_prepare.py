from deep_blue_genome.test.util import CLITester, cache_dir
import plumbum as pb
import pytest

cli = CLITester('prepare --output-dir dist --cache-dir {} --tmp-dir tmp'.format(cache_dir).split())

def init_tmp(tmpdir):
    pb.local.path(tmpdir)
    tmpdir.mkdir('dist')
    tmpdir.mkdir('tmp')
        
class TestPrepare(object):
    
    '''
    Database prepare tests
    ''' 
        
#     @pytest.mark.current
    def test_run(self, tmpdir):
        init_tmp(tmpdir)
        cli.tmpdir = tmpdir
        cli.run()