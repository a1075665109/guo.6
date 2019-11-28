#include<stdio.h>
#include<stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h> 

#define maxProcess 18
int maxTime = 5;

// structure for a clock.
struct clock{
        unsigned int sec;
        unsigned int nano_sec;
	sem_t sem;
};

// frame table with referecebyte,page number, dirty bit, pid, and indication of occupied.
struct frames{
	int referenceByte;
	int dirtyBit;
	int occupied;
	int fpid;
};

// page table with pid to show which process it belongs to and the pages it has.
struct pageTable{
        int pid;
	int page[32];
};


struct clock* clk;
struct frames* frame;
struct pageTable* prm;
char* outputFile;


void init_frames(){
	int i = 0;
	while(i < 256){
		frame[i].referenceByte = 0;
                frame[i].dirtyBit = 0;
                frame[i].occupied = 0;
                frame[i].fpid = -1;
                i += 1;
	}
}

void init_clock(){
	clk -> nano_sec = 0;
	clk -> sec = 0;
}

void init_pageTable(){
	int i = 0;
	while (i < 18){
		prm[i].pid = -1;
		int j = 0;
		while(j < 32){
			prm[i].page[j] = -1;
			j += 1;
		}
		i += 1;
	}

}

void alarmHandler(int sig){
    	
	FILE *fp;
        fp = fopen(outputFile,"a");
        fprintf(fp,"End of program, all processes terminated after max",maxTime);
        fclose(fp);
        
	int i=0;
        while(i <18){
                if(prm[i].pid>0){
                        kill(prm[i].pid,SIGTERM);
                }
                i=i+1;
        }
        printf("\nProgram terminated after %d seconds, review results in logFile\n",maxTime);
        kill(getpid(),SIGTERM);
}




int main(int argc, char*argv[]){
	int opt;
	int vflag;
	outputFile = "logFile"; 
	while((opt = getopt(argc,argv,"ht:"))!=-1){
                switch(opt){
                        case 'h':
                                printf("\n-h is used for listing the available command line arguments\n");
                                printf("the command line options are -h, -t\n");
                                printf("use -t command followed by an argument to change the max run time\n");
                                printf("\nprogram terminated\n\n");
                                return 0;
                        case 'v':
                                vflag = 1;
                                break;
                        case 't':
                                maxTime = atoi(optarg);
                                printf("\nDefault running time has changed from 5 seconds to %d seconds\n\n",maxTime);
                                break;
                        case '?':
                                printf("\n Invalid arguments, please use option -h for help, program terminated\n");
                                return 0;
                }
        }
	alarm(maxTime);

	// setup the PageTable in shared memory
		
	int prmid;
        int prmsize = 18 * sizeof(prm);
        prmid = shmget(0x1234,prmsize,0666|IPC_CREAT);
        if(prmid == -1){
                perror("Shared memory\n");
                return 0;
        }
        prm = shmat(prmid,NULL,0);
        if(prm == (void *)-1){
                perror("Shared memory attach\n");
                return 0;
        }
	
	// setup clock shared memory
	int clockid;
        int clocksize = sizeof(clk);
        clockid = shmget(0x3234,clocksize,0666|IPC_CREAT);
        if(clockid == -1){
                perror("Shared memory\n");
                return 0;
        }
        clk = shmat(clockid,NULL,0);
        if(clk== (void *)-1){
                perror("Shared memory attach\n");
                return 0;
        }

	// set up the frameTable in shared memory
	int frameid;
        int framesize = 256 * sizeof(frame);
        frameid = shmget(0x2234,framesize,0666|IPC_CREAT);
        if(frameid == -1){
                perror("Shared memory\n");
                return 0;
        }
        frame = shmat(frameid,NULL,0);
        if(frame == (void *)-1){
                perror("Shared memory attach\n");
                return 0;
        }
	
	init_frames();	
	init_clock();
	init_pageTable();

	return 0;
}
