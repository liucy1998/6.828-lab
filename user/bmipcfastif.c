#include <inc/lib.h>
#include <inc/x86.h>
#include <inc/ipcfast.h>

#define ROUND 1000000
void
umain(int argc, char **argv)
{
	envid_t who;

	uint64_t start_cycle = 0, end_cycle = 0;
    uint32_t i = 0;
	if ((who = fork()) != 0) {
		start_cycle = read_tsc();
        i = fast_ipc_send_recv_signal(who, 0, &who);
	}
    else {
        i = fast_ipc_recv_signal(&who);
    }

	while (1) {
		// cprintf("%x got %d from %x\n", sys_getenvid(), i, who);
		if (i == ROUND-1) {
			end_cycle = read_tsc();
			cprintf("total cycles: %lld\n", end_cycle - start_cycle);
			cprintf("cycles/IPC: %lld\n", (end_cycle - start_cycle)/(ROUND));
			return;
		}
		i++;
		i = fast_ipc_send_recv_signal(who, i, &who);
	}

}