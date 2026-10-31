#include"tests/test.h"

struct sll{
	struct sll *next;
	int data;
};
struct sll *header;
void addend(void);
