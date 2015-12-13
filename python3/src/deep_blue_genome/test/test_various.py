from deep_blue_genome.core.util import get_distinct_colours,\
    spread_points_in_hypercube
import pandas as pd
import matplotlib.pyplot as plt

# TODO split files: per interface, various for the rest
# TODO Base test class for exercising the CLI through the main func with args specified in python

def manual_test_distinct_colors():
    n=50
    colors = list(get_distinct_colours(n))
    print(colors)
    pd.Series([1]*n).plot.pie(colors=colors)
    plt.show()
    
def test_spread_points():
    for n, dims in [(4,2), (9,3), (8,2), (5,3)]:
        result = list(spread_points_in_hypercube(n, dims))
        print(result)
        assert len(result) == n
        assert len(result[0]) == dims

    