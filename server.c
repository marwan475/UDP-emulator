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

int s;
struct sockaddr_in caddr; /*senders info*/
socklen_t caddr_size;

char *window[2];
char *msg_to_send[10];

char *msg1 = "hey";
char *msg2 = "how";
char *msg3 = "are";
char *msg4 = "you";
char *msg5 = "doing";
char *msg6 = "today";
char *msg7 = "do";
char *msg8 = "you";
char *msg9 = "need";
char *msg10 = "anything";

char *currmsg;

int curindex;

int curwindowindex;

int flag;

/*vars for getaddrinfo for client*/
struct addrinfo *cinfo;

void slide_window()
{
  window[0] = msg_to_send[curindex];
  window[1] = msg_to_send[curindex+1];
}

void *send_thread()
{
  msg_to_send[0] = msg1;
  msg_to_send[1] = msg2;
  msg_to_send[2] = msg3;
  msg_to_send[3] = msg4;
  msg_to_send[4] = msg5;
  msg_to_send[5] = msg6;
  msg_to_send[6] = msg7;
  msg_to_send[7] = msg8;
  msg_to_send[8] = msg9;
  msg_to_send[9] = msg10;

  window[0] = msg_to_send[0];
  window[1] = msg_to_send[1];

  curindex = 0;

  curwindowindex = 0;

  while(1){

    /*sending*/
    if (flag == 3){
      currmsg = window[curwindowindex];

      sendto(s,currmsg,10,0,cinfo->ai_addr,sizeof((*cinfo->ai_addr)));

      fprintf(stdout,"Frame %d sent: %s\n",curindex+1,currmsg);

      curwindowindex++;

      currmsg = window[curwindowindex];

      sendto(s,currmsg,10,0,cinfo->ai_addr,sizeof((*cinfo->ai_addr)));

      fprintf(stdout,"Frame %d sent: %s\n",curindex+2,currmsg);

      flag = 4;
    }
    
    /*NACK on first sent packet*/
    else if (flag == 1){
      curwindowindex = 0;
      currmsg = window[curwindowindex];

      sendto(s,currmsg,10,0,cinfo->ai_addr,sizeof((*cinfo->ai_addr)));

      fprintf(stdout,"window Frame %d resent\n",curwindowindex);

      flag = 4;
    }
    
    /*NACK on second sent packet*/
    else if (flag == 2){
      curwindowindex = 1;
      currmsg = window[curwindowindex];

      sendto(s,currmsg,10,0,cinfo->ai_addr,sizeof((*cinfo->ai_addr)));

      fprintf(stdout,"window Frame %d resent\n",curwindowindex);

      flag = 4;
    }

    /*ACK recieved for both*/
    else if (flag == 0){
      curindex++;
      curindex++;

      curwindowindex = 0;
      if (curindex < 10){	
        slide_window();    
	fprintf(stdout,"Slidig window\n");    
        flag = 3;
      } else{
        flag = 5;
      }     
    }

    else if (flag == 5){
      fprintf(stdout,"All frames sent\n");	    
      break;
    }
    
  }

  return NULL;
}

int main(int argc,char* argv[])
{
  char* port;
  char* cport;
  int b;
  char* msg;
  
  /*vars for getaddrinfo*/
  int status;
  struct addrinfo hints;
  struct addrinfo *servinfo;

  /*vars for select*/
  int rv,n,z;
  fd_set readfds;
  struct timeval tv;
  int flags;
  int se;

  pthread_t thread;

  if (argc != 3){
    fprintf(stderr,"port and client port argument required");
    exit(1);
  } 

  port = argv[1];
  cport = argv[2];

  /*setting up structs for getaddrinfo*/
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  /*setting up time val structs*/
  tv.tv_sec = 1;
  tv.tv_usec = 500000;

  /*setting up socket for port*/
  status = getaddrinfo(NULL,port,&hints, &servinfo);

  /*checking return of getaddrinfo*/
  if (status != 0){
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  /*client info*/
  status = getaddrinfo(NULL,cport,&hints,&cinfo);

  /*checking return of getaddrinfo*/
  if (status != 0){
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  /*seting up main socket*/
  s = socket(servinfo->ai_family,
		  servinfo->ai_socktype,
		  servinfo->ai_protocol);

  /*checking return of socket*/
  if (s == -1){
    fprintf(stderr,"socket error:");
    exit(1);
  }

  /*bind socket to port*/
  b = bind(s,servinfo->ai_addr,servinfo->ai_addrlen);

  /*checking bind return value*/
  if (b == -1){
    fprintf(stderr,"bind error");
    exit(1);
  }

  flags = fcntl(s, F_GETFL, 0);

  fcntl(s, F_SETFL, flags | O_NONBLOCK);

  se = 0;

  z = 0;

  while(1){

    FD_ZERO(&readfds);
    FD_SET(s, &readfds);
    n = s + 1;
    rv = select(n, &readfds, NULL, NULL, &tv);
    
    if (rv == -1) fprintf(stderr,"select error");
    else if(FD_ISSET(s, &readfds)){
	
      msg = (char*)malloc(sizeof(char)*10);      

      recvfrom(s,msg,
		9,
                 0,
                (struct sockaddr *)&caddr,
		&caddr_size);

      /*if packet is an ACK*/
      if(atoi(msg) == 1){
        flag = 0;
	z = 1;
	fprintf(stdout,"ACK recv for current window state\n");
      }
      
      /*if packet is an NACK 1*/
      else if(atoi(msg) == 2 && se > 0){
        flag = 1;
	fprintf(stdout,"NACK recv for window index 1\n");
      }

      /*if packet is an NACK 2*/
      else if(atoi(msg) == 3 && se > 0){
        flag = 2;
        fprintf(stdout,"NACK recv for window index 2\n"); 
      }
      
      /*ready to recv packet from client*/
      else{
        flag = 3;
	fprintf(stdout,"sending frames to client\n");
	se = 1;
	pthread_create(&thread,NULL,send_thread,NULL);
      }

    }

  }

}
