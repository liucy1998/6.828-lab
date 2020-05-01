// Fast IPC library
#ifndef JOS_INC_IPCFAST_H
#define JOS_INC_IPCFAST_H 

#include <inc/types.h>
#include <inc/env.h>
#include <inc/syscall.h>
#include <inc/trap.h>
#include <inc/error.h>
#include <inc/assert.h>
#include <inc/lib.h>


#define FASTIPCMSG_REGN 32

struct FastIPCMsg {
    uint32_t _n[FASTIPCMSG_REGN];
};

// void ipc_send_fast(envid_t to_env, struct FastIPCMsg* src, int num);

int32_t fast_ipc_send_signal(envid_t to_env, int val);

int32_t fast_ipc_recv_signal(envid_t *from_env);

#endif