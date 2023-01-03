#include "buffer.h"
#include "page.h"
#include<iostream>

buffer_block* head;
buffer_block* tail;
uint64_t max_size, used_size;
pagenum_t pagenum;
buffer_block** buf_pool;
std::map<std::pair<int64_t, pagenum_t>,buffer_block*> disk;
bool space_exist;



pthread_mutex_t buffer_manager_latch;

// Open existing table file or create one if it doesn't exist
int64_t buffer_open_table_file(const char* pathname){
    int64_t table_id = file_open_table_file(pathname);
    printf("table_id: %d\n",table_id);
    return table_id;
}

void lock_buffer_latch(){
    pthread_mutex_lock(&buffer_manager_latch);
}

void unlock_buffer_latch(){
    pthread_mutex_unlock(&buffer_manager_latch);
}

//release pin
void unpin_buf(buffer_block* buf){
    printf("unpin! %d\n",buf->pagenum);
    pthread_mutex_unlock(&buf->page_latch);
}

//make pin
void pin_buf(buffer_block* buf){
    printf("pin! %d\n",buf->pagenum);
    pthread_mutex_lock(&buf->page_latch);
}

void unpin(int64_t table_id, pagenum_t pagenum){
    //pthread_mutex_lock(&buffer_manager_latch);
    //print_buffer();
    buffer_block* ith_buf = disk[std::pair(table_id,pagenum)];
    //ith_buf->Is_pinned--;
    printf("unpin!! %d\n",pagenum);
    pthread_mutex_unlock(&ith_buf->page_latch);

    // if(ith_buf->Is_pinned < 0){
    //     ith_buf->Is_pinned = 0;
    // }

    //pthread_mutex_unlock(&buffer_manager_latch);
    
    //print_buffer();
}

void pin(int64_t table_id, pagenum_t pagenum){
    //pthread_mutex_lock(&buffer_manager_latch);
    //print_buffer();
    buffer_block* ith_buf = disk[std::pair(table_id,pagenum)];
    //ith_buf->Is_pinned--;
    printf("pin! %d\n",pagenum);
    pthread_mutex_lock(&ith_buf->page_latch);

    //pthread_mutex_unlock(&buffer_manager_latch);

    //print_buffer();
}

// Read an on-disk page into the in-memory page structure(dest)
buffer_block* buffer_read_page_trx(int64_t table_id, pagenum_t pagenum, page_t* dest){
    printf("buffer_read_page_trx\n");
    pthread_mutex_lock(&buffer_manager_latch);
    //printf("buffer_read_page\n");
    //pin(table_id,pagenum);
    setting_buffer(table_id,pagenum,dest);
    buffer_block* buf = head->LRU_NEXT;
    pthread_mutex_lock(&buf->page_latch); //
    //pin(table_id,pagenum);
    printf("lock! %d\n",pagenum);
    std::memcpy(dest, buf->physical_frame,PAGE_SIZE);
    //buf->table_id = table_id;
    //buf->pagenum = pagenum;
    //pthread_mutex_lock(&buf->page_latch); //
    //buf->Is_pinned++;
    pthread_mutex_unlock(&buffer_manager_latch);
    return buf;
}


// Write an in-memory page(src) to the on-disk page
void buffer_write_page_trx(buffer_block* buf, int64_t table_id, pagenum_t pagenum, page_t* src){
    printf("buffer_write_page_trx\n");
    //setting_buffer(table_id,pagenum,src);
    //buffer_block* buf =  head->LRU_NEXT;
    if(buf ==  disk[std::pair(table_id,pagenum)]){
        printf("같다!\n");
    }
    std::memcpy(buf->physical_frame ,src,PAGE_SIZE);
    //buf->table_id = table_id;
    //buf->pagenum = pagenum;
    buf->Is_dirty =true;
    pthread_mutex_unlock(&buf->page_latch);
    printf("unlock! %d\n",pagenum);
    //unpin_buf(buf);
}

