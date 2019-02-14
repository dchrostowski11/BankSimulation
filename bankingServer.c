#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

pthread_mutex_t addaccount = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t createaccmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t serveaccmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t endmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t depositmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t withdrawmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t querymutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sessionmutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t accountwithdraw = PTHREAD_MUTEX_INITIALIZER;

sem_t semaphore;
int threadCount = 1;
int counter =0;
struct threadList{
	pthread_t threadid;
	struct threadList *next;
};

struct threadList *threadhead = NULL;
struct threadList *threadtemp;

struct account{
	char name[255];
	double balance;
	int inSession;
	struct account* next;
};

struct account *head = NULL;




void print(){
	struct account * temp = head;
	while(temp != NULL){
		
		temp->name[strlen(temp->name)-1]='\0';
		if(temp->inSession==1){
			
			printf("%s\t%lf\tIn session\n", temp, temp->balance);
		}else{
			printf("%s\t%lf\n", temp, temp->balance);
		}
		temp=temp->next;
	}
}

/*
void printAccountInfo(){
	struct account *info = (struct account*)malloc(sizeof(struct account));
	info = head;
	while(info != NULL){
		printf("%s\t%lf\t%d\n", info->name, info->balance, info->inSession);
		info = info->next;
	}
	return;
}*/	


void addAccount(int fd, char *name, double balance, int inSession){

	pthread_mutex_lock(&addaccount);

	struct account *newAccount = (struct account*) malloc(sizeof(struct account));
	name[strlen(name)-1]='\0';
	strcpy(newAccount->name, name);
	newAccount->balance = balance;
	newAccount->inSession = inSession;
	newAccount->next = head;
	head = newAccount;	
	
	//printf("new account created with: \n name %s\n balance %f\n insession %d\n", newAccount->name, newAccount->balance, newAccount->inSession);
	
	send(fd, "Account successfully created \n", 255, 0);
	
	pthread_mutex_unlock(&addaccount);
	
	return;
	
}


struct account *findAccount(int fd, char *name){
	
	printf("searching for: %s\n", name);
	
	if(head == NULL){
//		send(fd, "The system currently does not have any accounts!\n", 255, 0);
//		send(fd, "No accounts currently exist in the system\n", 255, 0);
		//printf("No accounts currently exist in the system\n");
		return NULL;
	} 
	struct account *searcher = head;

	while(searcher != NULL){
		if(strcmp(searcher->name, name) == 0){
			//printf("an account has been found\n");
//			send(fd, "This account has been found! What would you like to do with it?\n", 255, 0);
			return searcher;
		} else {
			searcher = searcher->next;
		}
	}		
		
	return NULL;
}


void disconnect(){
	
	char buffer[255] = {0};
	
	//printf("server is disconnecting...\n");
	
/*	struct threadList *pointer = threadhead;
	
	while(threadhead != NULL){
		pointer = threadhead;
		printf("thread id: %d\n", pointer->threadid);
		threadhead = threadhead->next;
	}
*/	
	struct account *ptr = head;
	


	print();
	while(head != NULL){
		ptr = head;
		head = head->next;
		free(ptr);
	}
	
	exit(0);	
}	


