#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include <math.h>
#include <fcntl.h>
#include "libfractal/fractal.h"

int N = 5;
pthread_mutex_t mutex;
pthread_mutex_t mutexmean;
sem_t empty;
sem_t full;


int maxthread = 5;
int showall = 0;
int nonfiles = 0;
int readFlag = 1;
int count = 0;
pthread_t reader;
pthread_t *calc;
char *outfile;

struct fractal *HeadRead; //head de la FIFO des fractales lues 
struct fractal **MaxMean; //fractale ayant la plus grande moyenne

void *thread_reader(void *args);
void *thread_calc();
void *file_open(char *filename);
void *std_open();
void pushRead(struct fractal* new_fract);
struct fractal* popRead();
int g_argc; //global argc


int file_exists(char *filename){
	int a = open(filename, O_RDONLY);
	if(a==-1) return -1;
	close(a);
	return 0;
	}

int main(int argc, char * argv[])
{
	struct fractal * MaxMeanP = fractal_new("placeholder",0,0,0.0,0.0);
	MaxMean = &MaxMeanP;
	g_argc = argc;
	pthread_mutex_init(&mutex, NULL);
	
	outfile = argv[argc-1]; //fichier output
	int i;
	int min = 3;
	if(argc < 3) {
		min = argc-1;
		}
	for(i = 1; i<min;i++){
		if(strcmp("-d",argv[i]) == 0){//récupération de showall
			showall = 1;
			i++;
			nonfiles++;
		}	
		if(strcmp("--maxthreads",argv[i]) == 0){//récupération de maxthread	
			maxthread = atoi(argv[i+1]);
			i+=2;
			nonfiles+=2;
		}
	}
	N = 3*maxthread;
	sem_init(&empty, 0 , N);  // initialisé buffer vide
	sem_init(&full, 0 , 0);   // initialisé buffer vide
	calc = (pthread_t *)malloc(sizeof(pthread_t)*maxthread);
	if(calc == NULL){
		exit(EXIT_FAILURE);
	}
	int err = pthread_create(&reader,NULL,&thread_reader,(void*) argv);//lecteur des fichiers
	if(err != 0){
		exit(EXIT_FAILURE);
	}
	for(i = 0; i < maxthread ; i++){
		err = pthread_create(&(calc[i]),NULL,&thread_calc,NULL);
		if(err != 0){
			exit(EXIT_FAILURE);
		}
	};
	pthread_join(reader,NULL);
	readFlag = 0;
	for(i = 0; i < maxthread; i++){
		//printf("join %d \n",i);
		pthread_join(calc[i],NULL);
		//printf("join2 %d \n",i);
	}
	//printf("best\n");
	if(file_exists(outfile)){
		printf("%s existe déjà",outfile);
		}
	else{
	write_bitmap_sdl(*MaxMean,outfile);
	printf("La meilleure fractale est %s\n",(*MaxMean)->name);
	}
	
	fractal_free(*MaxMean);
	free(calc);
	return(0);
}

