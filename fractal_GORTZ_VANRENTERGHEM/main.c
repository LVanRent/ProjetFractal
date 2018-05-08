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
//char *fract[][5];


typedef struct bufferList {
	struct bufferList * next;
	char * name;
	int w;
	int h;
	double cR;
	double cI;
		
} bufferList;

bufferList *lastread;

void *thread_reader(void args);
void file_open(string filename);

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
	pthread_create(calculator,&thread_calc,NULL);
	pthread_join();
	pthread_join();
	if(showall == 0){
		//afficher le BMP du meilleu
	}
	char* outfile = argv[argc-1]; //fichier output
	
}

void *thread_reader(void args){
	char* argv[] = (char *) args;
	int argc = length(argv);
	for(int i = argc-2;i > nonfiles;i--){
		file_open(argv[i]);
	}
}


/*ouvre et écrit dans le tableau fract les specs des fractales dans le fichier filename
type de fichier name-w-h-cR-cI*/
// /!\ commentaires #
void file_open(string filename){
	int scancount;
	bufferList * new_fract = (bufferList *) malloc(sizeof(bufferList));
	FILE *fp = fopen(filename,"r");
	for(int i = 0; pas fini; i++){
		scancount = fscanf(fp,"%s %d %d %f %f \n",new_fract->name,new_fract.w,new_fract.h,new_fract.cR,new_fract.cI);
		if(scancount != 5) {
		//erreur quelquepart
		}
		//à protéger
		new_fract->next = lastread;
		lastread = new_fract;
	}
	
}
