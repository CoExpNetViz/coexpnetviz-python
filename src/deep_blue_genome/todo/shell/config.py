'''
Loading this module sets up the environment appropriately.

Further, it contains:

- conf: a configuration object of configuration specific to the current
  working directory and environment
'''

def _get_config():
    from collections import namedtuple
    from plumbum import local
    from deep_blue_genome.shell.util import is_ancestor_or_equal
    import sys

    root_dir = local.path(local.env['root_dir']) # TODO might want to move config to data_preparation dir
    data_dir = root_dir / 'data'
    config = {'root_dir' : root_dir}

    path = local.cwd
    if is_ancestor_or_equal(data_dir, path):
        script_dir = root_dir / 'code/scripts'
        local.env['AWKPATH'] = root_dir / 'code/awk'
        config['src_dir'] = data_dir / 'src'
        config['script_dir'] = script_dir
        config['conv_script_dir'] = script_dir / "data_to_expression_matrix"

    Config = namedtuple('Configuration', config.keys())
    return Config(**config)

conf = _get_config()
del _get_config
