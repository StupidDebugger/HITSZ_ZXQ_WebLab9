#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define MAX_SIZE 65535
#define constswap16(x) ((((x)&0xFF) << 8) | (((x) >> 8) & 0xFF))  // add for big-end small-end transformation

char buf[MAX_SIZE+1];

void recv_mail()
{
    const char* host_name = "pop.163.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 110; // POP3 server port
    const char* user = "15071817852@163.com"; // TODO: Specify the user
    const char* pass = "CRPYNHMLHUEGEQSI"; // TODO: Specify the password
    char dest_ip[16];
    int s_fd; // socket file descriptor
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    int r_size;

    const char* end_flag = "\r\n";
    // Get IP from domain name
    if ((host = gethostbyname(host_name)) == NULL)
    {
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    addr_list = (struct in_addr **) host->h_addr_list;
    while (addr_list[i] != NULL)
        ++i;
    strcpy(dest_ip, inet_ntoa(*addr_list[i-1]));

    // TODO: Create a socket,return the file descriptor to s_fd, and establish a TCP connection to the POP3 server
    s_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sockin;
    sockin.sin_family = AF_INET;
    sockin.sin_port = constswap16(port);
    sockin.sin_addr.s_addr = inet_addr(dest_ip); 
    bzero(sockin.sin_zero, sizeof(sockin.sin_zero));
    connect(s_fd, (struct sockaddr*)&sockin, sizeof(struct sockaddr));
    // Print welcome message
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);

    // TODO: Send user and password and print server response
        // user
    char* USER = "user ";
    send(s_fd, USER, strlen(USER), 0);
    send(s_fd, user, strlen(user), 0);
    send(s_fd, end_flag, strlen(end_flag), 0); 
    // USER = malloc((strlen("user ")+strlen(user)+strlen("\r\n"))*sizeof(char));
    // memcpy(USER, "user ", sizeof("user "));
    // USER = strcat(USER, user);
    // USER = strcat(USER, "\r\n");
    // send(s_fd, USER, strlen(USER), 0);
    // free(USER);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);
    
        // password
    char* PWD = "pass ";
    send(s_fd, PWD, strlen(PWD), 0);
    send(s_fd, pass, strlen(pass), 0);
    send(s_fd, end_flag, strlen(end_flag), 0); 
    // PWD = malloc((strlen("pass ")+strlen(pass)+strlen("\r\n"))*sizeof(char));
    // memcpy(PWD, "pass ", sizeof("pass "));
    // PWD = strcat(PWD, pass);
    // PWD = strcat(PWD, "\r\n");
    // send(s_fd, PWD, strlen(PWD), 0);
    // free(PWD);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);

    // TODO: Send STAT command and print server response
    const char* STAT = "STAT\r\n";
    send(s_fd, STAT, strlen(STAT), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);
    // TODO: Send LIST command and print server response
    const char* LIST = "LIST\r\n";
    send(s_fd, LIST, strlen(LIST), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);
    // TODO: Retrieve the first mail and print its content
    const char* RETR = "RETR 1\r\n";
    send(s_fd, RETR, strlen(RETR), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);
    // TODO: Send QUIT command and print server response
    const char* QUIT = "QUIT\r\n";
    send(s_fd, QUIT, strlen(QUIT), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);

    close(s_fd);
}

int main(int argc, char* argv[])
{
    recv_mail();
    exit(0);
}
