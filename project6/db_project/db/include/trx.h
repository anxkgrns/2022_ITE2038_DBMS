#ifndef __TRX_H__
#define __TRX_H__

#include "lock_table_5.h"
#include "malloc.h"
#include "pthread.h"

//1(Slock): S lock(read)-find / 0(Xlock): Xlock(write)-update
#define Xlock 0
#define Slock 1


struct trx_header_t
{
    /* data */
    int trx_id;
    lock_t* lock;
};
typedef struct trx_header_t trx_header_t;

// 0 이면 성공
int init_trx_table();

// conflict하면 0 non conflict이면 -1
int trx_conflict(lock_table_element_t* entry,lock_t* lock);

//dead_lock 발생하면 0 아니면 -1;
int trx_dead_check(int trx_id);

//success return 0 / fail return -1
int trx_acquire(lock_t* lock,int trx_id);

int trx_abort(int trx_id);

//return unique_trx_id /fail: 0
int trx_begin();

//success: trx_id / fail: 0
int trx_commit(int trx_id);

#endif /* __TRX_H__ */