// buffer-> index
buffer_block* buffer_read_page(int64_t table_id, pagenum_t pagenum, page_t* dest){
    pthread_mutex_lock(&buffer_manager_latch);
    //printf("buffer_read_page\n");
    //pin(table_id,pagenum);
    setting_buffer(table_id,pagenum,dest);
    buffer_block* buf = head->LRU_NEXT;
    pthread_mutex_lock(&buf->page_latch);
    printf("lock! %d\n",pagenum);
    std::memcpy(dest, buf->physical_frame,PAGE_SIZE);
    //buf->table_id = table_id;
    //buf->pagenum = pagenum;
    //buf->Is_pinned++;
    pthread_mutex_unlock(&buf->page_latch);
    printf("unlock! %d\n",pagenum);
    pthread_mutex_unlock(&buffer_manager_latch);
    //unpin(table_id,pagenum);
    return buf;

    //pthread_mutex_unlock(&buffer_manager_latch);
    
}

// disk -> buffer
buffer_block* buffer_read_disk(int64_t table_id, pagenum_t pagenum, page_t* dest){
    buffer_block* ith_buf;
    //printf("used_size!: %d\n",used_size);
    
    ith_buf = buf_pool[used_size];
    ith_buf->pagenum = pagenum;
    ith_buf->table_id = table_id;
    file_read_page(table_id,pagenum,ith_buf->physical_frame);
    //std::memcpy(dest, ith_buf->physical_frame,PAGE_SIZE);
    
    return ith_buf;  
}

buffer_block* setting_buffer1(int64_t table_id, pagenum_t pagenum, page_t* dest){
    printf("use %d pagenum\n",pagenum);
    buffer_block* ith_buf;
    //printf("setting_buffer\n");
    if(disk.find(std::pair(table_id,pagenum)) != disk.end()){
        ith_buf = disk[std::pair(table_id,pagenum)];
        
        buffer_block* next = ith_buf->LRU_NEXT;
        buffer_block* prev = ith_buf->LRU_PREV;

        next->LRU_PREV = prev;
        prev->LRU_NEXT = next;  
        //printf("여기는 안 들어가나요?\n");
    }
    else{
        printf("used_size:%d max_size:%d\n",used_size,max_size);
        //print_buffer();
        if(max_size > used_size ){
            printf("새로운 buffer\n");
            ith_buf = buffer_read_disk(table_id,pagenum,dest);
        }
        else{
            printf("기존 buffer 지우기\n");
            ith_buf = buffer_write_disk(table_id,pagenum,dest);  
               
            buffer_block* next = ith_buf->LRU_NEXT;
            buffer_block* prev = ith_buf->LRU_PREV;

            next->LRU_PREV = prev;
            prev->LRU_NEXT = next;       
        }
        used_size++;
        disk.insert(std::pair(std::pair(table_id,pagenum),ith_buf));
        //print_buffer();
    }
    if(used_size==1){
        buffer_block* head_next = head->LRU_NEXT;
        head->LRU_NEXT = ith_buf;
        ith_buf->LRU_PREV =head;
        tail->LRU_PREV = ith_buf;
        ith_buf->LRU_NEXT = tail;
        if(tail->LRU_PREV == head){
            printf("head에 tail이 연결됨\n");
            exit(0);
        }
    }
    //printf("used_size:%d\n",used_size);
    if(used_size != 1 ){
        
        //printf("compare: %d,%d,%d\n",head,tail->LRU_PREV,tail);
        //buffer_block* head_next = head->LRU_NEXT;
        head->LRU_NEXT->LRU_PREV = ith_buf; // 중요!
        ith_buf->LRU_NEXT = head->LRU_NEXT;
        head->LRU_NEXT = ith_buf;
        ith_buf->LRU_PREV = head;
        if(tail->LRU_PREV == head){
            printf("head에 tail이 연결됨\n");
            exit(0);
       }

    }
    return ith_buf;
    //print_buffer();
    
    
}

