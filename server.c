#include "server_header.h"

FILE* conf_file;

void error(char *str) {
	perror(str);
	exit(-1);
}


void read_config_file(char *file_name){

    conf_file = fopen(file_name, "r+");
    if(conf_file == NULL)
        error("ERROR OPENING CONFIGURATION FILE\n");


    char line[256];
    int i = 0;
     while(fgets(line, 256, conf_file)) {
        line[strcspn(line, "\n")] = 0;
        char* token = strtok(line, ";");
        int count = 0; 

        while(token != NULL) {
            if(count == 0) 
                strcpy(users_array[i].username, token);
            if(count == 1) 
                strcpy(users_array[i].password, token);
            if(count == 2) 
                strcpy(users_array[i].type, token);

            count++;
            token = strtok(NULL, ";");
        }

        i++;
    }

    fclose(conf_file);
}

void init() {

    users_array = malloc(sizeof(User)*USERS_AMOUNT + sizeof(char*)*3);
	for(int i = 0; i < USERS_AMOUNT; ++i) {
		users_array[i].socketfd = 0;
        users_array[i].username = malloc(sizeof(char*));
		strcpy(users_array[i].username, "");
        users_array[i].password = malloc(sizeof(char*));
		strcpy(users_array[i].password, "");
        users_array[i].type = malloc(sizeof(char*));
		strcpy(users_array[i].type, "");
        
        
    }
}



void *udp(){


}

void *tcp(){

    struct sockaddr_in addr, client_addr;
    int tcp_fd, client_addr_size;

    bzero((void *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(tcp_port);

    if ( (tcp_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("na funcao socket");
    if ( bind(tcp_fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
        error("na funcao bind");
    if( listen(tcp_fd, 5) < 0)
        error("na funcao listen");

    client_addr_size = sizeof(client_addr);

    while (1) {
        //clean finished child processes, avoiding zombies
        //must use WNOHANG or would block whenever a child process was working
        while(waitpid(-1,NULL,WNOHANG)>0);

        //wait for new connection
        tcp_client_fd = accept(tcp_fd,(struct sockaddr *)&client_addr,(socklen_t *)&client_addr_size);

        if (tcp_client_fd > 0) {
        if (fork() == 0) {
            close(tcp_fd);
            process_client(tcp_client_fd);
            exit(0);
        }
        close(tcp_client_fd);
        }
  }
}

int get_user(char* user_input) {

    char username[64], password[64];
    int i = 0;
    char* token = strtok(user_input, " ");
    
    strcpy(username, token);
    token = strtok(NULL, " ");
    strcpy(password, token);

    for(i = 0; i < USERS_AMOUNT; ++i) {
        if(strcmp(username, users_array[i].username) == 0) {
            if(strcmp(password, users_array[i].password) == 0) {
                return i;
            }
       }
    }

    return -1;
}

void process_client(int client_fd)
{
	int nread = 0, user, size;
	char buffer[BUF_SIZE], name[64], action[20], classSize[4], subTxt[50];

    char msg[BUF_SIZE] = "Login: <username> <password>\n";
    write(client_fd, msg, strlen(msg));

    bzero((void*) buffer, strlen(buffer));
    nread = read(client_fd, buffer, BUF_SIZE-1);
    buffer[nread] = '\0';
    
    user = get_user(buffer);
    
    if(user == -1){

        write(client_fd, "REJECTED\n", 9);
        return;

    }else
        write(client_fd, "OK\n", 3);


    char* userType = users_array[user].type;
    users_array[user].socketfd = tcp_client_fd;

    do {

        memset(buffer,0,strlen(buffer));
        
        bzero((void*) buffer, strlen(buffer));
        nread = read(client_fd, buffer, BUF_SIZE-1);
        buffer[nread] = '\0';

        if(strcmp(buffer, "LIST_CLASSES") == 0){

            write(users_array[user].socketfd, "Class\n", 6);
        }

        if(strcmp(buffer, "LIST_SUBSCRIBED") == 0){

            write(users_array[user].socketfd, "Class\n", 6);
        }

        char* token = strtok(buffer, " ");
        strcpy(action, token);

        if(strcmp(action, "SUBSCRIBE_CLASS") == 0){

            token = strtok(NULL, " ");
            strcpy(name, token);

            write(users_array[user].socketfd, "ACCEPTED/REJECTED\n", 18);
        }


        if(strcmp(userType, "professor") == 0){

            if(strcmp(action, "CREATE_CLASS") == 0){

                token = strtok(NULL, " ");
                strcpy(name, token);
                token = strtok(NULL, " ");
                strcpy(classSize,token);
                size = atoi(classSize);

                write(client_fd, "OK\n", 3);
            }

            if(strcmp(action, "SEND") == 0){

                token = strtok(NULL, " ");
                strcpy(name, token);
                token = strtok(NULL, " ");
                strcpy(subTxt,token);

                write(client_fd, "SEND\n", 5);

            }

        }


    } while (nread>0);
}

int main(int argc, char *argv[]) {


    if(argc != 4) 
        error("class_server <PORTO_TURMAS> <PORTO_CONFIG> <FICHEIRO CONFIGURACAO>");

    pthread_t threads[2];

    //Signal handling
   // signal(SIGINT, sigint);

    tcp_port = atoi(argv[1]);
    udp_port = atoi(argv[2]);

	init();
	read_config_file(argv[3]);

    pthread_create(&threads[0], NULL, udp, NULL);
    pthread_create(&threads[1], NULL, tcp, NULL);

    //Wait for all threads to finish
	for(int i = 0; i < 2; ++i) {
		pthread_join(threads[i], NULL);
	}

  return 0;
}

//Assim que o programa recebe Ctrl-C, fecha todas as streams abertas,
//espera que todos os processos terminem e termina o programa do servidor
/* void sigint(int signum) {
  printf("\n");
  close(fd);
  close(client);
  wait(NULL);
  exit(0);
} */