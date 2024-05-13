#include "server_header.h"

FILE* conf_file;

void show_user_list(){

    pUser temp_node = user_list_head;

    while(temp_node != NULL){

        printf("%s %s %s\n", temp_node->username, temp_node->password, temp_node->type);

        temp_node = temp_node->next;
    }

}

void error(char *str) {
	perror(str);
	exit(-1);
}

void add_user(pUser new_user){

    if(user_list_tail == NULL){

        user_list_head = new_user;
        user_list_tail = new_user;
    }else{

        user_list_tail->next = new_user;
        new_user->prev = user_list_tail;
        user_list_tail = new_user;
    }


}

void free_list(){

    pUser temp_node = user_list_head;

    while(temp_node != NULL){

        free_node(temp_node);
        temp_node = temp_node->next;
    }


}

void free_node(pUser node){

    free(node->type);
    free(node->password);
    free(node->username);
    free(node);


}

int del_user(char *username){

    pUser current_user = user_list_tail;
    
    //list is empty
    if(user_list_tail == NULL)
        return 0;

    //list with only one user
    if(user_list_head == user_list_tail){

        user_list_head = NULL;
        user_list_tail = NULL;
        free_node(current_user);
        return 1;
    }

    //delete last user of list
    if(!strcmp(user_list_tail->username, username)){

        user_list_tail = user_list_tail->prev;
        user_list_tail->next = NULL;
        free_node(current_user);
        return 1;
    }
    
    current_user = user_list_head;

    //delete 1st user of list
    if(!strcmp(current_user->username, username)){


        user_list_head = current_user->next;
        free_node(current_user);
        return 1;
    }

    pUser prev_user = current_user;
    current_user = current_user->next;    

    while(current_user != NULL){

        if(!strcmp(current_user->username, username)){

            prev_user->next = current_user->next;
            current_user->next->prev = prev_user;
            free_node(current_user);
            return 1;
        }

        current_user = current_user->next;
        prev_user = prev_user->next;
    }

    return 0;
}

//check if user already exists
int validate_user(char *username){

    pUser current_user = user_list_head;

    while(current_user != NULL){

        if(!strcmp(current_user->username, username))
            return 0;

        current_user = current_user->next;
    }

    return 1;
}

pUser create_user(char *username, char *password, char *type){

    pUser new_user = malloc(sizeof(struct user));
    new_user->username = malloc(sizeof(strlen(username + 1)));
    new_user->password = malloc(sizeof(strlen(password + 1)));
    new_user->type = malloc(sizeof(strlen(type + 1)));

    if(new_user->type == NULL){

        error("MEM ALOC FAILED\n");
        free(new_user->type);
        free(new_user->password);
        free(new_user->username);
        free(new_user);
        return NULL;
    }

    if(new_user->password == NULL){

        error("MEM ALOC FAILED\n");
        free(new_user->password);
        free(new_user->username);
        free(new_user);
        return NULL;
    }

    if(new_user->username == NULL){

        error("MEM ALOC FAILED\n");
        free(new_user->username);
        free(new_user);
        return NULL;
    }

    if(new_user == NULL){

        error("MEM ALOC FAILED\n");
        free(new_user);
        return NULL;
    }

    strcpy(new_user->username, username);
    strcpy(new_user->password, password);
    strcpy(new_user->type, type);
    new_user->socketfd = 0;
    new_user->next = NULL;
    new_user->prev = NULL;

    users_amount++;

    return new_user;

}


pUser get_user(char *username, char *password){

    pUser current_user = user_list_head;

    while(current_user != NULL){


        if(!strcmp(username, current_user->username)) {
            if(!strcmp(password, current_user->password)) {
                return current_user;
            }

        }

        current_user = current_user->next;
    }

    return NULL;
}


