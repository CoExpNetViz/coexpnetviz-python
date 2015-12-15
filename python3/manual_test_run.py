import pytest
from deep_blue_genome.main import main

# TODO all our tests are manual, can't we automate? (md5sums are a start, allows you to autotest without keeping the actual files in repo; still need input file though...)

def manual_test_cli():
    # Note: in some cases you may want: http://click.pocoo.org/6/api/#click.testing.CliRunner
#     main(['wrong'])
#     main(['--help'])
    main(['prepare', '--help'])
#     main('prepare'.split())

#     manual_test_cli()    
#     manual_test_distinct_colors()

if __name__ == '__main__':
    pytest.main('--maxfail=1 -m current')

#     manual_test_cli()    
#     manual_test_distinct_colors()
    
    
    