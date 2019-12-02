#!/usr/bin/python

import random
import uuid
import os
import sys
import getopt


def main(argv):
    number_of_inputs = 1
    min_input_size = 10
    max_input_size = 100
    output_folder = "."
    help_msg = "gen_inputs.py -n <number_of_inputs> -l <min_input_size> -u <max_input_size> -f <output_folder>"

    try:
        opts, args = getopt.getopt(argv,"hn:l:u:f:",["number_of_inputs=","min_input_size=","max_input_size","output_folder="])
    except getopt.GetoptError:
        print help_msg
        sys.exit(2)

    for opt, arg in opts:
        if opt == "-h":
            print help_msg
            sys.exit()
        elif opt in ("-n", "--number_of_inputs"):
            number_of_inputs = int(arg)
        elif opt in ("-l", "--min_input_size"):
            min_input_size = int(arg)
        elif opt in ("-u", "--max_input_size"):
            max_input_size = int(arg)
        elif opt in ("-f", "--output_folder"):
            output_folder = arg

    for i in range(number_of_inputs):
        file_name = str(uuid.uuid4())
        input_size = random.randint(min_input_size, max_input_size)
        cmd = "python randomize_pnts.py -o {}/{}.pnts -s {}".format(output_folder, file_name, input_size)
        print cmd
        os.system(cmd)


if __name__ == "__main__":
    main(sys.argv[1:])
