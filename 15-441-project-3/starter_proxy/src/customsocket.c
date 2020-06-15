/********************************************************
*  Author: Sannan Tariq                                *
*  Email: stariq@cs.cmu.edu                            *
*  Description: This code provides funcitions          *
*                for creating simple sockets           *
*  Bug Reports: Please send email with                 *
*              subject line "15441-proxy-bug"          *
*  Complaints/Opinions: Please send email with         *
*              subject line "15441-proxy-complaint".   *
*              This makes it easier to set a rule      *
*              to send such mail to trash :)           *
********************************************************/
#include "customsocket.h"

/*
Opens a listen socket 
*/
int open_listen_socket(unsigned short port) {
    int listen_fd;
    int opt_value = 1;
    struct sockaddr_in server_address;

    /* Create a socket file descriptor */
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Failed to open listen socket\n");
        return -1;
    }

    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, 
		   (const void *)&opt_value , sizeof(int)) < 0) {
        fprintf(stderr, "open_listen_socket: Failed to set SO_REUSEADDR socket option\n");
        return -1;
    }
	
    bzero((char *)&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET; 
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); 
    server_address.sin_port = htons(port); 

    if (bind(listen_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        fprintf(stderr, "open_listen_socket: Failed to bind listening socket to port %hu\n", port);
        return -1;
    }

    if (listen(listen_fd, 5) < 0) {
        fprintf(stderr, "open_listen_socket: Failed to listen on port %hu\n", port);
    }
	
    return listen_fd;
}


/*
Opens a connection to server_ip and binds it to my_ip on the local machine
*/
int open_socket_to_server(char *my_ip, char *server_ip, unsigned short port) {
    int server_sock;
    struct sockaddr_in my_addr;
    struct sockaddr_in server_address;

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    bzero((char *)&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    if (inet_aton(my_ip, &(my_addr.sin_addr)) == 0) {
        fprintf(stderr, "open_socket_to_server: conversion failure: %s\n", strerror(errno));
        return -1;
    }
    my_addr.sin_port = 0;
    if (bind(server_sock, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
        fprintf(stderr, "open_socket_to_server: binding failiure: %s\n", strerror(errno));
        return -1;
    }

    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET; 
    inet_pton(AF_INET, server_ip, &(server_address.sin_addr)); 
    server_address.sin_port = htons(port); 
    if (connect(server_sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        fprintf(stderr, "open_socket_to_server: connect failed\n");
        return -1;
    }
    return server_sock;
}

// int main() {
//     open_socket_to_server("127.0.0.1", "127.0.0.1", 10000);
//     return 0;
// }