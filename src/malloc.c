#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include "utils.h"
#include "malloc.h"

//#define PRINT_MALLOC

t_memlayout MemoryLayout;

// TODO(felix): change this to support multithreaded programs
void	*map_memory(int ChunkSize) {
	void *ptrToMappedMemory = mmap(NULL,
					ChunkSize,
					PROT_READ | PROT_WRITE,
					MAP_ANON | MAP_ANONYMOUS | MAP_PRIVATE, 
					-1, //fd
					0); //offset_t

	if (ptrToMappedMemory == MAP_FAILED) {
		PRINT_ERROR("Failed to map memory, errno = "); PRINT_UINT64(errno); NL();
		return NULL;
	}

#ifdef PRINT_MALLOC
	PRINT("Successfully mapped "); PRINT_UINT64(ChunkSize); PRINT(" bytes of memory to addr "); PRINT_ADDR(ptrToMappedMemory); NL();
#endif	

	return ptrToMappedMemory;
}

t_header	*get_free_slot(t_header **begin_lst, size_t size) {
	t_header *lst = *begin_lst;

	// First, check for a perfect fit...	
	while (lst != NULL && lst->RealSize != size) 
        	lst = lst->NextFree;

	if (lst != NULL)
		return lst;

	lst = *begin_lst;
	
	// Then, check for at least a double fit...
	while (lst != NULL && lst->RealSize < size * 2) 
		lst = lst->NextFree;

	if (lst != NULL)
		return lst;

	lst = *begin_lst;

	// Then, check for any fit
	while (lst != NULL && lst->RealSize < size) 
		lst = lst->NextFree;

	return lst;
}

void	*alloc_chunk(t_memchunks *MemZone, size_t ChunkSize) {
	void *NewChunk = map_memory(ChunkSize);
	if (NewChunk == NULL)
		return NULL;

	SET_CHUNK_SIZE(NewChunk, ChunkSize);
	SET_NEXT_CHUNK(NewChunk, NULL);	
	SET_PREV_CHUNK(NewChunk, NULL);

	if (MemZone->StartingBlockAddr == NULL) {
		MemZone->StartingBlockAddr = NewChunk;
		return NewChunk;
	}

	void *LastChunk = MemZone->StartingBlockAddr;
	void *NextChunk = GET_NEXT_CHUNK(LastChunk);
	while (NextChunk != NULL) {
		LastChunk = NextChunk;
		NextChunk = GET_NEXT_CHUNK(NextChunk);
	}

	SET_NEXT_CHUNK(LastChunk, NewChunk);
	SET_PREV_CHUNK(NewChunk, LastChunk);
	return NewChunk;	
}

void	*malloc_block(size_t size) {
	size_t AlignedSize = SIZE_ALIGN(size);
	size_t RequestedSize = AlignedSize + HEADER_SIZE;

	t_memchunks *MemZone = NULL;
	int MinSlotSize = 0;
	if (AlignedSize > SMALL_ALLOC_MAX) {
		MemZone = &MemoryLayout.LargeZone;
		MinSlotSize = LARGE_SPACE_MIN;
	} else if (AlignedSize > TINY_ALLOC_MAX) {
		MemZone = &MemoryLayout.SmallZone;
		MinSlotSize = SMALL_SPACE_MIN;
	} else {
		MemZone = &MemoryLayout.TinyZone;
		MinSlotSize = TINY_SPACE_MIN;
	}

	t_header *Hdr = get_free_slot(&MemZone->FreeList, RequestedSize);
	if (Hdr == NULL) {
		// allocated new
		int ChunkSize = 0;
	   	if (size > SMALL_ALLOC_MAX) {
			ChunkSize = LARGE_CHUNK(size);
		
			if (ChunkSize < LARGE_PREALLOC)
				ChunkSize = LARGE_PREALLOC;
		} else if (size > TINY_ALLOC_MAX) {
			ChunkSize = SMALL_CHUNK;
    		} else {
			ChunkSize = TINY_CHUNK;
    		}

		void *NewChunk = alloc_chunk(MemZone, ChunkSize);
		if (NewChunk == NULL)
			return NULL;

		void *ChunkStartingAddr = CHUNK_STARTING_ADDR(NewChunk);

		Hdr = (t_header *)ChunkStartingAddr;
		Hdr->Prev = NULL;
		Hdr->Next = NULL;
		Hdr->Size = CHUNK_USABLE_SIZE(ChunkSize);
		Hdr->RealSize = Hdr->Size;

 		lst_free_add(&MemZone->FreeList, Hdr);
	}

	//void *Addr = get_free_addr(Slot); 
	//t_header *Hdr = (t_header *)Addr;

	if (Hdr->RealSize >= (RequestedSize + MinSlotSize)) {
		size_t NewSize = Hdr->RealSize - RequestedSize;
		Hdr->RealSize = NewSize;
		
		t_header *PrevHdr = Hdr;

		Hdr = (t_header *)((void *)Hdr + NewSize);

		//Hdr = (t_header *)Addr;
		
		Hdr->Size = AlignedSize;
		Hdr->RealSize = RequestedSize;
		Hdr->Prev = PrevHdr; 
		Hdr->Next = PrevHdr->Next;

		PrevHdr->Next = FLAG(Hdr);
	} else {
		Hdr->Size = AlignedSize;
		lst_free_remove(&MemZone->FreeList, Hdr);

		t_header *PrevHdr = UNFLAG(Hdr->Prev);
    		PrevHdr = UNFLAG(Hdr->Prev);
		if (PrevHdr != NULL) {
			PrevHdr->Next = FLAG(Hdr);
		}

	}

	// UNFLAG operation are expensive ??
	t_header *NextHdr = UNFLAG(Hdr->Next);		
	NextHdr = UNFLAG(Hdr->Next);

	if (NextHdr != NULL)
		NextHdr->Prev = FLAG(Hdr);

	void *AllocatedPtr = GET_SLOT(Hdr);

#ifdef PRINT_MALLOC
	PRINT("Allocated "); PRINT_UINT64(AlignedSize); PRINT(" ["); PRINT_UINT64(RequestedSize); PRINT("] bytes at address "); PRINT_ADDR(AllocatedPtr); NL();
#endif

	return AllocatedPtr;
}

void	*malloc(size_t size) {
	//PRINT("Malloc request of size "); PRINT_UINT64(size); NL();
	
	return malloc_block(size);
}
