// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/trap.h>
#include <kern/pmap.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "showmappings", "Display mapping table for address in a specified range", mon_showmappings},
	{ "setmpp", "Set mapping permission", mon_setmpp}
};

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(commands); i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	// Your code here.
	cprintf("Stack backtrace:\n");
	uint32_t cur_ebp = read_ebp();
	while(cur_ebp){
		uint32_t *ret_ptr = (uint32_t *)(cur_ebp) + 1;
		uint32_t *arg_ptr = (uint32_t *)(cur_ebp) + 2;
		struct Eipdebuginfo info;
		debuginfo_eip(*ret_ptr, &info);
		cprintf("  ebp %08x  eip %08x  args", cur_ebp, *ret_ptr);
		int i = 0;
		for(; i < 5; ++i){
			cprintf(" %08x", *arg_ptr);
			arg_ptr += 4;
		}
		cprintf("\n         %s:%d: %.*s+%d\n", info.eip_file, info.eip_line, info.eip_fn_namelen, info.eip_fn_name, *ret_ptr - info.eip_fn_addr);
		cur_ebp = *(uint32_t *)(cur_ebp);
	}
	return 0;
}

static uint32_t
_stox(char *str, int *flag){
	int l = strlen(str);
	if(l <= 2){
		*flag = false;
		return 1;
	}
	if(!(str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))){
		*flag = false;
		return 1;
	}
	uint32_t res = 0;
	for(int i = 2; i < l; ++i){
		uint32_t cur;
		if(str[i] >= '0' && str[i] <= '9')
			cur = str[i] - '0';
		else if(str[i] >= 'a' && str[i] <= 'f')
			cur = str[i] - 'a' + 10;
		else if(str[i] >= 'A' && str[i] <= 'F')
			cur = str[i] - 'A' + 10;
		else{
			*flag = false;
			return 1;
		}
		res = (res << 4) + cur;
	}
	*flag = true;
	return res;
}

static void
_show_pte(uintptr_t va){

	pte_t *pte = pgdir_walk(kern_pgdir, (void *)va, 0);
	cprintf("%08p ", va);
	if(!pte)
		cprintf("NO PAGETABLE");
	else if(!(*pte & PTE_P))
		cprintf("NO PAGE");
	else{
		cprintf("%08p ", PTE_ADDR(*pte));
		if(*pte & PTE_U){
			cprintf("R  ");
			if(*pte & PTE_W)
				cprintf("W  R  W");
			else
				cprintf("-  R  -");
		}
		else{
			cprintf("-  ");
			if(*pte & PTE_W)
				cprintf("-  R  W");
			else
				cprintf("-  R  -");
		}
		
	}
	cprintf("\n");
}

int mon_showmappings(int argc, char **argv, struct Trapframe *tf){
	if(argc == 1){
		cprintf("showmappings: Display mapping table for address in a [va_start, va_end]\n");
		cprintf("Usage: showmappings <va_start> <va_end>\n");
		cprintf("NOTICE: <va_start> <va_end> must be in HEX format\n");
		return 0;
	}
	if(argc != 3){
		cprintf("Two HEX(start with '0x') arguments need to be specified: [start_addr, end_addr]!\n");
		return 1;
	}
	int flag;
	uintptr_t v_st, v_ed;
	v_st = _stox(argv[1], &flag);
	if(!flag){
		cprintf("Illegal arg #1: arg should be in hex format '^0x[0123456789abcdefABCDEF]*$'!\n");
		return 1;
	}
	v_ed = _stox(argv[2], &flag);
	if(!flag){
		cprintf("Illegal arg #2: arg should be in hex format '^0x[0123456789abcdefABCDEF]*$'!\n");
		return 1;
	}
	cprintf("st = %p, ed = %p\n", v_st, v_ed);
	if(v_ed < v_st){
		cprintf("Arg #2 shouldn't be less than arg #1!");
		return 1;
	}
	uintptr_t sz = ROUNDDOWN(v_ed, PGSIZE) - ROUNDDOWN(v_st, PGSIZE);
	uintptr_t v_cur = ROUNDDOWN(v_st, PGSIZE);
	cprintf("    VA         PA     UR UW CR CW\n");
	for(uintptr_t i = 0; i <= sz; i += PGSIZE){
		_show_pte(v_cur);
		v_cur += PGSIZE;
	}
	return 0;
}

static void
_setpermbit(pte_t *pte, uintptr_t perm, uintptr_t flag){
	uintptr_t unperm = 0xffffffff ^ perm;
	if(flag)
		*pte |= perm;
	else
		*pte &= unperm;
}

int mon_setmpp(int argc, char **argv, struct Trapframe *tf){
	if(argc == 1){
		cprintf("setmpp: Set mapping permission\n");
		cprintf("Usage: setmpp [options] <input>\n");
		cprintf("Options:\n");
		cprintf("  -u <value>:\tSet user bit to value(0/1)\n");
		cprintf("  -w <value>:\tSet write bit to value(0/1)\n");
		return 0;
	}
	int f;
	uintptr_t va = _stox(argv[1], &f);
	if(!f){
		cprintf("Illegal arg #2: arg should be in hex format '^0x[0123456789abcdefABCDEF]*$'!\n");
		return 1;
	}
	pte_t *pte = pgdir_walk(kern_pgdir, (void *)va, 0);
	if(!pte || !(*pte & PTE_P)){
		cprintf("VA specified has no mapping!\n");
		return 1;
	}

	int optflag = 1;
	uintptr_t perm, flag;
	for(int i = 2; i < argc; ++i){
		int l = strlen(argv[i]);
		if(optflag){
			if(l != 2 || argv[i][0] != '-'){
				cprintf("Illegal option!\n");	
				return 1;
			}
			switch(argv[i][1]){
				case 'u':
					perm = PTE_U;
					break;
				case 'w':
					perm = PTE_W;
					break;
				default:
					cprintf("Illegal option!\n");	
					return 1;
			}
		}
		else{
			if(l != 1 || !(argv[i][0] == '1' || argv[i][0] == '0')){
				cprintf("Illegal arg!\n");	
				return 1;
			}
			flag = argv[i][0] - '0';
			_setpermbit(pte, perm, flag);
		}
		optflag ^= 1;
	}
	cprintf("    VA         PA     UR UW CR CW\n");
	_show_pte(va);
	return 0;
}



/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);
	set_ccolor(BG_BLUE | FG_GREEN | FG_LIGHT);
	cprintf("Colorful!\n");
	set_ccolor(BG_RED | FG_BLUE | FG_LIGHT);
	cprintf("Wonderful!\n");
	set_ccolor(BG_DARK | FG_WHITE);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
