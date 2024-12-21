#include "socket_utils.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

struct FactorialArgs {
   uint64_t begin;
   uint64_t end;
   uint64_t mod;
};

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
   uint64_t result = 0;
   a %= mod;

   while (b > 0) {
       if (b % 2 == 1)
           result = (result + a) % mod;

       a = (a * 2) % mod;
       b /= 2;
   }

   return result % mod;
}

uint64_t Factorial(const struct FactorialArgs *args) {
   uint64_t ans = 1;

   for (uint64_t i = args->begin; i <= args->end; i++) {
       ans = MultModulo(ans, i % args->mod, args->mod);
   }

   return ans;
}

void *ThreadFactorial(void *args) {
   struct FactorialArgs *fargs = (struct FactorialArgs *)args;
   uint64_t result = Factorial(fargs);

   return (void *)(uintptr_t)result; // Cast to uintptr_t for pointer compatibility
}

void handleClient(int client_fd) {
   while (true) {
       char buffer[sizeof(uint64_t) * 3];
       int read_size = receive_data(client_fd, buffer, sizeof(buffer)); 

       if (read_size <= 0)
           break;

       struct FactorialArgs fargs;

       memcpy(&fargs.begin, buffer, sizeof(uint64_t));
       memcpy(&fargs.end, buffer + sizeof(uint64_t), sizeof(uint64_t));
       memcpy(&fargs.mod, buffer + sizeof(uint64_t)*2 , sizeof(uint64_t));

       pthread_t thread_id;
       pthread_create(&thread_id, NULL, ThreadFactorial,
                      (void *)&fargs);

       void *result_ptr;
       pthread_join(thread_id, &result_ptr);

       uint64_t total_result = (uint64_t)(uintptr_t)result_ptr;

       send_data(client_fd,&total_result,sizeof(total_result)); 
   }

   close(client_fd);
}

int main(int argc,char **argv){
   int port=20001; // Default port

   int server_fd = create_socket();
   bind_socket(server_fd, port);
   listen_socket(server_fd);

   printf("Server listening at port %d\n", port);

   while(true){ 
      struct sockaddr_in client_addr; 
      int client_fd = accept_connection(server_fd,&client_addr);
      
      handleClient(client_fd); 
   } 

   close(server_fd); 

   return EXIT_SUCCESS; 
}
