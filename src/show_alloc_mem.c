#include "malloc.h"
#include "lst_free.h"
#include "utils.h"

//TODO(felix): add chunk subdivision in zones

void	show_alloc_zone(t_memchunks *Zone) {
	void *Chunk = Zone->StartingBlockAddr;

	while (Chunk) {
		void *StartingAddr = CHUNK_STARTING_ADDR(Chunk);	
		t_header *Hdr = (t_header *)StartingAddr;	

		while (Hdr->Next) {
			PRINT_ADDR(Hdr); PRINT(": "); PRINT_UINT64(Hdr->Size); PRINT(" bytes\n");
			Hdr = Hdr->Next;
		}
		
		Chunk = GET_NEXT_CHUNK(Chunk);
	}
}

void	show_alloc_mem() {
	PRINT("\n---- full mem ----\n");

	t_memchunks *Zone = &MemoryLayout.TinyZone;
	PRINT("TINY ZONE : "); PRINT_ADDR(Zone->StartingBlockAddr); NL();
	show_alloc_zone(Zone);
	NL();

	Zone = &MemoryLayout.SmallZone;
	PRINT("SMALL ZONE : "); PRINT_ADDR(Zone->StartingBlockAddr); NL();
	show_alloc_zone(Zone);
	NL();

	Zone = &MemoryLayout.LargeZone;
	PRINT("LARGE ZONE : "); PRINT_ADDR(Zone->StartingBlockAddr); NL();
	show_alloc_zone(Zone);
	NL();

	PRINT("------------------\n");
}
