#ifndef FT_MALLOC_H
# define FT_MALLOC_H

# include <unistd.h>

void	*malloc(size_t size);
void	free(void *ptr);

void	print_mem();

#endif
