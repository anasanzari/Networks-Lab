#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define MYPORT 2000
#define BACKLOG 10

int create(struct sockaddr_in *addr){
  addr->sin_family = AF_INET;
  addr->sin_port = htons(MYPORT);
  addr->sin_addr.s_addr = INADDR_ANY;
  memset(&(addr->sin_zero),'\0',8);

}

void main(){

	int sockfd,newfd;
	struct sockaddr_in myaddress;
	struct sockaddr_in clientaddress;
	int sin_size;
	sockfd = socket(PF_INET,SOCK_STREAM,0);
	if(sockfd<0){
		printf("Failed to create socket.\n");
		exit(0);
	}

	create(&myaddress);
	if(bind(sockfd,(struct sockaddr *)&myaddress,sizeof(struct sockaddr))<0){
		printf("Failed to bind.\n");
		exit(0);
	}
	if(listen(sockfd,BACKLOG)<0){
		printf("Failed to listen.\n");
		exit(0);
	}

	sin_size = sizeof(struct sockaddr_in);
	newfd = accept(sockfd,(struct sockaddr*)&clientaddress,&sin_size);
	if(newfd<0){
		printf("Failed to Accept.\n");
		exit(0);
	}

	int n,val,i,j;
	int array[100];
	recv(newfd,&n,sizeof(int), 0);


	for(i=0;i<n;i++){
		recv(newfd,&val,sizeof(int), 0);
		array[i] = val;
	}

	/* bubble sort */
	for(i=0;i<n;i++){
	 for(j=0;j<n-i-1;j++){
	  if(array[j]>array[j+1]){
	    int t = array[j];
	    array[j] = array[j+1];
	    array[j+1] = t;
	  }
	 }
	}


	for(i=0;i<n;i++){
	 val = array[i];
	 send(newfd,&val,sizeof(int), 0);

	}



}
