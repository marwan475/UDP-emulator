/*Marwan Mostafa mam024 11305332*/

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <list.h>
#include <sys/select.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

int p; /*probability to be dropped out of 100*/
int d; /*delay in seconds*/
int q; /*max quee size*/
char* A; /*port A*/
char* B; /*port B*/
char* C; /*port C*/
int a; /*main socket for port A*/
int b; /*main socket for port B*/
int c; /*main socket for port C*/

LIST* QtC; /*Que of packets that go to C*/
LIST* QtB; /*Que of packets that go to B*/

struct addrinfo *binfo;
struct addrinfo *cinfo;

void *send_thread()
{
  int turn; /*turn of which que to send*/
  char* msgts; /*msg to send*/

  turn = 0; /*0 for QtB 1 for QtC*/

  while(1){
    sleep(d);
    if (turn == 0 && ListCount(QtB) > 0 ){
      ListFirst(QtB);	    
      msgts = ListRemove(QtB);
      fprintf(stdout,"Ready to send Packet to B\n");
      sendto(a,msgts,10,0,binfo->ai_addr,sizeof(*(binfo->ai_addr)));
      free(msgts);      
      fprintf(stdout,"Packet sent to B\n");
    }

    turn = 1;

    if (turn == 1 && ListCount(QtC) > 0){
      ListFirst(QtC);
      msgts = ListRemove(QtC);
      fprintf(stdout,"Ready to send Packet to C\n");
      sendto(a,msgts,10,0,cinfo->ai_addr,sizeof(*(cinfo->ai_addr)));
      free(msgts);	    
      fprintf(stdout,"Packet sent to C\n");
    }

    turn = 0;
  }
}

int main(int argc,char*argv[])
{
  int bi; /*return value of bind*/
  int port; /* port of sender*/
  int r; /*random number used in prob calculations*/
  int br; /*bytes recv*/

  char *msg; /*recv msg*/
  struct sockaddr_in caddr; /*senders info*/
  socklen_t caddr_size;

  /*vars for getaddrinfo*/
  int status;
  struct addrinfo hints;
  struct addrinfo *servinfo;

  /*vars for select*/
  int rv,n;
  fd_set readfds;
  struct timeval tv;
  int flags;

  pthread_t thread;

  /*checking arg count*/
  if (argc < 7){
    fprintf(stderr,"need 6 arguments");
    exit(1);    
  }

  /*assaigning args to vars*/
  p = atoi(argv[1]);
  d = atoi(argv[2]);
  q = atoi(argv[3]);
  A = argv[4];
  B = argv[5];
  C = argv[6];

  fprintf(stdout,"server initalizing\n");

  /*initalizing list for ques*/
  QtC = ListCreate();
  QtB = ListCreate();

  /*setting up time val structs*/
  tv.tv_sec = 1;
  tv.tv_usec = 500000;

  /*setting up structs for getaddrinfo*/
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;     
  hints.ai_socktype = SOCK_DGRAM; 
  hints.ai_flags = AI_PASSIVE;

  /*setting up socket for port A*/
  status = getaddrinfo(NULL,A,&hints, &servinfo);

  /*checking return of getaddrinfo*/
  if (status != 0){
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  /*seting up main socket*/
  a = socket(servinfo->ai_family,
		  servinfo->ai_socktype,
		  servinfo->ai_protocol);

  /*checking return of socket*/
  if (a == -1){
    fprintf(stderr,"socket error:");
    exit(1);
  }

  /*bind socket to port*/
  bi = bind(a,servinfo->ai_addr,servinfo->ai_addrlen);

  /*checking bind return value*/
  if (bi == -1){
    fprintf(stderr,"bind error");
    exit(1);
  }

  /*get adddres info for Port B*/
  status = getaddrinfo(NULL,B,&hints, &binfo);

  /*checking return of getaddrinfo*/
  if (status != 0){
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  /*get adddres info for Port B*/
  status = getaddrinfo(NULL,C,&hints, &cinfo);

  /*checking return of getaddrinfo*/
  if (status != 0){
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  flags = fcntl(a, F_GETFL, 0);

  fcntl(a, F_SETFL, flags | O_NONBLOCK);

  pthread_create(&thread,NULL,send_thread,NULL);

  fprintf(stdout,"sender thread created \n");

  while (1){

    FD_ZERO(&readfds);
    FD_SET(a, &readfds);
    n = a + 1;
    rv = select(n, &readfds, NULL, NULL, &tv);
	  
    if (rv == -1) fprintf(stderr,"select error"); 
    else if(FD_ISSET(a, &readfds)){    
    /*buffer for recieving msgs*/
    msg = (char*)malloc(sizeof(char)*10);	  

    br = recvfrom(a,msg,
		9,
                 0,
                (struct sockaddr *)&caddr,
		&caddr_size);

    if (br < 0)fprintf(stderr,"recv error");

    /*extracting source port*/
    port = ntohs(caddr.sin_port);

    fprintf(stdout,"Port A recieved a packet from Port: %d \n",port);

    r = (rand()%(100 - 1 +1)+1);

    /*prob to drop packet*/
    if (r > p){
    
      /*packet from B*/	    
      if (port == atoi(B)){
	/*prob to send packet to C*/      
        r = (rand()%(100 - 1 +1)+1);
	if (r < (100 - p)){
	  if (ListCount(QtC) < q){
	    ListAppend(QtC,msg);
	    fprintf(stdout,"Packet added to Que to C\n");
	  }else fprintf(stdout,"Que to C is full, Packet Droped\n");
	}else{
	  fprintf(stdout,"Packet at droped before added to Que to C\n");
	  free(msg);
	}
      }

      /*packet from C*/
      else if (port == atoi(C)){
        /*prob to send packet to B*/
        r = (rand()%(100 - 1 +1)+1);
        if (r < (100 - p)){
          if (ListCount(QtB) < q){
	    ListAppend(QtB,msg);
	    fprintf(stdout,"Packet added to Que to B\n");
	  }
	  else fprintf(stdout,"Que to C is full, Packet Droped\n");
        }else{
	  fprintf(stdout,"Packet at droped before added to Que to B\n");
	  free(msg);
	}
      } 

      else{
	fprintf(stdout,"Packet at A was not form Known source\n");      
        free(msg);
      }
    }else {
      fprintf(stdout,"Packet at A droped\n");	    
      free(msg);
    }

    }
  }  
  /*freeing alocated structs*/
  freeaddrinfo(servinfo);

}
