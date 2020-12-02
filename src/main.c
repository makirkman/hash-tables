#include <stdio.h>
#include <stdlib.h>

#include "inthash.h"

int main(int argc, char **argv) {

	int64 a = 20 ;
	int64 b = 1029403912 ;
	int64 c = 91824934358 ;

	printf("h1: %d\nh2: %d\n\n", h1(a) % 64, h2(a) % 64) ;
	printf("h1: %d\nh2: %d\n\n", h1(b) % 64, h2(b) % 64) ;
	printf("h1: %d\nh2: %d\n\n", h1(c) % 64, h2(c) % 64) ;

}