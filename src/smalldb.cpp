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

#define PORT 28772							//Le port
#define BUFFSIZE 1024						//La taille du buffer permettant la communication
#define SERVER_BACKLOG 15					//nomber maximum de connexion
#define BUFFSIZE2 10000

//using namespace std;
typedef struct sockaddr_in SA_IN;			//structure utilisé pour l'addresage sur internet en IPv4
typedef struct sockaddr SA;

#include <signal.h>
#include "./common.h"						//fonction qui check le succes de la communication

int server_fd, client_socket, addr_size;	//permet de stocker les socket descriptor (est equivalent au file descriptir lors d'une lecture de fichier)
const char *db_path = "../students.bin";
database_t *db = (database_t*)malloc(sizeof(database_t));//permet d'allouer de la memoire dynamique


std::mutex readingA;								//permet de lock la db pour la lecture
std::mutex writingA;								//permet de lock la db pour l'ecriture
std::mutex generalAcess;							//mutex qui bloque l'acces géneral à la database(expliquer plus precisement dans db.cpp)

int readerQ;								//représente le nombre de demande de lecture en attente

SA_IN server_addr, client_addr;				//structUre d'addresage client/serveur			

void init_socket(int *server_fd);			//permet d'initialiser le socket
void * thread_connection(void* p_client_socket);//fonction utiliser par les threads (envoie, reception des messages)
void handler(int signum);					//fonction du control C
void handler_syn(int signum);				//fonction du SIGUSR1
bool reading(int client_socket,char buffer[BUFFSIZE], char answer[BUFFSIZE]);//lecture du message reçu, et traitement de ce message par la db
bool command_exist(const char* const buffer);//recherche si la command envoye existe
void init_sigA();							//initialisation des mask


int main(int argc, char const *argv[]) {

	if (argv[1] && argc!=0){
		db_path = argv[1];
	}

	db_load(db,db_path);					//initialise la database en fonction de l'argument donné
	signal(SIGPIPE, SIG_IGN); 				//permet de changer la fonction par défaut d'un signal
	signal(SIGUSR1, handler_syn);			//idem

	// Initialisation du socket/mask
	init_socket(&server_fd);
	init_sigA();
	addr_size = sizeof(SA_IN);
	
	while (true){
		client_socket = checked(accept(server_fd, (SA*) &client_addr, (socklen_t*)&addr_size));//accept la premiere connexion de la liste d'attente et lui assigne un socket descriptor
		printf("smalldb: (%d) accepted connection !\n", client_socket);//affiche le socket descriptor de la nouvelles connexion
		
		// permet que seul le thread principal récupère le SIGINT/SIGUSR1
		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGINT);
		sigaddset(&mask, SIGUSR1);
		sigprocmask(SIG_BLOCK, &mask, NULL);

		pthread_t t;						//initialise une structure pthread
		int *pclient = (int*)malloc(sizeof(int));//création d'un pointeur vers le socket descriptor du client
		*pclient = client_socket;
		pthread_create(&t, NULL,thread_connection, pclient);//cree le thread qui execute thread_connection

		sigprocmask(SIG_UNBLOCK, &mask, NULL);
	}
	return 0;
}



///Function /////////////////////////////////////////////////////////////////////////

/*
 * Permet de parametrer la reception des signaux, pour que le thread principale soit
 * le seul à intercepter un signal (SIGINT, SIGUSR1)
*/

void init_sigA(){
	struct sigaction action;
	struct sigaction actionSync;
	actionSync.sa_handler = handler_syn;
	action.sa_handler = handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	actionSync.sa_flags=SA_RESTART; 			//permet de continuer la fonction après le signal USR1
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGUSR1, &actionSync, NULL);
}

/*
 * Initialise les parametres du socket
*/

