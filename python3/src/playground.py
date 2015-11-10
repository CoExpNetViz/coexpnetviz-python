from Bio import Entrez
from deep_blue_genome.exceptions import GeneNotFoundException
Entrez.email = 'no-reply@psb.ugent.be'

# Gene lookup
#==============

# TODO
# Searching a single gene, easy enough. Searching multiple? You'd best query them as a whole, then query detailed info with ids and then search for the names in those records.
# Or, you could download their gene db (and occasionally(daily) download or subscribe to updates, to stay up to date) and use that. -> ftp://ftp.ncbi.nlm.nih.gov/gene/DATA/
# We also need to be careful with failures to get a result (network hickups -> retry a couple of times, then give up).
# Latency might kill performance, but let's wait and see
 
search_result = Entrez.read(Entrez.esearch("gene", term="AT1G01020[Gene Name]"))
ids = search_result['IdList']
if len(ids) > 1:
    raise NotImplemented  # Could throw something, could take the first, could use something somewhere as tiebreaker. Could make the behaviour configurable
elif not ids:
    raise GeneNotFoundException
else:
    gene = list(Entrez.parse(Entrez.efetch("gene", id=ids[0], retmode="xml")))[0]
    print(gene)

#record = records[0]

#print(len(records))
#print(records)
#print(record.keys())
#print(record['Entrezgene_gene'])