#ifndef MALLOC_H
# define MALLOC_H

# include <unistd.h>

typedef struct 	s_header {
	struct s_header	*Prev;
	struct s_header *Next;
	size_t Size;
	size_t RealSize;
}		t_header;

typedef struct	s_free {
	struct s_free	*Prev;
	struct s_free	*Next;
}		t_free;

typedef struct	s_memchunks {
	void	*StartingBlockAddr;
	t_free	*FreeList;
}		t_memchunks;

typedef struct	s_memlayout {
	t_memchunks	TinyZone;
	t_memchunks	SmallZone;
	t_memchunks	LargeZone;	
}		t_memlayout;

extern	t_memlayout MemoryLayout;

# define PAGE_SIZE		getpagesize()

# define TINY_ALLOC	64	//sizeof(t_free)
# define SMALL_ALLOC	1024 	//128 	
# define LARGE_ALLOC	(PAGE_SIZE * 20)
# define MIN_ENTRY	100

# define CHUNK_ALIGN(c)		(((c) + (PAGE_SIZE-1)) & ~(PAGE_SIZE-1)) 	

# define HEADER_SIZE		(sizeof(t_header))
# define CHUNK_HEADER     	(sizeof(size_t) + sizeof(void *)) // size of chunk + pointer to previous chunk
# define CHUNK_FOOTER	    	(sizeof(void *) + sizeof(void *)) // last header pointer + pointer to next chunk
# define CHUNK_OVERHEAD		(CHUNK_HEADER + CHUNK_FOOTER)
# define CHUNK_STARTING_ADDR(p) (p + CHUNK_HEADER)

# define MIN_CHUNK_SIZE(s)	((s + HEADER_SIZE) * MIN_ENTRY + CHUNK_OVERHEAD)

# define TINY_CHUNK		CHUNK_ALIGN(MIN_CHUNK_SIZE(TINY_ALLOC))
# define SMALL_CHUNK		CHUNK_ALIGN(MIN_CHUNK_SIZE(SMALL_ALLOC))
# define LARGE_CHUNK(s)		CHUNK_ALIGN(s + HEADER_SIZE + CHUNK_OVERHEAD)

# define CHUNK_USABLE_SIZE(s)	(size_t)(s - CHUNK_OVERHEAD)
# define GET_CHUNK_SIZE(p)	*((size_t *)(p))
# define SET_CHUNK_SIZE(p, s)	*((size_t *)p) = s
# define GET_NEXT_CHUNK(p)	(*((void **)(p + GET_CHUNK_SIZE(p) - sizeof(void *)))) // go to the end, then backtrack to previous
# define SET_NEXT_CHUNK(p, n)	(GET_NEXT_CHUNK(p) = n)
# define GET_PREV_CHUNK(p)	(*((void **)(p + sizeof(size_t)))) // go to the chunk's second slot
# define SET_PREV_CHUNK(p, n)	(GET_PREV_CHUNK(p) = n)

# define ALIGNMENT		16
# define SIZE_ALIGN(s)		(((s) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

# define TINY_SPACE_MIN		(ALIGNMENT + HEADER_SIZE)
# define SMALL_SPACE_MIN	(SIZE_ALIGN(TINY_ALLOC + 1) + HEADER_SIZE)
# define LARGE_SPACE_MIN	(SIZE_ALIGN(SMALL_ALLOC + 1) + HEADER_SIZE) 

# define GET_HEADER(p)		((t_header *)((void *)p - HEADER_SIZE))	
# define GET_SLOT(p)		  ((t_free *)((void *)p + HEADER_SIZE))

# define SLOT_USABLE_SIZE(p) 	(((GET_HEADER(p))->Size))
//# define SLOT_FULL_SIZE(p)  	((GET_HEADER(p))->Size) 

# define GET_FREE_SIZE(p)	(GET_HEADER(p)->RealSize)
# define SET_FREE_SIZE(p, s)	GET_HEADER(p)->RealSize = s

# define ALLOC_FLAG		1
# define FLAG(p)		((t_header *)((uint64_t)p | ALLOC_FLAG))
# define UNFLAG(p)		((t_header *)((uint64_t)p & (~ALLOC_FLAG)))
# define IS_FLAGGED(p)		((uint64_t)p & ALLOC_FLAG)

void	*lst_free_add(t_free **BeginList, void *Addr);
void	lst_free_remove(t_free **BeginList, t_free *Slot);
void	*get_free_addr(t_free *Slot);

#endif
