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
#include "queries.hpp"

#define PORT 28772
#define BUFFSIZE 1024
#define SERVER_BACKLOG 15

using namespace std;
typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;
int readLocker=0;

#include <signal.h>

#include "./common.h"


int server_fd, client_socket, addr_size;
const char *db_path = "../students.bin";
database_t *db = (database_t*)malloc(sizeof(database_t));

mutex readingA;
mutex writingA;
mutex generalAcess;
int readerQ;

SA_IN server_addr, client_addr;

void init_socket(int *server_fd);
void * thread_connection(void* p_client_socket);
void handler(int signum);
void handler_syn(int signum);
bool reading(int client_socket,char buffer[BUFFSIZE], char answer[BUFFSIZE]);
bool command_exist(const char* const buffer);

int main(int argc, char const *argv[]) {

	if (argv[1] && argc!=0){
		db_path = argv[1];
		printf("%s", db_path);
	}

	db_load(db,db_path);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGUSR1, handler_syn);
  // Initialisation du socket
	init_socket(&server_fd);
	
	struct sigaction action;
	struct sigaction actionSync;
	actionSync.sa_handler = handler_syn;
	action.sa_handler = handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	actionSync.sa_flags=SA_RESTART;
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGUSR1, &actionSync, NULL);
	
	
	while (true){
		addr_size = sizeof(SA_IN);
		client_socket = checked(accept(server_fd, (SA*) &client_addr, (socklen_t*)&addr_size));
		printf("smalldb: (%d) accepted connection !\n", client_socket);
		
		// permet que seul le thread principal récupère le SIGINT
		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGINT);
		sigaddset(&mask, SIGUSR1);
		sigprocmask(SIG_BLOCK, &mask, NULL);

		pthread_t t;
		int *pclient = (int*)malloc(sizeof(int));
		*pclient = client_socket;
		pthread_create(&t, NULL,thread_connection, pclient);

		sigprocmask(SIG_UNBLOCK, &mask, NULL);
	}

	
	return 0;
}

void init_socket(int *server_fd){
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

	while (true){
		if(reading(client_socket, buffer, answer)!=true){
			close(client_socket);
			return NULL;
		}
		//if(send(client_socket, buffer_w, strlen(buffer_w)+1, 0)<0){
		if(send(client_socket, answer, strlen(answer)+1, 0)<0){
			printf("smalldb: Lost connection to client (%d)\n", client_socket);
			printf("smalldb: closing connection %d\n", client_socket);
			printf("smalldb: closing thread for connection %d\n", client_socket);
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
	close(client_socket);
	close(server_fd);
}
void handler_syn(int signum){
	printf("\nSignal %d received\n", signum);
	//db.save();
}

bool reading(int client_socket,char buffer[BUFFSIZE], char answer[BUFFSIZE]){
	int lu;
	string text="";
	if((lu = read(client_socket, buffer, 1024))<0){
		printf("Failed !\n");
	}
	if (strncmp("exit", buffer, strlen("exit")-1)==0){
		printf("smalldb: Client (%d) disconnected (normal). Closing connection and thread\n", client_socket);
		return false;
	}
	buffer[strlen(buffer)-1] = 0;
	printf("Querry: %s\n", buffer);

	if(command_exist(buffer)){
		parse_and_execute(&text, db, buffer, &readingA, &writingA, &generalAcess, &readerQ);
		strcpy(answer, text.c_str());
		printf("%s", answer);
	}
	else{
		strcpy(answer, "Unknown query type");
	}
	return true;
}

bool command_exist(const char* const buffer){
	if(strncmp("select", buffer, strlen("select")-1)==0){
		return true;
	}
	else if(strncmp("update", buffer, strlen("update")-1)==0){
		return true;
	}
	else if(strncmp("insert", buffer, strlen("insert")-1)==0){
		return true;
	}
	else if(strncmp("delete", buffer, strlen("delete")-1)==0){
		return true;
	}
	else{
		return false;
	}
}
/*
void parse_and_execute_select(FILE* fout, database_t* db, const char* const query);

void parse_and_execute_update(FILE* fout, database_t* db, const char* const query);

void parse_and_execute_insert(FILE* fout, database_t* db, const char* const query);

void parse_and_execute_delete(FILE* fout, database_t* db, const char* const query);*/
