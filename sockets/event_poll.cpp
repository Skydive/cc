
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

#include <fcntl.h>
#include <sys/epoll.h> // epoll

#include "basic_client.hpp"  

void handle_client(int client_sockfd) {
    // Receive
    char buf[1024];
    int len = -1;
    while(true) {
        if((len = recv(client_sockfd, buf, sizeof(buf), 0)) == 0) {
            //Somebody disconnected , get his details and print
            struct sockaddr_in address;
            socklen_t addrlen = sizeof(address);
            getpeername(client_sockfd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
            printf("\033[1;36m[SERVER] Client disconnected: %s:%d\033[0m\n", inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
            // Close
            close(client_sockfd);
            return;
        } else {
            if(len < 0) {
                if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                    break;
                } else {
                    perror("recv");
                    break;
                }
            }

            // Print
            buf[len] = '\0';
            printf("[SERVER] Recv: %s\n", buf);

            // Send
            if(send(client_sockfd, buf, len, 0) < 0) {
                perror("send");
                return;
            }
        }   
    }
}

int main(int argc, char** argv) {
    // Fork and split
    for(int i=0; i<4; i++) {
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


    fcntl(sockfd, F_SETFL, O_NONBLOCK); // manipulate file descriptor flags
    if(sockfd < 0) {
        perror("fcntl");
        return 1;
    }

    int epoll_fd = epoll_create1(0);
    if(epoll_fd < 0) {
        perror("epoll_create");
        return 1;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = sockfd;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &ev) < 0) {
        perror("epoll_ctl");
        return 1;
    }

    while(true) {
        constexpr int MAX_EPOLL_EVENTS_PER_RUN = 5;
        constexpr int EPOLL_RUN_TIMEOUT = 1000;
        struct epoll_event events[MAX_EPOLL_EVENTS_PER_RUN];
        int nfds = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS_PER_RUN, -1);
        if(nfds < 0) {
            perror("epoll_wait");
            return 1;
        }
        for(int i=0; i<nfds; i++) {
            int fd = events[i].data.fd;
            if(fd == sockfd) {
                while(true) {
                    struct sockaddr_in client_addr;
                    socklen_t client_addr_len = sizeof(client_addr);
                    int client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
                    if(client_sockfd < 0) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            break;
                        } else {
                            perror ("accept");
                            break;
                        }
                    }

                    // Print client address
                    printf("\033[1;36m[SERVER] Connection from %s:%d\033[0m\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    fcntl(client_sockfd, F_SETFL, O_NONBLOCK);
                    if(client_sockfd < 0) {
                        perror("fcntl");
                        return 1;
                    }
                    // Add to epoll with send/recv events
                    struct epoll_event ev;
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = client_sockfd;
                    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sockfd, &ev) < 0) {
                        perror("epoll_ctl");
                        return 1;
                    }
                }
            } else {
                handle_client(fd);
            }
        }
    }

    return 0;
}