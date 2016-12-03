#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>

//test address after malloc

int main() {

	char *x = (char*) malloc(2000);
	printf("%d: %p\n",2000, x );

	char *y = (char*) malloc(2047);
	printf("%d: %p\n", 2047, y );

	char *z = (char*) malloc(1000);
	printf("%d: %p\n",1000, z );

	char *r = (char*) malloc(700);
	printf("%d: %p\n",700, r );

	void *a = malloc(300);
  	printf("%d: %p\n", 300, a);

  	void *b = malloc(400);
  	printf("%d: %p\n",400, b);

  	void *c = malloc(15);
  	printf("%d: %p\n", 15,c);

  	void *d = malloc(32);
  	printf("%d: %p\n",32,  d);

  	return (errno);
}
