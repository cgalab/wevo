# wevo (Version 0.1)

`wevo` is currently an experimental prototype implementation. It computes the multiplicatively weighted Voronoi diagram of a given point set. Example input files can be found in the `inputs` folder.

## Build
The easiest way to build `wevo` is by using the corresponding `CMakeLists.txt` file. If you also want to use the built-in view then don't forget to build `wevo` with the `BUILD_VIEW` option. To run some examples, execute the `run_examples.sh` script that is situated in the `build` directory. In particular, execute the following commands (from the `wevo` directory) to run our example inputs:
~~~~
cd build &&
cmake -DCMAKE_BUILD_TYPE=Release .. &&
make &&
bash run_examples.sh
~~~~

## Build Requirements
* build-essential
* cmake
* libgmp-dev
* libmpfr-dev
* libcgal-dev
* libcgal-qt5-dev
* libboost-all-dev
* libqt5opengl5-dev
* libqt5svg5-dev
* qtbase5-dev

## Command Line Options
`wevo` currently supports the following command line options:
~~~~
--help ... produce help message
--input-file <file> ... specify the input file
--ipe-file <file> ... write output to Ipe file
--csv-file <file> ... append runtime data to a CSV file
--view ... open graphical user interface
~~~~
You can close the `wevo` window by pressing `ESC`.

## Testing
The `wevo` project offers a simple test environment which is situated in the folder `test`. If you run the bash script `run_tests.sh` (within `test`), then a series of randomly generated inputs is produced, `wevo` is executed on each of them, and the runtime statistics are written into `test/results.csv`. Execute the following commands (from the `wevo` directory) to run our test scripts:
~~~~
cd test &&
cmake -DCMAKE_BUILD_TYPE=Release .. &&
make &&
bash run_tests.sh
~~~~
