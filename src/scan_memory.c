#include "malloc.h"
#include "utils.h"
#include <stdlib.h>

#define ANSI_COLOR_RED		"\x1b[31m"
#define	ANSI_COLOR_GREEN	"\x1b[32m"
#define ANSI_COLOR_RESET	"\x1b[0m"

#define MAX_MEM_TO_PRINT	90
#define LINE_SIZE	2

void	print_bins(t_memzone *Zone) {
	PRINT("BINS\n");

	t_header **Bins = Zone->Bins;

	int i = 0;
	while (i < Zone->BinsCount) {
		PRINT("["); PRINT_UINT64(i); PRINT("] (");
		if (i < 8)
			PRINT_UINT64((i + 1) * 8);
		else
			PRINT("+");

		PRINT(")"); NL();
		
		t_header *Ptr = Bins[i];
		while (Ptr != NULL) {
			PRINT_ADDR(Ptr); NL();
			Ptr = Ptr->NextFree;
		}
		NL();
		
		i++;
	}
}

void	print_hexdump(void *P) {
	void *Mem = *((void **)P);
	PRINT_ADDR(Mem);
}

void	print_hexdump_slot(t_header *Hdr) {
	if (Hdr == NULL) {
		PRINT("["); PRINT_ADDR(Hdr); PRINT("]: ");
		return;
	}

	size_t SlotSize = Hdr->SlotSize;
	if (SlotSize > MAX_MEM_TO_PRINT)
		SlotSize = MAX_MEM_TO_PRINT;

	int MemCount = SlotSize / 8;
	int LinesCount = MemCount / LINE_SIZE;
	void *P = (void *)Hdr;

	int i = 0;
	while (i < LinesCount) {
		PRINT("["); PRINT_ADDR(P); PRINT("]: ");
		int j = 0;
		while (j < LINE_SIZE) {
			print_hexdump(P);	
			P += 8;
			PRINT("  ");
			j++;
		}
	
		NL();
		i++;
	}

	int Remaining = MemCount - (LinesCount * LINE_SIZE);
	if (Remaining > 0) {
		PRINT("["); PRINT_ADDR(P); PRINT("]: ");
	}

	i = 0;
	while (i < Remaining) {
		print_hexdump(P);
		PRINT(" ");
		P += 8;
		i++;
	}

	if (Hdr->SlotSize > MAX_MEM_TO_PRINT)
		PRINT("...");

	NL();
}

void	scan_error(t_header *Hdr, t_header *Prev, char *Errmsg) {
	PRINT(ANSI_COLOR_RED); PRINT("["); PRINT_ADDR(Hdr); PRINT("]: CORRUPTED MEMORY - "); PRINT(Errmsg); PRINT(ANSI_COLOR_RESET); NL();

	PRINT("\n----------------- HEXDUMP -----------------\n");
	PRINT("\nPrevious header\n");
	print_hexdump_slot(Prev);
	PRINT("\nCorrupted header\n");
	print_hexdump_slot(Hdr);
	PRINT("\nNext header\n");
	print_hexdump_slot(Hdr->Next);

	exit(1);
}

void	bin_error(t_header *Hdr, char *Errmsg, t_memzone *Zone) {
	PRINT(ANSI_COLOR_RED); PRINT("["); PRINT_ADDR(Hdr); PRINT("]: CORRUPTED BINS - "); PRINT(Errmsg); PRINT(ANSI_COLOR_RESET); NL();

	PRINT("\n----------------- HEXDUMP -----------------\n");
	PRINT("\nPrev free header\n");
	print_hexdump_slot(Hdr->PrevFree);
	PRINT("\nCorrupted header\n");
	print_hexdump_slot(Hdr);
	PRINT("\nNext free header\n");
	print_hexdump_slot(Hdr->NextFree);

	print_bins(Zone);

	exit(1);
}

void	search_for_double_header(t_memzone *Zone, t_header *Hdr, int CurrentBin) {
	t_header **Bins = Zone->Bins;
	t_header *NextFree = Hdr->NextFree;
	while (CurrentBin < Zone->BinsCount) {
		while (NextFree != NULL) {
			if (Hdr == NextFree) {
				bin_error(Hdr, "Header found twice!", Zone);
			}

			NextFree = NextFree->NextFree;
		}

		CurrentBin++;
		if (CurrentBin < Zone->BinsCount)
			NextFree = Bins[CurrentBin];
	}
}

void	scan_free_integrity(t_memzone *Zone) {
	int BinIndex = 0;
	t_header **Bins = Zone->Bins;
	
	while (BinIndex < Zone->BinsCount) {
		t_header *Hdr = Bins[BinIndex];
		while (Hdr != NULL) {
			if (Hdr->PrevFree != NULL && Hdr->PrevFree->NextFree != Hdr)
				bin_error(Hdr, "Inconsistent free headers", Zone);

			if (Hdr->NextFree != NULL && Hdr->NextFree->PrevFree != Hdr)
				bin_error(Hdr, "Inconsistent free headers", Zone);

			search_for_double_header(Zone, Hdr, BinIndex);
			Hdr = Hdr->NextFree;
		}

		BinIndex++;
	}
}

void	scan_zone_integrity(t_memzone *Zone) {
	t_memchunk *Chunk = Zone->FirstChunk;

	while (Chunk != NULL) {
		t_header *Hdr = CHUNK_STARTING_ADDR(Chunk);
	
		// FIRST BLOCK CHECK
		t_header *Prev = NULL;
		while (Hdr != NULL) {

			void *AlignedHdr = (void *)SIZE_ALIGN((uint64_t)Hdr);
			if (Hdr != AlignedHdr) {
				scan_error(Hdr, Prev, "UNALIGNED HEADER");
				return;
			}

			t_header *Next = Hdr->Next;
			if (Next != NULL) {
				if (Next->Prev != Hdr) {
					scan_error(Hdr, Prev, "INCONSISTENT HEADERS");
					return;
				}
			}			

			if (Hdr->State != INUSE && Hdr->State != FREE && Hdr->State != UNSORTED_FREE) {
				scan_error(Hdr, Prev, "HEADER FREE FLAG OVERWRITTEN");
				return;
			}

			if (Hdr->State == FREE && ((uint64_t)Hdr->PrevFree == 0xffffffffffffffff || (uint64_t)Hdr->NextFree == 0xffffffffffffffff))
			{
				scan_error(Hdr, Prev, "HEADER MARKED FREE BUT INCONSISTENT HEADERS");
				return;
			}

			t_header *NextCalculated = (t_header *)((void *)Hdr + Hdr->SlotSize);
			if (Next != NULL && NextCalculated != Next) {
				scan_error(Hdr, Prev, "HOLE OR OVERLAPPING SLOT");
				return;
			}

			if (Next != NULL && (uint64_t)Next < (uint64_t)Hdr) {
				scan_error(Hdr, Prev, "CIRCULAR HEADER");
				return;
			}

			Prev = Hdr;
			Hdr = Hdr->Next;
		}
		
		Chunk = Chunk->Next;
	}
}

void	scan_memory_integrity() {
	t_memzone *Zone = GET_TINY_ZONE();
	scan_zone_integrity(Zone);
	scan_free_integrity(Zone);

	Zone = GET_SMALL_ZONE();
	scan_zone_integrity(Zone);
	scan_free_integrity(Zone);	

	Zone = GET_LARGE_ZONE();
	scan_zone_integrity(Zone);
	scan_free_integrity(Zone);
}
