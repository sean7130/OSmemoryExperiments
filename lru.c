#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

/* lru is the frame number of coremap that contains the tail of the linked list.
 * lru represents the least recently used frame.
 * mru is the frame number of coremap that contains the head of the linked list.
 * mru represents the most recently used frame.
 */
int lru; 
int mru; 
// number of nodes in the linked list
int LLSize;

/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {
	// remove the head of linked list
	int victim_frame = lru;
	coremap[victim_frame].prev = -1;
	lru = coremap[victim_frame].next;
	coremap[victim_frame].next = -1;

	// if linked list had only 1 node, empty the linked list
	if (LLSize == 1) {
		LLSize = 0;
		lru = -1;
		mru = -1;
	} else {
		// if linked list size was greater than 1, update the frame of the new head of the list
		LLSize = LLSize - 1;
		coremap[lru].prev = -1;
	}
	return victim_frame;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
	// locate the page on coremap
	// it will be in coremap because coremap updates before this is called
	int p_frame = 0;
	while (p_frame < memsize && coremap[p_frame].pte != p) {
		p_frame = p_frame + 1;
	}
	// the physmem is empty, so add the node to be the head of the linked list
	if (LLSize == 0) {
		LLSize = 1;
		coremap[p_frame].prev = -1;
		coremap[p_frame].next = -1;
		mru = p_frame;
		lru = p_frame;
	} else {
		// find if the page is already in the linked list
		int cur_frame = lru;
		while (cur_frame != p_frame && cur_frame != mru) {
			cur_frame = coremap[cur_frame].next;
		}

		// if page is in linked list, move the node of this page to the end of linked list
		if (cur_frame == p_frame) {
			// edge case: if cur_frame is linked list head, move head to the end of list
			if (cur_frame == lru) {
				int temp = lru;
				lru = coremap[lru].next;
				coremap[lru].prev = -1;
				coremap[temp].next = -1;
				coremap[temp].prev = mru;
				coremap[mru].next = temp;
				mru = temp;
			} else if (cur_frame != mru) {
				int prev_frame = coremap[p_frame].prev;
				int next_frame = coremap[p_frame].next;
				coremap[prev_frame].next = next_frame;
				coremap[next_frame].prev = prev_frame;
				coremap[mru].next = p_frame;
				coremap[p_frame].next = -1;
				coremap[p_frame].prev = mru;
				mru = p_frame;
			}
		} else {
			// if the page is not in linked list, add a node of this page to the end of linked list
			LLSize = LLSize + 1;
			coremap[mru].next = p_frame;
			coremap[p_frame].prev = mru;
			coremap[p_frame].next = -1;
			mru = p_frame;
		}
	}
	return;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
	int i;
	LLSize = 0;
	lru = -1;
	mru = -1;
	// initialize the fields for linked list for lru
	for (i = 0 ; i < memsize ; i++) {
		coremap[i].next = -1;
		coremap[i].prev = -1;
	}
}
