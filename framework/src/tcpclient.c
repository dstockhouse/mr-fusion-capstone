// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 

#define PORT 42069

int main(int argc, char const *argv[]) 
{ 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    char *hello = "Hello from client"; 
    char buffer[1024] = {0}; 
    char message[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
    printf("\tSocket created\n");

    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 

    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    }
    printf("\tAddress converted\n");

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    }
    printf("\tSocket connected\n\n");

    int i;
    for (i = 0; i < 10; i++) {

        snprintf(message, 1024, "cycle %d...", i);
        send(sock, message, strlen(message), 0 ); 
        printf("Cycle %d message sent\n", i); 
        valread = read(sock , buffer, 1024); 
        printf("   received- %s\n", buffer ); 
        sleep(1);

    }
    return 0; 
} 
