from deep_blue_genome.test.util import CLITester
import plumbum as pb
 
cli = CLITester('prepare --output-dir dist --cache-dir cache --tmp-dir tmp'.split())
prefix = 'data_preparation'

def init_tmp(tmpdir):
    pb.local.path(tmpdir)
    tmpdir.mkdir('dist')
    tmpdir.mkdir('cache')
    tmpdir.mkdir('tmp')
        
class TestPrepare(object):
    
    '''
    Database prepare tests
    ''' 
        
    def test_run(self, tmpdir):
        init_tmp(tmpdir)
        cli.tmpdir = tmpdir
        cli.run()
        print('hi')