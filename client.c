#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 9001
#define BUFFER_SIZE 1024

int main() {
    int sockD = socket(AF_INET, SOCK_STREAM, 0);
    if (sockD == -1) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    if (connect(sockD, (struct sockaddr*)&servAddr, sizeof(servAddr)) == -1) {
        perror("Error al conectar con el servidor");
        close(sockD);
        exit(EXIT_FAILURE);
    }

    printf("Conectado al servidor. Escribe 'salida' para cerrar la conexión.\n");

    char command[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    while (1) {
        printf("Comando> ");
        fgets(command, BUFFER_SIZE, stdin);
        command[strcspn(command, "\n")] = '\0'; // Eliminar salto de línea

        if (strcmp(command, "salida") == 0) {
            send(sockD, command, strlen(command), 0);
            printf("Desconectando...\n");
            break;
        }

        send(sockD, command, strlen(command), 0);

        int bytesReceived = recv(sockD, response, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            response[bytesReceived] = '\0';
            printf("Respuesta del servidor:\n%s\n", response);
        } else {
            printf("No se recibió respuesta del servidor.\n");
        }
    }

    close(sockD);
    return 0;
}
