#include "lock_table_5.h"
#include "buffer.h"

#define Xlock 0
#define Slock 1


// https://www.geeksforgeeks.org/how-to-create-an-unordered_map-of-pairs-in-c/ -> reference for making unordered map with paired key
struct hash_pair{
size_t operator()(const std::pair<int64_t,pagenum_t>& pair) const
{
    int64_t hash1 = std::hash<int64_t>{}(pair.first);
    pagenum_t hash2 = std::hash<pagenum_t>{}(pair.second);
    if(hash1 != hash2){
    return hash1 ^ hash2;
    }
    return hash1;
}
};
// table_id, record_id;
std::unordered_map<std::pair<int64_t,pagenum_t>,lock_table_element_t*,hash_pair> lock_table;

pthread_mutex_t lock_table_latch;


int init_lock_table() {
    printf("lock_table_latch 만들기\n");
    lock_table_latch = PTHREAD_MUTEX_INITIALIZER;
    return 0;
}

/// @brief 
/// @param table_id 
/// @param page_id pagenum
/// @param key = record_id(value / db에서 key 값)
/// @param trx_id 트렌젝션 아이디
/// @param lock_mode 1(Slock): S lock(read)-find / 0(Xlock): Xlock(write)-update
/// @return 
lock_t* lock_acquire(int64_t table_id,pagenum_t page_id, int64_t key,int trx_id,int lock_mode) {
    
    pthread_mutex_lock(&lock_table_latch);
    printf("lock_acquire\n");
    std::pair p1(table_id,page_id);

    try{
        lock_t* lock = (lock_t*)malloc(sizeof(lock_t));
        
        if(lock_table.find(p1) == lock_table.end()){ // 첫 entry
            printf("여기는 들어가니?\n");

            //entry 세팅
            lock_table_element_t* entry = (lock_table_element_t*)malloc(sizeof(lock_table_element_t));
            entry->set_table_id(table_id);
            entry->set_page_id(page_id);
            entry->head = (lock_t*)malloc(sizeof(lock_t));
            entry->tail = (lock_t*)malloc(sizeof(lock_t));
            entry->head->next_pointer = lock;
            entry->head->prev_pointer = nullptr;
            entry->tail->next_pointer = nullptr;
            entry->tail->prev_pointer = lock;

            //lock_t 세팅
            lock->next_pointer = entry->tail;
            lock->prev_pointer = entry->head;
            lock->Sentinel_pointer = entry;
            lock->Conditional_variable =PTHREAD_COND_INITIALIZER; //처음꺼는 깨워져 있는 상태
            lock->record_id = key;
            lock->lock_mode = lock_mode;
            lock->trx_next_lock_ptr = nullptr;
            lock->owner_trx_id = trx_id;
            
            lock_table[p1] = entry;
        }
        else{
            
            //entry 세팅
            lock_table_element_t* entry = lock_table[p1];
            lock_t* temp_tail = entry->tail->prev_pointer;
            temp_tail->next_pointer =lock;
            
            //lock_t 세팅
            lock->next_pointer = entry->tail;
            lock->prev_pointer = temp_tail;
            lock->Sentinel_pointer =entry;
            lock->Conditional_variable =PTHREAD_COND_INITIALIZER;
            entry->tail->prev_pointer =lock; // temp_tail->prev_pointer
            lock->record_id = key;
            lock->lock_mode = lock_mode;
            lock->trx_next_lock_ptr = nullptr; //trx_acquire에서 할당
            lock->owner_trx_id = trx_id;
            //lock_table_wait()
            //pthread_cond_wait(&lock->Conditional_variable,&lock_table_latch); //처음꺼가 아니면 wait
        }
        pthread_mutex_unlock(&lock_table_latch);
        printf("lock_acquire 끝\n");
        return lock;
    }
    catch(int exp){
        pthread_mutex_unlock(&lock_table_latch);
        printf("lock_acquire 끝\n");
        return nullptr;
    }
}

int lock_release(lock_t* lock_obj) {
    pthread_mutex_lock(&lock_table_latch);
    lock_table_element_t* entry = lock_obj->Sentinel_pointer;
    lock_t* prev =lock_obj->prev_pointer;
    lock_t* next =lock_obj->next_pointer;
    prev->next_pointer = next; //entry->head = next
    next->prev_pointer = prev;

    if(entry->head->next_pointer == entry->tail){
        //unpin(entry->get_table_id(),entry->get_page_id()); // 최종적으로 lock table entry가 사라질때 unlock
        std::pair p1(entry->get_table_id(),entry->get_page_id());
        lock_table.erase(p1);
        free(entry->head);
        free(entry->tail);
        free(entry);
        //pthread_mutex_destroy(&lock_table_latch);
    }
    else{ //마지막이 아니라면 다음 것을 깨워줌

        prev->next_pointer = lock_obj;
        next->prev_pointer = lock_obj;
        lock_awake(lock_obj);
        prev->next_pointer = next; //entry->head = next
        next->prev_pointer = prev;
        //entry->head = next;
        //pthread_cond_signal(&next->Conditional_variable);
        
    }
    //free(lock_obj);
    pthread_mutex_unlock(&lock_table_latch);
    //printf("lock_release끝\n");
    return 0;
}

void lock_awake(lock_t* lock){
    //printf("lock_awake\n");
    //int key = lock->record_id;
    //int trx_id = lock->owner_trx_id;
    //int lock_mode = lock->lock_mode;
    
    //       | Xlock | Slock |
    //------------------------
    // Xlock | TRUE  | TRUE  |
    //------------------------
    // Slcok | TRUE  | False |
    bool conflict_matrix[2][2] = //Lock Compatibility Matrix
    {
        {true, true},
        {true, false}
    };
    //       | Xlock | Slock |
    //------------------------
    // Xlock |   1   |   2   |
    //------------------------
    // Slcok |   3   |   4   |
    int lock_mode_matrix[2][2] =
    {
        {1, 2},
        {3, 4}
    };

    lock_table_element_t* entry = lock->Sentinel_pointer;
    lock_t* start = entry->head->next_pointer;

    while(start != entry->tail){
        if(lock->owner_trx_id != start->owner_trx_id && (lock->record_id == start->record_id && conflict_matrix[lock->lock_mode][start->lock_mode])){
            printf("%d thread wake!\n",start->owner_trx_id);
            //pthread_cond_signal(&start->Conditional_variable);
            
            if(start->lock_mode == Xlock){
                pthread_cond_signal(&start->Conditional_variable);
                break;
            }
            else{
                pthread_cond_signal(&start->Conditional_variable);
            }
        }
        start = start->next_pointer;
    }
}