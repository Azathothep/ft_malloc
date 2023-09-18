#include <stddef.h>
#include <stdio.h>

void	*_malloc(size_t n);
int		_atoi(char *s);

int main(int argc, char** argv)
{
	if (argc != 3)
		return 1;

	int i = 0;
	int size = _atoi(argv[1]);
	int max = _atoi(argv[2]);
	while (i < max)
	{
		char *p = _malloc(size);
		for (int j = 0; j < size; j++)
			p[j] = 0;
		printf("malloc return: %p\n\n", p);
		i++;
	}

	return 0;
}
