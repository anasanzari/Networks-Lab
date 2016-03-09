#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>



#define MYPORT 2006
#define BACKLOG 10
#define IP "127.0.0.3"

#define DEST_PORT 2006



/* Connection initiation structures */

//Connection initiation 

#define LIST 1
#define SEND 2
#define AUTH 3
#define QUIT 4
#define MAILSERVER 5

//SMTP Protocol

#define HELLO 1
#define FROM 2
#define TO 3
#define SUBJECT 4
#define BODY 5
#define OKAY 6
#define END 7
#define BODYEND 8


struct smtp_msg{
  int command;
  char text[256];
};

struct smtp_msg createSmtpMsg(int c,char t[256]){
  struct smtp_msg m;
  m.command = c;
  strcpy(m.text,t);
  return m;
}

void sendSmtp(int sockfd,struct smtp_msg m){
  send(sockfd, &m,sizeof(struct smtp_msg), 0);
}

struct smtp_msg recieveSmtp(int sockfd){
  struct smtp_msg m;
  recv(sockfd,&m,sizeof(m), 0);
  return m;
}





struct msg{
  int type;
};

void start(int sockfd,int type){
  struct msg m;
  m.type = type;
  send(sockfd, &m,sizeof(struct msg), 0);
}


//Users

struct user{
  char username[50];
  char password[50];
};

struct user createUser(char username[50], char password[50]){
  struct user m;
  strcpy(m.username,username);
  strcpy(m.password,password);
  return m;
}

int authenticate(struct user users[100],int len,char username[50], char password[50]){
	
  int i =0;
  for(i=0;i<len;i++){
	if(strcmp(users[i].username,username)==0&&strcmp(users[i].password,password)==0){
		return 1;
	}
  }

  return 0;
}

void sendUser(int sockfd,struct user m){
  send(sockfd, &m,sizeof(struct user), 0);
}

struct user recieveUser(int sockfd){
  struct user m;
  recv(sockfd,&m,sizeof(m), 0);
  return m;
}


//SMTP

struct mail{
  char from[50];
  char to[50];
  char subject[100];
  char body[500];
};

#define SUCCESS 1
#define QUEUE 3
#define ERROR 2

struct reply{
  int status;
};

struct mail createMail(char from[50],char to[50],char subject[50],char body[500]){
  struct mail m;
  strcpy(m.from,from);
  strcpy(m.to,to);
  strcpy(m.subject,subject);
  strcpy(m.body,body);
  return m;
}

void printMail(struct mail m){
  printf("\n\nFrom : %s\nTo : %s\nSubject : %s\nBody: %s\n\n",m.from,m.to,m.subject,m.body );
}

void printAll(struct mail m[100],int len){
  int i;
  for(i=0;i<=len;i++){
    printMail(m[i]);
  }
}

void sendMail(int sockfd,struct mail m){
  send(sockfd, &m,sizeof(struct mail), 0);
}

struct mail recieveMail(int sockfd){
  struct mail m;
  recv(sockfd,&m,sizeof(m), 0);
  return m;
}

void sendReply(int sockfd,int status){
  struct reply m;
  m.status = status;
  send(sockfd, &m,sizeof(struct reply), 0);
}

struct reply recieveReply(int sockfd){
  struct reply m;
  recv(sockfd,&m,sizeof(m), 0);
  return m;
}




void err(char *msg){
  perror(msg);
  exit(0);
}

int createsocket(){
  int sockfd = socket(PF_INET,SOCK_STREAM,0);
	if(sockfd<0){
		err("Failed to create socket.\n");
	}
  return sockfd;
}

void bindsocket(int sockfd,struct sockaddr_in *myaddress){
  if(bind(sockfd,(struct sockaddr *)myaddress,sizeof(struct sockaddr))<0){
		err("Failed to bind.\n");
	}
}

void start_listen(int sockfd,int log){
  if(listen(sockfd,log)<0){
		err("Failed to listen.\n");
	}
}

int acceptClient(int sockfd,struct sockaddr_in *addr){
  int sin_size = sizeof(struct sockaddr_in);
  int newfd = accept(sockfd,(struct sockaddr *)addr,&sin_size);
  if (newfd < 0){
    err("Coudn't Accept");
  }
  return newfd;
}

int create(struct sockaddr_in *addr,int port,char* ip){
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  addr->sin_addr.s_addr = inet_addr(ip);
  memset(&(addr->sin_zero),'\0',8);

}

int fork_process(){
  int pid = fork();
  if (pid < 0){
    err("Couldn't Fork.");
  }
  return pid;

}

int conn(int sockfd,struct sockaddr_in *addr){
  int x = connect(sockfd,(struct sockaddr *)addr,sizeof(struct sockaddr));
  if(x<0){
    printf("Failed to connect.\n");
  }
  return x;

}




/* end */


struct mail newmail;

struct user authuser;

struct user users[100];
int user_len = 0;

FILE *p;
FILE *inbox;



void hello(int fd){
struct smtp_msg m;
m = createSmtpMsg(HELLO,"Hi there.");
sendSmtp(fd,m);
}

void sendOkay(int fd){
struct smtp_msg m;
m = createSmtpMsg(OKAY,"OK.");
sendSmtp(fd,m);
}

void from(int fd,struct smtp_msg sm){
strcpy(newmail.from,sm.text);
sendOkay(fd);
}

void to(int fd,struct smtp_msg sm){
strcpy(newmail.to,sm.text);
sendOkay(fd);
}

void subject(int fd,struct smtp_msg sm){
strcpy(newmail.subject,sm.text);
sendOkay(fd);
}