/*routine du thread séparant les arguments en noms de fichiers pour les ouvrir individuellement dans la routine file_open()
* dans le cas d'un '-', fait appel à un thread std_open() pour écrire sur l'entrée standard en parrallèle de la création 
* des structures
*/
void *thread_reader(void *args){
	char** argv = (char **) args;
	int argc = g_argc;
	int input = 0;
	int i;
	pthread_t stdin;
	for(i = argc-2;i > nonfiles;i--){
		if(argv[i][0] == '-'){
			input = 1;
			pthread_create(&stdin,NULL,&std_open,NULL);
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
		if(count){
		current_fract = popRead();
		//struct fractal * newfract = current_fract;
		//printf("%s %d %d %lf %lf récupérée\n",newfract->name,newfract->width,newfract->height,newfract->a,newfract->b);
		w = fractal_get_width(current_fract);
		h = fractal_get_height(current_fract);
		int mean = 0;
		for(i = 0; i<w;i++){
			for(j = 0; j<h ; j++){
				//printf("mean\n");
				mean += fractal_compute_value(current_fract,i,j);
				//printf("mean 2\n");
			}
		}
		double current_mean = (mean)/(w*h); 
		//printf("mean 3 %s\n",newfract->name);
		current_fract->mean = current_mean;
		if(showall){
			//printf("mean 4 \n");
			if(file_exists(outfile)){
		printf("%s existe déjà",current_fract->name);
		}
	else{
			write_bitmap_sdl(current_fract,current_fract->name);
		}	//printf("mean 5 %s \n",newfract->name);
			//printf("readFlag at mean 5: %d\n",readFlag);
		}
		//printf("premutex\n");
		pthread_mutex_lock(&mutexmean);
		//printf("postmutex\n");
		//printf("(*MaxMean)->mean %f \n",(*MaxMean)->mean);
		//printf("lu\n");
		if((*MaxMean)->mean < current_mean){
			//printf("change MaxMean\n");
			fflush(stdout);
			fractal_free(*MaxMean);
			*MaxMean = current_fract;
			
			
		}
		else{
			//printf("fractal_free\n");
			fractal_free(current_fract);
		}
		pthread_mutex_unlock(&mutexmean);
		//printf("aucune boucle \n");
	}
	}
	return(NULL);
}

/*ouvre et écrit dans la fifo des fractales à calculer les structures des fractales obtennues à partir des fichier entrés
* au format spécifié
*
void file_open(char * filename){
	int scancount=5;
	char name[64];
	int width,height;
	double a,b;
	FILE *fp = fopen(filename,"r");
	if(fp == NULL){
		exit(EXIT_FAILURE);
	}
	struct fractal* new_fract ;
	while(scancount != EOF){
		scancount = fscanf(fp,"%s %d %d %lf %lf\n",&name[0],&width,&height,&a,&b);
		new_fract = fractal_new(name,width,height,a,b);
		if(scancount != 5 && scancount != EOF) {
			while(name[0]=='#'){
				while(fgetc(fp) !='\n'){}
				scancount = fscanf(fp,"%s %d %d %lf %lf \n",&name[0],&width,&height,&a,&b);
				fractal_new(name,width,height,a,b);		
			}
		}
		if(scancount == 5){
			pushRead(new_fract);
		}
	}
	fclose(fp);
	fractal_free(new_fract);		
}*/

/*ouvre et écrit dans la fifo des fractales à calculer les structures des fractales obtennues à partir des fichier entrés
* au format spécifié*/
void *file_open(char * filename){
	FILE *fp = fopen(filename,"r");
	if(fp == NULL){
		exit(EXIT_FAILURE);
	}
	char *line = (char *) malloc(sizeof(char)*130);
	char *name;
	int width,height;
	double a,b;
	struct fractal * newfract;
	while((fgets(line,130,fp)) != NULL){
		if(line[0] != '#'){
			name = strtok(line," ");
			width = atoi(strtok(NULL, " "));
			height = atoi(strtok(NULL, " ")); 
			a = atof(strtok(NULL, " "));
			b = atof(strtok(NULL, " "));
			newfract = fractal_new(name,width,height,a,b);
			pushRead(newfract);	
			//printf("%s %d %d %lf %lf\n",newfract->name,newfract->width,newfract->height,newfract->a,newfract->b);
		}
	}
	fclose(fp);
	//printf("%s closed\n",filename);
	free(line);
	return NULL;
}


/*routine du thread qui récupère les fractales sur l'entrée standard
*/
void *std_open(){
	
	
	char *line = (char *) malloc(sizeof(char)*130);
	char *name = "vide";
	int width,height;
	double a,b;
	struct fractal * newfract;
	printf("entrez la fractale au format name width height a b séparé par des espace ou exit suivi de 0 1 2 3 pour terminer, puis pressez entrée\n");
	while(((fgets(line,130,stdin)) != NULL )&& (strcmp(name,"exit")!=0)){
		if(line[0] != '#'){
			name = strtok(line," ");
			if(strcmp(name,"exit")!=0){
				width = atoi(strtok(NULL, " "));
				height = atoi(strtok(NULL, " ")); 
				a = atof(strtok(NULL, " "));
				b = atof(strtok(NULL, " "));
				newfract = fractal_new(name,width,height,a,b);
				pushRead(newfract);	
				//printf("%s %d %d %lf %lf\n",newfract->name,newfract->width,newfract->height,newfract->a,newfract->b);
				printf("entrez la fractale au format name width height a b séparé par des espace ou exit suivi de 0 1 2 3 pour terminer, puis pressez entrée\n");
			}
			else {
	printf("exit reçu\n");
	free(line);
	return NULL;}
		}
	}
	free(line);
	

	return(NULL);
}

/*ajoute sur le début de la lifo des fractales non-calculées la fractale créée, gère également les mutex et sémaphore*/
void pushRead(struct fractal* new_fract){
	sem_wait(&empty); // attente d'un slot libre
    pthread_mutex_lock(&mutex);
	count++;
	new_fract->next = HeadRead;
	HeadRead = new_fract;
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


