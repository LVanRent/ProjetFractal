#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "fractal.h"

int maxthread = 500;
int showall =0;
int nonfiles =0;
int fcount = 0;
pthread_t reader;
pthread_t calc[];
pthread_t comparateur;
char *fract[][5];

int main(int argc, char * argv[])
{
	for(int i = 1; i<3;i++){
		if(strcomp("-d",argv[i])){//récupération de showall
			showall = 1;
			i++;
			nonfiles++;
		}	
		if(strcomp("--maxthreads",argv[i])){//récupération de maxthread	
			maxthread = (argv[i+1]);
			i++;
			nonfiles+=2;
		}
	}
	pthread_create(reader,&thread_reader,(void*) &argv[])//créateur du fichier 

	char* outfile = argv[argc-1]; //fichier output
	
}

void *thread_reader(void args){
	char* argv[] = (char *) args;
	int argc = length(argv);
	for(int i = argc-2;i > nonfiles;i--){
		file_open(argv[i]);
	}
}


//ouvre et écrit dans le tableau fract les specs des fractales dans le fichier filename
//type de fichier name-w-h-cR-cI
void file_open(filename)
	FILE fp = fopen(filename,"r");
	for(int i = 0; pas fini; i++){
		fract[i][0] = fscanf(fp,'');
	}
	
}

void *thread_comparateur(){
	while(fcount > 0){
		sem_wait();
		if(){
			
		}
		sem_post();
		
	}
}
