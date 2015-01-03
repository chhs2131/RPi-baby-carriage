/*/////////////////////////////////
//
// Project : Rpi + baby carriger Part 1
// Last Update : 01.03.15
// Ver : Alpha
//
// Here We have "Gas, Temperature&Humidity, Dust, Bluetooth"
// Other Program Have "Light"
//
*//////////////////////////////////

#include <stdio.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <wiringPiSPI.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define CS_MCP3208 6 //GPIO 6
#define TEMPHUMI 1   //GPIO 1
#define CHOOSEUART 2 //GPIO 2

#define SPI_CHANNEL 0
#define SPI_SPEED 1000000 //1MHz
#define KEY_NUM 9527
#define MEM_SIZE 1024

int read_mcp3208_adc(unsigned char);

int main(void)
{
	/*/////////////////////////
	//
	// Setting Part
	//
	*//////////////////////////

	//Set Variable
	int shm_id;
	void *shm_addr;

	int adcChannel = 0;
	int adcValue = 0;

	int dustPCS = 0;
	float dustValue = 0;
	int fd = serialOpen("/dev/ttyAMA0", 9600);
	char send[] = { 0x11, 0x01, 0x01, 0xED };
	char respone[15] = { 0, };

	char bluesend[5] = {0,};

	//Set GPIO, SPI, Shareing Variable
	if (wiringPiSetup() == -1) return 1; //GPIO ON?
	if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1) return 1; //SPI ON?
	if ((shm_id = shmget((key_t)KEY_NUM, MEM_SIZE, IPC_CREAT:0666) == -1)) return -1; //Create Share Var?
	if ((shm_addr = shmat(shm_id, (void *)0, 0)) == -1) return -1; //Can Write Share Var?

	//Set Pin Input or Output
	pinMode(CS_MCP3208, OUTPUT);
	pinMode(TEMPHUMI, INPUT);
	pinMode(CHOOSEUART, OUTPUT);



	/*/////////////////////////
	//
	// Check Senser Part
	//
	*//////////////////////////
	
	while (1){

		//Gas Senser
		{
			adcValue = read_mcp3208_adc(adcChannel);
			printf("adcValue = %u\n", adcValue);
			if (adcValue >= 10000) bluesend[0] = 1;
		}

		//Dust senser
		{
			digitalWrite(CHOOSEUART, 1) //relay Switch DUST
			write(fd, send, 4);
			read(fd, respone, 7);
	
			if (respone[0] = 0x16) // Success
			{
				dustPCS = respone[3] * 256 * 256 * 256 + respone[4] * 256 * 256 + respone[5] * 256 + respone[6];
				dustValue = ((float)(dustPCS * 3528)) / 100000;
				printf(" Now! Dust : %f\n", dustValue);
				if (dustValue >= 10000) bluesend[1] = 3;
				else if (dustValue >= 7000) bluesend[1] = 2;
				else if (dustValue >= 5000) bluesend[1] = 1;
				else bluesend[1] = 0;
			}
			else if (respone[0] = 0x06) // Failed
			{
				printf("Hey! I Fail to Receive..\n");
			continue;
			}
			else printf("Can't connected Dust Senser..\n");
			
			serialClose(fd);
		}

		//Temp&Humi Senser
		{
			if (dustValue >= 10000) bluesend[2] = 3; //////
		}

		//Light Senser
		{
			printf("Light : %d\n", *(volatile int *)shm_addr);
			if (bluesend[3] < 255) bluesend[3] = *(volatile int *)shm_addr;
			else bluesend[4] += 1;
		}

		//Bluetooth
		{
			digitalWrite(CHOOSEUART, 0) //relay Switch Blue
			write(fd, bluesend, 5);
		}

	}
	return 0;
}