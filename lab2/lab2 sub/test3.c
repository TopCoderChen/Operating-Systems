#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

int main() {
  char *x = malloc(16);
    x[1]='a';
  free(x);
    printf("%c\n",x[1]);
  return (errno);
}