// Read an on-disk page into the in-memory page structure(dest)
// if: disk -> buffer && else: buffer -> disk
void setting_buffer(int64_t table_id, pagenum_t pagenum, page_t* dest){
    printf("use %d pagenum\n",pagenum);
    buffer_block* ith_buf;
    //printf("setting_buffer\n");
    if(disk.find(std::pair(table_id,pagenum)) != disk.end()){
        ith_buf = disk[std::pair(table_id,pagenum)];
        
        buffer_block* next = ith_buf->LRU_NEXT;
        buffer_block* prev = ith_buf->LRU_PREV;

        next->LRU_PREV = prev;
        prev->LRU_NEXT = next;  
        //printf("여기는 안 들어가나요?\n");
    }
    else{
        printf("used_size:%d max_size:%d\n",used_size,max_size);
        //print_buffer();
        if(max_size > used_size ){
            printf("새로운 buffer\n");
            ith_buf = buffer_read_disk(table_id,pagenum,dest);
        }
        else{
            printf("기존 buffer 지우기\n");
            ith_buf = buffer_write_disk(table_id,pagenum,dest);  
               
            buffer_block* next = ith_buf->LRU_NEXT;
            buffer_block* prev = ith_buf->LRU_PREV;

            next->LRU_PREV = prev;
            prev->LRU_NEXT = next;       
        }
        used_size++;
        disk.insert(std::pair(std::pair(table_id,pagenum),ith_buf));
        //print_buffer();
    }
    if(used_size==1){
        buffer_block* head_next = head->LRU_NEXT;
        head->LRU_NEXT = ith_buf;
        ith_buf->LRU_PREV =head;
        tail->LRU_PREV = ith_buf;
        ith_buf->LRU_NEXT = tail;
        if(tail->LRU_PREV == head){
            printf("head에 tail이 연결됨\n");
            exit(0);
        }
    }
    //printf("used_size:%d\n",used_size);
    if(used_size != 1 ){
        
        //printf("compare: %d,%d,%d\n",head,tail->LRU_PREV,tail);
        //buffer_block* head_next = head->LRU_NEXT;
        head->LRU_NEXT->LRU_PREV = ith_buf; // 중요!
        ith_buf->LRU_NEXT = head->LRU_NEXT;
        head->LRU_NEXT = ith_buf;
        ith_buf->LRU_PREV = head;
        if(tail->LRU_PREV == head){
            printf("head에 tail이 연결됨\n");
            exit(0);
       }

    }
    //print_buffer();
    
    
}



// Write an in-memory page(src) to the on-disk page
// index -> buffer
//void buffer_write_page(int64_t table_id, pagenum_t pagenum, const struct page_t* src)
void buffer_write_page(int64_t table_id, pagenum_t pagenum,  page_t* src){
    pthread_mutex_lock(&buffer_manager_latch);
    //pin(table_id,pagenum);
    //printf("buffer_write_page\n");
    setting_buffer(table_id,pagenum,src);
    buffer_block* buf = head->LRU_NEXT;
    pthread_mutex_lock(&buf->page_latch);
    printf("lock! %d\n",pagenum);
    
    std::memcpy(buf->physical_frame ,src,PAGE_SIZE);
    //buf->table_id = table_id;
    //buf->pagenum = pagenum;
    //buf->Is_pinned--;
    /*
    if(buf->Is_pinned < 0){
        buf->Is_pinned = 0;
    }*/
    buf->Is_dirty =true;
    pthread_mutex_unlock(&buf->page_latch);
    printf("unlock! %d\n",pagenum);
    pthread_mutex_unlock(&buffer_manager_latch);
    
    //disk.insert(std::pair(std::pair(table_id,pagenum),buf));
    //pthread_mutex_unlock(&buf->page_latch);  // -> segfalut의 원흉
    //printf("Is dirty?:%d\n",buf->Is_dirty);
}

