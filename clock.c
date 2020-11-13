#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int clock_arm;

/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int clock_evict() {
	// rotate the clock arm until it finds a page with ref bit of 0
	// any pages with ref bit 1 that the clock arm checked during the rotation will have ref bit set to 0
	while ((coremap[clock_arm].pte->frame & PG_REF) == PG_REF) {
		coremap[clock_arm].pte->frame = coremap[clock_arm].pte->frame & (~PG_REF);
		clock_arm = (clock_arm + 1) % memsize;
	}
	return clock_arm;
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
	// set the ref bit of the page to 1, since it was recently used
	p->frame = p->frame | PG_REF;
	return;
}

/* Initialize any data structures needed for this replacement
 * algorithm. 
 */
void clock_init() {
	clock_arm = 0;
}
