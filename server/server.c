#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define PORT 9001
#define BUFFER_SIZE 1024

void execute_command(const char* command, char* result, size_t result_size) {
    if (strlen(command) == 0) {
        snprintf(result, result_size, "Error: comando vacío.\n");
        return;
    }

    FILE* fp = popen(command, "r");
    if (fp == NULL) {
        snprintf(result, result_size, "Error al ejecutar el comando: %s\n", strerror(errno));
        return;
    }

    memset(result, 0, result_size);
    char buffer[BUFFER_SIZE];
    size_t total_read = 0;

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t len = strlen(buffer);
        if (total_read + len < result_size) {
            strcat(result, buffer);
            total_read += len;
        } else {
            snprintf(result + total_read, result_size - total_read, "\n[Salida truncada]\n");
            break;
        }
    }

    if (pclose(fp) != 0) {
        snprintf(result, result_size, "Error: el comando no se ejecutó correctamente.\n");
    }
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
    char result[BUFFER_SIZE * 2];
    char current_dir[BUFFER_SIZE];

    // Inicializar el directorio actual como el directorio inicial del servidor
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        perror("Error al obtener el directorio actual");
        close(clientSocket);
        close(servSockD);
        exit(EXIT_FAILURE);
    }

    while (1) {
        memset(command, 0, sizeof(command));
        int bytesReceived = recv(clientSocket, command, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            command[bytesReceived] = '\0';

            if (strcmp(command, "salida") == 0) {
                printf("Cliente desconectado.\n");
                break;
            }

            printf("Ejecutando comando: %s\n", command);

            memset(result, 0, sizeof(result));

            // Manejo especial para el comando "cd"
            if (strncmp(command, "cd ", 3) == 0) {
                const char* new_dir = command + 3; // Obtener el argumento después de "cd "
                if (chdir(new_dir) == 0) {
                    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
                        snprintf(result, sizeof(result), "Error al obtener el directorio actual: %s\n", strerror(errno));
                    } else {
                        snprintf(result, sizeof(result), "Directorio cambiado a: %s\n", current_dir);
                    }
                } else {
                    snprintf(result, sizeof(result), "Error al cambiar al directorio '%s': %s\n", new_dir, strerror(errno));
                }
            } else {
                execute_command(command, result, sizeof(result));
            }

            send(clientSocket, result, strlen(result), 0);
        }
    }

    close(clientSocket);
    close(servSockD);
    return 0;
}
