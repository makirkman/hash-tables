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
The program requires two arguments to start: `-t` (table type to use), and \[ `-s` \] (initial table/bucket size). This will create the desired hash table in memory, and initiate the interpreter to allow commands to be given.

Commands are given individually to manipulate or see details about the table. Options are: insert (`i`), lookup (`l`), print the table (`p`) or print statistics about it (`s`), get help (`h`), or quit (`q`).

***

## Code Structure
The key aspect of this project (the different hash table strategies) are in the `tables` folder.

The top of the `src` folder contains the interface for using and accessing the project: a cli for running and interacting with the project in `main`; and a code interface of general functions for accessing hash tables in `hashtbl`.

***

Program by Maxim Kirkman, max.kirkman94@gmail.com.

The interpreter, general table wrapping, and print behaviour was largely already provided; actual table behaviour (the tables folder) was written alone, with the framework as a starter.
