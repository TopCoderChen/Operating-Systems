#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#define FREE_POISON 0xab
#define ALLOC_POISON 0xcd
int main() {


  //test malloc



  void *xx = (char*) malloc(2000);

  char *yy = (char*) malloc(2048);

  int i;
  for(i=0; i<2000;i++){
    yy[i] = 'a';
  }

  void *aa = malloc(300);

  void *bb = malloc(15);

  void *cc = malloc(32);

  //test free

  
  char *c = malloc(1000);
  
  char *d = malloc(1700);

  free(c);
  
  free(d);

  char *another1 = malloc(1000);
  
  char *another2 = malloc(900);
  
  char *another3 = malloc(600);
  
  char *another4 = malloc(700);
  
  char *another5 = malloc(800);
  
  free(another1);

  free(another4);

  free(another3);

  // //test statistics

  // char *a6 = malloc(1800);
  // char *b6 = malloc(1900);
  // char *c6 = malloc(2000);
  // char *d6 = malloc(1700);
  // char *e6 = malloc(1300);
  // char *f6 = malloc(1400);

  // free(f6);
  // free(d6);
  // free(a6);
  // free(c6);


  //test poison


  // char *word = (char*) malloc(2000);

  // char *word2 = (char*) malloc(2048);


  // int *l = (int *) malloc(2048);

  // word[0] = 'y';
  // word2[3] = 'z';

  // l[2] = 11111;

  // free(l);

  // free(k);

  // free(word2);

  // free(word);

  // assert(word[0]!='y');
  // assert(word2[3]!='z');

  // assert(l[2]!=11111);

  //test 64-level



  char *a3 = malloc(50);

  char *b3 = malloc(60);

  char *c3 = malloc(37);

  assert(abs(a3-b3)%64==0);


  free(b3);
 
  free(c3);
  
  char *e3 = malloc(44);

  char *f3 = malloc(46);



  // test 128-level of super block pool
  // by check address


  char *a1 = malloc(128);

  char *b1 = malloc(128);

  char *c1 = malloc(128);

  free(b1);

  char *d1 = malloc(128);

  assert(d1==b1);

  free(c1);

  char *e1 = malloc(128);

  assert(c1==e1);

  char *f1 = malloc(128);


  // test 256-level 
  // by checking address

  char *a2 = malloc(255);

  char *b2 = malloc(254);

  char *c2 = malloc(253);

  free(b2);

  char *d2 = malloc(256);

  assert( d2 == b2 );

  free(c2);

  char *e2 = malloc(200);
  
  assert(c2==e2);


  // test memory poison


  char *a4 = malloc(128);



  char *b4 = malloc(128);




  free(b4);



  //test Memory Poison

  unsigned char *allocation = (unsigned char*) malloc(1000);

  //Alloc poison

  for(i=0;i<1000;i++){
    assert(allocation[i] == ALLOC_POISON);
  }


  unsigned char * allocation2 = (unsigned char*) malloc(100);

  for( i=0;i<100;i++){
    assert(allocation2[i] == ALLOC_POISON);
  }

  free(allocation2);

  free(allocation);

  //Free oison

  for( i=8;i<1000;i++){
    assert(allocation[i] == FREE_POISON);
  }

  for( i=8;i<100;i++){
    assert(allocation2[i] == FREE_POISON);
  }

  /*
    end of this test
  */


  //test return to OS

  char *m12 = malloc(1000);

  char *m11 = malloc(1000);

  char *m10 = malloc(1000);


  char *m9 = malloc(1000);

  
  char *m8 = malloc(1000);

  char *m6 = malloc(1000);

  
  char *m5 = malloc(1000);

  char *m4 = malloc(1000);

  char *m3 = malloc(1000);

  char *m2 = malloc(1000);

  m2[0]='c';

  char *m1 = malloc(1000);

  m1[2] = 'b';
  
  char *m0 = malloc(1000);
  
  m0[3]='a';

  free(m12);
  free(m11);
  free(m10);

  free(m9);
  free(m8);
  free(m6);

  free(m5);
  free(m4);
  free(m3);

  // m13 should have same address with m6
  char* m13 = malloc(1000);

  assert(m13==m6);

  //

  return (errno);
}
