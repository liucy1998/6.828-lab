// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	if(!((err & FEC_WR) && (((pte_t *)uvpt)[PGNUM(addr)] & PTE_COW)))
		panic("Unexpected page fault");


	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.

	// panic("pgfault not implemented");
	uintptr_t ali_addr = ROUNDDOWN(utf->utf_fault_va, PGSIZE); 
	// PFTEMP -> aliaddr
	sys_page_map(
		0,
		(void *) ali_addr,
		0,
		(void *)PFTEMP,
		PTE_U
	);
	// aliaddr -> new
	int alloc_err = sys_page_alloc(0, (void *)ali_addr, PTE_U | PTE_W);
	// PFTEMP copy -> aliaddr
	if(alloc_err)
		panic("Fail to alloc new page");
	memcpy((void *)ali_addr, (void *)PFTEMP, PGSIZE);
	sys_page_unmap(0, (void *) PFTEMP);
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	// panic("duppage not implemented");
	physaddr_t src_pte = ((pte_t *)uvpt)[pn];
	int perm = PTE_U;
	if(src_pte & (PTE_W | PTE_COW)){
		perm |= PTE_COW;
		int sys_page_err = sys_page_map(0, (void *)(pn<<PGSHIFT), envid, (void *)(pn<<PGSHIFT), perm);
		if(sys_page_err)
			panic("sys_page_map error : %d", sys_page_err);
		sys_page_err = sys_page_map(0, (void *)(pn<<PGSHIFT), 0, (void *)(pn<<PGSHIFT), perm);
		if(sys_page_err)
			panic("sys_page_map error : %d", sys_page_err);
	}
	else{
		int sys_page_err = sys_page_map(0, (void *)(pn<<PGSHIFT), envid, (void *)(pn<<PGSHIFT), perm);
		if(sys_page_err)
			panic("sys_page_map error : %d", sys_page_err);
	}


	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	// panic("fork not implemented");
	set_pgfault_handler(pgfault);
	envid_t child_env = sys_exofork();
	// child env will not start until parent env sets its status
	if(child_env == 0){
		// now we are in child env
		// set thisenv for further use
		thisenv = envs + ENVX(child_env);
		return 0;
	}
	else if(child_env < 0)
		panic("sys_exofork fail");
	
	int err;
	// child env now only has mappings above UTOP
	// we map parent's mappings below UTOP to child env
	for(uintptr_t pt_addr = 0; pt_addr < UTOP; pt_addr += PTSIZE){
		if(!(((pde_t *)uvpd)[PDX(pt_addr)] & PTE_P))
			continue;
		for(uintptr_t pte_addr = pt_addr; pte_addr < pt_addr + PTSIZE; pte_addr += PGSIZE){
			if(!(((pte_t *)uvpt)[PGNUM(pte_addr)] & PTE_P))
				continue;
			// alloc new page for user exception stack
			if(pte_addr == UXSTACKTOP-PGSIZE){
				err = sys_page_alloc(child_env, (void *)(UXSTACKTOP-PGSIZE), PTE_W | PTE_U);
				if(err)
					panic("sys_page_alloc error : %e", err);
				continue;
			}
			duppage(child_env, PGNUM(pte_addr));	
		}
	}

	// child env has already copied page_fault_handler pointer
	// but it still needs to set _page_upcall

	extern void _pgfault_upcall();
	err = sys_env_set_pgfault_upcall(child_env, _pgfault_upcall);
	if(err)
		panic("sys_env_set_pgfault_upcall error : %e", err);

	// finally, child env can run
	// change its status to runnable
	err = sys_env_set_status(child_env, ENV_RUNNABLE);
	if(err)
		panic("sys_env_set_status error : %e", err);
	return child_env;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
