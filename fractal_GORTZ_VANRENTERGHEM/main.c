#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include "libfractal/fractal.h"

#define N 20
pthread_mutex_t mutex;
pthread_mutex_t mutexmean;
sem_t empty;
sem_t full;

int maxthread = 5;
int showall = 0;
int nonfiles = 0;
int readFlag = 1;
int count = 0;
pthread_t *reader;
pthread_t **calc;
char *outfile;

struct fractal *HeadRead; //head de la FIFO des fractales lues 
struct fractal *MaxMean; //fractale ayant la plus grande moyenne

void *thread_reader(void *args);
void *thread_calc();
void file_open(char *filename);
void std_open();
void pushRead(struct fractal* new_fract);
struct fractal* popRead();


int main(int argc, char * argv[])
{
	pthread_mutex_init(&mutex, NULL);
	sem_init(&empty, 0 , N);  // initialisé buffer vide
	sem_init(&full, 0 , 0);   // initialisé buffer vide

	outfile = argv[argc-1]; //fichier output
	int i;
	for(i = 1; i<3;i++){
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
	reader = (pthread_t *) malloc(sizeof(pthread_t));	
	if(reader == NULL){
		exit(EXIT_FAILURE);
	}
	calc = (pthread_t *)malloc(sizeof(pthread_t)*maxthread);
	if(calc == NULL){
		exit(EXIT_FAILURE);
	}
	int err = pthread_create(reader,&thread_reader,(void*) &argv,NULL);//lecteur des fichiers
	if(err != 0){
		exit(EXIT_FAILURE);
	}
	for(i = 0; i < maxthreads ; i++){
		err = pthread_create(calc[i],&thread_calc,NULL,NULL);
		if(err != 0){
			exit(EXIT_FAILURE);
		}
	}
	pthread_join(reader,NULL);
	readFlag = 0;
	for(i = 0; i < maxthread-1; i++){
		pthread_join(calc[i],NULL);
	}
	write_bitmap_sdl(MaxMean,outfile);
	fractal_free(MaxMean);
}

/*routine du thread séparant les arguments en noms de fichiers pour les ouvrir individuellement dans la routine file_open()
* dans le cas d'un '-', fait appel à un thread std_open() pour écrire sur l'entrée standard en parrallèle de la création 
* des structures
*/
void *thread_reader(void *args){
	char* argv[] = (char **) args;
	int argc = length(argv);
	int input = 0;
	int i;
	for(i = argc-2;i > nonfiles;i--){
		if(argv[i][0] == '-'){
			input = 1;
			pthread_create(stdin,&std_open,NULL,NULL);
			i--;
		}
		else{
			file_open(argv[i]);
		}
	}
	if(input){
		pthread_join(stdin,NULL);
	}
	return(NULL);
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
			write_bitmap_sdl(current_fract,current_fract->name);
		}
		if(MaxMean->mean < current_mean){
			pthread_mutex_lock(&mutexmean);
			fractal_free(MaxMean);
			MaxMean = current_fract;
			pthread_mutex_unlock(&mutex);
		}
		else{
			fractal_free(current_fract);
		}
	}
	return(NULL);
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
		if(scancount == 5){
			pushRead(new_fract);
		}
	}
	free(new_fract);		
}


/*routine du thread qui récupère les fractales sur l'entrée standard
*/
void *std_open(){
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
			scanf("%d %d %lf %lf\n",&new_fract->width,&new_fract->height,&new_fract->a,&new_fract->b);
			pushRead(new_fract);	
			printf("fractale enregistrée\n");
		}
	}
	return(NULL);
}


/*ajoute sur le début de la lifo des fractales non-calculées la fractale créée, gère également les mutex et sémaphore*/
void pushRead(struct fractal* new_fract){
	sem_wait(&empty); // attente d'un slot libre
    	pthread_mutex_lock(&mutex);
	new_fract->next = HeadRead;
	HeadRead = new_fract;
	count++;
	pthread_mutex_unlock(&mutex);
    	sem_post(&full); // il y a un slot rempli en plus
}

/*retire la fractale sur le début de la lifo non-calculées, gère également les mutex et sémaphore*/
struct fractal* popRead(){
	sem_wait(&full); // attente d'un slot rempli
   	pthread_mutex_lock(&mutex);
	struct fractal* new_fract = HeadRead;
	HeadRead = new_fract->next;
	count--;
	pthread_mutex_unlock(&mutex);
  	sem_post(&empty); // il y a un slot libre en plus
	return(new_fract);
}



