# TODO rm, this is debug
def p(items):
    for i in items:
        print(i)

def even(i):
    return i % 2 == 0
 
def split(sink1, sink2):
    while True:
        i = yield
        if even(i):
            sink1.send(i)
        else:
            sink2.send(i)
    sink1.close()
    sink2.close()

def yielder():
    while True:
        yield

split(range(0,10))
split