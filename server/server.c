#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define PORT 9001
#define BUFFER_SIZE 1024

void execute_command(const char* command, char* result, size_t result_size) {
    if (strlen(command) == 0) {
        snprintf(result, result_size, "Error: comando vacío.\n");
        return;
    }

    // Manejo especial para `top` (modo batch)
    if (strncmp(command, "top", 3) == 0) {
        snprintf(result, result_size, "Ejecutando top...\n");
        execlp("top", "top", "-b", "-n", "1", NULL); // Modo batch, una sola instantánea
        exit(EXIT_FAILURE); // Termina si execlp falla
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        snprintf(result, result_size, "Error al crear el pipe: %s\n", strerror(errno));
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        snprintf(result, result_size, "Error al crear el proceso: %s\n", strerror(errno));
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (pid == 0) { // Proceso hijo
        close(pipefd[0]); // Cierra lectura del pipe
        dup2(pipefd[1], STDOUT_FILENO); // Redirige stdout al pipe
        dup2(pipefd[1], STDERR_FILENO); // Redirige stderr al pipe
        close(pipefd[1]); // Cierra escritura del pipe

        execlp("sh", "sh", "-c", command, NULL); // Ejecuta el comando
        exit(EXIT_FAILURE); // Termina si execlp falla
    } else { // Proceso padre
        close(pipefd[1]); // Cierra escritura del pipe
        wait(NULL); // Espera a que el hijo termine

        ssize_t bytesRead = read(pipefd[0], result, result_size - 1);
        if (bytesRead > 0) {
            result[bytesRead] = '\0'; // Asegura terminación de cadena
        } else {
            snprintf(result, result_size, "Comando ejecutado, pero sin salida.\n");
        }
        close(pipefd[0]); // Cierra lectura del pipe
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
    char cwd[BUFFER_SIZE]; // Para guardar y mostrar el directorio actual

    while (1) {
        memset(command, 0, sizeof(command));
        int bytesReceived = recv(clientSocket, command, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            command[bytesReceived] = '\0';

            if (strcmp(command, "salida") == 0) {
                printf("Cliente desconectado.\n");
                break;
            }

            // Manejo especial para el comando "cd"
            if (strncmp(command, "cd", 2) == 0) {
                char* path = command + 3; // Obtiene el path después de "cd "
                path[strcspn(path, "\n")] = '\0'; // Elimina el salto de línea

                if (chdir(path) == 0) {
                    snprintf(result, sizeof(result), "Directorio cambiado a: %s\n", getcwd(cwd, sizeof(cwd)));
                } else {
                    snprintf(result, sizeof(result), "Error al cambiar directorio: %s\n", strerror(errno));
                }
            } else {
                printf("Ejecutando comando: %s\n", command);
                memset(result, 0, sizeof(result));
                execute_command(command, result, sizeof(result));
            }

            send(clientSocket, result, strlen(result), 0);
        }
    }

    close(clientSocket);
    close(servSockD);
    return 0;
}
