#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include "base64_utils.h"

#define MAX_SIZE 4095
#define constswap16(x) ((((x)&0xFF) << 8) | (((x) >> 8) & 0xFF))  // add for big-end small-end transformation

char buf[MAX_SIZE+1];

// receiver: mail address of the recipient
// subject: mail subject
// msg: content of mail body or path to the file containing mail body
// att_path: path to the attachment
void send_mail(const char* receiver, const char* subject, const char* msg, const char* att_path)
{
    const char* end_msg = "\r\n.\r\n";
    const char* host_name = "smtp.163.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 25; // SMTP server port
    const char* user = "15071817852@163.com"; // TODO: Specify the user
    const char* pass = "CRPYNHMLHUEGEQSI"; // TODO: Specify the password
    const char* from = "15071817852@163.com"; // TODO: Specify the mail address of the sender
    char dest_ip[16]; // Mail server IP address
    int s_fd; // socket file descriptor
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    int r_size;

    // add for end input flag(maybe unused)
    char tbuf[MAX_SIZE+1];
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

    // TODO: Create a socket, return the file descriptor to s_fd, and establish a TCP connection to the mail server
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

    // Send EHLO command and print server response
    const char* EHLO = "EHLO 163.com\r\n"; // TODO: Enter EHLO command here
    send(s_fd, EHLO, strlen(EHLO), 0);
    // TODO: Print server response to EHLO command
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);
    // TODO: Authentication. Server response should be printed out.
    const char* AUTH = "AUTH login\r\n";
    send(s_fd, AUTH, strlen(AUTH), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);
        // user
    const char* user_base64 = encode_str(user);
    send(s_fd, user_base64, strlen(user_base64), 0);
    free(user_base64); // avoid memory leak
    // send(s_fd, end_flag, strlen(end_flag), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);
        // password
    const char* pass_base64 = encode_str(pass);
    send(s_fd, pass_base64, strlen(pass_base64), 0);
    free(pass_base64);
    // send(s_fd, end_flag, strlen(end_flag), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);
    // TODO: Send MAIL FROM command and print server response
    sprintf(tbuf, "MAIL FROM:<%s>\r\n", from);
    send(s_fd, tbuf, strlen(tbuf), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);
    // TODO: Send RCPT TO command and print server response
    sprintf(tbuf, "RCPT TO:<%s>\r\n", receiver);
    send(s_fd, tbuf, strlen(tbuf), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);
    // TODO: Send DATA command and print server response
    const char* DATA = "DATA\r\n";
    send(s_fd, DATA, strlen(DATA), 0);
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; 
    printf("%s", buf);
    // TODO: Send message data
        // 先写邮件头
    const char* msg_base64 = encode_str(msg); // 获取正文的Base64编码
    // 获取附件的Base64编码
    FILE* fin = fopen(att_path, "r");
    FILE* fout = fopen("temp_base64.txt", "w");
    encode_file(fin, fout);
    fclose(fin);
    fclose(fout);
    fout = fopen("temp_base64.txt", "r");
    char fbuf[MAX_SIZE];
    fgets(fbuf, MAX_SIZE, fout);
    fclose(fout);
    sprintf(tbuf, 
        "From: <%s>\r\n"
        "To: <%s>\r\n"
        "Subject: %s\r\n" 
        "MIME-Version: 1.0\r\n"
        "Content-Type: multipart/mixed; boundary=\"#BOUNDARY#\"\r\n\r\n"
        "--#BOUNDARY#\r\n"
        "Content-Type: text/plain; charset=gb2312\r\n"
        "Content-Transfer-Encoding: base64\r\n"
        "\r\n"
        "%s"
        "--#BOUNDARY#\r\n"
        "Content-Type: application/octet-stream; name=%s\r\n"
        "Content-Type: attachment; filename=%s\r\n"
        "Content-Transfer-Encoding: base64\r\n"
        "\r\n"
        "%s", from, receiver, subject, msg_base64, att_path, att_path, fbuf);
    send(s_fd, tbuf, strlen(tbuf), 0);
    free(msg_base64);
    // TODO: Message ends with a single period
    send(s_fd, end_msg, strlen(end_msg), 0);
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
    int opt;
    char* s_arg = NULL;
    char* m_arg = NULL;
    char* a_arg = NULL;
    char* recipient = NULL;
    const char* optstring = ":s:m:a:";
    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
        switch (opt)
        {
        case 's':
            s_arg = optarg;
            break;
        case 'm':
            m_arg = optarg;
            break;
        case 'a':
            a_arg = optarg;
            break;
        case ':':
            fprintf(stderr, "Option %c needs an argument.\n", optopt);
            exit(EXIT_FAILURE);
        case '?':
            fprintf(stderr, "Unknown option: %c.\n", optopt);
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Unknown error.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
    {
        fprintf(stderr, "Recipient not specified.\n");
        exit(EXIT_FAILURE);
    }
    else if (optind < argc - 1)
    {
        fprintf(stderr, "Too many arguments.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        recipient = argv[optind];
        send_mail(recipient, s_arg, m_arg, a_arg);
        exit(0);
    }
}
