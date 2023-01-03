#ifndef __LOCK_TABLE_H__
#define __LOCK_TABLE_H__

#include <stdint.h>
#include <malloc.h>
#include <pthread.h>
#include <unordered_map>
#include "page.h"



struct lock_t;
struct lock_table_element_t;

struct lock_t {
  /* GOOD LOCK :) */
  lock_t* prev_pointer;
  lock_t* next_pointer;
  lock_table_element_t* Sentinel_pointer;
  pthread_cond_t Conditional_variable;
  int record_id; // key
  int lock_mode; // 1(Slock): S lock(read)-find / 0(Xlock): Xlock(write)-update
  lock_t* trx_next_lock_ptr;;
  int owner_trx_id;
};

struct lock_table_element_t{
  int64_t table_id;
  pagenum_t page_id;
  lock_t* head;
  lock_t* tail;
  void set_table_id(int64_t table_id){
    this->table_id = table_id;
  }
  void set_page_id(pagenum_t page_id){
    this->page_id = page_id;
  }
  int64_t get_table_id(){
    return this->table_id;
  }
  int64_t get_page_id(){
    return this->page_id;
  }
};
typedef struct lock_t lock_t;
typedef struct lock_table_element_t lock_table_element_t;

/* APIs for lock table */
int init_lock_table();
void lock_awake(lock_t* lock);
lock_t *lock_acquire(int64_t table_id,pagenum_t page_id, int64_t key,int trx_id,int lock_mode);
int lock_release(lock_t* lock_obj);

#endif /* __LOCK_TABLE_H__ */
