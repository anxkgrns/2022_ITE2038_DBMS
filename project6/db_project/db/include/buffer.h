
#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "page.h"
#include <map>
#include <vector>
#include <mutex>
#include <pthread.h>



struct buffer_block
{
    page_t* physical_frame;
    int64_t table_id;
    pagenum_t pagenum;
    bool Is_dirty;
    //int Is_pinned;
    pthread_mutex_t page_latch;

    buffer_block* LRU_NEXT;
    buffer_block* LRU_PREV;

    /* data */
};

void lock_buffer_latch();

void unlock_buffer_latch();


#endif
// Open existing table file or create one if it doesn't exist
int64_t buffer_open_table_file(const char* pathname);

// Allocate an on-disk page from the free page list
pagenum_t buffer_alloc_page(int64_t table_id);

// Free an on-disk page to the free page list
void buffer_free_page(int64_t table_id, pagenum_t pagenum);

// Read an on-disk page into the in-memory page structure(dest)
buffer_block* buffer_read_page(int64_t table_id, pagenum_t pagenum, page_t* dest);

// Write an in-memory page(src) to the on-disk page
void buffer_write_page(int64_t table_id, pagenum_t pagenum, page_t* src);

// Close the table file
void buffer_close_table_file();

void make_buffer_space();

void buffer_shut_down();

int buffer_init_db(int num_buf);

//release pin
void unpin(int64_t table_id, pagenum_t pagenum);

//make pin
void pin(int64_t table_id, pagenum_t pagenum);

buffer_block* buffer_read_disk(int64_t table_id, pagenum_t pagenum, page_t* dest);

void setting_buffer(int64_t table_id, pagenum_t pagenum, page_t* dest);

buffer_block* setting_buffer1(int64_t table_id, pagenum_t pagenum, page_t* dest);

buffer_block* buffer_write_disk(int64_t table_id, pagenum_t pagenum, page_t* dest);

//trx

//release pin
void unpin_buf(buffer_block* buf);

//make pin
void pin_buf(buffer_block* buf);

// Read an on-disk page into the in-memory page structure(dest)
buffer_block* buffer_read_page_trx(int64_t table_id, pagenum_t pagenum, page_t* dest);

// Write an in-memory page(src) to the on-disk page
void buffer_write_page_trx(buffer_block* buf, int64_t table_id, pagenum_t pagenum, page_t* src);


void print_buffer();