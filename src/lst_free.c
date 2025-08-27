#include "malloc.h"
#include "utils.h"

void	*lst_free_get_slot(void *blockAddr) {
	void *current = blockAddr;	

	while (((t_free *)current)->Size != 0) {
		current += sizeof(t_free);
	}

	return current;
}

t_free	*lst_free_new(int size, void *addr) {
	t_memchunks *MemZone = &MemoryLayout.InternZone;
			
	if (MemZone->StartingBlockAddr == NULL) {
		//PRINT("Allocating page for free lists\n");
		void *ptr = map_memory(INTERN_CHUNK);
		if (ptr == NULL)
			return NULL;

		MemZone->StartingBlockAddr = ptr;
	}

	t_free *slot = lst_free_get_slot(MemZone->StartingBlockAddr);
	
	slot->Size = size;
	slot->Addr = addr;
	slot->Prev = NULL;
	slot->Next = NULL;

	return slot;
}

void	clear_slot(t_free *slot) {
	slot->Size = 0;
	slot->Next = NULL;
}

void	*lst_free_add(t_free **begin_lst, int size, void *addr) {
	t_free *slot = lst_free_new(size, addr);

	if (*begin_lst == NULL) {
		*begin_lst = slot;
		return slot;
	}

	t_free *lst = *begin_lst;
	while (lst->Next != NULL && lst->Next->Addr < addr) {
		lst = lst->Next;
	}

	if (lst->Next != NULL) {
		slot->Next = lst->Next;
		lst->Next->Prev = slot;
	}
	
	slot->Prev = lst;
	lst->Next = slot;
	return slot;
}

void	lst_free_remove(t_free **begin_lst, t_free *slot) {
	if (*begin_lst == slot) {
		*begin_lst = slot->Next;
		clear_slot(slot);
		return;
	}

	t_free *lst = *begin_lst;
	while (lst->Next != slot && lst->Next != NULL)
		lst = lst->Next;

	if (lst->Next == NULL) {
		PRINT("Couldn't find slot to remove in free list\n");
		return;
	}

	lst->Next = slot->Next;
	slot->Prev = lst;
	clear_slot(slot);
	return;
}
