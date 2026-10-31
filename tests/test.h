#include<stdio.h>

struct st{
	struct st*prev;
	int rollno;
	char name[20];
	float marks;
}

void addbeg(void);
