#define MACRO

#ifdef MACRO
	printf("hi");
	#ifdef MACRO1
		#define MACRO1 90
		printf("BI");
	#else
		MACRO1
	#endif
#else
	printf("HI");
#endif

printf("hello");
//#else
printf("hehe");
