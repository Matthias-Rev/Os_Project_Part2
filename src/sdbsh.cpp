#include <arpa/inet.h>
#include <stdio.h>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#define SIZE 1024

const char* PORT;           //port de la connexion
#include "./common.h"

/* 
 * Permet de lire la réponse du serveur, on arrête de lire le PORT quand le message lu contient à la fin "/end".
 */
void reading_file(int sockfd){
  char buffer[SIZE];
  while (1) {
    recv(sockfd, buffer, SIZE, 0);
    std::string my_str = std::string(buffer); //reconversion en string pour utiliser les fonctions de la classe string
    std::size_t ind = my_str.find("/end");    //trouver la position du mot recherché
    if(ind !=std::string::npos){              //string::npos représente la plus grand valeur de size_t, elle est équivalente à la position trouvé
                                              //par .find si le string ne contient pas le mot.
        my_str.erase(ind,my_str.length());    //si c'est le dernier message, on supprime le mot "/end".
        std::cout << my_str << "\n";
        break;
    }
    std::cout << buffer;                      //on affiche les messages reçus
    bzero(buffer, SIZE);
  }
}

int main(int argc, char const *argv[]) {

    if (argv[1] && argc != 0){
    PORT = argv[1];
    }
    /*
     * Parametrage du socket equivalent a celui de smalldb
    */
    int sock = checked(socket(AF_INET, SOCK_STREAM, 0));

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(28772);

    // Conversion de string vers IPv4 ou IPv6 en binaire
    inet_pton(AF_INET, PORT, &serv_addr.sin_addr);

    checked(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)));
  
    char buffer[1024];
  
    while (true){
        if (isatty(fileno(stdin))){ // detect si le stdin est un fichier ou un terminal
        printf("> ");               // caractere facilitant la lecture du terminal
        }

  	    if (fgets(buffer, 1024, stdin) != NULL) {
     	    checked_wr(write(sock, buffer, strlen(buffer) + 1));
     
        if(strncmp(buffer,"exit",strlen("exit")-1)==0){// si le mot "exit" est tape, le programme met fin a la communication
            close(sock);
            return 0;
        }
     	reading_file(sock);           //lit la réponse du serveur
    }
    else{
      break;
    }
  }
}
