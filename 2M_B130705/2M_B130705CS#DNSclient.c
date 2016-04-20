#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>



/******  DNS Msg structs and functions *****/
//query types
#define Q_A 1
#define Q_CNAME 2
#define Q_NS 3
#define Q_MX 4

//msg types
#define MSG_QUERY 0
#define MSG_REPLY 1
#define MSG_ERROR 2

struct query{
  int type;
  char name[25];
};

struct resource{
  char name[25];
  char value[20];
  int type;
  int ttl;

};

struct msg{
  int msg_id;
  int msg_type; //query(o) or reply(1) or error(2)
  int nof_q; //number of questions
  int nof_a; //number of answers

  struct query queries[4]; //list of queries
  struct resource answers[4]; //list of answers

};


char* getType(int type){
  switch(type){
    case Q_A: return "A Record";break;
    case Q_NS: return "NS Record";break;
    case Q_CNAME: return "CNAME Record";break;
    case Q_MX: return "MX Record";break;
  }
  return "";
}

struct msg createmsg(int id,int type,int q,int a){
  struct msg m;
  m.msg_id = id;
  m.msg_type = type;
  m.nof_q = q;
  m.nof_a = a;
  return m;
};

struct query createquery(int type,char name[25]){
  struct query q;
  q.type = type;
  strcpy(q.name,name);
  return q;
};

struct resource createresource(char name[25],char value[20],int type,int ttl){
  struct resource r;
  strcpy(r.name,name);
  strcpy(r.value,value);
  r.type = type;
  r.ttl = ttl;
  return r;
}

int createsocket(int d,int p,int flag){
  int sockfd = socket(PF_INET,SOCK_DGRAM,0);
	if(sockfd<0){
		printf("Failed to create socket.\n");
		exit(0);
	}
  return sockfd;
}

void bindsocket(int sockfd,struct sockaddr_in *myaddress){
  if(bind(sockfd,(struct sockaddr *)myaddress,sizeof(struct sockaddr))<0){
		printf("Failed to bind.\n");
		exit(0);
	}
}

int sendquery(int sockfd, struct msg m,int type,int nof_q, int nof_a,struct sockaddr_in destination){
  m.msg_type = type;
  m.nof_a = nof_a;
  m.nof_q = nof_q;
  int l = sendto(sockfd, (struct msg*) &m, sizeof(m) ,0,
                  (struct sockaddr *)&destination,
                  sizeof(destination));
  if(l<0){
        perror("error");
  }
  return l;
}

int recievequery(int sockfd, struct msg *m,struct sockaddr_in *addr){
  int len = sizeof(addr);
  int l = recvfrom(sockfd, m, sizeof(struct msg), 0,(struct sockaddr *)addr,&len);
  if(l<0){
        perror("error");
  }
  return l;
}

int create(struct sockaddr_in *addr,int port,char* ip){
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  addr->sin_addr.s_addr = inet_addr(ip);
  memset(&(addr->sin_zero),'\0',8);

}


void printRecords(int n, struct resource r[4]){
  int i;
  printf("Resources found : \n");
  for (i = 0; i < n; i++) {
    printf("(%s,%s,%d,%d)\n", r[i].name,r[i].value,r[i].type,r[i].ttl);
  }

}

int search(struct resource *records,int rlen,struct resource *found, char* name){
  int i,j=0;
  int foundlen = 0;
  for (i = 0; i < rlen; i++) {
    if(strcmp(records[i].name,name)==0){
      found[foundlen] = records[i];
      foundlen++;
      if(foundlen>=4) break;
    }
  }
  return foundlen;
}


/***** end of Dns helpers  ***/




#define DEST_PORT 2001
#define IP "127.0.0.3" //local dns ip

void displayRecords(int n, struct resource r[4]){
  int i;
  for (i = 0; i < n; i++) {
    printf("%s : (%s,%s,%d,%d)\n",getType(r[i].type), r[i].name,r[i].value,r[i].type,r[i].ttl);
  }

}


void main(){

  int sockfd;
  struct sockaddr_in destination;

  sockfd = createsocket(PF_INET,SOCK_DGRAM,0);

  create(&destination,DEST_PORT,IP);

  char domain[50];
  printf("Enter the domain: ");
  scanf("%s", domain);

  struct msg m = createmsg(1,MSG_QUERY,1,0);
  m.queries[0] = createquery(Q_A,domain);

  int l = sendquery(sockfd,m,MSG_QUERY,1,0,destination);

  recievequery(sockfd,&m,&destination);
  if(m.msg_type==MSG_REPLY){
    printf("\nResults:\n");
    displayRecords(m.nof_a,m.answers);
  }else{
    printf("\nSorry. No records found.\n");
  }



}
