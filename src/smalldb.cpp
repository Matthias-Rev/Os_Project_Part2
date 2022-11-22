#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include "db.hpp"

#define PORT 28772
#define BUFFSIZE 1024
#define SERVER_BACKLOG 15

using namespace std;
typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

#include <signal.h>

#include "./common.h"


int server_fd, client_socket, addr_size;
const char *db_path = "../students.bin";
database_t *db;

SA_IN server_addr, client_addr;

void init_socket(int *server_fd, int *new_socket);
void * thread_connection(void* p_client_socket);
void handler(int signum);
void reading(int client_socket,char buffer[BUFFSIZE], char answer[BUFFSIZE]);


int main(int argc, char const *argv[]) {

	int count = 0;
	if (argv[1]){
		db_path = argv[1];
	}

	//db_load(db,db_path);
	signal(SIGPIPE, SIG_IGN);
  // Initialisation du socket
	init_socket(&server_fd, &client_socket);
	
	struct sigaction action;
	action.sa_handler = handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGINT, &action, NULL);
	
	
	while (true){
		addr_size = sizeof(SA_IN);
		client_socket = checked(accept(server_fd, (SA*) &client_addr, (socklen_t*)&addr_size));
		printf("smalldb: (%d) accepted connection !\n", client_socket);
		
		// permet que seul le thread principal récupère le SIGINT
		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGINT);
		sigprocmask(SIG_BLOCK, &mask, NULL);

		pthread_t t;
		int *pclient = (int*)malloc(sizeof(int));
		*pclient = client_socket; 
		pthread_create(&t, NULL,thread_connection, pclient);

		sigprocmask(SIG_UNBLOCK, &mask, NULL);
	}

	
	return 0;
}

void init_socket(int *server_fd, int *new_socket){
	// Création d'un descripteur de fichier pour un socket
	*server_fd = checked(socket(AF_INET, SOCK_STREAM, 0));
	int opt = 1;
	setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	checked(bind(*server_fd, (SA*)&server_addr, sizeof(server_addr)));
	checked(listen(*server_fd, SERVER_BACKLOG));

	cout << "Init successful\n";
}

void * thread_connection(void* p_client_socket){

	int client_socket = *((int*) p_client_socket);
	free(p_client_socket);
	char buffer[BUFFSIZE];
	char answer[BUFFSIZE];
	char buffer_w[] = "received";
	size_t bytes_read;
	int msgsize = 0;


	while (true){
		reading(client_socket, buffer, answer);
		//if(send(client_socket, buffer_w, strlen(buffer_w)+1, 0)<0){
		if(send(client_socket, answer, strlen(answer)+1, 0)<0){
			printf("Client (%d) disconnected (normal). Closing connection and thread\n", client_socket);
			close(client_socket);
			return NULL;
		}
		bzero(buffer,BUFFSIZE);
		bzero(answer, BUFFSIZE);
	}
	return NULL;
}

void handler(int signum) {
	printf("\nSignal %d received\n", signum);
	//db.save();
	close(server_fd);
	close(client_socket);
}

void reading(int client_socket,char buffer[BUFFSIZE], char answer[BUFFSIZE]){
	int lu;
	if(lu = read(client_socket, buffer, 1024)<0){
		printf("Failed !\n");
	}
	buffer[strlen(buffer)-1] = 0;
	printf("Querry: %s\n", buffer);

	if(strncmp("select", buffer, strlen("select")-1)==0){
		printf("select find\n");
	}
	if(strncmp("update", buffer, strlen("update")-1)==0){
		printf("update find\n");
	}
	if(strncmp("insert", buffer, strlen("insert")-1)==0){
		printf("insert find\n");
	}
	if(strncmp("delete", buffer, strlen("delete")-1)==0){
		printf("delete find\n");
	}
	else{
		printf("Unknown query type.");
		strcpy(answer, "Unknown query type");
	}

}
