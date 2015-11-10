from celery import Celery

# Celery
# 1 app, call it celery, pass it around

print('construct celery')
celery = Celery('deep_blue_genome', backend='rpc://', broker='amqp://guest@localhost//')

@celery.task
def say_hi():
    print('hi')
    return 31

@celery.task
def add(x, y):
    return x + y

print('send task')
#result = say_hi.delay()
result = add.delay(4, 3)
print(result.result)
print(result.get(10))