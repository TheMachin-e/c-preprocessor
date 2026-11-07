/*
#define MACRO 80

#ifdef MACRO
	printf("hi");
	#ifdef MACRO1
		#define MACRO1 90   
		#ifdef macro hi
		printf("BI");
		#endif
	#else
		MACRO1
	#endif
#else
	printf("HI");
#endif

printf("hello");
//#else
printf("hehe");
*/
/*
#define A

#ifdef A
int x = 10;
#endif
*/
/*
#ifndef A
int x = 10;
#else
int x = 20;
#endif
*/
/*
#ifdef UNKNOWN
    #ifdef A
        int x = 10;
    #else
        int x = 20;
    #endif
#else
    int x = 30;
#endif
*/
#ifdef UNKNOWN
    #define B 100
    #undef A
    #include "missing.h"
#endif

int x = B;
