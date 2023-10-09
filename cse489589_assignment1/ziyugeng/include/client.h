// client.h
#ifndef CLIENT_H
#define CLIENT_H

#define TRUE 1
#define MSG_SIZE 256
#define BUFFER_SIZE 256

int connect_to_host(char *server_ip, char *server_port);

void start_client(char *server_ip, char *server_port);


#endif

