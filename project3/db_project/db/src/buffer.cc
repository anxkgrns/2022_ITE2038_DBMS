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


// Open existing table file or create one if it doesn't exist
int64_t buffer_open_table_file(const char* pathname){
    int64_t table_id = file_open_table_file(pathname);
    printf("table_id: %d\n",table_id);
    return table_id;
}

void unpin(int64_t table_id, pagenum_t pagenum){
    //print_buffer();
    buffer_block* ith_buf = disk[std::pair(table_id,pagenum)];
    ith_buf->Is_pinned--;
    printf("unpin!\n");
    //print_buffer();
}

//read!!!!!!

// buffer-> index
void buffer_read_page(int64_t table_id, pagenum_t pagenum, page_t* dest){
    printf("buffer_read_page\n");
    setting_buffer(table_id,pagenum,dest);
    buffer_block* buf = head->LRU_NEXT;
    std::memcpy(dest, buf->physical_frame,PAGE_SIZE);
    buf->Is_pinned++;
}

// disk -> buffer
buffer_block* buffer_read_disk(int64_t table_id, pagenum_t pagenum, page_t* dest){
    buffer_block* ith_buf;
    if(used_size==0){
       head->LRU_NEXT = buf_pool[0];
       buf_pool[0]->LRU_PREV =head;
       tail->LRU_PREV = buf_pool[0];
       buf_pool[0]->LRU_NEXT = tail;
       if(tail->LRU_PREV->LRU_PREV == head){
       }
    }
    ith_buf = buf_pool[used_size];
    ith_buf->pagenum =pagenum;
    ith_buf->table_id = table_id;
    file_read_page(table_id,pagenum,ith_buf->physical_frame);
    
    return ith_buf;  
}

// Read an on-disk page into the in-memory page structure(dest)
// if: disk -> buffer && else: buffer -> disk
void setting_buffer(int64_t table_id, pagenum_t pagenum, page_t* dest){
    buffer_block* ith_buf;
    if(disk.find(std::pair(table_id,pagenum)) != disk.end()){
        ith_buf = disk[std::pair(table_id,pagenum)];
        printf("여기는 안 들어가나요?\n");
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
        }
        used_size++;
        disk.insert(std::pair(std::pair(table_id,pagenum),ith_buf));
    }
    //printf("used_size:%d\n",used_size);
    if(used_size != 1 ){
    buffer_block* next = ith_buf->LRU_NEXT;
    buffer_block* prev = ith_buf->LRU_PREV;

    next->LRU_PREV = prev;
    prev->LRU_NEXT = next;
    
    ith_buf->LRU_PREV = head;
    ith_buf->LRU_NEXT = ith_buf->LRU_PREV->LRU_NEXT;
    head->LRU_NEXT = ith_buf;
    }
    
    
}

// Write an in-memory page(src) to the on-disk page
// index -> buffer
//void buffer_write_page(int64_t table_id, pagenum_t pagenum, const struct page_t* src)
void buffer_write_page(int64_t table_id, pagenum_t pagenum,  page_t* src){
    printf("buffer_write_page\n");
    setting_buffer(table_id,pagenum,src);
    buffer_block* buf = head->LRU_NEXT;
    std::memcpy(buf->physical_frame ,src,PAGE_SIZE);
    buf->Is_pinned--;
    if(buf->Is_pinned < 0){
        buf->Is_pinned = 0;
    }
    buf->Is_dirty =true;
    printf("Is dirty?:%d\n",buf->Is_dirty);
}

//buffer -> disk
buffer_block* buffer_write_disk(int64_t table_id, pagenum_t pagenum, page_t* dest){
    buffer_block* ith_buf;
        int i=0;
    for(ith_buf = tail->LRU_PREV; ith_buf != head && ith_buf->Is_pinned == 0 ; ith_buf = ith_buf->LRU_PREV){}
    //printf("compare%d,%d\n",ith_buf,head);
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
    }
    return ith_buf;
}


