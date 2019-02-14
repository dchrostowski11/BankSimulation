#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>


pthread_t input;
pthread_t output;

int clientSocket = -1;
int mode=0;


int validCmd(char *  str,char *pass){
	//printf("str %s, pass %s",str,pass);
	//printf("str\n");
	if(mode==0){
		if(strcmp(str,"create")==0){
			if(pass!=NULL){
				if(strlen(pass)>255){
					return 0;
				}
				return 1;
			}
			return 0;
		}else if(strcmp(str,"serve")==0){
			if(pass!=NULL){
				if(strlen(pass)>255){
					return 0;
				}
				mode=1;
				return 1;
			}
			return 0;
		}else if(strcmp(str,"quit\n")==0){		//consider return something that isn't 1 to mark it for delete
			//printf("\nquit!!!\n");
			mode=-1;
			return 1;
		}
		return 0;
	}
	if(strcmp(str,"withdraw")==0){
		if(pass!=NULL){					//consider checking if its at least positive and check if its a number
			char* ptr;		
			double m=strtod(pass,&ptr);		
			if(m==0){
				if(pass==0){	
					return 1;
				}
				return 0;
			}else if(pass>0){
				return 1;
			}
			printf("Withdraw is negative or invalid\n");	
		}
		return 0;
	}else if(strcmp(str,"deposit")==0){
		if(pass!=NULL){					//consider checking if its at least positive and check if its a number
			char* ptr;	
			double m=strtod(pass,&ptr);		
			if(m==0){
				if(pass==0){
					return 1;
				}
				return 0;
			}else if(m>0){
				return 1;
			}
			printf("Deposit is negative or invalid\n");	
		}
		return 0;
	}else if(strcmp(str,"query\n")==0){
		return 1;
	}else if(strcmp(str,"end\n")==0){
		mode=0;
		return 1;
	}else if(strcmp(str,"serve")==0){
		return 1;

	}
	return 0;	
}


void *sendingThread(void *filedescriptor){
	
//	printf("Inside sending thread...\n");
	int fd = *(int*)filedescriptor;	
//      char buffer[255];	
//	bzero(buffer, 255);
//	int num; int i=0; 
	char * arg1; char * arg2; 
	while(mode!=-1){
		arg1=NULL; arg2=NULL;	
		char *buffer=(char* )malloc(sizeof(char)*300);
		bzero(buffer, 270);
		char *pass=(char* )malloc(sizeof(char)*300);
	
		fgets(buffer, 270, stdin);
		if(strlen(buffer)>255){
			printf("Too long of an argument\n");
			free(pass);
			free(buffer);
		}
		strcpy(pass,buffer);
		arg1=strtok(buffer," ");
		if(arg1==NULL){
			printf("No argument\n");
			free(buffer);
			free(pass);
		}else{
			arg2=strtok(NULL," ");
			if(validCmd(arg1,arg2)==0){
				printf("Invalid cmd\n");
			}else{
				send(fd, pass, 255, 0);	
//				printf("pass %s.",pass);
				if( strcmp(pass,"quit\n")==0 ){
					return NULL;
				}
				sleep(2);
			}
			free(buffer);
			free(pass);
		}
	}
	return NULL;	
}


void *receivingThread(void *filedescriptor){
		
	int fd = *(int*)filedescriptor;
	char buffer[255];
	bzero(buffer, 255);
	
	while(recv(fd, buffer, sizeof(buffer), 0) > 0 ){
		printf("SERVER SENT: %s\n", buffer);
		
		if(strlen(buffer)>4){
			if(strstr(buffer,"SERVE ERROR")!=NULL){
				//if it is an error then i need to change the mode back this is esp so if it was a serve 
				mode = 0;
			}else if(strstr(buffer,"Shut Down")!=NULL){


			}else if(strstr(buffer,"ERROR")!=NULL){
				

			}
		}


	}
	pthread_exit(NULL);
	
}	

void disconnectClient(){
	printf("Exiting client...\n");
	close(clientSocket);
	exit(0);
}


int main(int argc, char **argv){

	int socketfd;
	int portnumber;
	signal(SIGINT, disconnectClient);
	
	struct sockaddr_in serveraddr;
	struct hostent *server;
		
//	char buffer[255];
	
	if(argc < 3){
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(0);
	}
	
	portnumber = atoi(argv[2]);
	
	printf("port # is %d\n", portnumber);
	
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(socketfd < 0){
		perror("Problem opening client socket\n");
		exit(1);
	}
	
	server = gethostbyname(argv[1]);
	
	if(server == NULL){
		perror("No such host exists!\n");
		exit(1);
	}	
	
	bzero((char *) &serveraddr, sizeof(serveraddr));
	
	serveraddr.sin_family = AF_INET;
	
	bcopy((char *) server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
	
	serveraddr.sin_port = htons(portnumber);
	
	while(connect(socketfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0){
		perror("Client side connection failed\n");
		sleep(3);
	}
	
	printf("Connection successful...\n");
	
	if(pthread_create(&input, NULL, receivingThread, (void*)&socketfd) < 0){
		printf("ERROR: Input thread was not created!\n");
		exit(0);
	}	
			
	if(pthread_create(&output, NULL, sendingThread, (void*)&socketfd) < 0){
		printf("ERROR: Output thread was not created!\n");
		exit(0);
	}
	pthread_join(output, NULL);
	
	return 1;
	pthread_join(input,NULL);
	close(socketfd);
	return 0;
}

