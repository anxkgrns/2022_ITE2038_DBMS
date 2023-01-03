

#ifndef DB_FILE_H_
#define DB_FILE_H_

#include <stdint.h>
#include <fcntl.h>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <string.h>




// These definitions are not requirements.
// You may build your own way to handle the constants.
#define INITIAL_DB_FILE_SIZE (10 * 1024 * 1024)  // 10 MiB
#define PAGE_SIZE (4 * 1024)                     // 4 KiB
#define FREE_PAGE_NUMBER ((INITIAL_DB_FILE_SIZE / PAGE_SIZE) -1)

typedef uint64_t pagenum_t;
#define SIZE_TYPE sizeof(pagenum_t)


#define header_magic_number 0
#define header_next_free_page_number sizeof(pagenum_t)*1
#define header_number_of_free_page sizeof(pagenum_t)*2
#define header_root_page_number sizeof(pagenum_t)*3
#define free_page_next_free_page_number 0

typedef struct page_t {
  char data[PAGE_SIZE];
  // in-memory page structure
} page_t;

void put_data(void*dest, pagenum_t source);

void get_data(pagenum_t *num, const void* source);


// Open existing table file or create one if it doesn't exist
int64_t file_open_table_file(const char* pathname);

// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page(int64_t table_id);

// Free an on-disk page to the free page list
void file_free_page(int64_t table_id, pagenum_t pagenum);

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(int64_t table_id, pagenum_t pagenum, struct page_t* dest);

// Write an in-memory page(src) to the on-disk page
void file_write_page(int64_t table_id, pagenum_t pagenum, const struct page_t* src);

// Close the table file
void file_close_table_file();

#endif  // DB_FILE_H_

