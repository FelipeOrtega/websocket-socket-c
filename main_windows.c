#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8080

void handle_client(SOCKET client_fd)
{
    char buffer[1024] = {0};
    int valread = recv(client_fd, buffer, 1024, 0);

    if (valread == 0)
    {
        printf("Client disconnected\n");
        return;
    }

    // Verifica se a requisi��o � um handshake do WebSocket
    if (strstr(buffer, "Upgrade: websocket") == NULL ||
        strstr(buffer, "Connection: Upgrade") == NULL ||
        strstr(buffer, "Sec-WebSocket-Key:") == NULL)
    {
        printf("Invalid WebSocket handshake\n");
        return;
    }

    // Envia a resposta de handshake do WebSocket
    char *response = "HTTP/1.1 101 Switching Protocols\r\n"
                     "Upgrade: websocket\r\n"
                     "Connection: Upgrade\r\n"
                     "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n\r\n";
    send(client_fd, response, strlen(response), 0);

    printf("WebSocket handshake successful\n");

    // Recebe e processa as mensagens do WebSocket
    while (1)
    {
        // L� o cabe�alho da mensagem
        int header_size = 2;
        valread = recv(client_fd, buffer, header_size, 0);

        if (valread == 0)
        {
            printf("Client disconnected\n");
            break;
        }

        // Calcula o tamanho da mensagem
        int payload_length = buffer[1] & 0x7F;
        if (payload_length == 126)
        {
            header_size += 2;
            valread = recv(client_fd, buffer + 2, 2, 0);
            payload_length = ntohs(*(unsigned short *)(buffer + 2));
        }
        else if (payload_length == 127)
        {
            header_size += 8;
            valread = recv(client_fd, buffer + 2, 8, 0);
            unsigned long long payload_length64 = 0;
            memcpy(&payload_length64, buffer + 2, 8);
            payload_length = ntohl(payload_length64 >> 32);
        }

        // L� a m�scara da mensagem
        if ((buffer[1] & 0x80) == 0)
        {
            printf("Unmasked message received\n");
            break;
        }

        unsigned char mask[4];
        valread = recv(client_fd, (char *)mask, 4, 0);

        if (valread == 0)
        {
            printf("Client disconnected\n");
            break;
        }

        // L� o corpo da mensagem
        char *payload = malloc(payload_length + 1);
        valread = recv(client_fd, payload, payload_length, 0);
        payload[payload_length] = '\0';

        if (valread == 0)
        {
            printf("Client disconnected\n");
            free(payload);
            break;
        }
		
		int i;
        // Aplica a m�scara na mensagem
        for (i = 0; i < payload_length; i++)
        {
            payload[i] = payload[i] ^ mask[i % 4];
        }

        // Imprime a mensagem recebida
        printf("Received message: %s\n", payload);

        // Envia uma mensagem de resposta
        char response[1024];
        sprintf(response, "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/plain\r\n"
                          "Content-Length: %d\r\n"
                          "Connection: close\r\n\r\n"
                          "%s",
                strlen(payload), payload);
        send(client_fd, response, strlen(response), 0);

        free(payload);
    }

    closesocket(client_fd);
}

int main()
{
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0)
    {
        printf("Failed to initialize Winsock: %d\n", result);
        return 1;
    }

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == INVALID_SOCKET)
    {
        printf("Failed to create socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == SOCKET_ERROR)
    {
        printf("Failed to bind socket: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    if (listen(server_fd, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("Failed to listen on socket: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("WebSocket server listening on port %d...\n", PORT);

    while (1)
    {
        SOCKET client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == INVALID_SOCKET)
        {
            printf("Failed to accept client socket: %d\n", WSAGetLastError());
            continue;
        }

        printf("Client connected\n");
        handle_client(client_fd);
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
    
}
