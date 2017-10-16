/**
 * @file   testebbchar.c
 * @author Derek Molloy
 * @date   7 April 2015
 * @version 0.1
 * @brief  A Linux user space program that communicates with the ebbchar.c LKM. It passes a
 * string to the LKM and reads the response from the LKM. For this example to work the device
 * must be called /dev/ebbchar.
 * @see http://www.derekmolloy.ie/ for a full description and follow-up descriptions.
*/
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>



#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM

int main(int argc, char **argv){
   int ret, fd;
   //char stringToSend[BUFFER_LENGTH];
   char operacao[1];
   char dados[BUFFER_LENGTH];
   //char text[256];
   printf("Starting device test code example...\n");
   fd = open("/dev/ebbchar", O_RDWR);             // Open the device with read/write access
   if (fd < 0){
      perror("Failed to open the device...");
      return errno;
   }

   strcpy(dados, argv[2]);
   
   //scanf("%[^\n]%*c", operacao);
   //gets(text);
   //scanf("%[^\n]%*c", stringToSend);                // Read in a string (with spaces)
   printf("Writing message to the device [%s].\n", dados);
   
   //ret = write(fd, argv, strlen(argv)); // Manda pro modulo qual opcao o usuario escolheu
   
   /*
   if(strcmp(argv[1], "c") == 0){  // Cifrar
     printf("Opcao 1\n\n");
     ret = write(fd, dados, 1); // Manda pro modulo qual opcao o usuario escolheu
   }
   if(strcmp(argv[1], "d") == 0){ // Decifrar
     printf("Opcao 2\n\n");
     ret = write(fd, dados, 2); // Manda pro modulo qual opcao o usuario escolheu
   }
   if(strcmp(argv[1], "h") == 0){ // Hash
     printf("Opcao 3\n\n");
     ret = write(fd, dados, 3); // Manda pro modulo qual opcao o usuario escolheu
   }
   */
   
   /*
   //ret = write(fd, dados, strlen(dados)); // Send the string to the LKM
   if (ret < 0){
      perror("Failed to write the message to the device.");
      return errno;
   }

   printf("Press ENTER to read back from the device...\n");
   getchar();
   */
   printf("Reading from the device...\n");
   ret = read(fd, receive, BUFFER_LENGTH);        // Read the response from the LKM
   /*
   if (ret < 0){
      perror("Failed to read the message from the device.");
      return errno;
   }*/
   printf("The received message is: [%s]\n", receive);
   printf("End of the program\n");
   return 0;
}
