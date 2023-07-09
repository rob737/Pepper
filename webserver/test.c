#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

// The area in memory(RAM) where data received from network would be stored in Bytes.
#define APP_MAX_BUFFER 1024
#define PORT 8080

/**
 * Server is basically a socket file descriptor and that's it.
 * That's why we can create multiple servers (file descriptors) and bind it to a different address on the same machine. 
*/
int main(){
    // define the server and client file descriptors.
    /*
    file descriptors basically represents the socket.
    It gives information such as port at which app is running, 
    address to which it is bound.
    */ 
    int server_fd, client_fd;
    /**
     * 
     * client_fd is created only when connection is established.
     * Basically, when we say connection is established, it's basically
     * the client file descriptor that is created.
    */
   /**
    * 
    * There is mostly one server_fd and can be multiple client_fd.
    * In our use case, we have one to one mapping though for simplicity.
   */

   // define the socket address.
    struct sockaddr_in address;
    int address_len = sizeof(address);

    /**
     * define the application buffer where we receive the requests.
     * data will be moved from receive buffer to here.
    */
    char buffer[APP_MAX_BUFFER] = {0};

    /**
     * Basically, what happens under the hood is we would copy 
     * client's connection data from kernel buffer to our buffer (called as zero copy)
    */

        // create socket 
        /**
         * INET is basically what we call IpV4.
         * SOCK_STREAM denotes that we would need streaming protocol.
         * Example of streaming protocol is TCP protocol.
         * UDP is message based protocol.
         * 
         * In Operating System, everything is a file.
         * socket is a file, file is a file, printer is a file.
         * Everything is a file and we get a file number which is 
         * called file descriptor which represents that file.
        */
        if((server_fd = socket(AF_INET,SOCK_STREAM,0)) == 0){
            perror("Socket failed.");
            exit(EXIT_FAILURE);
        }

        // Bind socket 
        /**
         * sin : socket to internet connectivity
         * sin_family : It accepts request from IPV4 machines.
        */
        address.sin_family = AF_INET; //IPV4
        address.sin_addr.s_addr = INADDR_ANY; // Listening to all interfaces (0.0.0.0)
        /**
         * Note : listening to all interfaces can lead to security breach.
         * e.g. let's say you have an admin app that is exposed to all public interfaces
         * like wifi, ehternet, cloud then anyone on the internet can connect and read data 
         * from.
        */

        /**
         * Everything on internet follows something called as network order.
         * We would need to convert out normal integer orientation to network order
         * to allow it to be connected to the internet.
        */
        address.sin_port = htons(PORT);

        // assign socket created (server_fd) to some address to fecilitate 
        // connection over internet.
        // If port field is already bound to some other socket then 
        // we get error like address already in use.
        if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
            perror("Bind failed");
            exit(EXIT_FAILURE);    
        }

        /**
         * This creates the queues (one queue each for send, receieve, accept etc.) for a particular socket.
         * Listen for clients, with 10 backlog (10 connections in accept queue).
         * 
         * After 3 way handshake is done (SYN, SYN_ACK, ACK from client) then kernel 
         * puts connection details from that client to accept queue.
         * 
         * Client initiates 3 way handshake with server.
         * 
         * So, if we are getting error while connecting to server then either socket fd is not listening itself
         * or accept queue is full.
         * 
         * example of error code is ECONNREFUSED.
         * 
         * Backend app must manually call accept() to pop connection from the queue and start application
         * processing.
        */
        if(listen(server_fd,10) < 0){
            perror("listen failed");
            exit(EXIT_FAILURE);
        }

        // we loop forever
        /**
         * Doubt : 
         * 
        */
        while(1){
            printf("\n waiting for a connection ...");
            // Accept a client connection client_fd == connection
            // This is a blocking operation.
            // we can have many connections (client_fd) connecting to same socket file (server_fd)
            /**
             * Each client request is mapped to one client_fd in order of arrival.
             * If the accept queue is empty then we are stuck here i.e. it blocks.
             * The popped element from the accept queue is what a client request maps to identified by client 
             * file descriptor.
             * Well, I think that's why Client-Server communication is stateless.
            */
            if((client_fd = accept(server_fd,(struct sockaddr *)&address, (socklen_t*)&address_len)) < 0){
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }

            /**
             * 
             * When a connection request is popped out from accept queue, then OS kernel 
             * fecilitates two more queues for it.
             * 1. receive queue : Where data from connection request is put like request body, query params etc.
             * (from NIC (Network Interface Controller) to receive queue)
             * 
             * 2. send queue : Where data that needs to be sent back to client as response from server needs to be put.
             * 
             * There is lot of copy going on here, from queue to memory for processing.
             * 
             * So, the response from server is that it goes to the send queue and then from send queue, it
             * goes to NIC and then it goes to Client.
            */

           /**
            * read data from the OS kernel receive buffer to our custom defined (application) buffer.
            * This is essentially reading the Http request.
            * max it would be read of size APP_MAX_BUFFER and remaining would be left as it is in receive queue.
            * 
           */
          read(client_fd,buffer,APP_MAX_BUFFER);
          printf("%s\n",buffer);

          /**
           * We send the request by writing to the socket send queue of OS kernel.
           * This string is defined in the stack.
           * Only malloc created object is in the heap.
          */
           char *http_response = "HTTP/1.1 200 OK\n"
                                 "Content-Type: text/plain\n"
                                 "Content-Length: 13\n\n"
                                 "Hello world!\n";

          // writes to the socket send queue
          write(client_fd, http_response, strlen(http_response));

          // close the client socket (terminate the TCP connection).
          close(client_fd);
        }


    return 0;
}