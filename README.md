# purisc-pipeline-simulator
Shows ideal pipeline behaviour given a program


#Compiling
Just run

    make

#Usage 
        usage: ./purisc-pipeline-simulator [options] filename 

        OPTIONS:
        -f FORMAT
            either csv or table
        -l LIMIT
            a positive integer specifying the number of cycles to run for. default=20
The input file is a human readable machine code file. Each line of the file
represents the contents of one memory location, represented in base 10. All
memory locations must have a value in the file.

The execution limit (the number of cycles the simulation should run for) can be
specified as the second argument. The default is 20.

By default the output is coloured for readability. This makes automated
analysis difficult. Do disable coloured output, set the NOCOLOR preprocessor
directive to 1.

