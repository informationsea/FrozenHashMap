Frozen Hash Map [![Build Status](https://travis-ci.org/informationsea/FrozenHashMap.svg)](https://travis-ci.org/informationsea/FrozenHashMap)
===============

Fast Immutable Hash Map

Warning
-------

API and database format is not stable. Feedback is welcome.

License
-------

* GNU GPL version 3 or later

Requirements
------------

* [Kyoto Cabinet](http://fallabs.com/kyotocabinet/) is required to
  build.

How to build?
-------------

### From Git Repository

    $ git clone https://github.com/informationsea/FrozenHashMap.git
    $ cd FrozenHashMap
    $ ./autogen.sh
    $ ./configure
    $ make
    $ sudo make install

Author
------

* Yasunobu OKAMURA

A following library is included in `src`.

* [MurmurHash3](https://code.google.com/p/smhasher/wiki/MurmurHash3) was written by Austin Appleby

Following libraries are included in `test/performance` to do
performance test. Thanks a lot to authors!

* [CDB++](http://www.chokkan.org/software/cdbpp/)
* [SFMT 1.4.1](http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/SFMT/index-jp.html)

