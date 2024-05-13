#ifndef SERVER_HEADER_H_INCLUDED
#define SERVER_HEADER_H_INCLUDED

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>


#define USERS_AMOUNT 50
#define BUF_SIZE	512

int tcp_client_fd, udp_port, tcp_port;

int users_amount;

char *filename;

typedef struct user{
	int socketfd;
	char *username;
	char *password;
	char *type;
	struct user *next;
	struct user *prev;
	

}User, *pUser;

User* users_array;

pUser user_list_head;
pUser user_list_tail;

void error(char *str);
void read_config_file();
void save_config_file();
void free_node(pUser node);
void free_list();
void *udp();
void *tcp();
void process_client(int client_fd);
int validate_user(char *username);
pUser create_user(char *username, char *password, char *type);
void add_user(pUser new_user);
int del_user(char *username);
pUser get_user(char *username, char *password);
void send_msg_udp(char *msg, int udp_fd, socklen_t cliente_socket_len, struct sockaddr_in client_addr);

#endif
