from deep_blue_genome.test.util import CLITester
 
cli = CLITester('prepare'.split())
prefix = 'data_preparation'

class TestPrepare(object):
    
    '''
    Database prepare tests
    ''' 
        
    def test_run(self, tmpdir):
        cli.tmpdir = tmpdir
        cli.run()
        print('hi')