#include <stddef.h>
#include <stdio.h>
#include "ft_malloc.h"
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	void *p = malloc(7);
	(void)p;
	void *p2 = malloc(7);
	(void)p2;
	void *p3 = malloc(7);
	(void)p3;

	free(p3);	
	free(p);
	free(p2);

	return 0;
}
