import pandas

def read(probes_path, genes_path):
    '''
    Read probe to gene map

    probes_path: file containing [(probe_name, join_id)], no header
    genes_path: file containing [(gene_name, join_id)], no header

    We assume and assert the following relation: probe [1..*] --- [1] gene

    Return probe -> gene dict
    '''
    probes = pandas.read_table(str(probes_path), names=['probe', 'join_id'], engine='python')
    genes = pandas.read_table(str(genes_path), names=['gene', 'join_id'], engine='python')
    merged = probes.merge(genes, on='join_id')
    del merged['join_id']
    merged = merged.applymap(str.lower)
    probe_to_gene = merged.set_index('probe')['gene'].to_dict()
    assert len(merged) == len(probe_to_gene), 'Duplicate probe entries in probes file'
    return probe_to_gene

