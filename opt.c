#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include "pagetable.h"

extern int memsize;

extern int debug;

extern struct frame *coremap;

extern char *tracefile;

extern pgdir_entry_t pgdir[PTRS_PER_PGDIR];

// Address points to an array of vaddrs
addr_t *list_vaddr;

int list_size;

int traverse_index;

int MAXLINE = 256;

// Check if vaddr is valid. If invalid, return -1, else return frame number
unsigned int if_valid_vaddr(addr_t vaddr){
	pgtbl_entry_t *p=NULL;
	unsigned idx = PGDIR_INDEX(vaddr);

	if (!(pgdir[idx].pde & PG_VALID)) {
		return (unsigned int) -1;
	}
	pgtbl_entry_t *pgtbl = (pgtbl_entry_t *)(pgdir[idx].pde & PAGE_MASK);
	unsigned pgtbl_index = PGTBL_INDEX(vaddr);
	p = &(pgtbl[pgtbl_index]);

	unsigned int frame_num = p->frame;
	frame_num = frame_num >> PAGE_SHIFT;

	return frame_num;
}


// Helper function that reads the trace file
addr_t *read_trace(char *to_trace) {

	FILE *infp;
	FILE *length_fp;

	addr_t *return_array;

	int length_count = 0;
	char buf[MAXLINE];
	addr_t vaddr = 0;
	char type;

	// Convert the to_trace string into two file pointers: 
	// 	1. 	The file pointer that reads the vaddr;
	// 	2. 	Another file pointer that foresees the length 
	// 		of this trace so that the return array can be properly 
	// 		sized.

	// Confirm that the trace file exists, and if so, define file pointers in
	// respect to it.
	if(to_trace != NULL) {
		if((infp = fopen(to_trace, "r")) == NULL) {
			perror("Error opening tracefile:");
			exit(1);
		}
		// this is another file pointer created to loop though before 
		// looping though tfp; this is to know how long this file is so we know
		// the size of array to be allocated. 
		if((length_fp = fopen(to_trace, "r")) == NULL) {
			perror("Error opening tracefile #2:");
			exit(1);
		} 
	}

	// This first run of file pointer (length_fp) is to calculate the length 
	// the trace.

	while(fgets(buf, MAXLINE, length_fp) != NULL) {
		length_count++; 
	}

	// Malloc an array of vaddr that is the size of length_count
	return_array = malloc(sizeof(addr_t) * (length_count));

	// Update global varible list_size
	list_size = length_count;

	int index = 0;

	while(fgets(buf, MAXLINE, infp) != NULL) {
		if(buf[0] != '=') {
			sscanf(buf, "%c %lx", &type, &vaddr);
			// This is the orgional line from replay_trace
			// access_mem(type, vaddr); 
			return_array[index] = vaddr; 
			
			index++;
		} else {
			index++;
			continue;
		}
	}

	return return_array;

	
}


/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {

	/* function opt_evict() is called at traverse_index, starting from that index
	 * each function currently in core-maps is a canadate to be evicted. 
	 *	
	 * Traverse forward from traverse_index and onwards, each node that is being 
	 * observed is removed from being a canadiate to be removed. 
	 *
	 * When all canadates to be removed are cancelled, except for the last item in
	 * the core-map, that frame# in the core-map will be removed. 
	 */

	unsigned int i;
	unsigned int frame_num;

	unsigned long remaining_candidates; 
	remaining_candidates = (unsigned long) memsize;

	// array candidate_status denotes the status of each candidate; 1 = is candidate
	// 0 otherwise. 
	int candidate_status[memsize];

	// Set everything into 1
	for (i=0;i<memsize;i++){
		candidate_status[i] = 1;
	}

	// Find the canadiate to remove by traversing alike the descriptions above;
	// If remaining_candidates == 1, the 1 remaining candidate shall be the 
	// victim page, otherwise if we traversed to the end of memsize, select an
	// arbitary remaining candidate to remove. As these pages will all never be 
	// referred again, the order of eviction does not matter. 
	i = traverse_index + 1;
	while ((i<list_size) && (remaining_candidates>1)) {
		frame_num = if_valid_vaddr(list_vaddr[i]); // MOCK RETURN VALUE IS 7

		// Make sure frame_num is not 1
		if (frame_num == (unsigned int) (-1)) {
			i++;
			continue;
		}

		// Otherwise if vaddr is pointing to an address in coremap, cross that 
		// address/frame out as candidate
		if (candidate_status[frame_num] == 1) {
			candidate_status[frame_num] = 0;
			remaining_candidates = remaining_candidates - 1;
		}
		i++;

	}

	// At this point loop exists, no matter if there is one remaining candidate or
	// multiples left, evicting the first one that is seen as canadidate will always
	// be one of the most effcient evictions.

	for (i=0; i<memsize; i++) {
		if (candidate_status[i] == 1){
			return i;
		}

	}

	return 0;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {

	traverse_index++;

	return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	// Read this trace 
	list_vaddr = read_trace(tracefile);

	traverse_index = 0;
}