// Allocate an on-disk page from the free page list
pagenum_t buffer_alloc_page(int64_t table_id){

    buffer_block* ith_buf;
    pagenum_t pagenum;

    if(disk.find(std::pair(table_id,0)) != disk.end()){
        printf("alloc_page\n");
        ith_buf = disk[std::pair(table_id,0)];
        file_write_page(table_id,0,ith_buf->physical_frame); 
        pagenum = file_alloc_page(table_id);
        printf("pagenum_alloc!: %d\n",pagenum);
        file_read_page(table_id,0,ith_buf->physical_frame);
        ith_buf->Is_dirty= true;
    }
    else{
        pagenum = file_alloc_page(table_id);
        printf("pagenum: %d\n",pagenum);
    }
    return pagenum;

}

// Free an on-disk page to the free page list
void buffer_free_page(int64_t table_id, pagenum_t pagenum){
    buffer_block* ith_buf;

    if(disk.find(std::pair(table_id,pagenum)) != disk.end()){
        ith_buf = disk[std::pair(table_id,pagenum)];
        file_write_page(table_id,pagenum,ith_buf->physical_frame); 
        file_free_page(table_id,pagenum);
        file_read_page(table_id,pagenum,ith_buf->physical_frame);
        ith_buf->Is_dirty= true;
    }
    else{
        file_free_page(table_id,pagenum);
    }

}

// Close the table file
void buffer_close_table_file(){
    file_close_table_file();
}


int buffer_init_db(int num_buf){
    
    if(space_exist == true){
        return -1;
    }
    buf_pool = (buffer_block**)malloc(sizeof(buffer_block*)*num_buf);
    space_exist = true;
    max_size = num_buf;
    used_size =0;
    head = (buffer_block*)malloc(sizeof(buffer_block));
    tail = (buffer_block*)malloc(sizeof(buffer_block));
    head->LRU_NEXT = tail;
    tail->LRU_PREV = head;
    
    for(int i=0;i<num_buf;i++){
        buf_pool[i] = (buffer_block*)malloc(sizeof(buffer_block));
    }
    
    for(int i=0;i<num_buf;i++){
        buf_pool[i]->Is_dirty =false;
        buf_pool[i]->Is_pinned =0;
        buf_pool[i]->LRU_NEXT = (buffer_block*)malloc(sizeof(buffer_block));
        buf_pool[i]->LRU_PREV = (buffer_block*)malloc(sizeof(buffer_block));
        //buf_pool[i]->LRU_NEXT = buf_pool[(i+1)%num_buf];
        //buf_pool[i]->LRU_PREV = buf_pool[(i-1+num_buf)%num_buf];
        buf_pool[i]->pagenum = 0;
        buf_pool[i]->physical_frame = (page_t*)malloc(sizeof(page_t));
        buf_pool[i]->table_id = -1;
    }
    printf("%d\n",buf_pool[0]->Is_pinned);
    printf("%d\n",buf_pool[0]->pagenum);
    printf("%d\n",buf_pool[0]->table_id);
    
    return 0;
}

void buffer_shut_down(){
    if(space_exist == true){
        for(int i=0;i<max_size;i++){
            if(buf_pool[i]->Is_dirty==true){
                file_write_page(buf_pool[i]->table_id,buf_pool[i]->pagenum,buf_pool[i]->physical_frame);
            }
            free(buf_pool[i]->physical_frame);
            free(buf_pool[i]->LRU_NEXT);
            free(buf_pool[i]->LRU_PREV);
            free(buf_pool[i]);
        }
        free(head);
        free(tail);
        free(buf_pool);
    }
    buffer_close_table_file();
}

void print_buffer(){
    for(int i=0;i<used_size;i++){
        printf("%dth buffer->pinned:%d\n",i+1,buf_pool[i]->Is_pinned);
    }
}