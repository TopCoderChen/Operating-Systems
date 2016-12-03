#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>
#define FREE_POISON 0xab
#define ALLOC_POISON 0xcd
//test return to OS

int main(){

	char *a = malloc(1800);
	char *b = malloc(1900);
	char *c = malloc(2000);
	char *d = malloc(1700);
	char *e = malloc(1300);
	char *f = malloc(1400);

	free(f);
	free(d);
	free(a);
	free(c);
	




	return (errno);
}