void save_config_file(){

    conf_file = fopen(filename, "w+");

    if(conf_file == NULL){
        error("ERROR SAVING TO CONFIGURATION FILE\n");
        return;
    }

    char line[256];
    pUser current_user = user_list_head;

    while(current_user != NULL){

        
        strcpy(line, current_user->username);
        strcat(line, ";");
        strcat(line, current_user->password);
        strcat(line, ";");
        strcat(line, current_user->type);
        
        fprintf(conf_file, "%s\n", line);

        memset(line, 0, strlen(line));
        current_user = current_user->next;
    }


    fclose(conf_file);
}

void read_config_file(){

    conf_file = fopen(filename, "r+");

    if(conf_file == NULL){
        error("ERROR OPENING CONFIGURATION FILE\n");
        return;
    }
        

    char line[BUF_SIZE];
    char username[64], password[64], type[15];


    while(fgets(line, BUF_SIZE, conf_file)) {

        line[strcspn(line, "\n")] = 0;

        if (sscanf(line, "%63[^;];%63[^;];%14s", username, password, type) == 3) {

            username[63] = '\0';
            password[63] = '\0';
            type[14] = '\0';


            if(!validate_user(username)){

                printf("Duplicated user in config file\n");
                continue;
            }


            pUser temp_user = create_user(username, password, type);
            
            if(temp_user != NULL)
                add_user(temp_user);


        }    

    }

    fclose(conf_file);
}


void send_msg_udp(char *msg, int udp_fd, socklen_t cliente_socket_len, struct sockaddr_in client_addr){

    if(sendto(udp_fd, msg, strlen(msg), 0, (struct sockaddr*)&client_addr,cliente_socket_len) == -1) 
        error("Error sending message.");
}