void* clientHandler(void* filedescriptor){
		
	int fd = *(int*)filedescriptor;
	char buffer[255];
	
//	bzero(buffer, 255);
	
	while(read(fd, buffer, sizeof(buffer)) > 0){
				
		//printf("CLIENT SENT: %s\n", buffer);
		
		if(strncmp(buffer, "create ", 7) == 0){
			//printf("Client wants to create an account!\n");	
						
			char *buf = (char*)malloc(sizeof(char) * 290);
		
			memset(buf, '\0', 290);
		
			buf = strdup(buffer+7);			
						
//			printf("BUF is %s\n", buf);			
						
			pthread_mutex_lock(&createaccmutex);
			
//			pthread_mutex_lock(&withdrawmutex);		
		
			struct account *temp;
			temp = findAccount(fd, buf);
			if(temp == NULL){
				send(fd, "CREATING NEW ACCOUNT\n", 255, 0);
				addAccount(fd, buf, 0.0, 0);
				//free(buf);
				//bzero(buffer,255);
			} else {
			//	printf("FOUND A DUPLICATE ACCOUNT %s\n", buf);
				send(fd, "ERROR, THIS ACCOUNT ALREADY EXISTS\n", 255, 0);
				//free(temp);
			}	
		
			pthread_mutex_unlock(&createaccmutex);		
			bzero(buffer, 255);
			free(buf);

			continue;
			
		} else if(strncmp(buffer, "serve ", 6) == 0){
			//printf("Client wants to start a service for an account!\n");
			
			char *servebuf = (char *)malloc(sizeof(char)* 290);
			
			memset(servebuf, '\0', 290);
			
			servebuf = strdup(buffer+6);
			
			//printf("SERVEBUF is %s\n", servebuf);
			
//			pthread_mutex_lock(&serveaccmutex);
					
			struct account *serveptr;
			serveptr = findAccount(fd, servebuf);
			if(serveptr == NULL){
				send(fd, "SERVE ERROR: THIS ACCOUNT DOES NOT EXIST IN THE SYSTEM\n", 255, 0);
//				pthread_mutex_unlock(&serveaccmutex);	
				continue;
			} else if(serveptr->inSession == 1){
				send(fd, "SERVE ERROR: This account has already been accessed, please wait\n", 255, 0);
				continue;
			} else {
				
//				pthread_mutex_lock(&serveaccmutex);
				pthread_mutex_lock(&sessionmutex);
				serveptr->inSession = 1;
				pthread_mutex_unlock(&sessionmutex);
			}
						
			send(fd, "What would you like to do with the account?\n", 255, 0);
			
//			pthread_mutex_unlock(&serveaccmutex);
						
			while(recv(fd, buffer, 255, 0) > 0){
				
				if(strncmp(buffer, "end", 3) == 0){
					//printf("Client wants to end the current service session\n");						
					pthread_mutex_lock(&endmutex);
					
					serveptr->inSession = 0;
					
					send(fd, "The service session has ended\n", 255, 0);

					pthread_mutex_unlock(&endmutex);
					
					pthread_mutex_unlock(&serveaccmutex);
					
					break;			
					
				}
				else if(strncmp(buffer, "deposit ", 8) == 0){
					//printf("Client wants to deposit to account\n");
				
					pthread_mutex_lock(&depositmutex);				
										
					char *depositbuf = (char*)malloc(sizeof(char) * 256);
					
					memset(depositbuf,'\0', 256);
					
					depositbuf = strdup(buffer+8);
					
					char *abd;
					
					//printf("DEPOSITBUF is %s\n", depositbuf);
					
					double deposit = strtod(depositbuf, &abd);
					
					//printf("AMOUNT TO BE DEPOSITED IS % lf\n", deposit);
					
					serveptr->balance += deposit;
										
					pthread_mutex_unlock(&depositmutex);
					
					//printf(" %s now has a balance of %f\n", serveptr->name, serveptr->balance);
								
					continue;
					
				} 
				else if(strncmp(buffer, "withdraw ", 9) == 0){
					//printf("Client wants to withdraw from account\n");
					
//					pthread_mutex_lock(&withdrawmutex);

					char *withdrawbuf = (char*)malloc(sizeof(char) * 256);
					
					memset(withdrawbuf, '\0', 256);
					
					withdrawbuf = strdup(buffer+9);
					
					char *abc;
					
					printf("WITHDRAWBUF is %s\n", withdrawbuf);
					
					double withdraw = strtod(withdrawbuf, &abc);
					
					printf("AMOUNT TO BE WITHDRAWN %lf\n", withdraw);

					if(withdraw > serveptr->balance){				
					
					//	printf("Cannot withdraw more than is in the account!\n");
						send(fd, "SERVE ERROR: Cannot withdraw more than is in the account!\n", 255, 0);
						continue;
						
					} else {
					
						pthread_mutex_lock(&withdrawmutex);				
						serveptr->balance -= withdraw;	
						
						pthread_mutex_unlock(&withdrawmutex);		
					//	printf(" %s now has a balance of %f\n", serveptr->name, serveptr->balance);						
					}
					
//					pthread_mutex_unlock(&withdrawmutex);
					
					continue;
					
				} 
				else if(strncmp(buffer, "query", 5) == 0){
					//printf("Client wants to query the account balance\n");
									
					pthread_mutex_lock(&querymutex);
					
					char num[50];
					char query[255];
					snprintf(num, 50, "%f", serveptr->balance);
					
				//	printf("Current account balance is %s\n", num);
					
					strcat(query, "Current account balance is: ");
					strcat(query, num);
					
					send(fd, query, 255, 0);
					bzero(query, 255);
					
					pthread_mutex_unlock(&querymutex);
					
					continue;
					
				} else {
					send(fd, "Please input a valid command\n", 255, 0);	
					continue;
				}
			}	
		}
		else if(strncmp(buffer, "quit", 4) == 0){
		//	printf("Client wants to quit\n");
			
			send(fd, "Shutting down the client... \n", 255, 0);	
			close(fd);
		//	printf("successfully closed the socket\n");
			
			break;
			
		} else {
			send(fd, "Please input a valid command\n", 255, 0);	
		}
		
	}
		
	pthread_exit(NULL);
				
}
void alarmHandler(int sig ){
	counter =1;
}


