/*
 * Tiny TCP proxy server
 *
 * Author: Krzysztof Kliś <krzysztof.klis@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version with the following modification:
 *
 * As a special exception, the copyright holders of this library give you
 * permission to link this library with independent modules to produce an
 * executable, regardless of the license terms of these independent modules,
 * and to copy and distribute the resulting executable under terms of your choice,
 * provided that you also meet, for each linked independent module, the terms
 * and conditions of the license of that module. An independent module is a
 * module which is not derived from or based on this library. If you modify this
 * library, you may extend this exception to your version of the library, but
 * you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <arpa/inet.h>
#include <errno.h>
#include <libgen.h>
#include <netdb.h>
#include <resolv.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <wait.h>

#define BUF_SIZE 1024

typedef enum {TRUE = 1, FALSE = 0} bool;

int create_socket(int port);
void sigchld_handler(int signal);
void sigterm_handler(int signal);
void server_loop();
void handle_client(int client_sock, struct sockaddr_in client_addr);
void forward_data(int source_sock, int destination_sock);

int server_sock, client_sock, remote_sock, remote_port;
char *remote_host;

/* Program start */
int main(int argc, char *argv[]) {
    int local_port;
    pid_t pid;
    struct sigaction sa;

    if (argc < 4) {
        printf("Syntax: %s local_port remote_host remote_port\n", argv[0]);
        return 0;
    }

    local_port = atoi(argv[1]);
    remote_host = argv[2];
    remote_port = atoi(argv[3]);

    if ((server_sock = create_socket(local_port)) < 0) { // start server
        perror("Cannot run server");
        return server_sock;
    }

    signal(SIGCHLD, sigchld_handler); // prevent ended children from becoming zombies
    signal(SIGTERM, sigterm_handler); // handle KILL signal

    switch(pid = fork()) {
        case 0:
            server_loop(); // daemonized child
            break;
        case -1:
            perror("Cannot daemonize");
            return pid;
        default:
            close(server_sock);
    }

    return 0;
}

/* Create server socket */
int create_socket(int port) {
    int server_sock;
    struct sockaddr_in server_addr;

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        return -1;
    }

    if (listen(server_sock, 20) < 0) {
        return -1;
    }

    return server_sock;
}

/* Handle finished child process */
void sigchld_handler(int signal) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/* Handle term signal */
void sigterm_handler(int signal) {
    close(client_sock);
    close(server_sock);
    exit(0);
}

/* Main server loop */
void server_loop() {
    struct sockaddr_in client_addr;
    int addrlen = sizeof(client_addr);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addrlen);
        if (fork() == 0) { // handle client connection in a separate process
            close(server_sock);
            handle_client(client_sock, client_addr);
            exit(0);
        }
        close(client_sock);
    }

}

/* Handle client connection */
void handle_client(int client_sock, struct sockaddr_in client_addr)
{
    if ((remote_sock = create_connection()) < 0) {
        perror("Cannot connect to host");
        return;
    }

    if (fork() == 0) { // a process forwarding data from client to remote socket
        forward_data(client_sock, remote_sock);
        exit(0);
    }

    if (fork() == 0) { // a process forwarding data from remote socket to client
        forward_data(remote_sock, client_sock);
        exit(0);
    }

    close(remote_sock);
    close(client_sock);
}

/* Forward data between sockets */
void forward_data(int source_sock, int destination_sock) {
    char buffer[BUF_SIZE];
    int n;

    while ((n = recv(source_sock, buffer, BUF_SIZE, 0)) > 0) {
        send(destination_sock, buffer, n, 0);
    }

    shutdown(destination_sock, SHUT_RDWR); // stop other processes from using socket
    close(destination_sock);

    shutdown(source_sock, SHUT_RDWR); // stop other processes from using socket
    close(source_sock);
}

/* Create client connection */
int create_connection() {
    struct sockaddr_in server_addr;
    struct hostent *server;
    int sock;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -2;
    }

    if ((server = gethostbyname(remote_host)) == NULL) {
        errno = EFAULT;
        return -3;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = htons(remote_port);

    if (connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        return -4;
    }

    return sock;
}
