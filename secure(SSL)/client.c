#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

int main(int argc, char *argv[]) {
    SSL_METHOD *my_ssl_method;
    SSL_CTX *my_ssl_ctx;
    SSL *my_ssl;
    int my_fd;
    struct sockaddr_in server;
    int error = 0;

    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    my_ssl_method = TLSv1_client_method();

    if((my_ssl_ctx = SSL_CTX_new(my_ssl_method)) == NULL) {
        ERR_print_errors_fp(stderr);
        exit(-1);
    }

    if((my_ssl = SSL_new(my_ssl_ctx)) == NULL) {
        ERR_print_errors_fp(stderr);
        exit(-1);
    }

    my_fd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(5353);
    inet_aton("127.0.0.1",&server.sin_addr);
    bind(my_fd, (struct sockaddr *)&server, sizeof(server));
    connect(my_fd,(struct sockaddr *)&server, sizeof(server));

    SSL_set_fd(my_ssl,my_fd);

    if(SSL_connect(my_ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(-1);
    }

    printf("Connected to the server\n");
    int closed = 1;
    char greeting[1024] = {'\0'};
    error = SSL_read(my_ssl,greeting,sizeof(greeting));
    if(error <= 0){
        SSL_shutdown(my_ssl);
        SSL_free(my_ssl);
        SSL_CTX_free(my_ssl_ctx);
        close(my_fd);
        printf("Disconnected from the server with read error\n");
        closed = 0;
    }else{
        printf("Server: %s",greeting);
    }
    while(closed){
        char buffer_out[1024] = {'\0'};
        char buffer_in[1024] = {'\0'};
        printf("Please write an input:");
        fgets(buffer_out, 1024, stdin);

        error = SSL_write(my_ssl,buffer_out,strlen(buffer_out));
        if(error <= 0){
            SSL_shutdown(my_ssl);
            SSL_free(my_ssl);
            SSL_CTX_free(my_ssl_ctx);
            close(my_fd);
            printf("Disconnected from the server with write error\n");
            closed = 0;
        }

        if(strcmp(buffer_out,"quit\n") == 0) {
            SSL_shutdown(my_ssl);
            SSL_free(my_ssl);
            SSL_CTX_free(my_ssl_ctx);
            close(my_fd);
            printf("Disconnected from the server\n");
            closed = 0;
        }else{
            error = SSL_read(my_ssl,buffer_in,sizeof(buffer_in));
            if(error <= 0){
                SSL_shutdown(my_ssl);
                SSL_free(my_ssl);
                SSL_CTX_free(my_ssl_ctx);
                close(my_fd);
                printf("Disconnected from the server with read error\n");
                closed = 0;
            }else{
                printf("Server: %s",buffer_in);
            }
        }
    }

    return 0;
}