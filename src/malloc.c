#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include "libft.h"
#include "utils.h"
#include "malloc.h"
#include "lst_free.h"

t_globalmemdatas globalmemdatas;

void	*map_memory(int ZoneSize) {
	void *ptrToMappedMemory = mmap(NULL,
					ZoneSize,
					PROT_READ | PROT_WRITE,
					MAP_ANON | MAP_ANONYMOUS | MAP_PRIVATE, // TODO(felix): change this to support multithreaded programs
					-1, //fd
					0); //offset_t

	if (ptrToMappedMemory == MAP_FAILED) {
		PRINT_ERROR("Failed to map memory, errno = "); PRINT_UINT64(errno); NL();
		return NULL;
	}

	PRINT("Successfully mapped "); PRINT_UINT64(ZoneSize); PRINT(" bytes of memory to addr ");
	PRINT_POINTER(ptrToMappedMemory); NL();
	return ptrToMappedMemory;
}

t_free	*get_free_slot(t_free **begin_lst, size_t size) {
	t_free *lst = *begin_lst;
	if (lst == NULL)
		return NULL;

	while (lst->Size < size) {
		lst = lst->Next;
	}

	return lst;
}

void	*malloc_slot(size_t size, t_memzone *MemZone) {
	size_t AlignedSize = SIZE_ALIGN(size);
	size_t RequestedSize = AlignedSize + HEADER_SIZE;

	t_free *Slot = get_free_slot(&MemZone->FreeList, RequestedSize);
	if (Slot == NULL) {
		PRINT("No free block of request size is available\n");
		return NULL;
	}

	void *Addr = Slot->Addr;
	int MinSlotSize = HEADER_SIZE + 8;
	if (Slot->Size >= (RequestedSize + MinSlotSize)) { // if there is enough space to make another slot
		Slot->Size -= RequestedSize;
		Addr += Slot->Size;
	} else {
		lst_free_remove(&MemZone->FreeList, Slot);
	}
	
	t_header *hdr = (t_header *)Addr;
	if (Slot->Addr != Addr)
		hdr->PrevSlot = Slot->Addr;
	else
		hdr->PrevSlot = NULL;

	hdr->NextSlot = Addr + RequestedSize;

	PRINT("Allocated "); PRINT_UINT64(AlignedSize); PRINT(" bytes at address "); PRINT_POINTER(HEADER_TO_SLOT(Addr)); NL();

	return HEADER_TO_SLOT(Addr);
}

void	*malloc_zone(size_t size, t_memzone *MemZone) {
	
	if (MemZone->StartingBlockAddr == NULL) {
		void *ptr = map_memory(MemZone->AllocSize);
		if (ptr == NULL)
			return NULL;
		MemZone->StartingBlockAddr = ptr;
		lst_free_add(&MemZone->FreeList, MemZone->AllocSize, ptr);
	}	

	return malloc_slot(size, MemZone);
}

void	malloc_init() {
	//PRINT("Initing malloc...\n");

	int PageSize = getpagesize();
	
	int MinZoneSize;
	int PageMultiple;	

	MinZoneSize = sizeof(t_free) * 100;
	PageMultiple = (MinZoneSize / PageSize) + 1;
	globalmemdatas.LstZone.AllocSize = PageSize * PageMultiple;
	//PRINT("Initializing FreeZone.AllocSize to "); PRINT_UINT64(PageSize * PageMultiple); NL();

	MinZoneSize = (TINY_ALLOC + HEADER_SIZE) * MIN_ENTRY;
	PageMultiple = (MinZoneSize / PageSize) + 1;
	globalmemdatas.TinyZone.AllocSize = PageSize * PageMultiple;
	//PRINT("Initializing TinyZone.AllocSize to "); PRINT_UINT64(PageSize * PageMultiple); NL();

	MinZoneSize = (SMALL_ALLOC + HEADER_SIZE) * MIN_ENTRY;
	PageMultiple = (MinZoneSize / PageSize) + 1;
	globalmemdatas.SmallZone.AllocSize = PageSize * PageMultiple;
	//PRINT("Initializing SmallZone.AllocSize to "); PRINT_UINT64(PageSize * PageMultiple); NL();
}

void	*malloc(size_t size) {
	PRINT("Malloc request of size "); PRINT_UINT64(size); NL();
	
	if (globalmemdatas.TinyZone.AllocSize == 0)
		malloc_init();

	if (size > SMALL_ALLOC) {
		//PRINT("Allocating large zone\n");
		return malloc_zone(size, &globalmemdatas.LargeZone);
	} else if (size > TINY_ALLOC) {
		//PRINT("Allocating small zone\n");
		return malloc_zone(size, &globalmemdatas.SmallZone);
	}	

	//PRINT("Allocating tiny zone\n");
	return malloc_zone(size, &globalmemdatas.TinyZone);
}
