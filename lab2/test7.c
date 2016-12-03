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

	char *a = malloc(128);
  	printf("a: %p\n", a);
  	printf("%c\n",*(a+10) );

  	char *b = malloc(128);
  	printf("b: %p\n", b);

  	printf("%c\n", *(b+10));

  	free(b);
  	printf("%c\n", *(b+10));

  	return (errno);

}