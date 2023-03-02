#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>

#define PORT 8080

static int handle_request(void *cls, struct MHD_Connection *connection, const char *url,
                          const char *method, const char *version, const char *upload_data,
                          size_t *upload_data_size, void **ptr) {
    if (strcmp(method, "GET") != 0 && strcmp(method, "POST") != 0) {
        // Se o método HTTP não for GET nem POST, retorna um erro 405 - Method Not Allowed
        const char *error_response = "HTTP/1.1 405 Method Not Allowed\r\n"
                                     "Content-Type: text/plain\r\n"
                                     "Content-Length: 0\r\n"
                                     "Connection: close\r\n\r\n";
        struct MHD_Response *response = MHD_create_response_from_buffer(0, NULL, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, MHD_HTTP_METHOD_NOT_ALLOWED, response);
        MHD_destroy_response(response);
        return ret;
    }

    if (*ptr == NULL) {
        // Primeira chamada da função, inicializa a memória de estado
        *ptr = (void*)1;
        return MHD_YES;
    }

    // Obtém o corpo da requisição
    char *data = NULL;
    if (*upload_data_size > 0) {
        data = malloc(*upload_data_size + 1);
        memcpy(data, upload_data, *upload_data_size);
        data[*upload_data_size] = '\0';
        *upload_data_size = 0;
    }

    // Prepara a mensagem de resposta
    char *response_str;
    if (data != NULL) {
        // Se a requisição contém dados, retorna os dados na resposta
        int len = snprintf(NULL, 0, "HTTP/1.1 200 OK\r\n"
                                    "Content-Type: text/plain\r\n"
                                    "Content-Length: %d\r\n"
                                    "Connection: close\r\n\r\n"
                                    "%s", strlen(data), data);
        response_str = malloc(len + 1);
        snprintf(response_str, len + 1, "HTTP/1.1 200 OK\r\n"
                                         "Content-Type: text/plain\r\n"
                                         "Content-Length: %d\r\n"
                                         "Connection: close\r\n\r\n"
                                         "%s", strlen(data), data);
        free(data);
    } else {
        // Se a requisição não contém dados, retorna uma mensagem de sucesso
        const char *success_response = "HTTP/1.1 200 OK\r\n"
                                       "Content-Type: text/plain\r\n"
                                       "Content-Length: 7\r\n"
                                       "Connection: close\r\n\r\n"
                                       "Success";
        response_str = malloc(strlen(success_response) + 1);
        strcpy(response_str, success_response);
    }

    // Envia a resposta
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(response_str), response_str, MHD_RESPMEM_MUST_COPY);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    free(response_str);
    return ret;
}

int main() {
    struct MHD_Daemon *daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                                                 &handle_request, NULL, MHD_OPTION_END);
    if (daemon == NULL) {
        printf("Falha ao iniciar o daemon.\n");
        return 1;
    }

    printf("Servidor rodando na porta %d.\n", PORT);

    // Aguarda a tecla 'q' para encerrar o servidor
    printf("Pressione 'q' para encerrar o servidor.\n");
    char c;
    do {
        c = getchar();
    } while (c != 'q');

    MHD_stop_daemon(daemon);
    return 0;

}