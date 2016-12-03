#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>

//test return to OS

int main(){

	char *a = malloc(1800);
	char *b = malloc(1900);
	char *c = malloc(2000);
	char *d = malloc(1700);
	free(c);
	free(b);

	char *aa = malloc(1000);
	char *bb = malloc(900);
	char *cc = malloc(600);
	char *dd = malloc(700);
	char *ee = malloc(800);
	free(bb);




	return (errno);
}