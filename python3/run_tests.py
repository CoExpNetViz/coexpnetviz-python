import pytest
from deep_blue_genome.main import main
import os
from deep_blue_genome.test.unittests.util.test_merge_overlapping_named_sets import group_results

# TODO all our tests are manual, can't we automate? (md5sums are a start, allows you to autotest without keeping the actual files in repo; still need input file though...)

def manual_test_cli():
    # Note: in some cases you may want: http://click.pocoo.org/6/api/#click.testing.CliRunner
#     main(['wrong'])
#     main(['--help'])
    main(['morph', '--help'])
    main(['prepare', '--help'])
#     main('prepare'.split())

#     manual_test_cli()    
#     manual_test_distinct_colors()

if __name__ == '__main__':
    args = (
        '-m current '
#         '-n auto --benchmark-disable'  # parallel testing (can't and shouldn't benchmark in parallel, so --benchmark-disable)
        '--maxfail=1 '
#         '--capture=no '
    )
    pytest.main(args)
#     group_results()

#     manual_test_cli()    
#     manual_test_distinct_colors()
    
    
    
    