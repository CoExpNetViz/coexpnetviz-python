'''
Author: Tim Diels <timdiels.m@gmail.com>
'''
import re
import numpy as np

class ExpressionMatrix(object):

    '''
    Gene expression matrix

    header: list of str
    rows: {gene -> values: list of float}
    '''

    def __init__(self, path):

        '''
        Read an expression matrix file
        '''

        with open(path) as src_file:
            lines = src_file.readlines()
            self._gene_to_row = {}

            # header
            self.header = lines[0].split('\t')

            # values
            self.rows = []
            for line in lines[1:]:
                # read line
                line = line[0:-1]
                parts = re.split('\s+', line)
                name = parts[0]
                values = list(map(float, parts[1:]))
                self.rows.append([name, values])
                self._gene_to_row[name] = len(self.rows) - 1

    def save(self, path):
        '''Write to file'''
        

