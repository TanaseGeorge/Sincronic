#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT 8080
#define MAX_CLIENTS 5
#define MIN 3

void handleClient(int clientSocket, int* clients, int* numReceivedMessages, char mesaje[MAX_CLIENTS][1024]) {
    char buffer[1024] = {0};
    int bytesReceived;
    int M = 2;

    memset(buffer, 0, sizeof(buffer));
    bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (bytesReceived <= 0) {
        close(clientSocket);
        return;
    }

    strncpy(mesaje[*numReceivedMessages], buffer, 1023);
    mesaje[*numReceivedMessages][1023]='\0';

    clients[*numReceivedMessages] = clientSocket;
    (*numReceivedMessages)++;

    if (*numReceivedMessages >= MIN) {
        int messagesMatch = 1;

        for(int i=0; i < *numReceivedMessages-1; i++){

            for(int j=0; j<strlen(mesaje[i]); j++){
                if(mesaje[i][j]!=mesaje[i+1][j]){
                    messagesMatch=0;
                    break;
                }
            }

            if(!messagesMatch)
                break;
        }



        if (messagesMatch) {
            char *response = "continua";
            for (int i = 0; i < *numReceivedMessages; i++) {
                send(clients[i], response, strlen(response), 0);
            }
        } else {
            char *response = "gata";
            for (int i = 0; i < *numReceivedMessages; i++) {
                send(clients[i], response, strlen(response), 0);
                close(clients[i]);
            }
        }

        *numReceivedMessages = 0;
    }
}



int main() {
    int server_fd, client_sockets[MAX_CLIENTS];
    struct sockaddr_in address;
    socklen_t addrlen;
    addrlen = sizeof(address);
    fd_set readfds;
    int max_sd, activity;
    int numReceivedMessages = 0;
    int clients[MAX_CLIENTS];
    char mesaje[MAX_CLIENTS][1024] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Eroare la crearea socket-ului");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Eroare");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Eroare");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("Eroare");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(server_fd, &readfds)) {
            int new_socket;
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
                perror("Eroare");
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    clients[i] = new_socket;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (FD_ISSET(sd, &readfds)) {
                handleClient(sd, clients, &numReceivedMessages, mesaje);
            }
        }
    }

    close(server_fd);

    return 0;
}