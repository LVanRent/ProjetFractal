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

struct fractal *HeadRead; //head de la FIFO des fractales lues 
struct fractal* HeadCalc; //head de la fifo des fractales calculés (soit à afficher, soit les fractales de même moyenne)

void *thread_reader(void args);
void file_open(string filename);
void pushRead(struct fractal* new_fract);
struct fractal* popRead();
void pushCalc(struct fractal * new_fract);
void freeCalc(struct fractal * head);


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
	for(int i = 0; i < maxthreads-1; i++){
		pthread_create(calc[i],&thread_calc,NULL);
	}
	pthread_join(reader,NULL);
	for(int i = 0; i < maxthreads-1; i++){
		pthread_join(cacl[i],NULL);
	}
	if(showall == 0){
		//afficher le BMP du meilleu
	}
	char* outfile = argv[argc-1]; //fichier output
	
}

/*routine du thread séparant les arguments en noms de fichiers pour les ouvrir individuellement dans la routine file_open()
*/
void *thread_reader(void args){
	char* argv[] = (char *) args;
	int argc = length(argv);
	int input = 0;
	for(int i = argc-2;i > nonfiles;i--){
		if(argv[i][0] == '-'){
			input = 1;
			pthread_create(stdin,&std_open,NULL);
			i++;
		}
		else{
			file_open(argv[i]);
		}
	}
	if(input){
		pthread_join(stdin,NULL);
	}
}


/*ouvre et écrit dans le tableau fract les specs des fractales dans le fichier filename
type de fichier name-w-h-cR-cI*/
void file_open(char * filename){
	int scancount=5;
	FILE *fp = fopen(filename,"r");
	if(fp == NULL) printf("failopen\n");
	struct fractal* new_fract ;
	while(scancount != EOF){
		new_fract = (struct fractal*) malloc(sizeof(struct fractal*));
		printf("malloc\n");
		printf("fscanf\n");
		scancount = fscanf(fp,"%s %d %d %lf %lf\n",&new_fract->name,&new_fract->w,&new_fract->h,&new_fract->a,&new_fract->b);
		if(scancount != 5 && scancount != EOF) {
			printf("%d %s \n", scancount,new_fract->name);
			while(new_fract->name[0]=='#'){
				while(fgetc(fp) !='\n'){}
				scancount = fscanf(fp,"%s %d %d %lf %lf \n",&new_fract->name,&new_fract->w,&new_fract->h,&new_fract->cR,&new_fract->cI);
			}
		}
		//producteur
		if(scancount == 5){
			pushRead(new_fract);
			//printf("ajout de %s %d %d %lf %lf \n",&new_fract->name,&new_fract->w,&new_fract->h,&new_fract->cR,&new_fract->cI);
		}
	}
	free(new_fract);
		
}

void std_open(){
	int scancount=5;
	int exit = 0;
	while(!exit){
		struct fractal* new_fract ;
		new_fract = (struct fractal*) malloc(sizeof(struct fractal*));
		printf("entrez la fractale au format nom withd height a b séparé par des espace et appuyez sur entrer pour valider ou entrez exit pour terminer\n");
		scancount = fscanf(stdin,"%s %d %d %lf %lf\n",&new_fract->name,&new_fract->w,&new_fract->h,&new_fract->a,&new_fract->b);
		if(strcomp(new_fract,"exit") == 1){
			fractal_free(new_fract);
			exit = 1;
		}
		else{
			pushRead(new_fract);	
			printf("fractale enregistrée\n");
		}
	}		
}

void pushRead(struct fractal* new_fract){
	//zone critique
	new_fract->next = HeadRead;
	HeadRead = new_fract;
	//end zone critique
}

struct fractal* popRead(){
	//zone critique
	struct fractal* new_fract = HeadRead;
	HeadRead = new_fract->next;
	//end zone critique
	return(new_fract);
}

void pushCalc(struct fractal* new_fract){
	struct fract *oldList;
	//zone critique
	if(!showall){
		//zone critique
		if(new_fract->mean > HeadCalc->mean){
			oldList = HeadCalc;
			HeadCalc = new_fract;
		}
		else if(new_fract->mean == HeadCalc->mean){
			new_fract->next = HeadCalc;
			HeadCalc = new_fract;
		}
		else{
			oldList = new_fract;
		}
	}
	else{
		new_fract->next = HeadCalc;
		HeadCalc = new_fract;	
	}
	//end zone critique
	if(oldList != NULL){
		freeCalc(oldList);
	}
}

void freeCalc(struct fract* head){
	struct fractal * toFree;
	while(head =! NULL){
		toFree = head;
		head = head->next;
		fractal_free(toFree);
	}
}



