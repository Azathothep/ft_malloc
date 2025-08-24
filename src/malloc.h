#ifndef MALLOC_H
# define MALLOC_H

# include <unistd.h>

//TODO(felix) : include payload overhead in the calculation
# define TINY_ALLOC	24 	// + overhead = + 16 = 40, * 100 = 4000
# define SMALL_ALLOC	256 	// + overhead = + 16 = 272, * 100 = 27200 
# define MIN_ENTRY	100

void	*map_memory(int size);

typedef struct 	s_header {
	void	*PrevSlot;
	void	*NextSlot;
}		t_header;

typedef struct	s_free {
	size_t	Size;
	void	*Addr;
	struct s_free	*Prev;
	struct s_free	*Next;
}		t_free;

typedef struct	s_memzone {
	int	AllocSize;
	void	*StartingBlockAddr;
	t_free	*FreeList;
}		t_memzone;

typedef struct	s_globalmemdatas {
	t_memzone	LstZone;
	t_memzone	TinyZone;
	t_memzone	SmallZone;
	t_memzone	LargeZone;	
}		t_globalmemdatas;

typedef struct	s_memblock {
}		t_memblock;

extern	t_globalmemdatas globalmemdatas;

# define HEADER_SIZE	sizeof(t_header)
# define SIZE_ALIGN(s)	((s / 8) + 1) * 8

# define SLOT_TO_HEADER(p)	(t_header *)(p - HEADER_SIZE)	
# define HEADER_TO_SLOT(p)	(void *)(p + HEADER_SIZE)

#endif
