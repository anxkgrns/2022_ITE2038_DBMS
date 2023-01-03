#include "trx.h"


//made trx_id
int unique_trx_id = 1;

//trx_id, lock_t(각 lock 요소)
std::unordered_map<int,trx_header_t*> trx_table;
pthread_mutex_t trx_table_latch;

int init_trx_table(){
    printf("trx_table_latch 만들기\n");
    trx_table_latch = PTHREAD_MUTEX_INITIALIZER;
    return 0;
}


int trx_conflict(lock_table_element_t* entry,lock_t* lock){
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
    
    lock_t* tmp = entry->head->next_pointer;
    //pagenum_t pagenum;
    while(tmp != entry->tail){
        if(tmp == lock) return -1;   
        if(tmp->owner_trx_id != lock->owner_trx_id && (tmp->record_id == lock->record_id && conflict_matrix[tmp->lock_mode][lock->lock_mode])){
            printf("conflict / %dthread(기존)가 %d thread와 충돌 발생\n",tmp->owner_trx_id,lock->owner_trx_id);
            printf("%dthread가 %d thread 기다리는 중\n",lock->owner_trx_id,tmp->owner_trx_id);
            //printf("conflict : %d, %d, %s\n",tmp->lock_mode,lock->lock_mode, conflict_matrix[tmp->lock_mode][lock->lock_mode]==1?true:false);
            return 0;
        }
        tmp = tmp->next_pointer;
    }
    /*
    lock_t* tmp = entry->tail->prev_pointer;
    //pagenum_t pagenum;
    while(tmp != entry->head){        
        if(tmp->owner_trx_id != lock->owner_trx_id && (tmp->record_id == lock->record_id && conflict_matrix[tmp->lock_mode][lock->lock_mode])){
            printf("conflict / %dthread(기존)가 %d thread와 충돌 발생\n",tmp->owner_trx_id,lock->owner_trx_id);
            printf("%dthread가 %d thread 기다리는 중\n",lock->owner_trx_id,tmp->owner_trx_id);
            //printf("conflict : %d, %d, %s\n",tmp->lock_mode,lock->lock_mode, conflict_matrix[tmp->lock_mode][lock->lock_mode]==1?true:false);
            return 0;
        }
        tmp = tmp->prev_pointer;
    }*/
    return -1;
}


int trx_dead_check(int trx_t){
    return -1; //나중에 구현
}

int trx_acquire(lock_t* lock,int trx_id){
    pthread_mutex_lock(&trx_table_latch);
    printf("trx_aquire\n");
    if(lock == nullptr){
        //trx_abort(trx_id);
        
        pthread_mutex_unlock(&trx_table_latch);
        return -1;
    }
    if(trx_table.find(trx_id) != trx_table.end()){
        trx_header_t* trx_header = trx_table[trx_id];
        lock_table_element_t* entry = lock->Sentinel_pointer;
        
        //trx_id 의 linked list의 가장 앞에 집어넣음
        lock->trx_next_lock_ptr = trx_header->lock;
        trx_header->lock = lock; // next_lock_ptr(linked list의 마지막) 에 lock 삽입

        if(trx_dead_check(trx_id) == 0){
            //trx_abort(trx_id);
            pthread_mutex_unlock(&trx_table_latch);
            return -1;
        }

        //conflict하면 재움
        while(trx_conflict(entry,lock) == 0){ 
            printf("%d thread sleep!\n",trx_id);
            //모든 mutex unlock해줘야 함
            //pthread_mutex_unlock(&trx_table_latch);
            pthread_cond_wait(&lock->Conditional_variable,&trx_table_latch);
            //pthread_mutex_lock(&trx_table_latch);
            printf("%d thread awaked!\n",trx_id);

        }
        printf("trx_aquire 끝\n");
        
        pthread_mutex_unlock(&trx_table_latch);
        return 0;
    }
    printf("trx_aquire 끝\n");
    pthread_mutex_unlock(&trx_table_latch);
    //trx_abort(trx_id);
    return -1;

}

//buffer가 file에다가 write한 경우를 전부 보고 다시 없던 일로 되돌림
void trx_abort(int trx_id){
    printf("aborted!\n");

}

int trx_begin(){
    pthread_mutex_lock(&trx_table_latch);
    printf("%dth thread start\n",unique_trx_id);
    trx_header_t* trx_header =nullptr;
    try{        
        trx_header = new trx_header_t();
        if(unique_trx_id == 1){
            printf("첫 thread start\n");
        }
        
        if(trx_header == nullptr){
            throw(1);
        }
        trx_header->trx_id = unique_trx_id++;
        trx_header->lock = nullptr;
        trx_table[trx_header->trx_id] = trx_header;
        pthread_mutex_unlock(&trx_table_latch);
        return trx_header->trx_id;

    }
    catch(int exp){
        pthread_mutex_unlock(&trx_table_latch);
        return 0;
    }
}

int trx_commit(int trx_id){
    pthread_mutex_lock(&trx_table_latch);
    printf("%d trx_commit\n",trx_id);
    if(trx_table.find(trx_id) != trx_table.end()){
        trx_header_t* trx_header = trx_table[trx_id];
        lock_t* trx_lock = trx_header->lock;
        int i =0;
        while(trx_lock != nullptr){
            lock_t* tmp_lock = trx_lock->trx_next_lock_ptr;
            lock_release(trx_lock);
            printf("lock_release %d\n",i);
    
            trx_lock = tmp_lock;
            i++;
        }
        free(trx_header);
        trx_table.erase(trx_id);
        pthread_mutex_unlock(&trx_table_latch);

        printf("%d trx_commit 끝 %d개 released\n",trx_id,i);
        return trx_id;
    }
    printf("can't find trx_id(%d)\n",trx_id);
    pthread_mutex_unlock(&trx_table_latch);
    return 0;
}