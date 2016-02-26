from plumbum import local
import deep_blue_genome.todo.probe_gene_map as probe_gene_map
from deep_blue_genome.shell.gup_script import gupup

root_dir = local.path(local.env['root_dir'])
data_dir = root_dir / 'data'
src_dir = data_dir / 'src/rice'
matrices_dir = src_dir / 'expression_matrices'
affy_dir = matrices_dir / 'affymetrix'

def get_probe_gene_map():
    probe_to_gene_dir = affy_dir / 'probe_to_gene'
    probe_names = probe_to_gene_dir / 'probe_names'
    gene_names = probe_to_gene_dir / 'gene_names'
    gupup(probe_names, gene_names)
    return probe_gene_map.read(probe_names, gene_names)
