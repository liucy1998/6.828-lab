#include <inc/lib.h>
#include <inc/x86.h>

#define ROUND 1000000
void
umain(int argc, char **argv)
{
	envid_t who;

	uint64_t start_cycle = 0, end_cycle = 0;
	if ((who = fork()) != 0) {
		start_cycle = read_tsc();
		ipc_send(who, 0, 0, 0);
	}

	while (1) {
		uint32_t i = ipc_recv(&who, 0, 0);
		// cprintf("%x got %d from %x\n", sys_getenvid(), i, who);
		if (i == ROUND-1)
			return;
		i++;
		ipc_send(who, i, 0, 0);
		if (i == ROUND-1) {
			if(who != 0) {
				end_cycle = read_tsc();
				cprintf("total cycles: %lld\n", end_cycle - start_cycle);
				cprintf("cycles/IPC: %lld\n", (end_cycle - start_cycle)/(ROUND));
			}
			return;
		}
	}

}