void *udp(){

    struct sockaddr_in server_addr, admin_addr;
    socklen_t admin_socket_len = sizeof(admin_addr);
    char  action[15], username[64], password[64], type[15];
    int udp_fd, recv_len;
    int user = 0, admin = 0;
    char input[BUF_SIZE];

    
    
    bzero((void*) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(udp_port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    
    if((udp_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) 
        error("Error creating udp socket");
    if(bind(udp_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) 
        error("Bind udp error");


    // login admin
    do{
        memset(input, 0, strlen(input));
        memset(action, 0, strlen(action));
        memset(username, 0, strlen(username));
        memset(password, 0, strlen(password));

        if((recv_len = recvfrom(udp_fd, input, BUF_SIZE, 0, (struct sockaddr *) &admin_addr, (socklen_t *)&admin_socket_len)) == -1) 
            error("Error in recvfrom");

        input[recv_len]='\0';

        if (sscanf(input, "%s %s %s",action, username, password)){
            if(!strcmp(action, "LOGIN")){

                if(strlen(username) == 0 || strlen(password) == 0){
                    send_msg_udp("<LOGIN> <username> <password>\n", udp_fd, admin_socket_len, admin_addr);
                    continue;
                }
                user = get_user(username, password);

                if(user == -1){
                    send_msg_udp("Login rejected.\n", udp_fd, admin_socket_len, admin_addr);
                    continue;
                }

                if(!strcmp(users_array[user].type, "administrator")){

                    send_msg_udp("Welcome!\n", udp_fd, admin_socket_len, admin_addr);
                    admin = 1;

                }else
                    send_msg_udp("Login rejected.\n", udp_fd, admin_socket_len, admin_addr);

            }
        }


    }while(!admin);


    while(admin) {
        
        memset(input, 0, strlen(input));
        memset(action, 0, strlen(action));
        memset(username, 0, strlen(username));
        memset(password, 0, strlen(password));
        memset(type, 0, strlen(type));

        if((recv_len = recvfrom(udp_fd, input, BUF_SIZE, 0, (struct sockaddr *) &admin_addr, (socklen_t *)&admin_socket_len)) == -1) 
            error("Error in recvfrom");

        
        input[recv_len]='\0';

        // add user
        if (sscanf(input, "%s %s %s %s", action, username, password, type)){
            if(!strcmp(action, "ADD_USER")){
                
                if(strlen(username) == 0 || strlen(password) == 0 || strlen(type) == 0){

                    send_msg_udp("<ADD_USER> <username> <password> <type>\n", udp_fd, admin_socket_len, admin_addr);    
                    continue;
                }

                if(!validate_user(username)){

                    send_msg_udp("Username already in use.\n", udp_fd, admin_socket_len, admin_addr);
                    continue;
                }


                pUser new_user = create_user(username, password, type);

                if(new_user != NULL){

                    add_user(new_user);
                    send_msg_udp("User created.\n", udp_fd, admin_socket_len, admin_addr);
                }
                    
                
            }
        }
        
        if (sscanf(input, "%s %s", action, username)) {
            if(!strcmp(action, "DEL")){
        
                if(strlen(username) == 0){

                    send_msg_udp("<DEL> <username>\n", udp_fd, admin_socket_len, admin_addr);    
                    continue;
                }

                if(del_user(username))
                    send_msg_udp("User deleted.\n", udp_fd, admin_socket_len, admin_addr);

            }
        }
        if (sscanf(input, "%s",action)){
            if(!strcmp(action, "LIST")){
                
                send_msg_udp("User list:\n", udp_fd, admin_socket_len, admin_addr);
            }

            if(!strcmp(action, "QUIT_SERVER")){
        
                send_msg_udp("Closing server...\n", udp_fd, admin_socket_len, admin_addr);
                close(udp_fd);
    
            }

        }
    }
}

/*void *tcp(){

    struct sockaddr_in addr, client_addr;
    int tcp_fd, client_addr_size;

    bzero((void *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(tcp_port);

    if ( (tcp_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("ERRO na funcao socket");
    if ( bind(tcp_fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
        error("ERRO na funcao bind");
    if( listen(tcp_fd, 5) < 0)
        error("ERRO na funcao listen");

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
}*/


/*void process_client(int client_fd){

	int nread = 0, user, size;
	char buffer[BUF_SIZE], name[64], action[20], classSize[4], subTxt[50];
    char username[64], password[64];

    char msg[BUF_SIZE] = "Login: <username> <password>\n";
    write(client_fd, msg, strlen(msg));

    do{

        bzero((void*) buffer, strlen(buffer));
        nread = read(client_fd, buffer, BUF_SIZE-1);
        buffer[nread] = '\0';
    

        if (sscanf(buffer, "%s %s", username, password)) {

            printf("%s\n", buffer);
            user = get_user(username, password);
        
            if(user == -1)
                write(client_fd, "REJECTED\n", 9);
            else
                write(client_fd, "OK\n", 3);

        }

    }while(user == -1);

    //---------- devolver ponteiro ------------
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

                write(users_array[user].socketfd, "OK\n", 3);
            }

            if(strcmp(action, "SEND") == 0){

                token = strtok(NULL, " ");
                strcpy(name, token);
                token = strtok(NULL, " ");
                strcpy(subTxt,token);

                write(users_array[user].socketfd, "SEND\n", 5);

            }

        }


    } while (nread>0);
}*/

int main(int argc, char *argv[]){


    if(argc != 4) 
        error("class_server <PORTO_TURMAS> <PORTO_CONFIG> <FICHEIRO CONFIGURACAO>");

    pthread_t threads[2];

    users_amount = 0;
    //Signal handling
   // signal(SIGINT, sigint);

    tcp_port = atoi(argv[1]);
    udp_port = atoi(argv[2]);

    filename = argv[3];

	read_config_file();

    //pthread_create(&threads[0], NULL, udp, NULL);
    //pthread_create(&threads[1], NULL, tcp, NULL);

 
    //Wait for all threads to finish
    /*
    for(int i = 0; i < 2; ++i) {
		pthread_join(threads[i], NULL);
	}
    */

    show_user_list();

    save_config_file();

    free_list();

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