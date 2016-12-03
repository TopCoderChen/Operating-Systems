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



  int *xx =  malloc(2000);

  xx[10] = 4444;

  int *yy =  malloc(2047);

  yy[2000] = 787;

  char *zz = (char*) malloc(1000);

  zz[909] = 'z';

  char *rr = (char*) malloc(700);
  rr[699] = 't';
  // printf("%d: %p\n",700, rr );

  void *aa = malloc(300);
  // printf("%d: %p\n", 300, aa);

  free(aa);
  
  // printf("%d: %p\n",400, bb);

  char *cc = malloc(15);
  cc[10] = '3';
    // printf("%d: %p\n", 15,cc);

  char *dd = malloc(32);
  dd[4] = 'r';
  // printf("%d: %p\n",32,  dd);


  //test poison

  // printf("test poison: \n");
  // printf("\n");

  char *y = (char*) malloc(2000);
  // printf("y malloced\n");
  char *z = (char*) malloc(2048);
  // printf("z malloced\n");
  double *k = (double*) malloc(2047);
  // printf("k malloced\n");
  int *l = (int *) malloc(2048);
  y[0] = 'y';
  z[0] = 'z';
  k[0] = 0;
  l[0] = 11111;
  free(l);
  // printf("l freed\n");
  free(k);
  // printf("k freed\n");
  free(z);
  // printf("z freed\n");
  free(y);
  // printf("y freed\n");
  // printf("check\n");
  // printf("y : %c\n",y[10]);
  // printf("z : %c\n",z[10]);

  //test free

  // printf("test free: \n");
  // printf("\n");



  char *aaa = malloc(1800);
  aaa[1111] = 'p';

  char *bbb = malloc(1900);
  char *ccc = malloc(2000);
  char *ddd = malloc(1700);

  free(ccc);
  free(bbb);
  free(ddd);
  char *aaaa = malloc(1000);
  aaaa[1]='d';
  char *bbbb = malloc(900);
  char *cccc = malloc(600);
  cccc[1]='e';
  char *dddd = malloc(700);
  char *eeee = malloc(800);
  eeee[1]='r';
  free(bbbb);
  free(dddd);

  // test 128-level of super block pool
  //by check address


  // printf("test 128-level of super block pool: \n");
  // printf("\n");


  char *a1 = malloc(128);
  a1[90] = 'i';
  // printf("a1: %p\n", a1);
  char *b1 = malloc(128);
  // printf("b1: %p\n", b1);
  char *c1 = malloc(128);
  // printf("c1: %p\n", c1);
  free(b1);
  char *d1 = malloc(128);

  d1 [100] = 'd';

  // printf("d1: %p\n", d1);
  free(c1);
  char *e1 = malloc(128);
  e1[100] = 'x';
  // printf("e1: %p\n", e1);
  int *f1 = malloc(128);

  f1[100] = 3;
  // printf("f1: %p\n", f1);

  //test 256-level 
  //by checking address
  // printf("test 256-level of super block pool: \n");
  // printf("\n");


  char *a2 = malloc(255);
  a2 [190] = 'y';
  // printf("a2: %p\n", a2);
  char *b2 = malloc(254);
  // printf("b2: %p\n", b2);
  char *c2 = malloc(253);
  // printf("c2: %p\n", c2);
  free(b2);
  char *d2 = malloc(256);
  assert( d2 == b2 );
  // printf("d2: %p\n", d2);
  free(c2);
  char *e2 = malloc(200);
  // printf("e2: %p\n", e2);
  
  assert(c2==e2);


//test 64-level
  // printf("test 64-level of super block pool: \n");
  // printf("\n");

  int *a3 = malloc(50);
  a3[40] = 7878;
  // printf("a3: %p\n", a3);
  char *b3 = malloc(60);
  // printf("b3: %p\n", b3);
  char *c3 = malloc(37);
  // printf("c3: %p\n", c3);
  free(b3);
 
  free(c3);
  char *e3 = malloc(44);

  e3[0] = 'e';
  // printf("e3: %p\n", e3);

  // printf("f3: %p\n", f3);


  // test memory poison

  // printf("test memory poison: \n");
  // printf("\n");


  // printf("a4: %p\n", a4);
  // printf("%c\n",*(a4+10) );

  char *b4 = malloc(128);
  // printf("b4: %p\n", b4);

  // printf("%c\n", *(b4+10));

  free(b4);
  // printf("%c\n", *(b4+10));


  //test memory poison
  // printf("another test memory poison: \n");
  // printf("\n");



  unsigned char *p5 = (unsigned char*) malloc(1000);
  int i;

  for( i=0;i<1000;i++){
    assert(p5[i] == ALLOC_POISON);
  }

  // printf("p5's Alloc poison works! \n");

  unsigned char * q5 = (unsigned char*) malloc(100);
  for( i=0;i<100;i++){
    assert(q5[i] == ALLOC_POISON);
  }

  // printf("q5's Alloc poison works! \n");

  free(q5);

  free(p5);

  for( i=8;i<1000;i++){
    assert(p5[i] == FREE_POISON);
  }

  // printf("p5's Free poison works! \n");
  for( i=8;i<100;i++){
    assert(q5[i] == FREE_POISON);
  }
  // printf("q5's Free poison works! \n");

  //test statistics

  // printf("test statistics: \n");
  // printf("\n");

  char *a6 = malloc(1800);

  char *b6 = malloc(1900);
  b6[1003] ='e';
  char *c6 = malloc(2000);
  char *d6 = malloc(1700);
  char *e6 = malloc(1300);
  e6[1003] ='f';
  char *f6 = malloc(1400);

  free(f6);
  free(d6);
  free(a6);
  free(c6);


  
  //test return to OS

  // printf("test return to OS: \n");
  // printf("\n");


  char *l7 = malloc(1000);
  // printf("l7: %p\n",l7 );
  char *k7 = malloc(1000);
  // printf("k7: %p\n",k7 );
  char *j7 = malloc(1000);
  // printf("j7: %p\n",j7 );

  char *i7 = malloc(1000);
  // printf("i7: %p\n",i7);
  
  char *h7 = malloc(1000);
    // printf("h7: %p\n",h7 );
  char *g7 = malloc(1000);
  // printf("g7: %p\n",g7 );
  
  char *f7 = malloc(1000);
  // printf("f7: %p\n",f7 );
  char *e7 = malloc(1000);
  // printf("e7: %p\n",e7 );
  char *d7 = malloc(1000);
  // printf("d7: %p\n",d7 );
  char *c7 = malloc(1000);
  c7[1] = 'e';
  char *b7 = malloc(1000);
  b7[333] = '1';
  char *a7 = malloc(1000);
  a7[9] = 't';
  
  free(l7);
  free(k7);
  free(j7);

  free(i7);
  free(h7);
  free(g7);

  free(f7);
  free(e7);
  free(d7);

  char* x7 = malloc(1000);
  int j;
  for( j=0;j<500;j++){
    x7[j]='a';
  }

  free(x7);
  // printf("\n");
  // printf("x7: %p\n",x7 );


  //

  return (errno);
}
