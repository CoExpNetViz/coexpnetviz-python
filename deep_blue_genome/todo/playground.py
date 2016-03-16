
def gene_lookup():

    from Bio import Entrez
    from deep_blue_genome.exceptions import GeneNotFoundException
    Entrez.email = 'no-reply@psb.ugent.be'

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
    
def dataframe():
    from pandas import DataFrame
    a = DataFrame([[1,'please split'],[2,'two words'],[3,'one']], columns=['id', 'names'])
    print(a['names'].str.split())
    for x in a.itertuples():
        print(x)
        
    name = 'x'
    for name in ['y']: pass
    print(name)
    
def bug():
    import sqlalchemy.types as sqltypes
    from sqlalchemy import create_engine, MetaData, Column, Integer, Table
    import numpy as np
    
    class MyType(sqltypes.TypeDecorator):
        '''Prefixes Unicode values with "PREFIX:" on the way in and
        strips it off on the way out.
        '''
    
        impl = sqltypes.Integer
    
        def process_bind_param(self, value, dialect):
            return int(value)
    
        def process_result_value(self, value, dialect):
            return np.int64(value)
    
        def copy(self):
            return MyType()
        
        def coerce_compared_value(self, op, value):
            print('it')
            return Integer()
        
    class MyInt(sqltypes.TypeDecorator):
        impl = sqltypes.Integer
        def process_bind_param(self, value, dialect):
            print('it')
            return super().process_bind_param(value, dialect)
        
        def coerce_compared_value(self, op, value):
            print('it')
            return super().coerce_compared_value(op, value)
        
    engine = create_engine('sqlite:///test.db', echo=True)
    metadata = MetaData()
    test_table = Table('test', metadata,
        Column('id', Integer, primary_key=True),
        Column('number', Integer)
    )
    metadata.drop_all(engine)  # clean
    metadata.create_all(engine)
    engine.execute(test_table.insert(), {'number': np.int32(123)})
    result = engine.execute(test_table.select()).first()
    assert result[1] == 123, result

def mro_hell():
    class A(object):
        def __init__(self):
            super().__init__()
            print('A')
            
    class B(object):
        def __init__(self):
            super().__init__()
            print('B')
            
    class AB(A, B):
        def __init__(self):
            super().__init__()
            print('AB(A,B)')
            
    class C(B):
        def __init__(self):
            super().__init__()
            print('C(B)')
            
    class AB_C(AB, C):
        def __init__(self):
            super().__init__()
            print('AB_C(AB, C)')
            
    class ABC(A, C, B):
        def __init__(self):
            super().__init__()
            print('ABC(A, C, B)')
            
    A()
    print('-'*20)
    B()
    print('-'*20)
    AB()
    print('-'*20)
    C()
    print('-'*20)
    AB_C()
    print('-'*20)
    ABC()
       
def mocking_time():
    import .playground_child
    import datetime
    
           
# bug()
# mro_hell()
mocking_time()

        # couldn't help it TODO rm
#         m1 = read_expression_matrix('')
#         m2 = read_expression_matrix('')
#         orths = read_gene_families_file('').groupby(index)
#         def f(mat, orths):
#             corrs = corr(mat)
#             [avg(diag(corrs[orths_,orths_])) for orths_ in orths]
#         avgs1 = f(m1, orths)
#         avgs2 = f(m2, orths)