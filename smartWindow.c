#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "cds_censor.h"
#include "water_sensor.h"
#include "dust_sensor.h"

int main(void){
	
	char *str;
	int i;
	char c[20];
	int data[4];
	
	int dht11;
	int dust;
	int cds;
	int water;
	
	int blind;
	int window;
	int summer;
	
	time_t timer;
	struct tm *t;
	
	FILE *fp;
	int output;

dht11:
	dht11 = open("/dev/dht11_dev" ,O_RDONLY);

	if(dht11 < 0){
		printf("ERROR : Dht11 Driver file open Failed! %d\n", dht11);
		sleep(1);
		goto dht11;
	}
	
	printf("DHT11 driver open Success\n");			

		
	while(1){
start:		
		i = 1;
		timer = time(NULL); 
		t = localtime(&timer); 
		
		window = 0;
		blind = 0;
		summer = 0;
		
		
		printf("Today is %d-%d-%d\n",(t->tm_year+1900), (t->tm_mon + 1), (t->tm_mday)); //오늘 날짜 확인 후 출력
		
		if(t->tm_mon >= 5 && t->tm_mon <= 9){ //5월에서 9월 사이일 경우 프로그램을 여름모드로 작동 (안보다 밖의 온도가 높을 경우 창문 개방)
			 summer = 1;
		 }
		
		read(dht11, c, 20);	
					
		str = strtok(c, " ");
		data[0] = atoi(str);
		
		while(str != NULL){
			str = strtok(NULL, " ");
			if(i <= 3){
				data[i] = atoi(str);
				i++;
			}
		}
		
		data[0] = (data[0] - 32)/1.8;  //갖고있는 DHT 센서는 화씨온도를 출력했으므로 값을 받은 뒤 변환
		data[2] = (data[2] - 32)/1.8;
		
		cds = readadc_cds();  //ADC 드라이버를 c 파일로 제작후 헤더파일만 추가하여 함수만 호출하여 각 센서의 값을 출력
		water = readadc_water();
		dust = dust_sensor();
		
		
		printf("out dht = %d, in dht =  %d\n",data[0], data[2]);		// check the value for debug
		printf("cds = %d\n", cds);
		printf("water = %d\n", water);
		printf("dust = %d\n", dust);
		
		
		if (summer == 1){		
			if(water > 150){
				window = 0;
			}
			else if(dust > 400){
				window = 0;
			}
			else if(data[0] > data[2]){
				window = 1;
			}			
		}		
		else{
	
			if(water > 150){
				window = 0;
			}
			else if(dust > 400){
				window = 0;
			}
			else if(data[0] < data[2]){
				window = 0;
			}		
		}
		
		
		if(cds >210){
			blind = 0;
		}		
		else if(cds > 180 && cds <= 210){
			blind = 1;
		}
		else if(cds <= 120 && cds >80){
			blind = 2;
		}
		else{
			blind = 3;
		}
				
		
		
		//printf("window = %d, blind = %d\n",window ,blind);   //window = 1 : open the window
															  //		  = 0 : close the window
															  //blind = 1 : open the blind
															  //      = 0 : close the blind
																     
		if(window == 1 && blind == 1){                       // 시나리오에 따른 output 값을 출력 
			printf("Window open , Blind 30%% open\n");
			output = 4;
		}
		else if(window == 1 && blind == 0){
			printf("Window open , Blind close\n");
			output = 3;
		}
		else if(window == 0 && blind == 1){
			printf("Window close , Blind 30%% open\n");
			output = 2;
		}
		else if(window == 0 && blind == 0){
			printf("Window close , Blind close\n");
			output = 1;						
		}	
		else if(window == 1 && blind == 2){
			printf("Window open , Blind 60%% open\n");
			output = 6;	
		}
		else if(window == 0 && blind == 2){
			printf("Window close , Blind 60%% open\n");
			output = 7;	
		}
		else if(window == 1 && blind == 3){
			printf("Window open , Blind 100%% open\n");
			output = 8;			
		}
		else if(window == 0 && blind == 3){
			printf("Window close, Blind 100%% open\n");
			output = 9;	
		}	
			
		if (data[0] > 50){   //emergency status maybe classroom on fire!!
			output = 0; 	 //open the window and blind!
		}
		
		printf("\n");
		//printf("output = %d\n",output);
		
		
	
fileopen:
		//printf("output.txt make....\n");
		fp = fopen("output.txt", "wt");    // MQTT 프로토콜을 사용하는 파이썬 APP 이 사용할 수 있게 output 값을 txt 형태로 저장
			
		if(fp < 0){
			printf("File open is Fail!\n");
			sleep(1);
			goto fileopen;
		}
			
		fprintf(fp,"%d",output);
			
		fclose(fp);
		//printf("Output.txt is Done!\n");
		sleep(10);
	
		
		
	}
	
	
	close(dht11);

	return 0;
}