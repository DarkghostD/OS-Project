#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 9001
#define BUFFER_SIZE 1024

void receive_response(int sockD) {
    char buffer[BUFFER_SIZE];
    while (1) {
        int bytesReceived = recv(sockD, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) break;

        buffer[bytesReceived] = '\0'; // Asegurar terminación de cadena
        printf("%s", buffer);

        if (bytesReceived < sizeof(buffer) - 1) break; // Si la respuesta está completa
    }
    printf("\n");
}

int main() {
    int sockD = socket(AF_INET, SOCK_STREAM, 0);
    if (sockD == -1) {
        perror("Error al crear el socket del cliente");
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

    printf("Conectado al servidor en el puerto %d\n", PORT);

    char command[BUFFER_SIZE];
    while (1) {
        printf("Comando> ");
        fgets(command, sizeof(command), stdin);

        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n') {
            command[len - 1] = '\0'; // Remover el salto de línea
        }

        send(sockD, command, strlen(command), 0);

        if (strcmp(command, "salida") == 0) {
            printf("Desconectando del servidor...\n");
            break;
        }

        printf("Respuesta del servidor:\n");
        receive_response(sockD);
    }

    close(sockD);
    return 0;
}
