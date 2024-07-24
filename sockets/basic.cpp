
// Linux socket
#include <sys/socket.h>
#include <cstdio>
#include <cstdlib>

// hto
#include <arpa/inet.h>

// CPP thread
#include <thread>
#include <unistd.h>
// C strlen
#include <cstring>

#include <sys/prctl.h> // prctl(), PR_SET_PDEATHSIG
#include <signal.h> // signals

#include "basic_client.hpp"  

void handle_client(int client_sockfd) {
    // Receive
    char buf[1024];
    int len = -1;
    while(len != 0) {
        len = recv(client_sockfd, buf, sizeof(buf), 0);
        if(len < 0) {
            perror("recv");
            return;
        }

        // Print
        buf[len] = '\0';
        printf("[SERVER] Recv: %s\n", buf);

        // Send
        if(send(client_sockfd, buf, len, 0) < 0) {
            perror("send");
            return;
        }

        // Close
    }
    close(client_sockfd);
}

int main(int argc, char** argv) {
    // Create TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("socket");
        return 1;
    }
    // Fork and split
    for(int i=0; i<2; i++) {
        if(fork() > 0) {
            printf("[%d] Forked! Sleeping for %d seconds\n", getpid(), 1+1*i);
            // prctl(PR_SET_PDEATHSIG, SIGTERM);
            sleep(1+1*i);

            printf("[%d] Launched client...\n", getpid());

            launch_client();
            printf("[%d] Exit: Fork\n", getpid());
            return 0;
        } 
    }

    pid_t pid = getpid();
    printf("[SERVER] PID: %d\n", pid);

    // Bind
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    // Listen
    if(listen(sockfd, 5) < 0) {
        perror("listen");
        return 1;
    }

    // Multi conns
    int opt = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt)) < 0) {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   


    // Accept Loop
    while(true) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
        if(client_sockfd < 0) {
            perror("accept");
            return 1;
        }

        // Print client address
        char client_ip[16];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        printf("\033[1;36mAccept Client: %s:%d\033[0m\n", client_ip, ntohs(client_addr.sin_port));

        // Thread
        std::thread(handle_client, client_sockfd).detach();
    }
    return 0;
}