#!/bin/bash

if [ "$TRAVIS_BUILD_DIR" = "" ];then
    export TRAVIS_BUILD_DIR="$PWD"
fi

echo "$TRAVIS_BUILD_DIR"

URL=http://fallabs.com/kyotocabinet/pkg/kyotocabinet-1.2.76.tar.gz
FILENAME=${URL##*/}

if [ -d kcbuild ];then
    rm -r kcbuild || exit 1
fi

mkdir -p kcbuild
pushd kcbuild
wget $URL || exit 1
tar xzf $FILENAME || exit 1
pushd ${FILENAME%.tar.gz}
sh -c "wget -O - https://gist.github.com/informationsea/fd62daafc40cc4d7dcc4/raw/15ff366c1e40a7f5def917ff74b82df5417b003c/kyotocabinet-1.2.76-clang.patch|patch -p1" || exit 1
./configure --prefix=$TRAVIS_BUILD_DIR/local || exit 1
make && make install || exit 1

popd
popd

