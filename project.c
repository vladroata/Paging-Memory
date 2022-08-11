#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

	//initialize page table & frame table
	/*rather than create a new array to attach valid/invalid bits to page table entries,
	we can just set all entries to -1 (they are all invalid at the start) and then if the entry is NOT -1,
	then it is valid*/
	int pageTable[256];
	for(int i = 0; i<256; i++){
		pageTable[i] = -1;
	}
	int frameTable[256][256];
	for(int i = 0; i<256; i++){
		for(int j = 0; j<256; j++){
			frameTable[i][j] = -1;
		}
	}
	
	char buf[6]; //buffer for reading integers from addresses.txt
	FILE * input = fopen(argv[1], "r");

	int catch = 2; //represents the # of bytes we must fseek() to swallow carriage return characters (/r)
	int pageFaults = 0; //# of page faults
	
	while(fgets(buf, 6, input ) != NULL ){
		int no = atoi(buf); //represents the virtual address
		if(no > 99){ //3 digits long (no values in the addresses.txt that are 2 digits, if there were, catch should be negative?)
			catch = 0;
		}
		if(no > 999){ //4 digits long
			catch = 1;
		}
		if(no > 9999){ //5 digits long
			catch = 2;
		}
		fseek(input, catch, SEEK_CUR); //Because fseek() stops reading at newline characters, the file pointer has to be adjusted to bring it to the start of the next integer
										//The amount of bytes that the pointer has to be adjusted by is related to the number of digits read for a number (since it reads that same number of bytes)
		
		
		//find binary representation of number
		int remaining = 0;
		int binary[16] = {0};
		int f = 1;
		int i = 0;
		
		int noCopy = no; //temporary copy of the virtual address number;
		while(noCopy != 0){
			remaining = noCopy%2;
			binary[i] = remaining;
			f *= 10;
			noCopy = noCopy/2;
			i++;
		}
		
		//reverse binary representation array (displays it in the right order)
		for(int i = 0; i<8; i++){
			int temp = binary[i];
			binary[i] = binary[15-i];
			binary[15-i] = temp;
		}
		
		/* None of the addresses in addresses.txt are greater than 5 digits long, meaning we can convert them
		to binary directly and obtain our 16 bits of data without needing to do any masking, since the first 16 bits
		would just be padded 0's anyways.*/
		
		unsigned char pageNo = 0;
		unsigned char offset = 0;
		
		//Store the binary representations of the page number & offset by left shifting the binary digits 
		//in reverse (least significant to most significant & performing a bitwise OR operation
		//bitwise OR compares each bit of its first operand to the corresponding bit of the second operand.
		//If either are 1, the result bit is set to 1
		
		//Essentially, what this does is: if binary[i] == 1, then set the bit in temp[i] to 1 as well.
		for(int i = 7; i>=0; i--){
			unsigned char temp = 1;
			if(binary[i] == 1){
				temp <<= (7-i); //left shift operation
				pageNo |= temp; //bitwise OR
			}
			temp = 1;
			if(binary[i+8] == 1){
				temp <<= (7-i);
				offset |= temp;
			}
		}
		
		char buffer[256];
		int pageInt = (int)pageNo; //integer cast of the pagenumber, for easier comparison
		int frameNumber = 0; //represents the # of the frame that maps to the page in question
		int physicalAddress = 0;
		
		if(pageTable[pageInt] == -1){ //bit is invalid (entry does not exist) -- a page fault occurs
			pageFaults++;
			FILE * fp = fopen("BACKING_STORE.bin", "rb"); //open & read from backing_store
			fseek(fp, ((pageInt)*256), SEEK_SET); //set file position to the start of page # <pageNo>
			fread(buffer, 256, 1, fp);
			
			//find first empty frame
			for(int i = 0; i<256; i++){
				if(frameTable[i][0] == -1){
					frameNumber = i;
					pageTable[pageInt] = frameNumber;
					break;
				}
			}
			fclose(fp);
			//fill frame table
			for(int i = 0; i<256; i++){
				frameTable[frameNumber][i] = (int)buffer[i];
			}
			//printf("frame number is %d, offset is %d\n", frameNumber, offset);
			physicalAddress = frameNumber*256 + offset;
			printf("Virtual address: %d Physical address: %d Value: %d\n", no, physicalAddress, frameTable[frameNumber][offset]);
		}
		else{ //bit is valid (entry exists) - we don't need to modify the frame table
			frameNumber = pageTable[pageInt];
			physicalAddress = frameNumber * 256 + offset;
			printf("Virtual address: %d Physical address: %d Value: %d\n", no, physicalAddress, frameTable[frameNumber][offset]);
		}
	}
	printf("Page Faults = %d\n", pageFaults);
	printf("Page Fault Rate = %0.3f\n", (float)pageFaults/1000);
	fclose(input);
}