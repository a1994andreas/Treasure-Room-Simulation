#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ifaddrs.h> 
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <stddef.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE 1024
#define BUF_SIZE 512
pthread_t tid;
struct thread_param{
	int sock;
	int sh_mem_id;
};

void* wait_results(void *param){
	int coins_num ,prev=-1;
	char *shm;
	struct thread_param *value =(struct thread_param *)param;
	char buff[BUF_SIZE];
	int result ,received;
	printf("server thread starts here\n");
	if ((shm = shmat(value->sh_mem_id, NULL, 0)) == (char *) -1) {
 	       perror("shmat");
 	       exit(1);
	}
	sscanf(shm, "%d",&coins_num);
	while(coins_num!=0){
	sleep(1);
	if(coins_num!=prev)
		printf("current coins:%d\n",coins_num);
	prev=coins_num;
	sscanf(shm, "%d",&coins_num);
	}
	printf("current coins:%d\n",coins_num);
	received =recv(value->sock, buff, 511 ,0);
	result=(int)atoi(buff);
	if(result==1){
		printf("Team A wins\n");
		fflush(stdout);
	}else if(result==2){
		printf("Team B wins\n");
		fflush(stdout);
	}else {
		printf("Even Score\n");
		fflush(stdout);
	}
	return NULL;
	}



int main(int argc, char *argv[]){
	char buffer[BUF_SIZE] , buffer2[20], *shm, *s;
	int sock ,accepted ,received, shmid , k=0;
	//create a key
	key_t key;
	if ((key = ftok("server.c", 'R')) == -1) {
		perror("ftok");
		exit(1);
	}
	//create the shared memory segment
	if ((shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666)) == -1) {
		perror("shmget");
		exit(1);
	}
	//attach the segment to our data space
	if((shm =(char*)shmat(shmid, NULL, 0)) == (char*)-1){
		perror("shmat");
		exit(1);
	}
	//write number of coins in the shared memory
	//sprintf(shm,"%.9g",(double) coins_num);
	sprintf(shm,"%s" , (char*)argv[1]);
	//.......................
	struct sockaddr_in sin;
	struct sockaddr client_addr;
	socklen_t client_addr_len;
	if((sock= socket(AF_INET , SOCK_STREAM, IPPROTO_TCP)) == -1){
		perror("Opening tcp socket");
		exit(EXIT_FAILURE);
	}
	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(7155);
	// Bind to all available network interfaces
	sin.sin_addr.s_addr = INADDR_ANY;
	if(bind(sock, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) == -1){
		perror("TCP bind");
		exit(EXIT_FAILURE);
	}
	if(listen(sock, 1000) == -1){
		perror("TCP listen");
		exit(EXIT_FAILURE);
	}
	client_addr_len = sizeof(struct sockaddr);
	//we have to copy the id of the shared memory to a buffer 
	sprintf(buffer , "%d" ,shmid); 
	struct thread_param param1;
	int err;
	//........................................................
	//accept new connections , create one thread for each client
	while((accepted = accept(sock, &client_addr, &client_addr_len)) > 0 ){
		send(accepted,buffer , BUF_SIZE-1 , 0);
		printf("new client connected\n");
		fflush(stdout);
		param1.sock= accepted;
		param1.sh_mem_id=shmid;
		err = pthread_create(&tid ,NULL, &wait_results,(void*)&param1);
		if(err !=0){
			printf("\n cant create thread :[%s]",strerror(err));
			fflush(stdout);
    		}
		pthread_join(tid, NULL);
	}
	return 0;
	}
