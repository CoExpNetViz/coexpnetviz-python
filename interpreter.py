import pandas as pd
import numpy as np
import plumbum as pb
import itertools
import more_itertools
import deep_genome as dbg
import matplotlib.pyplot as plt
df = pd.DataFrame([[1,2,3],[4,5,6],[7,8,9]], columns=['a','b','c'])
s = df['a']
n = np.array([1,2,3])
p = pb.local.path('/bin/ls')
