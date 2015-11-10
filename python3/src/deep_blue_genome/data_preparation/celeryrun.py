from celerytasks import add

print('send task')
#result = say_hi.delay()
result = add.delay(4, 3)
print(result.result)
print(result.get(10))