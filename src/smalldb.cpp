#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include "db.hpp"

using namespace std;

// Permet de définir un gestionnaire de signaux pour SIGPIPE,
// ce qui évite une fermeture abrupte du programme à la réception
// du signal (utile si vous avez des opérations de nettoyage à
// faire avant de terminer le programme)
#include <signal.h>

#include "./common.h"

void init(int *server_fd, int *new_socket);


int main(void) {
  // Permet que write() retourne 0 en cas de réception
  // du signal SIGPIPE.
  signal(SIGPIPE, SIG_IGN);
  int server_fd, new_socket;

  // Initialisation du socket
  init(&server_fd, &new_socket);


  char buffer[1024];
  int lu;
  
  // Pour le serveur, on se contente de renvoyer
  // au client tout ce qui est reçu. Comme le
  // socket est SOCK_STREAM, plusieurs appels à
  // read() peuvent être nécessaires pour lire
  // le message en entier.
  while ((lu = read(new_socket, buffer, 1024)) > 0) {
     checked_wr(write(new_socket, buffer, lu) < 0);
  }
  
  close(server_fd);
  close(new_socket);
  return 0;
}

void init(int *server_fd, int *new_socket){
  // Création d'un descripteur de fichier pour un socket
  *server_fd = checked(socket(AF_INET, SOCK_STREAM, 0));

  int opt = 1;
  setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
  
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(28772);

  checked(bind(*server_fd, (struct sockaddr *)&address, sizeof(address)));
  checked(listen(*server_fd, 3));

  size_t addrlen = sizeof(address);
  *new_socket = checked(accept(*server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen));
}
