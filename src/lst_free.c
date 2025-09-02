#include "malloc.h"
#include "utils.h"

void	*get_free_addr(t_free *Slot) {
	return (void *)GET_HEADER(Slot);
}

void	*lst_free_add(t_free **BeginList, void *Addr) {
	
	t_free *Slot = (t_free *)Addr;
	Slot->Prev = NULL;
	Slot->Next = NULL;

	t_free *List = *BeginList;
	
	*BeginList = Slot;
	Slot->Next = List;
	
	if (List != NULL)
		List->Prev = Slot;
	
	return Slot;
}

void	lst_free_remove(t_free **BeginList, t_free *Slot) {
	if (*BeginList == Slot) {
		*BeginList = Slot->Next;
		return;
	}

	t_free *Prev = Slot->Prev;
	t_free *Next = Slot->Next;
	if (Prev != NULL)
		Prev->Next = Next;

 	if (Next != NULL)
		Next->Prev = Prev;	
	
	return;
}
