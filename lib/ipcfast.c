#include <inc/ipcfast.h>



// void ipc_send_fast(envid_t to_env, struct FastIPCMsg* src, int num){
//     int32_t ret;
//     switch (num) {
//         case 0:
//             return -1;
//         case 1:
//             asm volatile("int %1\n"
//                     : "=a" (ret)
//                     : "i" (T_IPC),
//                     "a" (to_env),
//                     "d" (num),
//                     "c" (src->_n[0])
//                     : "cc", "memory");
//             break;
//         case 2:
//             asm volatile("int %1\n"
//                     : "=a" (ret)
//                     : "i" (T_IPC),
//                     "a" (to_env),
//                     "d" (num),
//                     "c" (src->_n[0]),
//                     "b" (src->_n[1])
//                     : "cc", "memory");
//             break;
//         case 3:
//             asm volatile("int %1\n"
//                     : "=a" (ret)
//                     : "i" (T_IPC),
//                     "a" (to_env),
//                     "d" (num),
//                     "c" (src->_n[0]),
//                     "b" (src->_n[1]),
//                     "D" (src->_n[2])
//                     : "cc", "memory");
//             break;
//         default:
//             asm volatile("int %1\n"
//                     : "=a" (ret)
//                     : "i" (T_IPC),
//                     "a" (to_env),
//                     "d" (num),
//                     "c" (src->_n[0]),
//                     "b" (src->_n[1]),
//                     "D" (src->_n[2]),
//                     "S" (src->_n[3])
//                     : "cc", "memory");
//             break;
//     }
//     return ret;
// }
int32_t fast_ipc_send_recv_signal(envid_t to_env, int val, envid_t *from_env) {
    int32_t ret;
    asm volatile("int %1\n"
            : "=a" (ret)
            : "i" (T_IPC_SEND_SIG),
            "a" (to_env),
            "d" (val),
            "c" (1)
            : "cc", "memory");
    if(ret < 0) {
        *from_env = -1;
        return ret;
    }
    else {
        *from_env = thisenv->env_ipc_from;
        return thisenv->env_ipc_value;
    }
}
int32_t fast_ipc_send_signal(envid_t to_env, int val) {
    int32_t ret;
    asm volatile("int %1\n"
            : "=a" (ret)
            : "i" (T_IPC_SEND_SIG),
            "a" (to_env),
            "d" (val),
            "c" (0)
            : "cc", "memory");
    return ret;
}

int32_t fast_ipc_recv_signal(envid_t *from_env){
    int32_t ret;
    asm volatile("int %1\n"
            : "=a" (ret)
            : "i" (T_IPC_RECV_SIG)
            : "cc", "memory");
    if(ret < 0) {
        *from_env = -1;
        return ret;
    }
    else {
        *from_env = thisenv->env_ipc_from;
        return thisenv->env_ipc_value;
    }
}