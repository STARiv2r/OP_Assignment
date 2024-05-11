#define ADDR_SIZE 16
#include <stdio.h>
#include <stdlib.h>

struct pcb *current;
unsigned short *pdbr;
char *pmem, *swaps;
int pfnum, sfnum;

void ku_dump_pmem(void);
void ku_dump_swap(void);


int *pmfree;
int *swfree;

struct pcb{
	unsigned short pid;
	FILE *fd;
	unsigned short *pgdir;
	/* Add more fields as needed */
};
void ku_freelist_init(){
	/* Initializes the free list for pfnum frames in the physical memory
	 • Initializes the free list for sfnum frames in the swap space
	 • Either linked list, array, or bitmap is fine
	*/

	pmfree = (int*)malloc(pfnum);
	swfree = (int*)malloc(sfnum);

}
int ku_proc_init(int argc, char *argv[]){


/* Initializes PCBs and page directories for n processes
	 • Either linked list or array is fine
	 • Allocates page directory– Zero-filling
	 » Will be mapped one-by-one by the page fault handler (i.e., on demand paging)– Page directory is not swapped-out
	 */



}
int ku_scheduler(unsigned short arg1){/*
	» unsigned short: current process ID (10 for the first call)
	 » Return value 0: success,
	1: error (no processes)
	 » Selects the next process in a round-robin manner (starts from
	PID 0)
	 » Updates current and pdbr
	 */

}
int ku_pgfault_handler(unsigned short arg1){
	/*unsigned short : virtual address that generates a page fault
	 » Return value 0: success,
	1: error (segmentation fault or no space)
	 » Searches an available page frame (sequential search from PFN 0)
	 » Performs page eviction (FIFO) and swap-out if there is no
	available page frame
	 » Fills the first-loaded page with 0s
	 » Performs swap-in if the touched page is swapped-out
	 » Updates free lists
	 » Updates PED or PTE
* */

}
int ku_proc_exit(unsigned short arg1){
	/* » unsigned short: process ID
	 » Return value 0: success,
	1: error (invalid PID)
	 » Removes PCB
	 » Reaps page frames and swap frames mapped (i.e., updates free
	lists
*
* */

}