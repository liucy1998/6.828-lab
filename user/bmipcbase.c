#include <inc/lib.h>
#include <inc/x86.h>

#define ROUND 10000
int __attribute__ ((noinline)) add(int i) {
    i++;
    return i;
}
void
umain(int argc, char **argv)
{
	uint64_t start_cycle = 0, end_cycle = 0;
    start_cycle = read_tsc();
    uint32_t i = 0;
	while (1) {
		i = add(i);
		if (i == ROUND-1) {
            end_cycle = read_tsc();
            cprintf("total cycles: %lld\n", end_cycle - start_cycle);
            cprintf("cycles/IPC: %lld\n", (end_cycle - start_cycle)/(ROUND));
			return;
		}
        i = add(i);
	}

}