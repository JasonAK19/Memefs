// shree Ram

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define DEVICE "/dev/meme_bridge"

int main(){
int file;
char  read_buff[100];
// opening the device
 file=open(DEVICE,O_RDWR);
 if(file == -1){
          printf("Fasiled to open file \n ");
       return -1;    
      }else{
          printf("The device opened  \n ");
         }
// read from the device
read(file,read_buff,sizeof(read_buff));  
  // print
printf("The kernel Space passed  %s",read_buff);

 close(file);
return 0;
}
