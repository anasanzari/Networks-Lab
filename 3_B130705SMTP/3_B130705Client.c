#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


#define DEST_PORT 2006
#define IP "127.0.0.3"




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


struct user u; 


void displayMenu(){
	
  printf("\n\n 1. List Messages");
  printf("\n 2. Send Message");
  printf("\n 3. Quit");
  printf("\n Enter your choice: ");
}

void list(int fd){
start(fd,LIST);

}

void sendMessage(int fd){
 start(fd,SEND);

 struct mail m;
 strcpy(m.from,u.username);
 
 printf("\nEnter Receiver's id: ");
 scanf("%s",m.to);
 printf("\n\nEnter Subject : ");
 scanf("%s",m.subject);
 printf("\n\nEnter Body: ");
 scanf("%s",m.body);
 
 sendMail(fd,m);

 struct reply r;
 r = recieveReply(fd);
 if(r.status == SUCCESS){
   printf("\nMessage Sent.\n");
 }else{
   printf("\nMessage in Queue.\n");
 }


}

void quit(int fd){
start(fd,QUIT); //send QUIT message

}

void menu(int sockfd){

  int option;
  do{
	displayMenu();
        scanf("%d",&option);
	printf("\n Option : %d\n",option);
        switch(option){
	 case 1: list(sockfd);break;
	 case 2: sendMessage(sockfd);break;
	 case 3: quit(sockfd);break;
	 default: printf("\nInvalid Option.\n");break;
	}
  }while(option!=3);


}




void main(){

  int sockfd;
  struct sockaddr_in destination;

  sockfd = createsocket();
  create(&destination,DEST_PORT,IP);

  conn(sockfd,&destination); 

  struct reply r; // struct hold replies from server
	

  printf("Please enter your email id: ");
  scanf("%s",u.username);

  printf("Please enter your password: ");
  scanf("%s",u.password);
   
  start(sockfd,AUTH); //notify auth
  sendUser(sockfd,u); //send user credentials to server
  r = recieveReply(sockfd);

  if(r.status == SUCCESS){
	//auth success
    printf("\n Authenticated Successfully. \n");
    menu(sockfd);

  }else{
	//auth error
    printf("\n Invalid Username or Password. \n");
    
  }

  close(sockfd);

}
