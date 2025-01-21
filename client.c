/*Marwan Mostafa mam024 11305332*/

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

struct sockaddr_in caddr; /*senders info*/
socklen_t caddr_size;

int curindex; /*current recv frame aka sequence number*/

char *port; /*port to listen on*/
char *servport; /*server port*/
int s; /*main socket*/

char *RTR = "RTR";
char *ACK = "1";
char *NACK1 = "2";
char *NACK2 = "3";

char frames[10][10];
char RACK[2];

char *window[2];

int curwindowindex;

int main(int argc,char* argv[])
{

  /*vars for getaddrinfo*/
  int status,flags;
  struct addrinfo hints;
  struct addrinfo *myinfo;
  int delay;

  /*vars for getaddrinfo for server*/
  struct addrinfo *servinfo;

  int b;
  int r;
  struct timeval t;
  
  if (argc != 4){
    fprintf(stderr,"port, server port, and delay required");
    exit(1);
  }

  port = argv[1];
  servport = argv[2];
  delay = atoi(argv[3]);

  /*setting up structs for getaddrinfo*/
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  status = getaddrinfo(NULL,port,&hints, &myinfo);

  /*checking return of getaddrinfo*/
  if (status != 0){
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  status = getaddrinfo(NULL,servport,&hints, &servinfo);

  /*checking return of getaddrinfo*/
  if (status != 0){
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  /*seting up main socket*/
  s = socket(myinfo->ai_family,
		  myinfo->ai_socktype,
		  myinfo->ai_protocol);

  /*checking return of socket*/
  if (s == -1){
    fprintf(stderr,"socket error:");
    exit(1);
  }

  /*bind socket to port*/
  b = bind(s,myinfo->ai_addr,myinfo->ai_addrlen);

  /*checking bind return value*/
  if (b == -1){
    fprintf(stderr,"bind error");
    exit(1);
  }

  /*setting up time val structs*/
  t.tv_sec = 5;
  t.tv_usec = 500000;
  setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,&t,sizeof(t));

  /*setting up time val structs*/
  curindex = 0;

  curwindowindex = 0;

  flags = fcntl(s, F_GETFL, 0);

  fcntl(s, F_SETFL, flags | O_NONBLOCK);

  while (1){

    sleep(4);	  
    if(1){
	  
      r = recvfrom(s,frames[curindex],
     		  9,
                   0,
                  (struct sockaddr *)&caddr,
		  &caddr_size);

      if ( r < 0 ){
        if (curwindowindex == 0){
	  sendto(s,NACK1,2,0,servinfo->ai_addr,sizeof(*(servinfo->ai_addr)));
	  fprintf(stdout,"Frame %d not recv send NACK for window index 1\n",curindex);
	  sleep(delay);
	}

	else if(curwindowindex == 1){
	  sendto(s,NACK2,2,0,servinfo->ai_addr,sizeof(*(servinfo->ai_addr)));
	  fprintf(stdout,"Frame %d not recv send NACK for window index 2\n",curindex);
	  sleep(delay);
	}
	else if(curwindowindex == 3){
	  sendto(s,ACK,2,0,servinfo->ai_addr,sizeof(*(servinfo->ai_addr)));
          fprintf(stdout,"resending ACK\n");
	  sleep(delay);
	}
      }

      else{
	
	if (curwindowindex == 3) curwindowindex = 0;      
	      
        window[curwindowindex] = frames[curindex];

        fprintf(stdout,"Frame recv: %s\n",frames[curindex]);
        curindex++;

        if (curwindowindex == 1){
          curwindowindex = 3;
	  sendto(s,ACK,2,0,servinfo->ai_addr,sizeof(*(servinfo->ai_addr)));
	  fprintf(stdout,"Both Frames recv sending ACK\n");

        }else curwindowindex++;
      }

      if (curindex == 10) break;

    }
  }

  fprintf(stdout,"All Frames recieved exiting");

  return 0;

}
