# Copyright (C) 2016 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
#
# This file is part of CoExpNetViz.
#
# CoExpNetViz is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# CoExpNetViz is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with CoExpNetViz.  If not, see <http://www.gnu.org/licenses/>.

import signal

from pytil.test import temp_dir_cwd  # @UnusedImport
import numpy as np
import pytest


# http://stackoverflow.com/a/30091579/1031434
signal.signal(signal.SIGPIPE, signal.SIG_IGN)  # Ignore SIGPIPE

@pytest.fixture(autouse=True)
def auto_reset_random_seed():
    np.random.seed(0)