#ifndef NETWORK
#define NETWORK

#include "structs_n_main.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

void startup(int, char**, all_info*, ringfd*);
int add_read_fd(fd_set*, ringfd);
int init_UDPsv(all_info*);
int init_UDPcl(all_info*, struct addrinfo**);
int init_TCP_Listen(all_info*);
int init_TCP_connect(char*, char*);
void send_udp(int, char*, struct sockaddr*, socklen_t);
void recv_udp(int, char*, struct sockaddr*, socklen_t*);
void send_message(int, const char*);
int get_incoming(int);
int get_message(int,char*);
void close_all(ringfd*, all_info*);
int isAlive(int, fd_set*);

int Find_key(all_info,char*, ringfd);

#endif
