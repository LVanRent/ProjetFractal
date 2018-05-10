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


typedef struct bufferNode {
	struct buffer * next;
	struct fractal * fract;		
} bufferNode;

bufferNode *HeadRead;
bufferNode *HeadCalc;

void *thread_reader(void args);
void file_open(string filename);
void addToBuffer

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
void file_open(char * filename){
	int scancount=5;
	printf("fopen\n");
	printf("filename1, %s\n",filename);
	
	FILE *fp = fopen(filename,"r");
	if(fp == NULL) printf("failopen\n");
	bufferList * new_fract ;
	while(scancount != EOF){
		new_fract = (bufferList *) malloc(sizeof(bufferList));
		printf("malloc\n");
		printf("fscanf\n");
		scancount = fscanf(fp,"%s %d %d %lf %lf\n",&new_fract->name,&new_fract->w,&new_fract->h,&new_fract->cR,&new_fract->cI);
		if(scancount != 5 && scancount != EOF) {
			printf("%d %s \n", scancount,new_fract->name);
			while(new_fract->name[0]=='#'){
				while(fgetc(fp) !='\n'){}
				scancount = fscanf(fp,"%s %d %d %lf %lf \n",&new_fract->name,&new_fract->w,&new_fract->h,&new_fract->cR,&new_fract->cI);
			}
		}
		//producteur
		if(scancount == 5){
			addToBuffer();
			//printf("ajout de %s %d %d %lf %lf \n",&new_fract->name,&new_fract->w,&new_fract->h,&new_fract->cR,&new_fract->cI);
		}
	}
	free(new_fract);
		
}

