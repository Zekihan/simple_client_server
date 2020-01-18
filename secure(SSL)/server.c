#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

int main(int argc, char *argv[]) {
    SSL_METHOD *my_ssl_method;         // The SSL/TLS method to negotiate
    SSL_CTX *my_ssl_ctx;               // The CTX object for SSL
    SSL *my_ssl;                     // The actual SSL connection
    int my_fd,client_fd;
    struct sockaddr_in server, client;
    int client_size;
    int error = 0;
    char greeting[1024] = "Welcome to the simple secure echo server!\n";

    OpenSSL_add_all_algorithms();   // Initialize the OpenSSL library
    SSL_load_error_strings();       // Have the OpenSSL library load its error strings

    my_ssl_method = TLSv1_server_method();

    if((my_ssl_ctx = SSL_CTX_new(my_ssl_method)) == NULL) {
        ERR_print_errors_fp(stderr);
        exit(-1);
    }

    SSL_CTX_use_certificate_file(my_ssl_ctx,"server.pem",SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(my_ssl_ctx,"server.pem",SSL_FILETYPE_PEM);

    if(!SSL_CTX_check_private_key(my_ssl_ctx)) {
        fprintf(stderr,"Private key does not match certificate\n");
        exit(-1);
    }

    my_fd = socket(PF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(5353);
    server.sin_addr.s_addr = INADDR_ANY;
    bind(my_fd, (struct sockaddr *)&server, sizeof(server));
    listen(my_fd, 5);

    while(1) {
        client_size = sizeof(client);
        bzero(&client,sizeof(client));
        client_fd = accept(my_fd, (struct sockaddr *)&client, (socklen_t *)&client_size);

        if((my_ssl = SSL_new(my_ssl_ctx)) == NULL) {
            ERR_print_errors_fp(stderr);
            exit(-1);
        }

        SSL_set_fd(my_ssl,client_fd);

        if(SSL_accept(my_ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            exit(-1);
        }

        int closed = 1;
        printf("Client connected\n");

        error = SSL_write(my_ssl,greeting,strlen(greeting));
        if(error <= 0){
            SSL_shutdown(my_ssl);
            SSL_free(my_ssl);
            close(client_fd);
            printf("Client disconnected write error\n");
            closed = 0;
        }

        while(closed){
            char buffer_out[1024] = {"\0"};
            char buffer_in[1024] = {'\0'};
            error = SSL_read(my_ssl,buffer_in,sizeof(buffer_in));

            if(error <= 0){
                SSL_shutdown(my_ssl);
                SSL_free(my_ssl);
                close(client_fd);
                printf("Client disconnected with read error\n");
                closed = 0;
            }

            if(strcmp(buffer_in,"quit\n") == 0) {
                SSL_shutdown(my_ssl);
                SSL_free(my_ssl);
                close(client_fd);
                printf("Client disconnected\n");
                closed = 0;
            }else{
                printf("Client: %s",buffer_in);

                sprintf(buffer_out,"%s",buffer_in);
                error = SSL_write(my_ssl,buffer_out,strlen(buffer_out));
                if(error <= 0){
                    SSL_shutdown(my_ssl);
                    SSL_free(my_ssl);
                    close(client_fd);
                    printf("Client disconnected write error\n");
                    closed = 0;
                }
            }
        }
    }
    SSL_shutdown(my_ssl);
    SSL_free(my_ssl);
    close(client_fd);
    SSL_CTX_free(my_ssl_ctx);

    return 0;
}