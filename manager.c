/*
	Name: Maria Miliou
	A.M: 1115201300101
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

#include "item.h"
#include "queue.h"

#define READ 0
#define WRITE 1
#define BUFSIZE 255

extern int errno;

//Queues with available and existing workers
Queue Qavailable;
Queue Qcount;

pid_t listener; //Listener pid


int er_write(int,char*,size_t);
void sigint_handler(int);
void sigchld_handler(int);

int main(int argc, char *argv[]){

    int fd_lst[2];
    char buffer[BUFSIZE];
    pid_t pid,worker_pid;
    size_t rsize, n;
	int status;

	char *arg;
	//Arguments check
	if(argc == 1)
		arg=".";
	else if(argc == 3 && memcmp(argv[1],"-p",2)==0)
		arg = argv[2];
	else{
		printf("Wrong arguments! Check README and try again :)\n");
		exit(1);
	}

    static struct sigaction work, term;
    Qavailable = queue_init();
	Qcount = queue_init();

	//Signal handler for SIGCHLD
    work.sa_handler=sigchld_handler;
    work.sa_flags = SA_RESTART;
    sigfillset(&(work.sa_mask));
    sigaction(SIGCHLD,&work,NULL);

	//Signal handler for SIGINIT
    term.sa_handler=sigint_handler;
    sigfillset(&(term.sa_mask));
    sigaction(SIGINT,&term,NULL);

	//Create directory for files.out from worker
	if(mkdir("out",0777)<0)
		perror("mkdir");
	//Create directory for named pipes manager-worker
	if(mkdir("fifos",0777)<0)
		perror("mkdir");

	//Manager-Listener pipe
    if(pipe(fd_lst) == -1){
        perror("pipe");
        exit(1);
    }

	//Creating Listener
    if((pid=fork())<0){
        perror(" Listener fork");
        exit(1);
    }

    if(pid == 0){
    	/*------Listener-----*/
    	dup2(fd_lst[WRITE],1); //Connect stdout to pipe end
    	close(fd_lst[READ]);

		/* Exec inotify. Listens only moved_to and create file and writes only name of file*/
    	execlp("inotifywait","inotifywait",arg,"-m","-e","moved_to","-e","create","--format","%f",NULL);
    	perror("execlp");

    }else{
		/*----Still in manager----*/
    	int total,i=0;
		int countWorkers=0;
		bool flag=false;
		listener = pid;
		char filename[BUFSIZE];
		close(fd_lst[WRITE]);

		/*Begin infinity loop. Stops only with SIGINIT*/
		while(1){

			/*-------Reading filename from listener--------*/

			memset(filename,0,BUFSIZE);
			memset(buffer,0, sizeof(buffer));
			//Read until you have the whole name of file(until \n)
			do{
				//Read again if read() is interrupted by a signal (SIGCHILD from worker)
				do{
		   			rsize = read(fd_lst[READ],buffer,BUFSIZE);
				}while(rsize == -1 && errno != EINTR);
		  		strncat(filename,buffer,rsize);  //Append to filename
		  	}while(buffer[rsize-1]!='\n');

			n=strlen(filename);
			if(filename[n-1]=='\n')
				n--;

			/*-------Sending filename to worker---------*/
			if (queue_isempty(Qavailable)){
				//If no available worker create a new one

				countWorkers++;
				pid_t pidw;
				int fdw;

				//Give a name to new pipe
				char fifon[20];
				memset(fifon,0,20);
				sprintf(fifon, "./fifos/fifo.%d",countWorkers);   //Fifo name fifo.n where n=1,2,3..

				//Create named pipe
				if ( (mkfifo(fifon, 0777) < 0) && 
					(errno != EEXIST) ) {
					perror("can't create fifo");
				}

				switch(pidw=fork()){
     		   		case -1:
						perror("Worker fork");
        				exit(1);
					case 0:
						/*-----Worker----*/
						worker_pid = getpid();
						while(1){
							//Open fifo for reading
							if ((fdw = open(fifon, O_RDONLY)) < 0){
								perror("Worker: can't open read fifo \n");
							}

							worker(fdw,arg);

							//Stops when done its job
							if((i=kill(worker_pid, SIGSTOP))<0)
       							perror("kill");
						}
						break;
					default:
						/*-------Manager-------*/

						//Push worker to queue
						printf("\n");
						item_t w;
						w.pid = pidw;
						memset(w.fifo,0,sizeof(w.fifo));
						strncpy(w.fifo,fifon,strlen(fifon));

						queue_push(Qcount, w);

						//Open fifo for writing
						if ( (fdw = open(fifon, O_WRONLY)) < 0) {
							perror("Manager1: open fifo");
						}

						//Write filename without \n
						er_write(fdw,filename,n);

						close(fdw);
    			}
			}else{

				//If there is available worker
				int fd;

				//Pop worker-manager pid and fifo name
				item_t wa;
				wa = queue_pop(Qavailable);

				//Wake up worker
				if(kill(wa.pid, SIGCONT)<0)
					perror("kill worker SIGCONT");

				//Open fifo for writing
				if(( fd = open(wa.fifo, O_WRONLY)) < 0)
					perror("Manager2: open fifo");

				//Write filename
				er_write(fd,filename,n);

				close(fd);
			}
	    }
	}
}


void sigchld_handler(int sig){
	pid_t pid;
    int status;
    char *f;

    while((pid=waitpid(-1,&status,WNOHANG|WUNTRACED))>0){
    	if(pid != listener){
			struct Worker wa;
			wa.pid = pid;

			strcpy(wa.fifo,queue_find(Qcount, pid));
			queue_push(Qavailable, wa); //Push to available queue
		}
	}
}

void sigint_handler(int sig){
	struct Worker w;

	//Killing workers
    while(!queue_isempty(Qcount)){
		w = queue_pop(Qcount);

		//Delete fifos
		if(unlink(w.fifo)<0)
			perror("unlink");
		//Kill worker
        if(kill(w.pid,SIGKILL)<0)
        	perror("kill worker");
    }
    free(Qcount);

    //Remove directory of fifos
    if(rmdir("fifos")<0)
    	perror("rmdir");

	//Destroy queue available (free)
	queue_destroy(Qavailable);

	//Kill listener
	if(kill(listener,SIGKILL)<0)
		perror("kill listener");

	//Suiside (manager)
    if(kill(getpid(),SIGKILL)<0)
    	perror("kill manager");
}

/* - er_write()
	 Writes size-bytes to buffer.If writes less than size writes again.
*/
int er_write(int fd,char *buffer,size_t size){
	size_t n;
	while((n=write(fd,buffer,size))<size){
		if(n>0){
			size -=n;
			buffer+=n;
		}else{
			perror("write");
			break;
		}
	}

}


