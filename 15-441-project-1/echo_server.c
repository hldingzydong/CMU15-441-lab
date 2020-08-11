/******************************************************************************
* echo_server.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.  It does not support          *
*              concurrent clients.                                            *
*                                                                             *
* Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
*          Wolf Richter <wolf@cs.cmu.edu>                                     *
*                                                                             *
*******************************************************************************/

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "parse.h"

#define ECHO_PORT 9999              // the server port
#define BUF_SIZE 4096               // the buffer size
#define MAX_CLIENTS_NUMBER 1024     // the number of available ﬁle descriptors in the operating system
                                    // (the min of ulimit -n and FD SETSIZE—both typically 1024)
#define NO_SOCKET -1
#define LISTEN_SOCKET_QUNUE_SIZE 5

int listen_socket = NO_SOCKET;             // share data
int client_socket_fds[MAX_CLIENTS_NUMBER]; // share data
const char *BAD_REQUEST = "HTTP/1.1 400 Bad Request\\r\\n";

/**
 * helper functions
 */
int close_socket(int socket)
{
    if (close(socket))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

void shutdown_properly(int code)
{
    if(listen_socket != NO_SOCKET) {
        close_socket(listen_socket);
        listen_socket = NO_SOCKET;
    }

    for (int i = 0; i < MAX_CLIENTS_NUMBER; ++i) {
        if (client_socket_fds[i] != NO_SOCKET) {
            close_socket(client_socket_fds[i]);
            client_socket_fds[i] = NO_SOCKET;
        }
    }

    printf("Shutdown server properly.\n");
    exit(code);
}

void handle_signal_action(int sig_number)
{
    if (sig_number == SIGINT) {
        printf("SIGINT was catched!\n");
        shutdown_properly(EXIT_SUCCESS);
    } else if (sig_number == SIGPIPE) {
        printf("SIGPIPE was catched!\n");
        shutdown_properly(EXIT_SUCCESS);
    }
}

/**
 * setup functions
 * @return
 */
int setup_signals()
{
    struct sigaction sa;
    sa.sa_handler = handle_signal_action;
    if (sigaction(SIGINT, &sa, 0) != 0) {
        fprintf(stderr, "sigaction()\n");
        return -1;
    }
    if (sigaction(SIGPIPE, &sa, 0) != 0) {
        fprintf(stderr, "sigaction()\n");
        return -1;
    }

    return 0;
}

int setup_listening_socket() {
    struct sockaddr_in addr;

    if ((listen_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket.\n");
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_socket, (struct sockaddr *) &addr, sizeof(addr)))
    {
        close_socket(listen_socket);
        fprintf(stderr, "Failed binding socket.\n");
        return -1;
    }

    if (listen(listen_socket, LISTEN_SOCKET_QUNUE_SIZE))
    {
        close_socket(listen_socket);
        fprintf(stderr, "Error listening on socket.\n");
        return -1;
    }
    return 0;
}

void setup_timeout(struct timeval *timeout) {
    timeout->tv_sec  = 3 * 60;
    timeout->tv_usec = 0;
}

void setup_server() {
    if(setup_signals() != 0) {
        shutdown_properly(EXIT_FAILURE);
    }
    if(setup_listening_socket() != 0) {
        shutdown_properly(EXIT_FAILURE);
    }
    for(int i = 0; i < MAX_CLIENTS_NUMBER; i++) {
        client_socket_fds[i] = NO_SOCKET;
    }
}

/**
 * reconfigure socket fd for every cycle
 * @param read_fds
 * @return the max socket_fd
 */
int build_client_scoket(fd_set *read_fds) {
    int max_fd = listen_socket;

    FD_ZERO(read_fds);
    FD_SET(listen_socket, read_fds);
    for(int i = 0; i < MAX_CLIENTS_NUMBER; i++) {
        if(client_socket_fds[i] != NO_SOCKET) {
            FD_SET(client_socket_fds[i], read_fds);
            if(client_socket_fds[i] > max_fd) {
                max_fd = client_socket_fds[i];
            }
        }
    }
    return max_fd;
}

/**
 * A pair operate function to client_socket_fds[]
 * @param client_fd
 */
void set_client_fd(int client_fd) {
    // TODO mutex lock?
    int index = client_fd % MAX_CLIENTS_NUMBER;
    while(client_socket_fds[index] != NO_SOCKET) {
        index = (index + 1) % MAX_CLIENTS_NUMBER;
    }
    client_socket_fds[index] = client_fd;
}

void clear_client_fd(int client_fd) {
    // TODO mutex lock?
    int index = client_fd % MAX_CLIENTS_NUMBER;
    while(client_socket_fds[index] != client_fd) {
        index = (index + 1) % MAX_CLIENTS_NUMBER;
    }
    client_socket_fds[index] = NO_SOCKET;
}

/**
 * when listening socket receive a new connection
 */
int handle_new_connection() {
    int client_sock_fd;
    socklen_t cli_size;
    struct sockaddr_in cli_addr;

    cli_size = sizeof(cli_addr);
    if ((client_sock_fd = accept(listen_socket, (struct sockaddr *) &cli_addr,
                              &cli_size)) == -1)
    {
        fprintf(stderr, "Error accepting connection.\n");
        // TODO consider when accept() failed, it means need to shut down server entirely?
        shutdown_properly(EXIT_FAILURE);
        return -1;
    }
    set_client_fd(client_sock_fd);
    return 0;
}

/**
 * thread to receive client message
 * for now, just a echo handler
 */
void* handle_client_message(void* arg) {
    int* client_socket_fd = (int*)arg;
    char buffer[BUF_SIZE];

    int read_ret = 0;
    while((read_ret = recv(*client_socket_fd, buffer, BUF_SIZE, 0)) >= 1)
    {
        printf("message length = %d\n", read_ret);
        fprintf(stdout, "server received message = %s\n", buffer);
        Request* request = parse(buffer, read_ret, *client_socket_fd);
        if(request == NULL) {
            strncpy(buffer, BAD_REQUEST, strlen(BAD_REQUEST));
            read_ret = (int)strlen(BAD_REQUEST);
        }
        if (send(*client_socket_fd, buffer, read_ret, 0) != read_ret)
        {
            close_socket(*client_socket_fd);
            clear_client_fd(*client_socket_fd);
            fprintf(stderr, "Error sending to client.\n");
            return NULL;
        }
        memset(buffer, 0, BUF_SIZE);
    }

    if (read_ret == -1)
    {
        close_socket(*client_socket_fd);
        clear_client_fd(*client_socket_fd);
        fprintf(stderr, "Error reading from client socket.\n");
    }
    return NULL;
}

/**
 * liso server entrance
 */
int main(int argc, char* argv[])
{
    struct timeval timeout;
    fd_set read_fds;

    fprintf(stdout, "----- Echo Server -----\n");
    setup_server();
    setup_timeout(&timeout);

    /* finally, loop waiting for input and then write it back */
    while (1)
    {
       int max_fd = build_client_scoket(&read_fds);

       int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

       if(activity == -1) {
            fprintf(stderr, "server meet unknown error.\n");
       } else if (activity == 0) {
            fprintf(stderr, "server timeout(3 minutes).\n");
       } else {
            if(FD_ISSET(listen_socket, &read_fds)) {
                if(handle_new_connection() < 0) {
                    return EXIT_FAILURE;
                }
            }

            for(int i = 0; i < MAX_CLIENTS_NUMBER; i++) {
                // for every readable socket fd start thread for each one
                // and deal with the message respectively
                if(FD_ISSET(client_socket_fds[i], &read_fds)) {
                    // not use multiple thread now
                    handle_client_message(&client_socket_fds[i]);
                }
            }
       }

    }
    return EXIT_SUCCESS;
}
