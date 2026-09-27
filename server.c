#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void find_student(char *reg_no, char *response)
{
    FILE *fp = fopen("students.txt", "r");

    if (fp == NULL)
    {
        strcpy(response, "Error opening file");
        return;
    }

    char file_reg[50];
    char name[50];
    char branch[50];
    char college[100];

    int found = 0;

    while (fscanf(fp, "%49s %49s %49s %99[^\n]",
                  file_reg, name, branch, college) == 4)
    {
        if (strcmp(file_reg, reg_no) == 0)
        {
            snprintf(response, BUFFER_SIZE,
                     "Name: %s\nBranch: %s\nCollege: %s",
                     name, branch, college);

            found = 1;
            break;
        }
    }

    if (!found)
    {
        strcpy(response, "Data not found");
    }

    fclose(fp);
}

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Allow reuse of address
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Configure server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for client
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    // Accept client connection
    new_socket = accept(server_fd,
                        (struct sockaddr *)&address,
                        (socklen_t *)&addrlen);

    if (new_socket < 0)
    {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected.\n");

    // Persistent communication
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        int bytes_read = read(new_socket, buffer, BUFFER_SIZE - 1);

        if (bytes_read <= 0)
        {
            printf("Client disconnected.\n");
            break;
        }

        buffer[bytes_read] = '\0';

        // Remove newline if present
        buffer[strcspn(buffer, "\r\n")] = '\0';

        // Terminate session when client sends "bye"
        if (strcmp(buffer, "bye") == 0)
        {
            printf("Client requested termination.\n");
            break;
        }

        printf("Client query: %s\n", buffer);

        memset(response, 0, BUFFER_SIZE);

        find_student(buffer, response);

        send(new_socket, response, strlen(response), 0);
    }

    close(new_socket);
    close(server_fd);

    printf("Connection terminated.\n");

    return 0;
}