#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>

//test 64 level

int main() {
  char *a = malloc(50);
  printf("a: %p\n", a);
  char *b = malloc(60);
  printf("b: %p\n", b);
  char *c = malloc(37);
  printf("c: %p\n", c);
  free(b);
 
  free(c);
  char *e = malloc(44);
  printf("e: %p\n", e);
  char *f = malloc(46);
  printf("f: %p\n", f);

  return (errno);
}