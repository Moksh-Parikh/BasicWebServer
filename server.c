#include "server_functions.h"

int main(int argc, char **argv) {
    int server_fd;
    struct sockaddr_in server_addr;

    pthread_t thread;

    // create socket
    // and check it is valid
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0) ) < 0) {
        fprintf(stderr, "Failed to create socket\n");
        exit(1);
    }

    // configure socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // bind socket to port
    if (bind(server_fd,
        (struct sockaddr *) &server_addr,
        sizeof(server_addr) ) < 0) {
        fprintf(stderr, "Failed to bind port %d\n", PORT);
        exit(1);
    }

    // listen for connections
    if (listen(server_fd, 10) < 0) {
        fprintf(stderr, "Error while listening for connections on port %d\n", PORT);
        exit(1);
    }

    printf("Listening on port %d\n", PORT);

    while (1) {
        // pull client info
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int *client_fd = malloc( sizeof(int) );

        if ( (*client_fd = accept(
                            server_fd,
                            (struct sockaddr *)&client_addr,
                            &client_addr_len)) < 0)
        {

            fprintf(stderr, "Failed to accept connection on port %d\n", PORT);
            continue;
        }

        pthread_create(&thread, NULL, handleClient, (void*)client_fd);
        /* handleClient((void*)client_fd); */
    }

    return 0;

}
