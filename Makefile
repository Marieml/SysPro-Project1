#Name: Maria Miliou
#AM: 1115201300101

#-----Makefile: type make to compile----#

all: sniffer

sniffer: manager.c worker.c queue.c
	gcc manager.c worker.c queue.c -o sniffer

clean:
	rm -rf sniffer out
