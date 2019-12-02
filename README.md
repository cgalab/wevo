# Multiplicatively Weighted Voronoi Diagrams (Version 0.1)

## Command Line Options
`mwvd` currently supports the following command line options:
~~~~
--help ... produce help message
--input-file <file> ... specify the input file
--ipe-file <file> ... write output to Ipe file
--csv-file <file> ... append runtime data to CSV file
--view ... open graphical user interface
~~~~

## Testing
The `mwvd` project offers a simple test environment which is situated in the folder `test`. If you run the bash script `run_tests.sh` (within `test`), then a series of randomly generated inputs is produced, `mwvd` is executed on each of them, and the runtime statistics are written into `test/results.csv`.
