import pytest
from plumbum import local
from functools import partial
from deep_blue_genome.coexpnetviz.main import main_ as cenv_, CorrelationMethod
from deep_blue_genome.morph.main import main_ as morph_
from deep_blue_genome.data_preparation.main import main as data_prep
from deep_blue_genome.core.util import get_distinct_colours,\
    spread_points_in_hypercube
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import time

data_dir = local.path('data')

def copy_data(destination_dir, prefix, file):
    '''
    Copy file to tmpdir, return basename
    '''
    if not file:
        return file
    elif isinstance(file, list):
        return [copy_data(destination_dir, prefix, item) for item in file]
    else:
        file =  prefix / file
        file.copy(destination_dir)
        return file.name

# TODO all these tests are manual, can't we automate? (md5sums are a start, allows you to autotest without keeping the actual files in repo; still need input file though...)
class TestCENV(object):
    
    '''
    CoExpNetViz tests
    ''' 
    
    def run(self, tmpdir, prefix, baits_file, expression_matrices, gene_families_file=None, similarity_metric=None, lower_percentile_rank=None, upper_percentile_rank=None):    
        copy_data_ = partial(copy_data, tmpdir, prefix)
        
        # prefix and copy
        baits_file = copy_data_(baits_file)
        expression_matrices = copy_data_(expression_matrices)
        gene_families_file = copy_data_(gene_families_file)
        
        args = ['--baits-file', baits_file, '--expression-matrices'] + expression_matrices
        if gene_families_file:
            args.extend(['--gene-families', gene_families_file])
        if similarity_metric:
            args.extend(['--similarity-metric', similarity_metric.name])
        if lower_percentile_rank:
            args.extend(['--lower-percentile-rank', lower_percentile_rank])
        if upper_percentile_rank:
            args.extend(['--upper-percentile-rank', upper_percentile_rank])
            
        # run test
        with local.cwd(tmpdir):
            cenv_(['dbg-coexpnetviz'] + list(map(str, args)))
        
    #TODO 1 species custom
    # TODO check output automatically
    def test_custom_fam_2_species(self, tmpdir):
        self.run(tmpdir, prefix=data_dir / 'custom_fams',
            baits_file='baits',
            expression_matrices=['caros_expression_matrix', 'itag_expression_matrix'],
            gene_families_file='gene_families' 
        )
        
    def test_missing_bait(self, tmpdir):
        self.run(tmpdir, prefix=data_dir / 'custom_fams',
            baits_file='baits_missing',
            expression_matrices=['caros_expression_matrix', 'itag_expression_matrix'],
            gene_families_file='gene_families' 
        )
        
    def test_plaza_1_species(self, tmpdir):
        self.run(tmpdir, prefix=data_dir / 'plaza_fams',
            baits_file='baits_one_species',
            expression_matrices=['arabidopsis_expression_matrix'],
            gene_families_file='merged_plaza.gene_fams.txt' 
        )
        
    def test_plaza_1_species_no_genefam(self, tmpdir):
        self.run(tmpdir, prefix=data_dir / 'plaza_fams',
            baits_file='baits_one_species',
            expression_matrices=['arabidopsis_expression_matrix'] 
        )
        
    def test_plaza_2_species(self, tmpdir):
        self.run(tmpdir, prefix=data_dir / 'plaza_fams',
            baits_file='baits_two_species',
            expression_matrices=['arabidopsis_expression_matrix', 'apple_expression_matrix'],
            gene_families_file='merged_plaza.gene_fams.txt',
            #similarity_metric=SimilarityMetric.mutual_information 
        )
        
    def test_plaza_2_species_percentiles(self, tmpdir):
        self.run(tmpdir, prefix=data_dir / 'plaza_fams',
            baits_file='baits_two_species',
            expression_matrices=['arabidopsis_expression_matrix', 'apple_expression_matrix'],
            gene_families_file='merged_plaza.gene_fams.txt',
            lower_percentile_rank=1,
            upper_percentile_rank=99,
        )
        
        
class TestDataPrep(object):
    
    '''
    Data prep tests
    ''' 
    
    def run(self, tmpdir, prefix, files):    
        copy_data_ = partial(copy_data, tmpdir, prefix)
        
        # prefix and copy
        copy_data_(files)
            
        # run test
        with local.cwd(tmpdir):
            #['dbg-data-prep'] + 
            data_prep()
        
    def test_run(self, tmpdir):
        self.run(tmpdir, prefix=data_dir / 'data_preparation',
            files='pgsc_itag_mapping plaza/dicot_families plaza/family_clusters plaza/monocot_families'.split(), 
        )

def manual_test_distinct_colors():
    n=50
    colors = list(get_distinct_colours(n))
    print(colors)
    pd.Series([1]*n).plot.pie(colors=colors)
    plt.show()
    
def test_spread_points():
    for n, dims in [(4,2), (9,3), (8,2), (5,3)]:
        result = list(spread_points_in_hypercube(n, dims))
        print(result)
        assert len(result) == n
        assert len(result[0]) == dims

class TestMORPH(object):
    
    '''
    CoExpNetViz tests
    ''' 
    
    def run(self, tmpdir, prefix, baits_file, top_k=None):    
        copy_data_ = partial(copy_data, tmpdir, prefix)
        
        # prefix and copy
        baits_file = copy_data_(baits_file)
        
        # args
        args = ['--baits-file', baits_file]
        if top_k:
            args.extend(['--top-k', str(top_k)])
            
        # run test
        with local.cwd(tmpdir):
            morph_(['dbg-morph'] + list(map(str, args)))
           
    # TODO Tests:
    # - Are the correct clusterings an matrices selected? Think of min amount of baits that need to be present. Simply don't mention those that did not qualify. If none qualify, do mention that, duh.
    # - Sanity check on output: AUSR in valid range? Scores in valid range (not NaN)?
    # - Compare to an old MORPH run: run with your stuff, take selected list and feed it as a job to old MORPH, then you can compare outputs. 
    def test_tmp(self, tmpdir):
        '''
        No matrices or clusterings are matched
        '''
        self.run(tmpdir, prefix=data_dir / 'plaza_fams',
            baits_file='baits_two_species',
        )
         
#     def test_no_match(self, tmpdir):
#         '''
#         No matrices or clusterings are matched
#         '''
#         self.run(tmpdir, prefix=data_dir / 'plaza_fams',
#             baits_file='baits_two_species',
#         )
            
if __name__ == '__main__':
#     test = 'test_all.py::TestCENV::test_custom_fam_2_species'
#     test = 'test_all.py::TestCENV::test_plaza_1_species_no_genefam'
#     test = 'test_all.py::TestCENV::test_plaza_1_species'
#     test = 'test_all.py::TestCENV::test_plaza_2_species_percentiles'
#     test = 'test_all.py::TestCENV::test_plaza_2_species'
#     test = 'test_all.py::TestCENV::test_missing_bait'
    test = 'test_all.py::TestMORPH::test_tmp'
#     test = 'test_all.py::TestDataPrep::test_run'
#     pytest.main('--maxfail=1 ' + test)
    
    
    #cenv_(['--similarity-metric', 'magic'])
    #cenv_(['cenv','-h'])
    morph_(['morph', '-h'])
    
    #manual_test_distinct_colors()
    
    
    