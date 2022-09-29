/*
	Name: Maria Miliou
	A.M.:1115201300101
*/

#include<stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#define BUFSIZE 255

struct location{
	char name[1024];	//Location
   	unsigned int count; //Number of appearances
};

//Extra functions
void rewrite_out(pid_t, char*);
int loc_write(int, char*);


/* - worker()
	Reads from pipe with manager <filename>.
	Creates new file with name <filename>.out - removes file extension if exists e.g 10.txt-->10.out
	Reads size-of-file bytes to buffer and take tokens with delimiter [space].
	Checks if begin with "http://" and calls loc_write() for writing, not considering "www." if exists.
*/
int worker(int readfd, char *path){

	int fd,fout;
	char rfile[BUFSIZE];
	char pathfile[BUFSIZE];
	char *buffer, *ptr, *prefix;
    int n;
	struct stat statbuf;

	memset(rfile,0,BUFSIZE);

    //Read the filename that manager send through named pipe
	if((n=read(readfd,rfile,BUFSIZE))<0){
		perror("Worker: read error");
	}
	close(readfd);

	//Create path to file whose name was sent from fifo
	sprintf(pathfile,"%s%c%s",path,'/',rfile);

	//Open file
	if((fd = open(pathfile, O_RDONLY))<0){
    	perror("open");
	}

	//Size of buffer for reading equal to size of file
	if(fstat(fd, &statbuf))
		perror("fstat");

	int size=statbuf.st_size;
	buffer = malloc(size);
	if(buffer == NULL)
		perror("worker malloc");

	//Name for <filename>.out
    char fileOUT[BUFSIZE];
    prefix=strtok_r(rfile,".",&ptr);	//Not consider file extension
    sprintf(fileOUT, "%s%s%s","./out/",prefix,".out");

	//Create <filename>.out
	if ((fout = open(fileOUT, O_RDWR | O_CREAT | O_TRUNC,0777)) < 0){
		perror("Worker:  can't open read fifo \n");
	}

	char *token, *TLB, *tok, *ptr1, *ptr2, *sub;
	bool flag;

	while((n=read(fd, buffer,size))>0){

        //Breaking input into series of tokens separated by space
        token=strtok_r(buffer," ",&ptr1);

		while((token=strtok_r(NULL," ",&ptr1)) != NULL){
			if(memcmp(token,"http://",7)==0){ //If begin with http://
                tok = token;

                //Call strtok_r now for token with delim /(twice)
				sub=strtok_r(tok,"/",&ptr2);
            	sub=strtok_r(NULL,"/", &ptr2);

				//Checks for "www."
				//No consider if sub begin with www.
				//Call loc_write() for writing
				if(memcmp(sub,"www.",4)==0){
					loc_write(fout,sub+4);

				}else{
					loc_write(fout,sub);
				}
					if(write(fout,"\n", 1)<0)
						perror("worker write");
			}
		}
	}

	if(n<0)
		perror("worker read");

    close(fd);
    close(fout);

	//Call rewrite_out()
	rewrite_out(getpid(), fileOUT); //Create a user-friendly file

	free(buffer);

	printf("Output file %s just created!\n",fileOUT);
}


/* - loc_write(int,char*)
 	 Writes to output file entries --> struct location
	 If location.name is already written just increase its counter
	 Otherwise add location to end

*/
int loc_write(int fout, char sub[]){
	size_t n;
	struct location old, new;
	off_t offset;

	offset=lseek(fout, 0, SEEK_SET); //Offset set to begining of file

	//Reading line by line until end
	while((n=read(fout,&old, sizeof(struct location)))!=0){
		if (memcmp(old.name,sub,strlen(old.name))==0){	//If old location name is equal to sub

			memset(new.name,0,1024);
			strcpy(new.name,old.name); //Location's name remain same
			new.count=old.count+1; //Increase counter

			lseek(fout,offset,SEEK_SET); //Go to begining of this entry
			if(write(fout, &new, sizeof(struct location))<0) //Write updated entry(new) with counter + 1
				perror("loc_write write");
			break;
		}
		memset(old.name,0,1024);
		lseek(fout,1,SEEK_CUR); //Skip \n
		offset +=1029;	//Offset increased by size of struct location + 1 for \n
	}
	if(n==0){//New location

		memset(new.name,0,1024);
		strncpy(new.name,sub, strlen(sub));
		new.count = 1;

		lseek(fout,0,SEEK_END);//Write to end of file
		if(write(fout,&new, sizeof(struct location))<0)
			perror("loc_write write");

	}else if(n==-1)
		perror("loc_write read");
	return 0;
}

/* - rewrite_out()
	Creates a new file with name pid (so not be confused by other workers)
	Reads file.out which contain entries struct location
	Writes to file pid "location num_of_appearances\n" so it can be readable from user
	Deletes file.out
	Rename pid(file) to file.out
*/

void rewrite_out(pid_t pid, char filename[]){

   	int fr, fn, n;
	char name[20];
    char str[1024];
   	struct location m;

	//Open file.out that worker created with entries struct location
	if((fr = open(filename,O_RDONLY))<0)
		perror("open");

	//Create new file with name pid in directory out
	sprintf(name,"%s%d","./out/",pid);
	if ((fn = open(name, O_RDWR | O_CREAT | O_TRUNC,0777)) < 0)
		perror("open");

 	while((n=read(fr, &m,sizeof(struct location)))>0){

		lseek(fr,1,SEEK_CUR);
		char temp[512], count[4];

		//To ensure there is no trash
		memset(temp,0,sizeof(temp));
		memset(str,0,sizeof(str));
		memset(count,0,sizeof(count));

		strncpy(temp,m.name,strlen(m.name));  //Keep only strlen characters from location's name not \0

		sprintf(str,"%s%c%d%c",temp,' ',m.count,'\n'); //location[space]num_of_ap\n
		sprintf(count,"%d",m.count);	//To check strlen of num_of_ap

		if(write(fn, str, strlen(temp) + strlen(count)+2)<0)
			perror("rewrite_out write");

	}

	if(n<0)
		perror("rewrite_out read");

	close(fr);

	//Delete file.out
	unlink(filename);

	close(fn);
	//Rename
	rename(name,filename);
}
