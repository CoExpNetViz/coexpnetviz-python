# Author: Tim Diels <timdiels.m@gmail.com>

'''
Homogeneity and separation are calculated as defined in:
Sharan, R., A. Maron-Katz, and R. Shamir. "CLICK and EXPANDER: A System for
Clustering and Visualizing Gene Expression Data." Bioinformatics (2003):
    1787-799. Print.
'''

import sys
import os
from scipy.stats import pearsonr
import re
import numpy as np
from collections import defaultdict
import itertools

def pearson_distance(vector1, vector2):
    '''
    Pearson distance normalised to be in range of [0,1]
    Definition: http://www.improvedoutcomes.com/docs/WebSiteDocs/Clustering/Clustering_Parameters/Pearson_Correlation_and_Pearson_Squared_Distance_Metric.htm
    '''
    return pearsonr(vector1, vector2)[0]

class Cluster(object):
    def __init__(self):
        self._genes = []

    def add_gene(self, gene):
        assert gene not in self._genes, 'Gene appears twice in cluster: ' + cluster_id + repr(self._genes) + gene
        self._genes.append(gene)

    def get_average(self, expr_mat, np_mat):
        '''
        expr_mat: ExpressionMatrix
        np_mat: numpy array of expression matrix
        '''
        indices = map(expr_mat.get_row_index, self._genes)
        return sum(np_mat[indices])/len(self)

    def get_homogeneity(self, expr_mat, np_mat):
        '''
        expr_mat: ExpressionMatrix
        np_mat: numpy array of expression matrix
        '''
        correlations = np.array([pearson_distance(np_mat[expr_mat.get_row_index(gene1)], np_mat[expr_mat.get_row_index(gene2)]) for gene1, gene2 in itertools.product(self._genes, self._genes) if gene1 < gene2])
        homogeneity = np.mean(correlations)
        #assert 0.0 - 1e-10 <= homogeneity, homogeneity
        #assert homogeneity <= 1.0 + 1e-10, homogeneity
        return homogeneity

    def __len__(self):
        '''Number of genes in cluster'''
        return len(self._genes)

class Clustering(object):
    def __init__(self, path):
        '''Load clustering from file'''
        self.clusters = defaultdict(Cluster) # {cluster_id : cluster_info}
        with open(path) as src_file:
            # values
            for line in src_file.readlines():
                # read line
                line = line[:-1]
                parts = re.split('\s+', line)
                assert len(parts) == 2
                gene = parts[0]
                cluster_id = parts[1]
                self.clusters[cluster_id].add_gene(gene)
    # TODO add unclustered cluster with all genes not in a cluster, ask Oren whether to include that in average homogeneity

    def get_average_homogeneity(self, expr_mat, np_mat):
        '''
        expr_mat: ExpressionMatrix
        np_mat: numpy array of expression matrix
        '''
        homogeneities = np.array([cluster.get_homogeneity(expr_mat, np_mat) for cluster in self.clusters.values()])
        weights = np.array([len(cluster) for cluster in self.clusters.values()])
        average_homogeneity = np.average(homogeneities, weights=weights)
        return average_homogeneity

    # separation: http://www3.stat.sinica.edu.tw/statistica/oldpdf/A12n112.pdf section 4.1
    def get_separation(self, expr_mat, np_mat):
        '''
        expr_mat: ExpressionMatrix
        np_mat: numpy array of expression matrix
        '''
        return float('nan') #TODO took too long to run
        if len(self.clusters) <= 1:
            return float('NaN')
        separation = 0.0
        sum_counts = 0.0
        pairs = [(c1, c2) for c1, c2 in itertools.product(self.clusters.values(), self.clusters.values()) if c1.name < c2.name]
        correlations = np.array([pearson_distance(c1.get_average(expr_mat, np_mat), c2.get_average(expr_mat, np_mat)) for c1, c2 in pairs])
        weights = np.array([len(c1) * len(c2) for c1, c2 in pairs])
        separation = np.average(correlations, weights=weights)
        return separation

    def __len__(self):
        '''Number of clusters'''
        return len(self.clusters)

