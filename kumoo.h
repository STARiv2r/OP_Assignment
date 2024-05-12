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

int *pmfree;
int *swfree;
int n;


struct pcb{
	unsigned short pid;
	FILE *fd;
	unsigned short *pgdir;
	int vbase;
	int vlength;
	/* Add more fields as needed */
};

int ku_traverse(unsigned short va, int write);

void ku_freelist_init(){
	printf("in freelist init\n");
	/* Initializes the free list for pfnum frames in the physical memory
	 • Initializes the free list for sfnum frames in the swap space
	 • Either linked list, array, or bitmap is fine
	*/

	pmfree = (int*)malloc(pfnum*sizeof(int));
	swfree = (int*)malloc(sfnum*sizeof(int));

	for(int i= 0; i< 64<<12; i++){
		pmem[i] = 0;
	}
	for(int i= 0; i< 64<<14; i++){
		swaps[i] = 0;
	}

	for(int i= 0; i<pfnum; i++){
		pmem[i] = 0;
		pmfree[i] = 0;
	}

	for(int i= 0; i<sfnum; i++){
		swaps[i] = 0;
		swfree[i] = 0;
	}

}
int ku_proc_init(int argc, char *argv[]){


/* Initializes PCBs and page directories for n processes
	 • Either linked list or array is fine
	 • Allocates page directory– Zero-filling
	 » Will be mapped one-by-one by the page fault handler (i.e., on demand paging)– Page directory is not swapped-out
	 */
//page directory는 page fault에서 제외하고

	printf("check");
	printf("in proc init and n: %d\n", n);
	printf("check");
	printf("%d\n", argc);
	printf("checka");
	for(int i= 0; i < argc; i++) {
		printf(" %s ", argv[i]);
	}
	FILE *input =fopen(argv[1], "r");
	myPcb = (struct pcb *)malloc(sizeof(struct pcb)*n);
	pdbr = (unsigned short *)malloc(sizeof(unsigned short)*n);
	page_cursor = 0;
	char com; //d
	int ind=0;
	int pnum;
	char* proc[10];
	while(!feof(input)) {
		proc[ind] = (char *)malloc(sizeof(char)*10);
		fscanf(input, " %d %s", &pnum, proc[ind]);
		ind++;
		printf(" proc: %s ", proc[ind]);
		char str[100];
		fgets(str, 100, input);
		printf("%s", str);
	}
	n = pnum;
	for(int i=0; i < n; i++){//argc = 7
		myPcb[i].fd = fopen(proc[i], "r");
		printf("check 1\n");
		if(fscanf(myPcb[i].fd, " %c", &com)==EOF) {
			return 1;
		}
		printf("check 2\n");
		if (fscanf(myPcb[i].fd, "%d %d", &myPcb[i].vbase, &myPcb[i].vlength) ==EOF) {
			return 1;
		}
		printf("check 3\n");
		myPcb[i].pid = i;
		myPcb[i].pgdir = (unsigned short *)calloc(32, sizeof(unsigned short));
		pdbr[i] = 0x0000;
		printf("check 4\n");
	}
	//ku_dump_pmem();
	for(int i=0; i < n; i++){
		free(proc[i]);
	}
	printf("proc init end successful: %d\n", n);
	return 0;

}
int ku_scheduler(unsigned short arg1){/*
	» unsigned short: current process ID (10 for the first call)
	 » Return value 0: success,
	1: error (no processes)
	 » Selects the next process in a round-robin manner (starts from
	PID 0)
	 » Updates current and pdbr
	 */
//pid 10이 들어오면 0번부터 시작
	printf("in scheduler\n");
	int pid = arg1;
	if(arg1 == 10) pid = 0;

	if(pid < 0 || pid >=n) return 1;

	current = &myPcb[pid+1];
	pdbr[pid] = (unsigned short)current->pgdir;
	return 0;
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
	printf("in pgfault handler\n");
	unsigned short va, pa;
	int pid = current->pid;
	va = (arg1 & 0xFFC0) >> 6;
	pa = ku_traverse(va, 0);

	if(va<myPcb[pid].vbase || va>myPcb[pid].vbase+myPcb[pid].vlength) {
		return 1;
	}
	if(pmfree[pa]==0) {
		pmfree[pa] = 1;
		myPcb[pid].pgdir[va] = pa;
	}
	else {//swap out

	}
	return 0;
//page directory는 스왑안함

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
	printf("in proc exit");
	int pid = arg1;

	if(pid < 0 || pid >=n) return 1; //invalid PID

	fclose(myPcb[pid].fd);
	free(pdbr[pid]);
	for(int i =0; i < 32; i++){
		unsigned short vpn = i;
		unsigned short pfn = myPcb[pid].pgdir[vpn];
		if(pfn < pfnum) {
			pmem[pfn] = 0;
		}
	}
	free(myPcb[pid].pgdir);

	return 0;

}