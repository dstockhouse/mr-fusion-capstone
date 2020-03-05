// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <sys/time.h> 

#define PORT 42069

int main(int argc, char const *argv[]) 
{ 
    int server_fd, new_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    char buffer[1024] = {0}; 
    char response[1024] = {0}; 
    char *hello = "Hello from server"; 

    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    }
    printf("\tSocket created\n");

    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    printf("\tSocket options set\n");
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 

    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address,  
                sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    printf("\tSocket bound\n");

    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    printf("\tListening...\n");


    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                    (socklen_t*)&addrlen))<0) 
    { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 
    printf("\tConnection accepted\n\n");

    struct timeval timetemp;
    int starttime, curtime;

    gettimeofday(&timetemp, NULL);
    curtime = starttime = timetemp.tv_sec;

    while(curtime < starttime + 10) {

        valread = read( new_socket , buffer, 1024); 
        if(valread > 0) {
            printf("Rx: %s\n   sending ack...\n\n",buffer ); 
            snprintf(response, 1024, "ack: %s", buffer);
            send(new_socket, response, strlen(response), 0 ); 
        }

        gettimeofday(&timetemp, NULL);
        curtime = timetemp.tv_sec;

    }

    return 0; 
}
