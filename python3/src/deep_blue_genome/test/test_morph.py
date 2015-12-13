from deep_blue_genome.test.util import CLITester

cli = CLITester('morph'.split())
        
class TestMORPH(object):
    
    '''
    MORPH tests
    ''' 
           
    # TODO Tests:
    # - Are the correct clusterings an matrices selected? Think of min amount of baits that need to be present. Simply don't mention those that did not qualify. If none qualify, do mention that, duh.
    # - Sanity check on output: AUSR in valid range? Scores in valid range (not NaN)?
    # - Compare to an old MORPH run: run with your stuff, take selected list and feed it as a job to old MORPH, then you can compare outputs. 
    def test_tmp(self, tmpdir):
        '''
        No matrices or clusterings are matched
        '''
        cli.tmpdir = tmpdir
        files = cli.copy_data(
            dict(
                baits_file='baits_two_species',
            ),
            'plaza_fams'
        )
        cli.run(**files)
         
#     def test_no_match(self, tmpdir):
#         '''
#         No matrices or clusterings are matched
#         '''
#         self.run(tmpdir, prefix=data_dir / 'plaza_fams',
#             baits_file='baits_two_species',
#         )