//buffer -> disk
buffer_block* buffer_write_disk(int64_t table_id, pagenum_t pagenum, page_t* dest){
    buffer_block* ith_buf ;
    
    int i=0;
    
    for(ith_buf = tail->LRU_PREV; ith_buf != head; ith_buf = ith_buf->LRU_PREV){
        if(pthread_mutex_trylock(&ith_buf->page_latch) == EBUSY){ //unlock 상태 (lock X 인 상태)
            printf("lockeddd!!!\n");
            i++;
            continue;
        }
        else{
            pthread_mutex_unlock(&ith_buf->page_latch);
            break;
        }
    }
    printf("i: %d\n",i);
    
    if(ith_buf == head){
        printf("all allocated pin is using so i can't get page\n");
        exit(0);
    }
    disk.erase(std::pair(ith_buf->table_id ,ith_buf->pagenum));
    used_size--;
    
    if(ith_buf->Is_dirty == true) // 의문? fflush
    {   
        printf("덮어쓰기\n");
        file_write_page(ith_buf->table_id ,ith_buf->pagenum,ith_buf->physical_frame); 
        ith_buf->Is_dirty =false;
    }
    ith_buf->table_id =table_id;
    ith_buf->pagenum =pagenum;
    //ith_buf->page_latch = PTHREAD_MUTEX_INITIALIZER;
    //file_read_page(ith_buf->table_id,ith_buf->pagenum,ith_buf->physical_frame);
    file_read_page(table_id,pagenum,ith_buf->physical_frame);
    
    return ith_buf;
}


// Allocate an on-disk page from the free page list
pagenum_t buffer_alloc_page(int64_t table_id){
    
    pthread_mutex_lock(&buffer_manager_latch);

    buffer_block* ith_buf;
    pagenum_t pagenum;

    if(disk.find(std::pair(table_id,0)) != disk.end()){
        //printf("alloc_page\n");
        ith_buf = disk[std::pair(table_id,0)];
        //pthread_mutex_lock(&ith_buf->page_latch);
        file_write_page(table_id,0,ith_buf->physical_frame); 
        pagenum = file_alloc_page(table_id);
        printf("pagenum_alloc!: %d\n",pagenum);
        file_read_page(table_id,0,ith_buf->physical_frame);
        //ith_buf->Is_dirty= true;
        //pthread_mutex_unlock(&ith_buf->page_latch);
    }
    else{
        pagenum = file_alloc_page(table_id);
        printf("pagenum: %d\n",pagenum);
    }
    
    pthread_mutex_unlock(&buffer_manager_latch);
    return pagenum;

}

// Free an on-disk page to the free page list
void buffer_free_page(int64_t table_id, pagenum_t pagenum){
    pthread_mutex_lock(&buffer_manager_latch);
    buffer_block* ith_buf;

    if(disk.find(std::pair(table_id,pagenum)) != disk.end()){
        ith_buf = disk[std::pair(table_id,pagenum)];
        
        pthread_mutex_lock(&ith_buf->page_latch);
        file_write_page(table_id,pagenum,ith_buf->physical_frame); 
        file_free_page(table_id,pagenum);
        file_read_page(table_id,pagenum,ith_buf->physical_frame);
        ith_buf->Is_dirty= true;
        pthread_mutex_unlock(&ith_buf->page_latch);
    }
    else{
        file_free_page(table_id,pagenum);
    }
    pthread_mutex_unlock(&buffer_manager_latch);

}

// Close the table file
void buffer_close_table_file(){
    file_close_table_file();
}


int buffer_init_db(int num_buf){
    //printf("space_exist? : %d",space_exist);
    if(space_exist == true){
        printf("space_exist? : 참");
        return -1;
    }
    else{
        printf("space_exist? : 거짓");
    }
    buf_pool = (buffer_block**)malloc(sizeof(buffer_block*)*num_buf);
    buffer_manager_latch = PTHREAD_MUTEX_INITIALIZER;
    space_exist = true;
    max_size = num_buf;
    used_size =0;
    head = (buffer_block*)malloc(sizeof(buffer_block));
    tail = (buffer_block*)malloc(sizeof(buffer_block));
    head->LRU_NEXT = tail;//(buffer_block*)malloc(sizeof(buffer_block))
    tail->LRU_PREV = head;//(buffer_block*)malloc(sizeof(buffer_block))
    
    for(int i=0;i<num_buf;i++){
        buf_pool[i] = (buffer_block*)malloc(sizeof(buffer_block));
    }
    
    for(int i=0;i<num_buf;i++){
        buf_pool[i]->Is_dirty =false;
        //buf_pool[i]->Is_pinned =0;
        buf_pool[i]->page_latch = PTHREAD_MUTEX_INITIALIZER;
        buf_pool[i]->LRU_NEXT = nullptr; //buf_pool[i];//tail;//(buffer_block*)malloc(sizeof(buffer_block));
        buf_pool[i]->LRU_PREV = nullptr;//buf_pool[i];//head;//(buffer_block*)malloc(sizeof(buffer_block));
        //buf_pool[i]->LRU_NEXT = buf_pool[(i+1)%num_buf];
        //buf_pool[i]->LRU_PREV = buf_pool[(i-1+num_buf)%num_buf];
        buf_pool[i]->pagenum = 0;
        buf_pool[i]->physical_frame = (page_t*)malloc(sizeof(page_t));
        buf_pool[i]->table_id = -1;
    }
    //printf("%d\n",buf_pool[0]->Is_pinned);
    printf("%d\n",buf_pool[0]->pagenum);
    printf("%d\n",buf_pool[0]->table_id);
    
    return 0;
}

