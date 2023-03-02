#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT 8080

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    char *response = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n\r\n";

    // Cria o socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Define as opções do socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(int){1}, sizeof(int)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Define as informações do servidor
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Faz o bind do socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Escuta as conexões
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1)
    {
        printf("Entrou no while\n");
        // Espera a conexão de um cliente
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            printf("Recebeu conexao de um cliente\n");
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("Client connected\n");

        // Envia a resposta de handshake
        send(new_socket, response, strlen(response), 0);

        // Loop principal de recebimento de mensagens
        while (1)
        {
            char buffer[1024] = {0};
            int valread = read(new_socket, buffer, 1024);

            if (valread == 0)
            {
                printf("Client disconnected\n");
                break;
            }

            // Trata a mensagem recebida
            // ...

            // Envia a mensagem de resposta
            char response_buffer[1024] = "Response to ";
            strcat(response_buffer, buffer);
            send(new_socket, response_buffer, strlen(response_buffer), 0);
        }

        // Fecha o socket do cliente
        close(new_socket);
    }

    // Fecha o socket do servidor
    close(server_fd);

    return 0;
}
