#!/bin/sh
set -e

rm -rf venv
python3.4 -m venv venv
. venv/bin/activate
pip3 install --upgrade pip
pip3 install --upgrade setuptools
pip3 install --upgrade wheel
pip3 install -r requirements.txt
pip3 install '.[test,dev]'
