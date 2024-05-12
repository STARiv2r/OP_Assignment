#define ADDR_SIZE 16
#include <stdio.h>
#include <stdlib.h>

struct pcb *current;
unsigned short *pdbr;
char *pmem, *swaps;
int pfnum, sfnum;

struct pcb *myPcb;

void ku_dump_pmem(void);
void ku_dump_swap(void);

int page_cursor;
int tSlice;
char *pmfree;
char *swfree;
int n;


struct pcb {
	unsigned short pid;
	FILE *fd;
	unsigned short *pgdir;
	int vbase;
	int vlength;
	/* Add more fields as needed */
};

int ku_traverse(unsigned short va, int write);

void ku_freelist_init() {
	printf("in freelist init\n");
	/* Initializes the free list for pfnum frames in the physical memory
	 • Initializes the free list for sfnum frames in the swap space
	 • Either linked list, array, or bitmap is fine
	*/

	pmfree = (int*)malloc(pfnum * 64);
	swfree = (int*)malloc(sfnum * 64);

	for (int i = 0; i < 64 << 12; i++) {
		pmem[i] = 0;
		pmfree[i] = 0;
	}
	for (int i = 0; i < 64 << 14; i++) {
		swaps[i] = 0;
		swfree[i] = 0;
	}


}
int ku_proc_init(int argc, char *argv[]) {


	/* Initializes PCBs and page directories for n processes
		 • Either linked list or array is fine
		 • Allocates page directory– Zero-filling
		 » Will be mapped one-by-one by the page fault handler (i.e., on demand paging)– Page directory is not swapped-out
		 */
		 //page directory는 page fault에서 제외하고

	printf("proc init start");
	
	for (int i = 0; i < argc; i++) {
		printf(" %s ", argv[i]);
	}
	FILE *input = fopen(argv[1], "r");
	//FILE *inpu--t = fopen("input.txt", "r");
	if (input == NULL) {
		perror("File error: ");
		return 1;
	}

	
	myPcb = (struct pcb *)malloc(sizeof(struct pcb)*n);
	pdbr = (unsigned short *)malloc(sizeof(unsigned short)*n);
	page_cursor = 0;
	char com; //d
	int ind = 0;
	int pnum;
	char* proc[10];
	
	while (!feof(input)) {
		proc[ind] = (char *)malloc(sizeof(char) * 10);
		fscanf(input, " %d %s", &pnum, proc[ind]);
		
		
		char str[100];
		fgets(str, 100, input);
		
		ind++;
	}
	
	n = pnum;
	for (int i = 0; i < n+1; i++) {//argc = 7
		myPcb[i].fd = fopen(proc[i], "r");
		
		if (fscanf(myPcb[i].fd, " %c", &com) == EOF) {
			return 1;
		}
		
		if (fscanf(myPcb[i].fd, "%d %d", &myPcb[i].vbase, &myPcb[i].vlength) == EOF) {
			return 1;
		}
		
		myPcb[i].pid = i;
		myPcb[i].pgdir = (unsigned short *)calloc(32, sizeof(unsigned short));
		pdbr[i] = 0x0;
		
	}
	//ku_dump_pmem();
	/*for (int i = 0; i < n; i++) {
		free(proc[i]);
	}*/
	printf("proc init end successful: %d\n", n);
	return 0;

}
int ku_scheduler(unsigned short arg1) {/*
	» unsigned short: current process ID (10 for the first call)
	 » Return value 0: success,
	1: error (no processes)
	 » Selects the next process in a round-robin manner (starts from
	PID 0)
	 » Updates current and pdbr
	 */
	 //pid 10이 들어오면 0번부터 시작
	//printf("in scheduler\n");
	int pid = arg1;

	if (arg1 == 10) pid = 0;
	if (pid < 0 || pid > n) return 1;
	if (tSlice >= 5) {
		if (arg1 == n) pid = 0;
		else pid++;
		tSlice = 0;
	}
	
	current = &myPcb[pid];
	pdbr[pid] = (unsigned short)current->pgdir;
	//printf("sched end successful\n");
	tSlice++;
	return 0;
}
int current_pfn = 0;
int ku_pgfault_handler(unsigned short arg1) {
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
	//printf("in pgfault handler with: %d \n", arg1);
	unsigned short va, pa;
	int pid = current->pid;
	unsigned short *page_table = current->pgdir;
	int PFN = current_pfn;
	va = arg1;
	pa = page_table;
	pa = pa >> 6;
	pa = pa + arg1;
	printf("pa: %d\n", pa);
	

	if (va<myPcb[pid].vbase || va>=myPcb[pid].vbase + myPcb[pid].vlength) {// 범위 바깥
		return 1; // Segfault
	}
	if (pmfree[pa] == 0) {//page fault, mapping
		pmfree[pa] = 1;
		myPcb[pid].pgdir[va] = pa;
		page_table[PFN] = pa|1;
		current_pfn++;
	}
	else {//이미 있는곳 swap
		

	}
	
	//printf("pgfault handler successful\n");
	return 0;
	//page directory는 스왑안함

}
int ku_proc_exit(unsigned short arg1) {
	/* » unsigned short: process ID
	 » Return value 0: success,
	1: error (invalid PID)
	 » Removes PCB
	 » Reaps page frames and swap frames mapped (i.e., updates free
	lists
*
* */
	printf("in proc exit\n");
	int pid = arg1;

	if (pid < 0 || pid >= n) return 1; //invalid PID

	fclose(myPcb[pid].fd);
	
	for (int i = 0; i < 32; i++) {
		unsigned short vpn = i;
		unsigned short pfn = myPcb[pid].pgdir[vpn];
		if (pfn < pfnum) {
			pmem[pfn] = 0;
		}
	}
	free(myPcb[pid].pgdir);

	return 0;

}