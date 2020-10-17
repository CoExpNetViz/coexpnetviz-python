# Solve 'Could not find the matplotlib data files'
# https://stackoverflow.com/a/63265728
#
# This will be fixed upstream by pyinstaller#5004. At that point we will be
# able to delete this hook and it will use the upstream one.
from PyInstaller.utils.hooks import exec_statement

mpl_data_dir = exec_statement(
    "import matplotlib; print(matplotlib._get_data_path())")

datas = [
    (mpl_data_dir, "matplotlib/mpl-data"),
]
