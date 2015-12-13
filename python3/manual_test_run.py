import pytest
from deep_blue_genome.main import main

# TODO all our tests are manual, can't we automate? (md5sums are a start, allows you to autotest without keeping the actual files in repo; still need input file though...)

def manual_test_cli():
    # Note: in some cases you may want: http://click.pocoo.org/6/api/#click.testing.CliRunner
#     main(['wrong'])
#     main(['--help'])
#     main(['prepare', '--help'])
    main('prepare'.split())

#     manual_test_cli()    
#     manual_test_distinct_colors()

if __name__ == '__main__':
#     test = 'test_all.py::TestCENV::test_custom_fam_2_species'
#     test = 'test_all.py::TestCENV::test_plaza_1_species_no_genefam'
#     test = 'test_all.py::TestCENV::test_plaza_1_species'
#     test = 'test_all.py::TestCENV::test_plaza_2_species_percentiles'
#     test = 'test_all.py::TestCENV::test_plaza_2_species'
#     test = 'test_all.py::TestCENV::test_missing_bait'
#     test = 'test_all.py::TestMORPH::test_tmp'
#     test = 'test_prepare.py::TestPrepare::test_run'
    test = 'test_prepare.py'
    pytest.main('--maxfail=1 ' + 'src/deep_blue_genome/test/' + test)

#     manual_test_cli()    
#     manual_test_distinct_colors()
    
    
    