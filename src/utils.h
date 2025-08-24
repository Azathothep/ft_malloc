#ifndef MALLOC_UTILS_H
# define MALLOC_UTILS_H

#include "libft.h"
#include <stdint.h>

void	write_uint64(int fd, uint64_t n);
void	write_uint64_hex(int fd, uint64_t n);

# define PRINT(s) write(1, s, ft_strlen(s))
# define PRINT_ERROR(s) write(2, s, ft_strlen(s))
# define PRINT_UINT64(n) write_uint64(1, n)
# define PRINT_POINTER(p) write_uint64_hex(1, (uint64_t)p)
# define NL() write(1, "\n", 1);

#endif
