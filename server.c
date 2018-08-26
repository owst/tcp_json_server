#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define guard(n, what) if (n < 0) { perror(what); exit(1); }

int child_count = 0;

int write_message_in_chunks(int fd, const char *message, int len, int sleep) {
    int res, written, pos;

    written = 0;

    for (pos = 0; pos < (len / 2) * 2; pos += 2) {
        res = write(fd, &message[pos], 2);
        if (res < 0) { return -written; }
        written += res;

        usleep(sleep);
    }

    if (len % 2 == 1) {
        res = write(fd, &message[pos], 1);
        if (res < 0) { return -written; }
        written += res;
    }

    return written;
}

void accept_and_handle_client_connection(int server_sock) {
    int client_sock, res, client_rand_sleep;
    struct sockaddr_in client;
    socklen_t client_size;
    char client_ip[INET_ADDRSTRLEN];
    // 22: 15 for IPv4 address, 1 for colon, 6 for port <= 65535, 1 for NULL
    char client_addr_and_port[22];
    pid_t child_pid;

    client_size = sizeof client;

    client_sock = accept(server_sock, (struct sockaddr *)&client, &client_size);
    guard(client_sock, "accept");
    client_rand_sleep = rand() % 10000;

    inet_ntop(AF_INET, &client.sin_addr, client_ip, sizeof client_ip);
    snprintf(client_addr_and_port, sizeof(client_addr_and_port) - 1, "%s:%d", client_ip, ntohs(client.sin_port));
    printf("Accepted new client: %s with sleep %d\n", client_addr_and_port, client_rand_sleep);

    child_pid = fork();
    guard(child_pid, "fork");

    if (child_pid == 0) {
        guard(close(server_sock), "close(server_sock) in child");

        for(int loop = 0; loop < 3; loop++) {
            char message[64];
            snprintf(message, 63, "{\"iteration\":%d,\"client\":\"%s\"}", loop, client_addr_and_port);

            // Include NULL byte (with strlen + 1) to delimit messages
            res = write_message_in_chunks(client_sock, message, strlen(message) + 1, client_rand_sleep);
            printf("Written %d to client %s\n", res, client_addr_and_port);
            if (res < 0) { break ; }
        }

        guard(close(client_sock), "close(client_sock) in child");

        _Exit(0);
    } else {
        child_count++;

        printf("Handling client %s in child PID %d of parent with PID %d\n", client_addr_and_port, child_pid, getpid());
        guard(close(client_sock), "close(client_sock) in parent");
    }
}

void loop_waitpid(options) {
    pid_t childpid;

    while ((childpid = waitpid(-1, NULL, options)) > 0) {
        printf("Child with PID %d exited\n", childpid);
        child_count--;
    }
}

void sigchld_handler(int signum)
{
    loop_waitpid(WNOHANG);
}

int main(int argc, char* argv[]) {
    srand(time(NULL));

    int server_sock;
    struct sockaddr_in server;

    // Ignore SIGPIPE (e.g. when the remote end closes the connection before we
    // try to write)
    signal(SIGPIPE, SIG_IGN);
    // Reap exited child processes
    signal(SIGCHLD, sigchld_handler);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    guard(server_sock, "socket");

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(2048);

    guard(bind(server_sock, (struct sockaddr *)&server, sizeof(struct sockaddr_in)), "bind");
    guard(listen(server_sock, 2), "listen");
    printf("Server listening on port 2048\n");

    for(;;) {
        accept_and_handle_client_connection(server_sock);
    }

    guard(close(server_sock), "close (server)");

    // Wait for any remaining children
    if (child_count > 0) {
        loop_waitpid(0);
    }

    return 0;
}
