#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Permet de définir un gestionnaire de signaux pour SIGPIPE,
// ce qui évite une fermeture abrupte du programme à la réception
// du signal (utile si vous avez des opérations de nettoyage à
// faire avant de terminer le programme)
#include <signal.h>
const char* PORT;
#include "./common.h"

int main(int argc, char const *argv[]) {
  // Permet que write() retourne 0 en cas de réception
  // du signal SIGPIPE.
  signal(SIGPIPE, SIG_IGN);
  if (argv[1]){
  	PORT = argv[1];
  }
  
  int sock = checked(socket(AF_INET, SOCK_STREAM, 0));

  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(28772);

  // Conversion de string vers IPv4 ou IPv6 en binaire
  inet_pton(AF_INET, PORT, &serv_addr.sin_addr);

  checked(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)));
  
  char buffer[1024];
  int longueur, i, ret;
  while (true){
  	if (fgets(buffer, 1024, stdin) != NULL) {
     	longueur = strlen(buffer) + 1;
     	printf("Envoi...\n");
     	checked_wr(write(sock, buffer, strlen(buffer) + 1));
     
     // Pour rappel, nous utilisons des sockets en mode
     // SOCK_STREAM. Par conséquent, il n'y a pas de
     // garanties sur le fait qu'un message implique
     // exactement une réception (plusieurs pourraient
     // être nécessaires). Il faut donc boucler sur read().
     	char msg[25];
     	recv(sock, &msg, 26, 0);
     
     	printf("Recu : %s\n", msg);
  	}
  }
  
  close(sock);
  return 0;
}
