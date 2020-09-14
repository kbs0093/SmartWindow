#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(void){
  char k_180, k_360;
  int fd_180, fp_180;
  int fd_360, fp_360;
  
  int blind = 0; // 0(open), 1 ,2 ,3(close)
  int change = 0;
  int rotate = 0; // change - blind
  
  int count = 0;
  
  char buffer[20];
  char criteria[1]={0};
  
  FILE *fp;
  
  fd_180 = open("/dev/test_motor180_dev", O_RDWR);
  fd_360 = open("/dev/test_motor360_dev",O_RDWR);

  if(fd_180 < 0){
    perror("Failed to open the device:180\n");
    return errno;
  }
  if(fd_360 < 0){
    perror("Failed to open the device:360\n");
    return errno;
  }
   
  while(1){
	  sleep(1);
	  count += 1;
	  printf("\nWhilestart\n");
	  printf("count : %d\n\n",count);
		  
	  fp = fopen("output.txt", "rt");
	  fgets(buffer, sizeof(buffer), fp);
	  printf("val :%s\n",buffer);
	  fclose(fp);
	  
	  if(*buffer == '0'){ // Emergency - window = open, curtain = open;
		  k_180 = '1';
		  change = 0;
		  rotate = change - blind;
		  blind = change;
		  
		  rotate = abs(rotate);
		  k_360 = '2'; // -90
		  for(int i=0;i<rotate;i++)
			fp_360 = write(fd_360, &k_360, 1);
		  
		  fp_180 = write(fd_180, &k_180, 1);
	  }
	  
	  
		  //if(count == 5){
			count = 0;
			  
			if(*buffer == '1'){ // window = open, curtain = 30% open
				  k_180 = '1';
				  change = 2;
			}else if(*buffer == '2'){ // window = open , curtain = 100% close
				k_180 = '1';
				change = 3;
			}else if(*buffer == '3'){// window = close, curtain = 30% open
				k_180 = '0';
				change = 2;
			}else if(*buffer == '4'){ // window = close, curtain = 100% close
				k_180 = '0';
				change = 3;
			}else if(*buffer == '6'){ // window = open, curtain = 60% open
				k_180 = '1';
				change = 1;
			}else if(*buffer == '7'){ // window = close, curtain = 60% open
				change = 1;
			}else if(*buffer == '8'){ // window = open, curtain = 100% open
				k_180 = '1';
				change = 0;
			}else if(*buffer == '9'){ // window = close, curtain = 100% open
				k_180 = '0';
				change = 0;
			}
			
			rotate = change - blind;
			blind = change;
		  
			if(rotate<0){ // up
				rotate = abs(rotate);
				k_360 = '2'; // -90

			}else{ // down
				rotate = abs(rotate);
				k_360 = '0'; // +90
			}
			
			for(int i=0;i<rotate*2.5;i++)
				fp_360 = write(fd_360, &k_360, 1);
				
			fp_180 = write(fd_180, &k_180, 1);		  
		  
		  //} // 15
		  
		  
	  } // while
	
  close(fd_180);
  close(fd_360);
  return 0;
}