void threadfunc(){
	while(1){
		if(counter ==1){
		sem_wait(&semaphore);
		signal(SIGALRM,SIG_IGN);
		print();
		signal(SIGALRM,alarmHandler);
		counter =0;
		sem_post(&semaphore);
		alarm(15);
		}
	}
}

int main(int argc, char **argv){
	if(argc < 2){
		perror("Port number not provided!\n");
		exit(0);
	}
	
	pthread_t *mythread;
	mythread=(pthread_t*)malloc(sizeof(*mythread));
	pthread_create(mythread,NULL,(void*)threadfunc,NULL);	
	sem_init(&semaphore,0,1);

	alarm(15);
	
	signal(SIGALRM,alarmHandler);
	signal(SIGINT, disconnect);

	int socketfd;
	int newsocketfd;
	int portnumber;
	int n;
	
	threadhead = (struct threadList*)malloc(sizeof(struct threadList));
	threadtemp = threadhead;

	char buffer[255];
	
			
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;	
	
	socklen_t clientlen;
	
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(socketfd < 0){
		perror("Cannot create a socket\n");
		exit(EXIT_FAILURE);
	}
	
	bzero((char *) &server_addr, sizeof(server_addr));
	
	portnumber = atoi(argv[1]);
	
	//printf("PORT NUMBER IS: %d\n", portnumber);
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(portnumber);
	
	if(bind(socketfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
		perror("Binding failed\n");
		exit(1);
	}
	
	//printf("listening for connections...\n");
	
	listen(socketfd, 10);

//	clientlen = sizeof(client_addr);	
	
	while(1){
		
		clientlen = sizeof(client_addr);
		newsocketfd = accept(socketfd, (struct sockaddr *)&client_addr, &clientlen);
		
		if(newsocketfd < 0){
			perror("Error accepting a connection\n");
			exit(1);
		}
		
		printf("Server got a connection...\n");
		
		send(newsocketfd, "successfully connected to the server, what would you like to do?", 255, 0);
		
//		pthread_t thread;
//		pthread_create(&thread, NULL, clientHandler, (void*)&newsocketfd);

		pthread_create(&threadtemp->threadid, NULL, clientHandler, (void*)&newsocketfd);

		threadCount++;
		
		threadtemp->next = (struct threadList*)malloc(sizeof(struct threadList));
	
		threadtemp = threadtemp->next;
				
	}
	
	
	
	close(newsocketfd);
	close(socketfd);	
	
	return 0;
		
}
