# Solve pkg_resources.DistributionNotFound https://stackoverflow.com/a/60529533
from PyInstaller.utils.hooks import copy_metadata
datas = copy_metadata('humanize')
