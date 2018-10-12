#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpsse.h>

#define RDCMD	"\x06" 		//0000 0011
#define WRCMD   "\x05"		//0000 0010
#define WECMD   "\x04c0"	//0000 0110
#define WDCMD   "\x0400"	//0000 0100

char *readChip(int readSize, int addrSize, char *addr, char *file) {

	FILE *fp = NULL;
	char *rdCmd;
	char *data = NULL;
	char *verifyData = NULL;
	struct mpsse_context *flash = NULL;
	char *out;

	out = malloc(readSize);
	memset(out, 0, sizeof(out));

	if((flash = MPSSE(SPI0, ONE_HUNDRED_KHZ, MSB)) != NULL && flash->open) {

		SetCSIdle(flash, 0);  //0 for microwire since cs must idle low then go high to select

		printf("%s initialized at %dHz (SPI mode 0)\n", GetDescription(flash), GetClock(flash));

		//build read command
		rdCmd = malloc(addrSize+1);
		rdCmd[0] = *RDCMD;
		for (int x = 0; x < addrSize; x++)
			rdCmd[x+1] = addr[x];

		for (int i = 0; i < (1+addrSize); i++) {
				if (!(i % 16))
					puts("");
				printf("%.2X ", rdCmd[i] & 0xff);
				}

		//initial read
		Start(flash);
		Write(flash, rdCmd, 1+addrSize);
		data = Read(flash, readSize);
		Stop(flash);

		//verify read
		//Start(flash);
		//Write(flash, rdCmd, 1+addrSize);
		//verifyData = Read(flash, readSize);
		//Stop(flash);
		}
	else
		printf("Failed to initialize MPSSE: %s\n", ErrorString(flash));

	Close(flash);

	//handle dummy bit
	for (int i = 0; i < readSize-1; i++) {
		out[i] = (data[i] << 1);

		if (data[i+1] & (1<<7))
			out[i] |= (1 << 0);

	}

	//if (strcmp(data, verifyData) != 0) {
	//	data = NULL;
	//	verifyData = NULL;
	//	}
	//else {
		if (file != NULL){
			fp = fopen(file, "wb");
			if(fp) {
				fwrite(data, 1, readSize, fp);
				fclose(fp);

				printf("Dumped %d bytes to %s\n", readSize, file);
				}
			}
	//}

	return out;
}

int writeChip(int size, int addrSize, char *addr, char *value) {

	struct mpsse_context *flash = NULL;
	char *wrtCmd;
	char *valData;

	char cmd[] = { 0x00, 0x00};

	//write data
	if((flash = MPSSE(SPI0, 500, MSB)) != NULL && flash->open) {

		SetCSIdle(flash, 0);  //0 for microwire since cs must idle low then go high to select

		//build write command
		wrtCmd = malloc(size+addrSize+1);
		wrtCmd[0] = *WRCMD;
		for (int x = 0; x < addrSize; x++)
			wrtCmd[x+1] = addr[x];
		for (int x = 0; x < size; x++)
			wrtCmd[x+addrSize+1] = value[x];

		for (int i = 0; i < (size+addrSize); i++) {
				if (!(i % 16))
					puts("");
				printf("%.2X ", wrtCmd[i] & 0xff);
				}


		printf("%s initialized at %dHz (SPI mode 0)\n", GetDescription(flash), GetClock(flash));

		cmd[0] = 0x04;
		cmd[1] = 0xc0;

		//Write enable
		Start(flash);
		Write(flash, cmd, 2);
		Stop(flash);

		//Write
		Start(flash);
		Write(flash, wrtCmd, size+addrSize+1);
		Stop(flash);

		cmd[0] = 0x04;
		cmd[1] = 0x00;

		//Write disable
		Start(flash);
		Write(flash, cmd, 2);
		Stop(flash);
		}
	else
		printf("Failed to initialize MPSSE: %s\n", ErrorString(flash));

	Close(flash);

	//validate write
	valData = readChip(size, addrSize, addr, NULL);
	if (valData != NULL)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

int main(int argc, char *argv[]) {

	int chipSize = 256;
	char *data = NULL;
	char wrtAddr[] = { 0x7f };
	char addr[] = { 0x00 };
	//char *myData = NULL; //[] = { 0xf4, 0xcf, 0x4f };
	//char *file = "try.bin" + 0x00;
	char sendData[] = { 0xe7, 0x51};



	data = readChip(chipSize, 1, addr, NULL);
	//writeChip(1, 1, "\x0D", "\x4D") == EXIT_SUCCESS ? printf("worked\n"): printf("failed\n");
	//writeChip(1, 1, "\x19", "\x4D") == EXIT_SUCCESS ? printf("worked\n"): printf("failed\n");
	//writeChip(1, 1, "\x30", "\xE8") == EXIT_SUCCESS ? printf("worked\n"): printf("failed\n");
	//writeChip(1, 1, wrtAddr, sendData) == EXIT_SUCCESS ? printf("worked\n"): printf("failed\n");
	writeChip(2, 1, wrtAddr, sendData) == EXIT_SUCCESS ? printf("worked\n"): printf("failed\n");
	data = readChip(chipSize, 1, addr, NULL);
	//data = readChip(chipSize, 1, addr, NULL);

	for (int i = 0; i < chipSize; i++) {
		if (!(i % 16))
			puts("");
		printf("%.2X ", data[i] & 0xff);
		}

	return EXIT_SUCCESS;
}
