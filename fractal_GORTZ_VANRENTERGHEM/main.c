#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "fractal.h"

int maxthread = 5;
int showall = 0;
int nonfiles = 0;
int readFlag = 1;
int count = 0;
pthread_t reader;
pthread_t calc[];
char *outfile;

struct fractal *HeadRead; //head de la FIFO des fractales lues 
struct fractal *HeadCalc; //head de la fifo des fractales calculés (soit à afficher, soit les fractales de même moyenne)
struct fractal *MaxMean; //fractale ayant la plus grande moyenne

void *thread_reader(void args);
void *thread_calc();
void file_open(string filename);
void pushRead(struct fractal* new_fract);
struct fractal* popRead();
void pushCalc(struct fractal * new_fract);
void freeCalc(struct fractal * head);


int main(int argc, char * argv[])
{
	outfile = argv[argc-1]; //fichier output
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
	pthread_create(reader,&thread_reader,(void*) &argv[])//lecteur des fichiers
	for(int i = 0; i < maxthreads ; i++){
		pthread_create(calc[i],&thread_calc,NULL);
	}
	pthread_join(reader,NULL);
	readFlag = 0;
	for(int i = 0; i < maxthreads-1; i++){
		pthread_join(cacl[i],NULL);
	}
	write_bitmap_sdl(MaxMean,outfile));
	fractal_free(MaxMean);
}

/*routine du thread séparant les arguments en noms de fichiers pour les ouvrir individuellement dans la routine file_open()
* dans le cas d'un '-', fait appel à un thread std_open() pour écrire sur l'entrée standard en parrallèle de la création 
* des structures
*/
void *thread_reader(void args){
	char* argv[] = (char *) args;
	int argc = length(argv);
	int input = 0;
	for(int i = argc-2;i > nonfiles;i--){
		if(argv[i][0] == '-'){
			input = 1;
			pthread_create(stdin,&std_open,NULL);
			i--;
		}
		else{
			file_open(argv[i]);
		}
	}
	if(input){
		pthread_join(stdin,NULL);
	}
}


/*routine des threads qui calculent les valeurs des pixels, calcule également la moyenne, vérifie si il s'agit de la meilleure
* et converti en BMP si '-d' est spécifié
*/
void *thread_calc(){
	struct fractal * current_fract;
	int w;
	int h;
	int i;
	int j;
	while(readFlag || (count != 0)){
		current_fract = popRead();
		w = fractal_get_width(current_fract);
		h = fractal_get_height(current_fract);
		int mean = 0;
		for(i = 0; i<w;i++){
			for(j = 0; j<h ; j++){
				mean += fractal_compute_value(current_fract,i,j);
			}
		}
		double current_mean = ((double) mean)/(w*h); 
		current_fract->mean = current_mean;
		if(showall){
			write_bitmap_sdl(current_fract,current_fract->name));
		}
		if(MaxMean->mean < current_mean){
			fractal_free(MaxMean);
			MaxMean = current_frac;
		}
		else{
			fractal_free(current_fract);
		}
	}
}

/*ouvre et écrit dans la fifo des fractales à calculer les structures des fractales obtennues à partir des fichier entrés
* au format spécifié
*/
void file_open(char * filename){
	int scancount=5;
	FILE *fp = fopen(filename,"r");
	if(fp == NULL) printf("failopen\n");
	struct fractal* new_fract ;
	while(scancount != EOF){
		new_fract = (struct fractal*) malloc(sizeof(struct fractal*));
		printf("malloc\n");
		printf("fscanf\n");
		scancount = fscanf(fp,"%s %d %d %lf %lf\n",&new_fract->name,&new_fract->width,&new_fract->height,&new_fract->a,&new_fract->b);
		if(scancount != 5 && scancount != EOF) {
			printf("%d %s \n", scancount,new_fract->name);
			while(new_fract->name[0]=='#'){
				while(fgetc(fp) !='\n'){}
				scancount = fscanf(fp,"%s %d %d %lf %lf \n",&new_fract->name,&new_fract->width,&new_fract->height,&new_fract->a,&new_fract->b);
			}
		}
		//producteur
		if(scancount == 5){
			pushRead(new_fract);
		}
	}
	free(new_fract);
		
}


/*routine du thread qui récupère les fractales sur l'entrée standard
*/
void std_open(){
	int exit = 0;
	while(!exit){
		struct fractal* new_fract ;
		new_fract = (struct fractal*) malloc(sizeof(struct fractal*));
		printf("entrez le nom de la fractale ou exit pour terminer suivit de la touche entré\n");
		scanf("%s\n",&new_fract->name);
		if(strcomp(new_fract,"exit") == 0){
			fractal_free(new_fract);
			exit = 1;
		}
		else{
			printf("entrez les paramètres de la fractale au format width height a b séparé par des espace et appuyez sur entrer pour valider\n");
			scancount = scanf("%d %d %lf %lf\n",&new_fract->w,&new_fract->h,&new_fract->a,&new_fract->b);
			pushRead(new_fract);	
			printf("fractale enregistrée\n");
		}
	}		
}


/*ajoute sur le début de la lifo des fractales non-calculées la fractale créée, gère également les mutex et sémaphore*/
void pushRead(struct fractal* new_fract){
	//zone critique
	new_fract->next = HeadRead;
	HeadRead = new_fract;
	count++;
	//end zone critique
}

/*retire la fractale sur le début de la lifo non-calculées, gère également les mutex et sémaphore*/
struct fractal* popRead(){
	//zone critique
	struct fractal* new_fract = HeadRead;
	HeadRead = new_fract->next;
	count--;
	//end zone critique
	return(new_fract);
}

/* dans le cas showall = 1 ajoute la fractale calculée sur la lifo des fractales de moyenne maximum en cas d'égalité,
la remplace si meilleur, est ignoré sinon
dans le cas showall= 1 ajoute la fractale à la lifo de toutes les fractales déjà calculées*/
void pushCalc(struct fractal* new_fract){
	struct fract *oldList;
	//zone critique
	if(!showall){
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



