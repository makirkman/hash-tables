# Hash Tables
A program which allows storage of integer values in hash tables, using a variety of algorithms.

## Summary
The purpose of this project is to explore the different strategies for storing and accessing data in hash tables, both demonstrating those strategies in practice, and providing data with which their performance can be analysed, such as time and space usage across similar input data.

The hashing strategies available are:
* Cuckoo Hashing (cuckoo.c)
* Extendible Hashing (xtndbln.c)
* Extendible Cuckoo Hashing (xuckoo.c)

***

## Usage

### Build
To build, simply run `make` in the program directory.

To clean the program folder after a build, run `make clean` - this will call `rm -f` for all .o files.

To clean the program folder of all build files and the executable, run `make clobber` - this will call `rm -f` for all .o files and the executable.

### Run
To run, run the executable, with the desired arguments.

The program requires one argument to start: `-t` (table type to use), and can take the optional \[ `-s` \] argument to specify initial table size or bucket size for Cuckoo and Extendible tables respectively. This will create the desired hash table in memory, and initiate the interpreter to allow commands to be given.

There are three options for `-t`:

| -t  | Table Type        |
| --- | ----------------- |
| 0   | Cuckoo            |
| 1   | Extendible        |
| 2   | Extendible Cuckoo |

An example command:
```
./ht -t 1 -s 16
```

### Interact
Once the program is running, commands can be given individually to manipulate or see details about the table. Options are: insert (`i`), lookup (`l`), print the table (`p`) or print statistics about it (`s`), get help (`h`), or quit (`q`).

`i` and `l` must be followed an argument, a number to insert or look for (e.g. `i 20`)

### Quick Test
To get a quick look at the behaviour of the program on a large collection of commands, there is a `sample-input.txt` file, containing 100,000 `i` commands, 100,000 `l` commands, a `p`, and an `s`, in that order. This can be fed into the program by first building it, but instead of running the interpreter, giving the following command:

```
./ht -t <table_type> < sample-input.txt
```

The output of the sample commands when given for each type of hash table are stored in respective files in the `sample-output` folder. The statistics at the bottom of the output files allow a user to see the behaviour of each table without needing to build and run the program directly.

***

## Code Structure
The key aspect of this project, the different hash table strategies, is the contents of the `tables` folder, plus examples of their use in the `sample-output` folder.

The top of the `src` folder contains the interface for using and accessing the project: a cli for running and interacting with the project in `main`; and a code interface of general functions for accessing hash tables in `hashtbl`.

***

Program by Maxim Kirkman, max.kirkman94@gmail.com.

The interpreter, general table wrapping, and print behaviour was largely already provided; actual table behaviour (the tables folder) was written alone, with the framework as a starter.
