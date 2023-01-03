

#ifndef __PAGE_H__
#define __PAGE_H__

#include <stdint.h>
#include <cstring>
#include <string>
#include "file.h"


#define INITIAL_DB_FILE_SIZE (10 * 1024 * 1024)  // 10 MiB
#define PAGE_SIZE (4 * 1024)                     // 4 KiB
#define FREE_PAGE_NUMBER ((INITIAL_DB_FILE_SIZE / PAGE_SIZE) -1)

#define SLOT_SIZE 12 // key(8) + size(2) + offset(2)
#define BRANCHING_FACTOR_SIZE 16 // key(8) + pagenumber(8)


typedef uint64_t pagenum_t; // 8바이트 uint64
typedef int64_t longlong_t; //8바이트 int64
typedef int32_t int_4bit; // 4바이트 int
typedef uint16_t ushortint_t; //2바이트 short int

#define SIZE_TYPE sizeof(pagenum_t)

//header page setting
#define header_magic_number 0
#define header_next_free_page_number sizeof(pagenum_t)*1
#define header_number_of_free_page sizeof(pagenum_t)*2
#define header_root_page_number sizeof(pagenum_t)*3

//free page settingx
#define free_page_next_free_page_number 0

//page header setting
#define page_header_parent_page_number 0
#define page_header_leaf_check 8 //sizeof(pagenum_t)*1
#define page_header_number_of_keys 12 //sizeof(pagenum_t)*1 + sizeof(int_4bit)*1

//leaf page setting
#define leaf_page_amount_of_free_space 112 // used in slot
#define leaf_page_right_sibling_page_number 120 // used in slot
#define leaf_page_first_slot_start 128 // used in slot

#define leaf_page_inital_amount_of_free_space 3968
#define leaf_page_inital_offset 4096

#define leaf_min_value_size 50
#define leaf_max_value_size 112


//internal page setting
#define internal_page_leafmost_key 120
#define internal_page_branching_factor_start 128


// void put_data_size(void*dest, const void* source, size_t size);

// void get_data_size(void *num, const void* source, size_t size);

// void put_data_4(void*dest, int_4bit source); //4바이트 집어넣음

// void get_data_4(int_4bit *num, const void* source); //4바이트 가져옴

// void put_data_2(void*dest, ushortint_t source); //2바이트 집어넣음

// void get_data_2(ushortint_t *num, const void* source); //2바이트 가져옴

// void put_data_slot(void*dest, const void* source);

// void get_data_slot(void *num, const void* source);

// void put_data_branch(void*dest, const void* source);

// void get_data_branch(void* *num, const void* source);

/*
typedef struct page_t {
    char data[PAGE_SIZE];
    // in-memory page structure
} page_t;*/

typedef struct slot_t {
    char data[SLOT_SIZE] ={0,}; // key(8)+size(2)+offset(2)
    pagenum_t get_key(){
        pagenum_t key;
        std::memcpy(&key,data+0,8);
        return key;
    }
    ushortint_t get_size(){
        ushortint_t size;
        std::memcpy(&size,data+8,2);
        return size;
    }
    ushortint_t get_offset(){
        ushortint_t offset;
        std::memcpy(&offset,data+10,2);
        return offset;

    }
    void set_key(pagenum_t key){
        std::memcpy(data+0,&key,8);
        //printf("%d\n",(pagenum_t)this->data);
        //put_data(this->data+0,key);
    }
    void set_size(ushortint_t size){
        std::memcpy(data+8,&size,2);
    }
    void set_offset(ushortint_t offset){
        std::memcpy(data+10,&offset,2);
    }
} slot_t;

typedef struct branching_factor_t{
    char data[BRANCHING_FACTOR_SIZE]; // key(8) + pagenumber(8)
    pagenum_t get_keys(){
        pagenum_t key;
        get_data(&key,&data[0]);
        return key;
    }
    pagenum_t get_pagenumber(){
        pagenum_t pagenumber;
        get_data(&pagenumber,&data[8]);
        return pagenumber;
    }
    void set_keys(pagenum_t key){
        put_data(&data[0],key);
    }
    void set_pagenumber(pagenum_t pagenumber){
        put_data(&data[8],pagenumber);
    }
}branching_factor_t;

namespace header_page{
    pagenum_t get_magic_number(page_t* page_t);
    pagenum_t get_next_free_page_number(page_t* page_t);
    pagenum_t get_number_of_free_page(page_t* page_t);
    pagenum_t get_root_page_num(page_t* page_t);
    void set_magic_number(page_t* page_t,pagenum_t magic_number);
    void set_next_free_page_number(page_t* page_t, pagenum_t next_free);
    void set_number_of_free_page(page_t* page_t, pagenum_t num_of_free);
    void set_root_page_num(page_t* page_t, pagenum_t root_page_num);
}

namespace free_page{
    pagenum_t get_next_free_page_number(page_t* page_t);    
    void set_next_free_page_number(page_t* page_t, pagenum_t next_free_page_num);
}
namespace page_header{
    page_t* make_page();//미구현
    pagenum_t get_parent_page_num(page_t* page_t);
    int_4bit get_leaf_check(page_t* page_t);
    int_4bit get_number_of_keys(page_t* page_t);
    void set_parent_page_num(page_t* page_t, pagenum_t parent_page_num);
    void set_leaf_check(page_t* page_t, int_4bit is_leaf);
    void set_number_of_keys(page_t* page_t, int_4bit number_of_keys);
}
namespace leaf_page{
    page_t* make_new_leaf();//미구현
    pagenum_t get_amount_of_free_space(page_t* page_t);
    pagenum_t get_right_sibling_page_number(page_t* page_t);
    slot_t* get_nth_slot(page_t* page_t, int n); //사용하면 free
    char* get_value(page_t* page_t, ushortint_t offset, size_t size); //사용하면 free
    void set_amount_of_free_space(page_t* page_t, pagenum_t amount_of_free);
    void set_right_sibling_page_number(page_t* page_t, pagenum_t right_sibling_page_number);
    void set_nth_slot(page_t* page_t, int n, slot_t* slot); //0번쨰부터 최대 63번째까지(64개) 최소 31개
    void set_value(page_t* page_t, ushortint_t offset, const char* value, size_t size); //slot의 value를 삽입, 50이상 112이하
}

namespace internal_page{
    page_t* make_new_internal();//미구현
    pagenum_t get_leftmost_key(page_t* page_t);
    branching_factor_t* get_nth_branching_factor(page_t* page_t, int n); //사용하면 free
    void set_leftmost_key(page_t* page_t, pagenum_t leftmost_key);
    void set_nth_branching_factor(page_t* page_t, int n, branching_factor_t* nth_branching_factor); //1번쨰부터 248번째까지(248개)
}


#endif