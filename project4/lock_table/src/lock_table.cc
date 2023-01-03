#include "lock_table.h"


struct lock_t {
  /* GOOD LOCK :) */
  lock_t* prev_pointer;
  lock_t* next_pointer;
  lock_table_element_t* Sentinel_pointer;
  pthread_cond_t Conditional_variable;
  
};

struct lock_table_element_t{
  int64_t table_id;
  int64_t record_id;
  lock_t* head;
  lock_t* tail;
  void set_table_id(int64_t table_id){
    this->table_id = table_id;
  }
  void set_record_id(int64_t record_id){
    this->record_id = record_id;
  }
  int64_t get_table_id(){
    return this->table_id;
  }
  int64_t get_record_id(){
    return this->record_id;
  }
};
typedef struct lock_t lock_t;
typedef struct lock_table_element_t lock_table_element_t;

// https://www.geeksforgeeks.org/how-to-create-an-unordered_map-of-pairs-in-c/ -> reference for making unordered map with paired key
struct hash_pair{
  size_t operator()(const std::pair<int64_t,int64_t>& pair) const
  {
    int64_t hash1 = std::hash<int64_t>{}(pair.first);
    int64_t hash2 = std::hash<int64_t>{}(pair.second);
    if(hash1 != hash2){
      return hash1 ^ hash2;
    }
    return hash1;
  }
};
// table_id, record_id;
std::unordered_map<std::pair<int64_t,int64_t>,lock_table_element_t*,hash_pair> lock_table;
pthread_mutex_t lock_table_latch;



int init_lock_table() {
  lock_table_latch = PTHREAD_MUTEX_INITIALIZER;
  return 0;
}

lock_t* lock_acquire(int64_t table_id, int64_t key) {
  pthread_mutex_lock(&lock_table_latch);
  std::pair p1(table_id,key);
  lock_t* lock;
  try{
    lock = (lock_t*)malloc(sizeof(lock_t));
  }
  catch(int exp){
    return nullptr;
  }
  if(lock_table.find(p1) == lock_table.end()){
    lock_table_element_t* entry = (lock_table_element_t*)malloc(sizeof(lock_table_element_t));
    entry->set_table_id(table_id);
    entry->set_record_id(key);
    entry->head = (lock_t*)malloc(sizeof(lock_t));
    entry->tail = (lock_t*)malloc(sizeof(lock_t));

    entry->head->next_pointer = lock;
    entry->head->prev_pointer = nullptr;
    entry->tail->next_pointer = nullptr;
    entry->tail->prev_pointer = lock;
    lock->next_pointer = entry->tail;
    lock->prev_pointer = entry->head;
    lock->Sentinel_pointer = entry;
    lock->Conditional_variable =PTHREAD_COND_INITIALIZER;
    
    lock_table[p1] = entry;
  } 
  
  else{
    lock_table_element_t* entry = lock_table[p1];
    lock_t* temp_tail = entry->tail->prev_pointer;
    temp_tail->next_pointer =lock;
    lock->next_pointer = entry->tail;
    lock->prev_pointer = temp_tail;
    lock->Sentinel_pointer =entry;
    lock->Conditional_variable =PTHREAD_COND_INITIALIZER;
    entry->tail->prev_pointer =lock;
    pthread_cond_wait(&lock->Conditional_variable,&lock_table_latch);
  }

  pthread_mutex_unlock(&lock_table_latch);
  return lock;
};

int lock_release(lock_t* lock_obj) {
  pthread_mutex_lock(&lock_table_latch);
  lock_table_element_t* entry = lock_obj->Sentinel_pointer;
  lock_t* prev =lock_obj->prev_pointer;
  lock_t* next =lock_obj->next_pointer;
  prev->next_pointer = next;
  next->prev_pointer = prev;

  if(entry->head->next_pointer == entry->tail){
    std::pair p1(entry->get_table_id(),entry->get_record_id());
    lock_table.erase(p1);
    free(entry->head);
    free(entry->tail);
    free(entry);
  }
  else{
    //entry->head = next;
    pthread_cond_signal(&next->Conditional_variable);
  }
  free(lock_obj);
  pthread_mutex_unlock(&lock_table_latch);
  return 0;
}
