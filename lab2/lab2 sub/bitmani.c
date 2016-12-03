
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h> 


int main(){
	int a = 0x431a01c;
	int b = 0xfffff000;
	printf("%04x\n",a );
	printf("%04x\n",a & b );
	
}