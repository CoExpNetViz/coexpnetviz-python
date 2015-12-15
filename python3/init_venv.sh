#!/bin/sh
set -e

rm -rf venv
python3.4 -m venv venv
. venv/bin/activate
pip install --upgrade pip
pip install --upgrade setuptools
pip install --upgrade wheel
pip install -r requirements.txt
pip install '.[test,dev]'
