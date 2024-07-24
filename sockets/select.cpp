
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

#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <sys/select.h> // select function

#include "basic_client.hpp"  

void handle_client(int* client_sockfd, int sock_idx) {
    // Receive
    char buf[1024];
    int len = -1;
    if((len = recv(client_sockfd[sock_idx], buf, sizeof(buf), 0)) == 0) {
        //Somebody disconnected , get his details and print
        struct sockaddr_in address;
        socklen_t addrlen = sizeof(address);
        getpeername(client_sockfd[sock_idx] , (struct sockaddr*)&address , (socklen_t*)&addrlen);
        printf("\033[1;36m[SERVER] Client disconnected: %s:%d\033[0m\n", inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
        // Close
        close(client_sockfd[sock_idx]);
        client_sockfd[sock_idx] = 0;
    } else {
        if(len < 0) {
            perror("recv");
            return;
        }

        // Print
        buf[len] = '\0';
        printf("[SERVER] Recv: %s\n", buf);

        // Send
        if(send(client_sockfd[sock_idx], buf, len, 0) < 0) {
            perror("send");
            return;
        }
    }
        

    
}


int main(int argc, char** argv) {
    // Fork and split
    for(int i=0; i<5; i++) {
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

    // Create TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("socket");
        return 1;
    }

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

    //set master socket to allow multiple connections , this is just a good habit, it will work without this
    int opt = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt)) < 0) {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   


    //set of socket descriptors
    fd_set readfds;
    int max_clients = 32;
    int client_socket[32] = {0};


    // Accept Loop
    while(true) {
        FD_ZERO(&readfds);
        
        FD_SET(sockfd, &readfds); // Add master socket to set
        int max_sd = sockfd;
        
        for(int i=0; i<max_clients; i++) {
            int sd = client_socket[i];
            if(sd > 0)FD_SET(sd, &readfds);
            if(sd > max_sd)max_sd = sd;
        }
        int activity = select(max_sd+1, &readfds, NULL, NULL, NULL);
        if((activity < 0) && (errno!=EINTR)) {
            printf("select error");
        }

        if(FD_ISSET(sockfd, &readfds)) {
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

            // Add new socket to array of sockets
            for(int i=0; i<max_clients; i++) {
                if(client_socket[i] == 0) {
                    client_socket[i] = client_sockfd;
                    break;
                }
            }
        }

        for (int i = 0; i < max_clients; i++) {
            int sd = client_socket[i];
            if(FD_ISSET(client_socket[i], &readfds)) {
                handle_client(client_socket, i);
            }
        }
    }
    return 0;
}