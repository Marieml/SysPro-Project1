# Project 1

## Goal
The purpose of this assignment is to get familiar with the following:
* Create processes using fork/exec system calls
* Communication between processes through pipes and named pipes(FIFOs)
* Low-level I/O
* Singnal handling
* Shell script

## Part 1 (85%)
Basic entities: Listener, Manager, Workers

**Listener**: Uses *inotifywait* to monitor changes in files in a directory. *Inotifywait* will be executed (using exec) in an other process *listener*

**Manager**: 
* The main entity of system which communicates with process *listener* through pipe. *Listener* will inform *manager* for each new file he detects in directory he follows up. 

* *Manager* communicates with workers through named pipes(FIFOs). For each file-name manager recieves from *listener*, he (*manager*) will notify or create (if not exist) a *worker* process. Then *worker-process* processes the file in specific way that is decribed later. In the beggining, *manager* creates as many worker-processes as  the total existing files on this directory as he is informed. *Manager* keeps the availabale *workers* in a queue.

**Workers**: Workers goal is to open the file and search for URLs that use http protocol (use only low-level I/O). Files are .txt which can contain simple text or URLs. More specific, each URL begin with http:// and ends with space character. Furthermore, for each URL worker locates, he extracts the information about its *location*.

e.g. For URL  *http://www.di.uoa.gr* location is *di.uoa.gr*.

If the name of the file *Worker* reads is <filename>, then he creates a new file <filename.out>. In this file he records all the detected locations together with the number of its appearances, one line for each pair <location, appearances>.


## Part 2 (15%)
*finder.sh* : A shell script that processes all the files that *workers* created in part 1. As arguments it takes one or more Top Level Domain(TLD) and returns the total number of appearances of each TLD in all created files. (.com , .gr)



## Implementation

### Submitted files:
	- Makefile	- queue.h
	- manager.c	- item.h
	- worker.c	- finder.sh
	- queue.c

### Compilation:
Type make to create executable sniffer

### Execution:
* Run 
```
./sniffer
```
with no arguments to listen current directory or
```
./sniffer -p <path>
```
for given path
* Note: Before new execution make sure to clean by typing "make clean"
to delete .out files. Otherwise unlink fails when program is terminated.
I choose do it with make clean and not in program so user can execute
finder.sh afterwards too (before cleaning of course).

### System:
I run my program on Linux DIT, specially linux02.di.uoa.gr with 2 terminals,
one for execution of sniffer and an other one for moving. I tested to move files 
with command mv e.g "mv 100.txt <path>" for one file or "mv *.txt <path>" for 
multiple files.

I tried as input files given from proffesors (100.txt, 50.txt, 10.txt).

More detailed explanation of code can be found in README.txt
