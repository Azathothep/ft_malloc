#include <sys/mman.h>
#include <errno.h>
#include "malloc.h"
#include "utils.h"
#include "lst_free.h"
#include "ft_malloc.h"

#define ANSI_COLOR_RED		"\x1b[31m"
#define	ANSI_COLOR_GREEN	"\x1b[32m"
#define ANSI_COLOR_RESET	"\x1b[0m"

size_t	coalesce_with_prev(t_header *MiddleHdr) {
	t_header *PrevHdr = (UNFLAG(MiddleHdr))->Prev;
	t_header *NextHdr = (UNFLAG(MiddleHdr))->Next;

	(UNFLAG(PrevHdr))->Next = NextHdr;
	if (!IS_LAST_HDR(UNFLAG(NextHdr)))
		(UNFLAG(NextHdr))->Prev = PrevHdr;

  	size_t MiddleHdrSize = ((void *)(UNFLAG(MiddleHdr->Next))) - ((void *)(UNFLAG(MiddleHdr))); //MiddleHdr->Size;
  	size_t PrevHdrSize = ((void *)(UNFLAG(MiddleHdr))) - ((void *)(UNFLAG(PrevHdr)));
	size_t NewSize = MiddleHdrSize + PrevHdrSize;//(UNFLAG(PrevHdr))->Size + (UNFLAG(MiddleHdr))->Size;
	(UNFLAG(PrevHdr))->Size = NewSize;
	return NewSize;
}

void	free(void *Ptr) {

	//TODO(felix): verify if block need to be freed beforehand	

 	size_t BlockSize = SLOT_USABLE_SIZE(Ptr);
	
	PRINT("Freeing address "); PRINT_ADDR(Ptr); PRINT(" (size: "); PRINT_UINT64(BlockSize); PRINT(", Header: "); PRINT_ADDR(GET_HEADER(Ptr)); PRINT(")"); NL();
	
  	t_memchunks *MemBlock = NULL;
	if (BlockSize > SMALL_ALLOC) {
		MemBlock = &MemoryLayout.LargeZone;
	} else if (BlockSize > TINY_ALLOC) {
		MemBlock = &MemoryLayout.SmallZone;
 	} else {
  		MemBlock = &MemoryLayout.TinyZone;
	}

	t_free *Slot = lst_free_add(&MemBlock->FreeList, (void *)Ptr);

	t_header *Hdr = GET_HEADER(Slot);

	//Coalesce
	if (UNFLAG(Hdr->Prev) != NULL && IS_FLAGGED(Hdr->Prev) == 0) {
    lst_free_remove(&MemBlock->FreeList, GET_SLOT(Hdr));
		size_t NewSize = coalesce_with_prev(Hdr);
		(void)NewSize;
		//PRINT("Coalesced with previous slot for total size "); PRINT_UINT64(NewSize); NL();
		Hdr = UNFLAG(Hdr->Prev);
  	}

	if (!IS_LAST_HDR(UNFLAG(Hdr->Next)) && IS_FLAGGED(Hdr->Next) == 0) {
    t_free *NextSlot = GET_SLOT(UNFLAG(Hdr->Next));
    lst_free_remove(&MemBlock->FreeList, NextSlot);
    		size_t NewSize = coalesce_with_prev(UNFLAG(Hdr->Next));
		(void)NewSize;
		PRINT("Coalesced with following slot for total size "); PRINT_UINT64(NewSize); NL();
	}

	if (UNFLAG(Hdr->Prev) != NULL) {
		(UNFLAG(Hdr->Prev))->Next = UNFLAG(Hdr);
  	} else {
    		void **HdrPtr = (void **)(((void *)(UNFLAG(Hdr))) - sizeof(void *)); 
    		*HdrPtr = UNFLAG(Hdr);
  	}

	if (!IS_LAST_HDR(UNFLAG(Hdr->Next)))
		(UNFLAG(Hdr->Next))->Prev = UNFLAG(Hdr);

  	Hdr = UNFLAG(Hdr);

	//Unmap
  	void *PrevChunk = NULL;
	void *CurrentChunk = MemBlock->StartingBlockAddr;
	void *CurrentChunkStartingAddr = CHUNK_STARTING_ADDR(CurrentChunk);
	
  	while (CurrentChunk != NULL)
	{
		size_t ChunkSize = GET_CHUNK_SIZE(CurrentChunk);		
		size_t ChunkUsableSize = CHUNK_USABLE_SIZE(ChunkSize);

		if (Hdr == CurrentChunkStartingAddr && Hdr->Size == ChunkUsableSize) {
			lst_free_remove(&MemBlock->FreeList, GET_SLOT(Hdr));

      			PRINT(ANSI_COLOR_RED);
 			PRINT("Unmapping chunk at address "); PRINT_ADDR(CurrentChunk); PRINT(" and size "); PRINT_UINT64(GET_CHUNK_SIZE(CurrentChunk)); NL();
      			PRINT(ANSI_COLOR_RESET);

      			void *NextChunk = GET_NEXT_CHUNK(CurrentChunk);
      			if (munmap(CurrentChunk, GET_CHUNK_SIZE(CurrentChunk)) < 0)
      			{
        			PRINT_ERROR("Cannot unmap chunk, errno = "); PRINT_UINT64(errno); NL();
				return;
			}

			if (PrevChunk == NULL)	
				MemBlock->StartingBlockAddr = NextChunk;
      			else
				SET_NEXT_CHUNK(PrevChunk, NextChunk);
			
			break;
		} else {
      			PrevChunk = CurrentChunk;
			      CurrentChunk = GET_NEXT_CHUNK(CurrentChunk);
		}
	}
}
