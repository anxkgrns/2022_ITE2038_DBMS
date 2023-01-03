
#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "page.h"
#include <map>
#include <vector>



struct buffer_block
{
    page_t* physical_frame;
    int64_t table_id;
    pagenum_t pagenum;
    bool Is_dirty;
    int Is_pinned;

    buffer_block* LRU_NEXT;
    buffer_block* LRU_PREV;

    /* data */
};



#endif
// Open existing table file or create one if it doesn't exist
int64_t buffer_open_table_file(const char* pathname);

// Allocate an on-disk page from the free page list
pagenum_t buffer_alloc_page(int64_t table_id);

// Free an on-disk page to the free page list
void buffer_free_page(int64_t table_id, pagenum_t pagenum);

// Read an on-disk page into the in-memory page structure(dest)
void buffer_read_page(int64_t table_id, pagenum_t pagenum, page_t* dest);

// Write an in-memory page(src) to the on-disk page
void buffer_write_page(int64_t table_id, pagenum_t pagenum, page_t* src);

// Close the table file
void buffer_close_table_file();

void make_buffer_space();

void buffer_shut_down();

int buffer_init_db(int num_buf);


void unpin(int64_t table_id, pagenum_t pagenum);

buffer_block* buffer_read_disk(int64_t table_id, pagenum_t pagenum, page_t* dest);

void setting_buffer(int64_t table_id, pagenum_t pagenum, page_t* dest);

buffer_block* buffer_write_disk(int64_t table_id, pagenum_t pagenum, page_t* dest);



void print_buffer();