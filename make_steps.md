## change the config.m4 for compile/link options and other changes
vi config.m4  #...

## build the libgo with split-stack
cd ../libgo
cd build
cmake .. -DCMAKE_CXX_FLAGS="-fsplit-stack"
make clean
make
make install

## to build phpgo
cd ../phpgo

## if you've just recompiled the libgo, do a ldconfig first
ldconfig

./phpize
./configure -enable-phpgo  -with-php-config=/usr/local/php/bin/php-config
make clean
make
make install