void body(int fd,struct smtp_msg sm){
strcpy(newmail.body,sm.text);
sendOkay(fd);
}

void bodyend(int fd){
//end of message
//save it in inbox

inbox = fopen("inbox.txt", "a");
fprintf(inbox,"%s %s %s %s\n", newmail.from, newmail.to, newmail.subject, newmail.body);
fclose(inbox);

sendOkay(fd);

}

void handleSMTPMsg(int fd,struct smtp_msg sm){
switch(sm.command){
   case HELLO: printf("\nHELLO: %s",sm.text); hello(fd);break;
   case FROM: printf("\nFROM: %s",sm.text);from(fd,sm);break;
   case TO: printf("\nTO: %s",sm.text);to(fd,sm);break;
   case SUBJECT: printf("\nSUBJECT: %s",sm.text);subject(fd,sm);break;
   case BODY: printf("\nBODY: %s",sm.text);body(fd,sm);break;
   case BODYEND: printf("\nBODYEND.");bodyend(fd);break;
   case OKAY: printf("\nOKAY: %s",sm.text);break;
   case END: printf("\nEND.\n");sendOkay(fd);break;
}

}

void sendAndRecieve(int sockfd,int command,char t[256]){ 
  //send smtp and receive okay
  struct smtp_msg sm;
  sm.command = command;
  strcpy(sm.text,t);
  sendSmtp(sockfd,sm);
  
  sm = recieveSmtp(sockfd);
  if(sm.command==OKAY){
  	handleSMTPMsg(sockfd,sm);
  }else if(sm.command==HELLO){
	printf("\nHELLO: %s",sm.text);
  }
}


/* User Agent */

int auth(int fd){
  
  authuser = recieveUser(fd);

  printf("\nUser: %s , Pass: %s\n",authuser.username,authuser.password);
  if(authenticate(users,user_len,authuser.username, authuser.password)==1){
	sendReply(fd,SUCCESS);
        return SUCCESS;
  }else{
	sendReply(fd,ERROR);
	return ERROR;
  }
  
}

void listMsg(int fd){
printf("Query for LIST \n");

printf("User: %s",authuser.username);

inbox = fopen("inbox.txt", "a");
struct mail m;
do{

  fscanf(inbox,"%s %s %s %s\n", m.from, m.to, m.subject, m.body);
  if(strcmp(authuser.username,m.to)==0){
	printf("%s %s %s %s\n",  m.from, m.to, m.subject, m.body);
  }
}
while( !feof(inbox) );

fclose(inbox);


}



void sendMsg(int fd){

printf("Query for SEND \n");

struct mail m = recieveMail(fd);


printMail(m);


//connect to reciever's mail server.
  int sockfd;
  struct sockaddr_in destination;

  sockfd = createsocket();
  create(&destination,DEST_PORT,IP);
  if(conn(sockfd,&destination)<0){
	//failed to connect. Put the message in queue and retry later.
	printf("\n Message is added to Queue\n");
	p = fopen("queue.txt", "a");
	fprintf(p,"%s %s %s %s\n", m.from, m.to, m.subject, m.body);
	fclose(p);
	sendReply(fd,QUEUE);
  }else{

	  struct reply r; // struct hold replies from server
	
	  start(sockfd,MAILSERVER); //notify smtp

	  struct smtp_msg sm;

	  sendAndRecieve(sockfd,HELLO,"Hey there.");
	  sendAndRecieve(sockfd,FROM,m.from);
	  sendAndRecieve(sockfd,TO,m.to);
	  sendAndRecieve(sockfd,SUBJECT,m.subject);
	  sendAndRecieve(sockfd,BODY,m.body);
	  sendAndRecieve(sockfd,BODYEND,"");
	  sendAndRecieve(sockfd,END,"");
	  sendReply(fd,SUCCESS);
	
  }

  

}


void serve(int fd){
  //called by child
  struct msg m;
  recv(fd,&m,sizeof(m), 0);
  if(m.type==AUTH){
  
   if(auth(fd)==SUCCESS){

    do{
       recv(fd,&m,sizeof(m), 0); // Recieve subsequent commands

	  switch(m.type){
		 case LIST: listMsg(fd);break;
		 case SEND: sendMsg(fd);break;
	  }
    }while(m.type!=QUIT);  
   }

  }else if(m.type==MAILSERVER){
	//Mail Server Communication. SMTP Protocol

	//recieve 
	
	struct smtp_msg sm;
	do{
	  sm = recieveSmtp(fd);
          handleSMTPMsg(fd,sm);
	}while(sm.command!=END);

  }else{
       //shouldn't happen
  }




  close(fd);
  exit(0);

}



void main(){

  int sockfd,newfd,sin_size;
  struct sockaddr_in clientaddress,myaddress;



  //users
  users[user_len++] = createUser("a@gmail.com","1234");
  users[user_len++] = createUser("b@gmail.com","1234");
  users[user_len++] = createUser("c@gmail.com","1234");
  users[user_len++] = createUser("d@gmail.com","1234");

  sockfd = createsocket();
  create(&myaddress,MYPORT,IP);
  bindsocket(sockfd,&myaddress);
  start_listen(sockfd,BACKLOG);

  signal(SIGCHLD, SIG_IGN);
  int pid;

  while(1){

    sin_size = sizeof(struct sockaddr_in);
    newfd = acceptClient(sockfd,&clientaddress);

    pid = fork_process();
    if(pid == 0){
      close(sockfd);
      printf("Child Process : %d", getpid());
      serve(newfd);
    }else{
      close(newfd);
    }
  }
  close(sockfd);

}
