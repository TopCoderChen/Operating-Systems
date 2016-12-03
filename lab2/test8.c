#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#define FREE_POISON 0xab
#define ALLOC_POISON 0xcd
//poison  test

int main() {

	unsigned char *p = (unsigned char*) malloc(1000);
	int i;

	for( i=0;i<1000;i++){
		assert(p[i] == ALLOC_POISON);
	}

	printf("p's Alloc poison works! \n");

	unsigned char * q = (unsigned char*) malloc(100);
	for( i=0;i<100;i++){
		assert(q[i] == ALLOC_POISON);
	}

	printf("q's Alloc poison works! \n");

	free(q);

	free(p);

	for( i=8;i<1000;i++){
		assert(p[i] == FREE_POISON);
	}

	printf("p's Free poison works! \n");
	for( i=8;i<100;i++){
		assert(q[i] == FREE_POISON);
	}
	printf("q's Free poison works! \n");


  	return (errno);

}