#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define DEST_PORT 2000

char junk[256];

int getInt(char* msg){

  int n,error;
  do{
    error = scanf("%d",&n);
    if(error==0){
      printf("%s",msg);
      scanf("%s",junk);
    }
  }while(error==0);
  return n;

}

int create(struct sockaddr_in *addr){
  addr->sin_family = AF_INET;
  addr->sin_port = htons(DEST_PORT);
  addr->sin_addr.s_addr = INADDR_ANY;
  memset(&(addr->sin_zero),'\0',8);

}

void main(int argc, char const *argv[]){

  int sockfd;
  struct sockaddr_in destination;

  sockfd = socket(PF_INET, SOCK_STREAM,0);
  if(sockfd<0){
    printf("Failed to create socket.\n");
    exit(0);
  }

  create(&destination);
  int x = connect(sockfd,(struct sockaddr *)&destination,sizeof(struct sockaddr));
  if(x<0){
    printf("Failed to connect.\n");
    exit(0);
  }


  int n,val,i;
  printf("Enter the no of integers: ");
  while((n=getInt("Please Enter a positive integer.\n"))<=0);


  send(sockfd, &n,sizeof(int), 0);
  printf("Enter the integers: ");

  for(i=0;i<n;i++){
    val=getInt("Please enter a valid integer: ");
    send(sockfd, &val,sizeof(int), 0);
  }

  printf("Sorted Order: ");
  for(i=0;i<n;i++){
    recv(sockfd, &val,sizeof(int), 0);
    printf("%d ",val);
  }
  printf("\n");


}
