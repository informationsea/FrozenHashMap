#!/bin/sh

export BASE_DIR="$PWD/`dirname $0`"
cd $BASE_DIR
top_dir="$BASE_DIR/../.."

if test -z "$NO_MAKE"; then
    make -C $top_dir > /dev/null || exit 1
fi

mkdir -p testdata

date >> testdata/log.txt
./kyotocabinet_kch 2>> testdata/log.txt
./kyotocabinet_kct 2>> testdata/log.txt
./cdbpp 2>> testdata/log.txt
./frozenhashmap 2>> testdata/log.txt
