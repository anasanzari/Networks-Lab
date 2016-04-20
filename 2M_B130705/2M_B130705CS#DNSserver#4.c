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




#define MYPORT 2001
#define IP "127.0.0.3"

#define ROOTPORT 2002
#define ROOTIP "127.0.0.4"



struct msg doRecursion(struct msg m,struct sockaddr_in destination){

	if(m.msg_type==MSG_REPLY){
		printRecords(m.nof_a,m.answers);

		if(m.answers[0].type==Q_NS){

			printf("\nForwarding to Next Server...\n");
			int fd = createsocket(PF_INET,SOCK_DGRAM,0);
			create(&destination,m.answers[1].ttl,m.answers[1].value); //using ttl value as port number just for convenience
			sendquery(fd,m,MSG_QUERY,1,0,destination);

			recievequery(fd,&m,&destination);
			printf("\nResponse:\n");
			return doRecursion(m,destination);

		}else{
			//not an NS. Probably we have the answer. send back to client
			printf("\n\nServing Records back to client.\n");
			return m;

		}

	}else if(m.msg_type==MSG_ERROR){
		//error. send back to client
		printf("Server responded with Error. No Records found.\n\n");
		return m;

	}else{
		//not a reply. set error flag and send back to client.
		printf("Server sent back a question!. Error.\n\n");
		m.msg_type = MSG_ERROR;
		return m;

	}

}



void main(){

	int sockfd,newfd;
	struct sockaddr_in myaddress,destination,clientaddress;
	int sin_size;

  sockfd = createsocket(PF_INET,SOCK_DGRAM,0);


	create(&myaddress,MYPORT,IP);

  bindsocket(sockfd,&myaddress);

	struct resource cache[50];
  int cachelen = 0;


  struct msg m;

  while(1){

		printf("\n\nWaiting for query.\n\n");

	  recievequery(sockfd,&m,&clientaddress);

	  if(m.msg_type==MSG_QUERY){

	    printf("Query recieved.\n");

			//looking in th cache
			struct resource s[4];
			int k = search(cache,cachelen,s,m.queries[0].name);

			if(k>0){

				printf("\n Found in the cache.\n");
				m.answers[0] = s[0];

				if(s[0].type!=Q_A){

					struct resource a[4];
					int l = search(cache,cachelen,a,s[0].value);
					m.answers[1] = a[0];
					sendquery(sockfd,m,MSG_REPLY,0,2,clientaddress);
				}else{
					sendquery(sockfd,m,MSG_REPLY,0,1,clientaddress);
				}

			}else{
				int rootfd = createsocket(PF_INET, SOCK_DGRAM,0);
		    create(&destination,ROOTPORT,ROOTIP);

				printf("\n\nForwarding to Root...\n");
		    sendquery(rootfd,m,MSG_QUERY,m.nof_q,m.nof_a,destination);


		    recievequery(rootfd,&m,&destination);
		    printf("\nRoot Response:\n");
				m = doRecursion(m,destination);
				int i=0;
				for (i = 0; i < m.nof_a; i++) {
					cache[cachelen++] = m.answers[i];
				}
				sendquery(sockfd,m,m.msg_type,m.nof_q,m.nof_a,clientaddress);
			}


	  }

	}



}
