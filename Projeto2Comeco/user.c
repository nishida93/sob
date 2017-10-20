/*
 Alex Venturini  - RA: 15294739 
 Luan Bonomi - RA: 15108780
 Pedro Catalini - RA: 15248354
 Matheus Nishida - RA: 12212692
 Daniel Toloto  - RA: 16436065	
 Leonardo Guissi - RA: 15108244
*/

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>



#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM


int main(){
   int ret, fd;
   char escolha[BUFFER_LENGTH];
   char stringToSend[BUFFER_LENGTH];
   int mensagemHexa[BUFFER_LENGTH];
   printf("Starting device test code example...\n");
   fd = open("/dev/ebbchar", O_RDWR);             // Open the device with read/write access
   if (fd < 0){
      perror("Failed to open the device...");
      return errno;
   }
   printf("Escolha entre uma das opcoes c - Criptar, d - Decriptar, h - hash:\n");
   scanf("%[^\n]%*c", escolha);                // Read in a string (with spaces)
   
   if(escolha[0] == 'd'){
        printf("Decriptar, digite o texto em Hexa:\n");
        scanf("%x", mensagemHexa);                // Read in a string (with spaces)
   }else{
        printf("Escreva uma curta mensagem:\n");
        scanf("%[^\n]%*c", stringToSend);                // Read in a string (with spaces)
   }
   
   
   printf("Writing message to the device [%s].\n", stringToSend);
   ret = write(fd, stringToSend, escolha[0]); // Send the string to the LKM

   printf("Reading from the device...\n");
   ret = read(fd, receive, BUFFER_LENGTH);        // Read the response from the LKM
   if (ret < 0){
      perror("Failed to read the message from the device.");
      return errno;
   }
   printf("The received message is: [%s]\n", receive);
   printf("End of the program\n");
   return 0;
}


