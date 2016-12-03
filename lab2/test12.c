#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>
#define FREE_POISON 0xab
#define ALLOC_POISON 0xcd
//test return to OS



int main(){

	char *l = malloc(1000);
	printf("l: %p\n",l );
	char *k = malloc(1000);
	printf("k: %p\n",k );
	char *j = malloc(1000);
	printf("j: %p\n",j );

	char *i = malloc(1000);
	printf("i: %p\n",i);
	
	char *h = malloc(1000);
		printf("h: %p\n",h );
	char *g = malloc(1000);
	printf("g: %p\n",g );
	
	char *f = malloc(1000);
	printf("f: %p\n",f );
	char *e = malloc(1000);
	printf("e: %p\n",e );
	char *d = malloc(1000);
	printf("d: %p\n",d );
	char *c = malloc(1000);
	char *b = malloc(1000);
	char *a = malloc(1000);
	
	free(l);
	free(k);
	free(j);

	free(i);
	free(h);
	free(g);

	free(f);
	free(e);
	free(d);

	char* x = malloc(1000);

	printf("\n");
	printf("x: %p\n",x );
	

}

