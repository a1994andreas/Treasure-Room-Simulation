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

#define BUF_SIZE 512
pthread_t tid[2];
int countA=0, countB=0;
pthread_t teamA, teamB;
pthread_mutex_t lock;

struct thread_param{
	int sock_num;
	char* buffer2;
	int id;
	int sock;
};

void* doSomeThing(void *param)
{
	int coins_num;
	int rand_value_A=rand();
	int rand_value_B=rand();
	int A=rand_value_A % 10;
	int B=rand_value_B % 10;
	unsigned long i = 0 ;
	//get thread id
	struct thread_param *values =(struct thread_param *)param;
	if(values->id==0)
		printf("The team A wait %d sec to enter the room and take %d coins\n", A , B);
	else 
		printf("The team B wait %d sec to enter the room and take %d coins\n", A , B);
	int sock ,shmid, k =0;
	char *shm ,buff2[BUF_SIZE] ,buff3[BUF_SIZE];
	shmid=(int)atoi(values->buffer2);
	sleep(A);
	//lock thread here!!!!!!!!!!!!!!!!!
	pthread_mutex_lock(&lock);
	if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
	        perror("shmat");
	        exit(1);
	}
	//read from memory
	sscanf(shm,"%d" ,&coins_num );
	//stop when coins run out	
	if(coins_num==0) 
		return NULL;
	printf("current number of coins= %d\n", coins_num);
	//remove B coins
	while(k<B){
		coins_num--;
	if(values->id==0){
		countA++;
		printf("Team A: %d\n", coins_num);
	}else if(values->id==1){
		countB++;	
		printf("Team B: %d\n", coins_num);
	}
	//read number of coins remaining
	sprintf(shm,"%.9g",(double)coins_num);
	if(coins_num==0) {
		//about to exit so unlock the thread first
		pthread_mutex_unlock(&lock);
		return NULL;
	}
	sleep(1);
	k++;
	}
	//unlock thread!!!!!!!!!!!!!!!!!!!!!!!!
	pthread_mutex_unlock(&lock);
	return NULL;
}



int main (int args , char *argv[]){
	int i = 0 ,coins=1 ,res=0;
	int err;
	char buffer2[BUF_SIZE] ,buff3[BUF_SIZE];
	char* shm;
	int sock ,shmid;
	struct sockaddr *client_addr;
	socklen_t client_addr_len;    
	if (pthread_mutex_init(&lock, NULL) != 0){
		printf("\n mutex init failed\n");
		return 1;
	}
	//open socket...
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
		perror("opening TCP socket");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	//Port that server listens at
	sin.sin_port = htons(7155);
	// The server's IP
	sin.sin_addr.s_addr = inet_addr("0.0.0.0");
	//connect to server..
	if(connect(sock, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) == -1){
		perror("tcp connect");
		exit(EXIT_FAILURE);
	}
	recv(sock , buffer2 , BUF_SIZE-1 , 0);
	struct thread_param param1 , param2;
	shmid=(int)atoi(buffer2);
	//repeat until the coins run out
	while(coins>0){
		while(i < 2){
			param1.buffer2=buffer2;
			param2.buffer2=buffer2;
			param1.id=0;
			param2.id=1;
			param2.sock=sock;
			//create the threads
			if(i==0)
				err = pthread_create(&(tid[i]), NULL, &doSomeThing, (void*)&param1);
			else
				err = pthread_create(&(tid[i]), NULL , &doSomeThing,(void*)&param2);        
			if (err != 0)
            			printf("\ncan't create thread :[%s]", strerror(err));
        		i++;
    		}
		//wait until the threads return
		pthread_join(tid[0], NULL);
		pthread_join(tid[1], NULL);    
		if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
		perror("shmat");
		exit(1);
	}
	sscanf(shm,"%d" ,&coins);
	i=0;
	}
	printf("Team A has %d coins\n", countA);
	printf("Team B has %d coins\n", countB);
	//send the result to the server
	if(countA>countB){
		res=1;
		printf("So Team A wins\n");
	}else if(countB>countA){
		res=2;
		printf("So Team B wins\n");
	}else{
		res=0;
		printf("-Even score-");
	}
	sprintf(buff3 , "%d" ,res);
	send(sock , buff3 ,BUF_SIZE-1 , 0);
	pthread_mutex_destroy(&lock);
	return 0;
}

