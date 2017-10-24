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
   int ret, fd, i;
   char escolha;
   char stringToSend[16];
   char mensagemHexa[16];
   printf("Starting device test code example...\n");
   fd = open("/dev/ebbchar", O_RDWR);             // Open the device with read/write access
   if (fd < 0){
      perror("Failed to open the device...");
      return errno;
   }
   printf("Escolha entre uma das opcoes c - Criptar, d - Decriptar, h - hash:\n");
   scanf("%c", &escolha);                // Read in a string (with spaces)
   getchar();
   
   if(escolha == 'd'){
        printf("Decriptar, digite o texto em Hexa:\n");
        for(i = 0; i < 16; i++){
        	scanf("%02x", &mensagemHexa[i]);	
        }
        for(i = 0; i < 16; i++){
        	printf("HexaOr: %x", mensagemHexa[i]);
        }
        //scanf("%x", mensagemHexa);
        ret = write(fd, &mensagemHexa, escolha); // Send the string to the LKM
        //scanf("%x", mensagemHexa);                // Read in a string (with spaces)
   }else{
        printf("Escreva uma curta mensagem:\n");
        scanf("%[^\n]%*c", stringToSend);                // Read in a string (with spaces)
   	ret = write(fd, stringToSend, escolha); // Send the string to the LKM
   }
   
   printf("Writing message to the device [%s].\n", stringToSend);
   

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


