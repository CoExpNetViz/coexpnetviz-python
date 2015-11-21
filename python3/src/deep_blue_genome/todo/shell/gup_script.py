'''
Python gup scripting aids
'''

from glob import glob
from plumbum import local, FG, BG
from plumbum.cmd import find

from plumbum.machines.local import LocalCommand
from plumbum.path.utils import *
from deep_blue_genome.config import conf
import sys

class _FGLocalCommand(LocalCommand):

    '''Wrapper to always run in foreground (like & FG)'''

    def __init__(self, command):
        self.___command = command

    def __getitem__(self, arg):
        return _FGLocalCommand(self.___command.__getitem__(arg))

    def __call__(self, *args):
        self.___command.__getitem__(args) & FG

gup = _FGLocalCommand(local['gup'])
gupup = gup['-u']
target = local.path(sys.argv[1])
target_name = sys.argv[2]
target_path = local.path(target_name)

def gup_checksum_target():
    '''Treat target as result of a checksum task

    https://github.com/gfxmonk/gup#checksum-tasks
    '''
    gup('--always')
    gup('--contents', target)

def gup_directory_contents_target(directory):
    '''Fill current target with abs path listing of files in directory'''
    (find[directory, '-mindepth', 1] > target)()
    gup_checksum_target()
