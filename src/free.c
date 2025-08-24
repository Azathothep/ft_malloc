#include "malloc.h"
#include "utils.h"
#include "lst_free.h"

void	free(void *Ptr) {
	PRINT("Freeing address "); PRINT_POINTER(Ptr); NL();

	if (globalmemdatas.TinyZone.AllocSize == 0)
		return;

	t_header *Header = SLOT_TO_HEADER(Ptr);

	int SlotSize = Header->NextSlot - (void *)Header;
	//PRINT("Slot size is "); PRINT_UINT64(SlotSize); NL();
	
	t_memzone MemBlock = globalmemdatas.TinyZone;
	if (SlotSize > SMALL_ALLOC)
		MemBlock = globalmemdatas.LargeZone;
	else if (SlotSize > TINY_ALLOC)
		MemBlock = globalmemdatas.SmallZone;

	//TODO(felix): verify if block needs to be freed beforehand
	t_free *Slot = lst_free_add(&MemBlock.FreeList, SlotSize, (void *)Header);

	//Coalesce
	if (Slot->Prev != NULL
	&& Slot->Prev->Addr + Slot->Prev->Size == Slot->Addr) {
		t_free *Prev = Slot->Prev;
		PRINT("Coalescing with previous slot for total size "); PRINT_UINT64(Prev->Size + Slot->Size); NL();
		Prev->Size += Slot->Size;
		lst_free_remove(&MemBlock.FreeList, Slot);
		Slot = Prev;
	}

	if (Slot->Next != NULL
	&& Slot->Addr + Slot->Size == Slot->Next->Addr) {
		PRINT("Coalescing with following slot for total size "); PRINT_UINT64(Slot->Size + Slot->Next->Size); NL();
		Slot->Size = Slot->Size + Slot->Next->Size;
		lst_free_remove(&MemBlock.FreeList, Slot->Next);
	}

	//PRINT("Free successful\n");
}
