#!/bin/bash

cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_VIEW=ON ..
make

./wevo --input-file ../inputs/ex2.pnts --view
./wevo --input-file ../inputs/ex3.pnts --view
./wevo --input-file ../inputs/ex1.pnts --view
./wevo --input-file ../inputs/ex5.pnts --view
./wevo --input-file ../inputs/ex6.pnts --view
./wevo --input-file ../inputs/ex1.graphml --view
./wevo --input-file ../inputs/collinear.pnts --view
./wevo --input-file ../inputs/koch_snowflake.pnts --view
