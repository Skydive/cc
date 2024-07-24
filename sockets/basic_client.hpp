
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

int launch_client() {
    pid_t pid = getpid();

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);

    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        return 1;
    }

    for(int i=0; i<10; i++) {
        // Send with PID
        char buf_p[1024];
        char buf_n[1024];
        char* msg = "Hello!";
        sprintf(buf_p, "\033[1;32m[%d] Send: %s i=%d\033[0m", pid, msg, i);
        sprintf(buf_n, "%s from [%d] i=%d", msg, pid, i);
        printf("%s\n", buf_p);
        if(send(sockfd, buf_n, strlen(buf_n), 0) < 0) {
            perror("send");
            return -1;
        }
        sleep(1);

        char buf[1024];
        int len = -1;
        while(len < 0) {
            len = recv(sockfd, buf, sizeof(buf), 0);
            if(len < 0) {
                perror("recv");
                return -1;
            }
            buf[len] = '\0';
            printf("\033[1;32m[%d] Response: %s\033[0m\n", pid, buf);
        }
    }
    close(sockfd);
    printf("\033[1;32m[%d] Connection closed!\033[0m\n", pid);

    return 0;
}