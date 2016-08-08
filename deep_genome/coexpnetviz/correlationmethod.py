# Copyright (C) 2015 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
# 
# This file is part of Deep Genome.
# 
# Deep Genome is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Deep Genome is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with Deep Genome.  If not, see <http://www.gnu.org/licenses/>.

from deep_genome.core.metrics import pearson_r, similarity_matrix
from sklearn.metrics import mutual_info_score
from enum import Enum

CorrelationMethod = Enum('CorrelationMethod', 'pearson_r mutual_information')

def call(self, *args, **kwargs):
    if self == CorrelationMethod.pearson_r:
        return pearson_r(*args, **kwargs)
    elif self == CorrelationMethod.mutual_information:
        return similarity_matrix(*args, metric=mutual_info_score, **kwargs)
    else:
        assert False
CorrelationMethod.__call__ = call
del call