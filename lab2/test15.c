#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>

#define FREE_POISON 0xab
#define ALLOC_POISON 0xcd

int main(){

	char *b = malloc(3000);
	char *y = malloc(16);
	b[10] = 'a';
	printf("\n");
	printf("c9: %c\n",b[10]);
	printf("\n");
	free(y);
	char *a = malloc(5000);
	char *d = malloc(3400);
	char *e = malloc(100);
	free(d);
	free(e);
	// printf("c9: %c\n",c9[10]);
	return (errno);

}