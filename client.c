#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define SEC 10

int main() {
    int clientSocket;
    struct sockaddr_in serverAddress;

    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Eroare la crearea socket-ului");
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &serverAddress.sin_addr) <= 0) {
        perror("Adresa serverului invalida");
        exit(EXIT_FAILURE);
    }

    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Eroare la conectarea la server");
        exit(EXIT_FAILURE);
    }

    char message[1024];
    while (1) {
        printf("Introduceti un mesaj ('quit' pentru a iesi): ");
        fgets(message, sizeof(message), stdin);

        send(clientSocket, message, strlen(message), 0);

        if (strncmp(message, "quit", 4) == 0) {
            break;
        }

        char serverResponse[1024];
        memset(serverResponse, 0, sizeof(serverResponse));
        recv(clientSocket, serverResponse, sizeof(serverResponse), 0);
        printf("Raspuns: %s\n", serverResponse);

        sleep(SEC);
    }
    close(clientSocket);

    return 0;
}