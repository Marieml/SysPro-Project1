Name: Maria Miliou
A.M.: 1115201300101
Course: K24 System Programming
		 -------------
		| ASSIGNMENT 1 |
		 -------------

----BASICS----------------------------------------------------------------------------------

- Submitted files:
	- Makefile	- queue.h
	- manager.c	- item.h
	- worker.c	- finder.sh
	- queue.c

- Compilation: Type make to create sniffer

- Execution: * Run ./sniffer with no arguments to listen current directory
		or ./sniffer -p <path> for given path

	     * Before new execution make sure to clean by typing "make clean"
		to delete .out files. Otherwise unlink fails when program is terminated.
		I choose do it with make clean and not in program so user can execute
		finder.sh afterwards too (before cleaning of course).

- System: I run my program in Linux DIT, specially linux02.di.uoa.gr with 2 terminals,
	  one for execution of sniffer and an other one for moving. I tested to move files 
	  with command mv e.g "mv 100.txt <path>" for one file or "mv *.txt <path>" for 
	  multiple files.

	  I tried as input files given from prof. (100.txt, 50.txt, 10.txt) and all worked fine.	  		

----PROGRAM-------------------------------------------------------------------------------------

-- Manager (manager.c)

- main()
	* Creates 2 queues one for available(Qavailable) workers and one for all workers(Qcount).
	* Creates 2 directories one for fifos with worker (/fifos) and one for output files
	  of workers (/out).
	* Defines 2 signal handlers for signals SIGCHLD and SIGINIT. Flag SA_RESTART is set in the 
	  first one so manager open() will not be interrupted by child signal when worker waits to read
	  from fifo while manager trying to open to write to it.
	* Creates pipe and fork child listener.
		*Listener
		- Listener connects stdout to pipe end and execute inotifywait with flags
		  "-m" to execute indefinitely
	          "-e moved_to, create" to listen for this 2 events only
		  "--format %f" to send only the filename
	* Manager goes to an infinity loop
	  * Reads from pipe filename that listener wrote
	  * If queue with available workers is empty (Qavailable), he creates a fifo and fork worker.
	     * Worker
		- Goes to an infinity loop
		- Opens fifo, call worker() to do his job and stops by sending SIGSTOP himself.
             * Manager
		- Push worker to queue with all existing workers Qcount
		- Opens fifo and writes filename.
	  * If there is available worker
		- Pop it from queue (Qavailable)
		- Send to worker SIGCONT
		- Opens fifo and writes filename

- sigchld_handler()
	* Listens for any child
	* If it is worker push it to Qavailable

- siginit_handler()
	* Pop all existing workers from Qcount to delete fifos and kill them.
	* Remove directory fifos
	* Destroy Qavailable
	* Kill listener and commit suiside

- er_write()
	* Writes size-bytes to buffer. If writes less than size, try again to write the rest. This
	  function is called by manager when writing to fifo to ensure that in case of interrupted 
	  write by signal, it will write the whole word.



-- Worker (worker.c)

- worker(int, char*)
	* Take as arguments file descriptor of pipe and path that listner listens to.
	* Reads from fifo filename that manager wrote and opens that file for reading
	* Creates new file with name <filename>.out, without considering file extension if exists.
	* Allocates size-of-file buffer for reading to avoid losing urls and reads file.
	* Break buffer into series of tokens seperated by space with strtok_r()
	* When token than begins with http:// is found, using strtok_r(), take only location.
	  No consider "www." if exists.
	* Call loc_write() for writing to file.out
	* Call rewrite_out() to create a user-friendly file (readable)

- loc_write(int, char*)
	* Arguments: file descriptor of file.out and location.
	* Writes to file.out entries struct location. If location exists just increase its counter
	  otherwise add location to end of file. 

- rewrite_out(pid_t, cahr*)
	* Arguments: worker pid, filename of file.out
	* Because the file that worker created is written using structs and specially appearances is type 
	  of int, it is not readable from user and finder.sh. So this function converts its to the required form.
	* Creates a new file with name pid, to ensure that it is unique
	* Reads from file.out that worker created the entries and writes it to new file.
	* Deletes file.out and rename new file pid to file.out.


-- Queue (queue.c)
	* Implementation of queue usining linked list stucture and functions
	* Stores items struct Worker{ workers pid, fifo name }
	* Operations: init, pop, push, delete, is_empty, find (custom)
	* The first 5 is a basic implemention of these operations like I learned from course Data Structures.
	* queue_find() : By given workers pid returns his fifo name


-- Libraries

	- queue.h
	* Definition of queue functions in queue.c as long as definition of worker() in worker.c
	- item
	* Definition of type of struct Worker

-- BASH (finder.sh)

	* For each file in directory /out with file extension .out:
	  - Searches for entries .<TLB>[space] with grep.
	  - grep sends output to cut through pipe which takes 2nd column with delimiter [space], aka num_of_apperances
	  - cut sends output to tr through pipe to translate new line to +
	  - output will be e.g "n1+n2+n3+" so in the ends just adds 0.
	  - Compute the sum with let
	  - Prints TLB and sum of apperances in one line.