void buffer_shut_down(){
    if(space_exist == true){
        for(int i=0;i<max_size;i++){
            if(buf_pool[i]->Is_dirty == true){
                //pthread_mutex_lock(&buffer_manager_latch);
                //pthread_mutex_unlock(&buf_pool[i]->page_latch);
                printf("dirty이기 때문에 write\n");
                file_write_page(buf_pool[i]->table_id,buf_pool[i]->pagenum,buf_pool[i]->physical_frame);
                //pthread_mutex_unlock(&buffer_manager_latch);
            }
            disk.clear();
            free(buf_pool[i]->physical_frame);
            buf_pool[i]->physical_frame = nullptr;
            pthread_mutex_destroy(&buf_pool[i]->page_latch);
            //free(buf_pool[i]->LRU_NEXT);
            //buf_pool[i]->LRU_NEXT = NULL;
            //free(buf_pool[i]->LRU_PREV);
            //buf_pool[i]->LRU_PREV = NULL;
            free(buf_pool[i]);
            buf_pool[i] = nullptr;
        }
        pthread_mutex_destroy(&buffer_manager_latch);
        free(head);
        head = nullptr;
        free(tail);
        tail = nullptr;
        free(buf_pool);
        buf_pool = nullptr;
    }
    space_exist = false;
    //buffer_close_table_file();
}

void print_buffer(){

    printf("%d buffer has been used\n",used_size);
    
    for(int i=0;i<used_size;i++){
        printf("#%d pagenum buffer\n",buf_pool[i]->pagenum);
        if(pthread_mutex_trylock(&buf_pool[i]->page_latch) == 0 ){
            pthread_mutex_unlock(&buf_pool[i]->page_latch);
            if(buf_pool[i]->Is_dirty == true){
            printf("%dth buffer->unlocked / buffer->dirty: true\n",i+1);
            }
            else{
                printf("%dth buffer->unlocked / buffer->dirty: false\n",i+1);
            }
        }
        else{
            if(buf_pool[i]->Is_dirty == true){
            printf("%dth buffer->locked / buffer->dirty: true\n",i+1);
            }
            else{
                printf("%dth buffer->locked / buffer->dirty: false\n",i+1);
            }
        }
        
    }
    int flag = 0;
    for(int i=0;i<used_size;i++){
        printf("#%d pagenum buffer\n",buf_pool[i]->pagenum);
        for(int j=i+1;j<used_size;j++){
            if(buf_pool[i]->pagenum == buf_pool[j]->pagenum){
                printf("겹치는 buffer 발견\n");
                flag =1;
                break;
            }
        }
    }
    if(flag == 0){
        printf("모든 buffer 안 겹침!\n");
    }
    buffer_block* ith_buf;
    int i=0;
    //printf("compare_최종: %d,%d,%d\n",head,tail->LRU_PREV,tail);
    for(ith_buf = tail->LRU_PREV; ith_buf != head ; ith_buf = ith_buf->LRU_PREV){
        i++;
    }
    
    printf("유기적 연결-tail: %d used_size:%d\n",i,used_size);
    i=0;
    for(ith_buf = head->LRU_NEXT; ith_buf != tail ; ith_buf = ith_buf->LRU_NEXT){
        i++;
    }
    printf("유기적 연결-head: %d used_size:%d\n",i,used_size);
    
}