void init_socket(int *server_fd){

	// Création d'un descripteur de fichier pour un socket
	*server_fd = checked(socket(AF_INET, SOCK_STREAM, 0));// AF_NET ->IPV4 et SOCK_STREAM ->communicatiob TCP
	int opt = 1;
	setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));// permet d'eviter les erreurs "Port already used", en appliquant le param REUSE PORT and ADDRESS
	
	server_addr.sin_family = AF_INET;			//adresse "familiale" par défaut AF_INET
	server_addr.sin_addr.s_addr = INADDR_ANY;	//contient l'adresse IPV4
	server_addr.sin_port = htons(PORT);			//contient le port de communication

	int rc = ::bind(*server_fd, (SA*)&server_addr, sizeof(server_addr));
	if (rc == -1)//permet de lie le socket au port et a l'addresse mis en parametre 
	{
		printf("Failed to bind the socket");
	}
	checked(listen(*server_fd, SERVER_BACKLOG));//permet de switcher le port en mode "ecoute", attendant la connexion d'un client. Le param BACKLOG permet de limiter le nombre de connexcion au port

	std::cout << "Init successful\n";				//message visuel
}

/*
 * Fonction execute par les threads, qui consiste a lire le port pour une lecture 
 * ou une ecriture
*/

void * thread_connection(void* p_client_socket){

	int client_socket = *((int*) p_client_socket);
	free(p_client_socket);						//libere la memoire alloue au pointeur
	char buffer[BUFFSIZE];						//buffer contenant la requete
	char answer[BUFFSIZE];						//buffer contenant la reponse du serveur

	while (true){
		if(reading(client_socket, buffer, answer)!=true){//check si le client a ferme la connexion
			close(client_socket);// ferme le socket du client
			return NULL;
		}
		if(send(client_socket, answer, strlen(answer)+1, 0)<0){//envoie la reponse du serveur
			printf("smalldb: Lost connection to client (%d)\n", client_socket);//si c'est un echec alors le client a subi un probleme fermant la connexion
			printf("smalldb: closing connection %d\n", client_socket);
			printf("smalldb: closing thread for connection %d\n", client_socket);
			close(client_socket);
			return NULL;
		}
		bzero(buffer,BUFFSIZE);//vide le buffer pour la prochaine requete
		bzero(answer, BUFFSIZE);
	}
	return NULL;
}

/*
 * la redefinition des fonctions par defaut des signaux SIGINT/SIGUSR1
*/
void handler(int signum) {
	db_save(db);
	close(client_socket);
	close(server_fd);
}

void handler_syn(int signum){
	db_save(db);
}

/*
 * Permet de lire le message envoye par un client, et de detecter une fin de connexion venant du client ("exit").
 * Execute command_exist.En fonction du resultat le serveur renvoie un msg d'erreur au client,
 * ou appelle la fonction parse_execute.
*/

bool reading(int client_socket,char buffer[BUFFSIZE], char answer[BUFFSIZE2]){
	int lu;
	std::string text="";//string stockant la reponse des fonctions parse_exec
	if((lu = read(client_socket, buffer, 1024))<0){//lit le port
		printf("Failed !\n");
	}
	if (strncmp("exit", buffer, strlen("exit")-1)==0){//si le client ferme la connexion grace a un exit
		printf("smalldb: Client (%d) disconnected (normal). Closing connection and thread\n", client_socket);//le client ferme la connexion de son plein gre
		return false;
	}
	buffer[strlen(buffer)-1] = 0;

	if(command_exist(buffer)){//recherche si une command existante se trouve dans la requete
		parse_and_execute(&text, db, buffer, &readingA, &writingA, &generalAcess, &readerQ);
		strcpy(answer, text.c_str());//copie la reponse dans answer afin de l'envoyer
	}
	else{
		strcpy(answer, "Unknown query type");
	}
	return true;
}

/*
 * command_exist, recupere un buffer contenant la requete envoye par le client
 * La fonction parse la requete pour detecter si il y a la presence d'une commande connue.
 * Si c'est le cas la fonction renvoie true, sinon false.
*/
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
