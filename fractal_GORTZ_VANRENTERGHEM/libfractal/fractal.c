#include <stdlib.h>
#include <string.h>
#include "fractal.h"

/* inutilisé car la foction read utilise directement */
struct fractal *fractal_new(const char *name, int width, int height, double a, double b){
     struct fractal *new=(struct fractal *)malloc(sizeof(struct fractal));
     if(new==NULL){  //En cas d'erreur lors du malloc, retourn NULL
	return NULL;
     }
     strcpy(new->name,name);
     new->width=width;
     new->height=height;
     new->a=a;
     new->b=b;   
    return new; //Une fois la nouvelle structure créée, on retourne le pointeur vers celle-ci
}

void fractal_free(struct fractal *f){
	for(int i=0;i < f->width; i++){
		if(f->dessin[i] != NULL){
		free(f->dessin[i]);
		}
	}
	free(f->dessin);
}

const char *fractal_get_name(const struct fractal *f){
    return f->name;
}

int fractal_get_value(const struct fractal *f, int x, int y){
     return f->dessin[x][y];
}

void fractal_set_value(struct fractal *f, int x, int y, int val){
    f->dessin[x][y]=val;
}

int fractal_get_width(const struct fractal *f){
    return f->width;
}

int fractal_get_height(const struct fractal *f){
    return f->height;
}

double fractal_get_a(const struct fractal *f){
    return f->a;
}

double fractal_get_b(const struct fractal *f){
    return f->b;
}
