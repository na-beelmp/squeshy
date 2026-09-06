#!/bin/bash
DIR=`dirname $(readlink -f $0)`
pushd .
cd $DIR
git submodule init $DIR/files
git -c submodule.tests/files.update=checkout submodule update $DIR/files
popd
