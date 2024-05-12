#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define BUFFER 512

void erro(char *msg) {
    printf("Error: %s\n", msg);
	exit(-1);
}

int main(int argc, char *argv[]) {
    char endServer[100];
    int fd;
    int nread = 0;
    struct sockaddr_in addr;
    struct hostent *hostPtr;

    if (argc != 3) 
        erro("cliente <host> <PORTO_TURMAS>");

    strcpy(endServer, argv[1]);
    if ((hostPtr = gethostbyname(endServer)) == 0)
        erro("Host not found");

    bzero((void *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
    addr.sin_port = htons((short) atoi(argv[2]));

    if ((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
        erro("socket");
    if (connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
        erro("Connect");
    
    
    char svMsg[BUFFER];
    char input[BUFFER];
    char username[64];
    char password[64];

    nread = read(fd, svMsg, sizeof(svMsg));
    svMsg[nread] = '\0';
    printf("%s", svMsg);

    while(1) {
        
        memset(svMsg, 0, strlen(svMsg));
        memset(username, 0, strlen(username));
        memset(password, 0, strlen(password));

        if(scanf(" %63s %63s", username, password) == 2){

            strcpy(input, username);
            strcat(input, " ");
            strcat(input, password);
            
            
            write(fd, input, strlen(input));
            nread = read(fd, svMsg, sizeof(svMsg));
            svMsg[nread] = '\0';

            //Imprime no ecra resposta do servidor
            printf("%s", svMsg);

        }else
            printf("Bad input\n");

        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        
    }

    close(fd);
    return 0;
}