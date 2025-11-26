#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    char buffer[256], c[2000], cc[20000];
    FILE *fd;

    if (argc < 2)
    {
        printf("Usage: %s <port>\nExample: %s 7777\n", argv[0], argv[0]);
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket creation failed");
        exit(1);
    }

    portno = atoi(argv[1]);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        close(sockfd);
        exit(1);
    }

    printf("UDP Server started... waiting for requests on port %d\n", portno);
    len = sizeof(cliaddr);

    bzero(buffer, sizeof(buffer));
    n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&cliaddr, &len);
    if (n < 0)
    {
        perror("recvfrom error");
        close(sockfd);
        exit(1);
    }

    buffer[n] = '\0';
    printf("Received file request for: %s\n", buffer);

    if ((fd = fopen(buffer, "r")) != NULL)
    {
        printf("File found. Reading and sending...\n");
        bzero(cc, sizeof(cc));
        while (fgets(c, sizeof(c), fd) != NULL)
        {
            strcat(cc, c);
        }
        fclose(fd);

        n = sendto(sockfd, cc, strlen(cc), 0, (struct sockaddr *)&cliaddr, len);
        if (n < 0)
            perror("sendto error");
        else
            printf("File sent successfully.\n");
    }
    else
    {
        printf("File not found: %s\n", buffer);
        n = sendto(sockfd, "Error: file not found", 22, 0, (struct sockaddr *)&cliaddr, len);
        if (n < 0)
            perror("sendto error");
    }

    close(sockfd);
    return 0;
}