#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

const char* PORT;           //port de la connexion
#include "./common.h"

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
     	char msg[25];
     	recv(sock, &msg, 1024, 0);    
     	printf("%s\n", msg);
    }
    else{
      break;
    }
  }
}
