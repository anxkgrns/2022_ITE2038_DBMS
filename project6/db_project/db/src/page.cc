

#include "page.h"

// void put_data_size(void*dest, const void* source, size_t size){
//     std::memcpy(dest,source,size);
    
//         printf("suceess3\n");
// }

// void get_data_size(void* num, const void* source, size_t size){
//     std::memcpy(num,source,size);
//         printf("suceess1\n");
// }


// void put_data(void*dest, pagenum_t source){
//     std::memcpy(dest,&source,SIZE_TYPE);
// }

// void get_data(pagenum_t *num, const void* source){
//     std::memcpy(num,source,SIZE_TYPE);
// }

// void put_data_4(void*dest, int_4bit source){
//     std::memcpy(dest,&source,4);//sizeof(int_4bit));
// }

// void get_data_4(int_4bit *num, const void* source){
//     std::memcpy(num,source,4);//sizeof(int_4bit));
// }

// void put_data_4(void*dest, int32_t source){
//     std::memcpy(dest,&source,4);//sizeof(int_4bit));
// }

// void get_data_4(void* num, const void* source){
//     std::memcpy(num,source,4);//sizeof(int_4bit));
// }

// void put_data_2(void*dest, ushortint_t source){
//     std::memcpy(dest,&source,sizeof(ushortint_t));
    
//         printf("suceess4\n");
// }

// void get_data_2( ushortint_t* num, const void* source){
//     std::memcpy(num,source,sizeof(ushortint_t));
// }

// void put_data_slot(void*dest, const void* source){
//     std::memcpy(dest,&source,SLOT_SIZE);
// }

// void get_data_slot( void* num, const void* source){
//     std::memcpy(num,source,SLOT_SIZE);
// }

// void put_data_branch(void*dest, const void* source){
//     std::memcpy(dest,&source,BRANCHING_FACTOR_SIZE);
// }

// void get_data_branch( void *num, const void* source){
//     std::memcpy(num,source,BRANCHING_FACTOR_SIZE);
// }

pagenum_t header_page::get_magic_number(page_t* page_t){
    pagenum_t magic_number;
    get_data(&magic_number,&page_t->data[header_magic_number]);
    return magic_number;
}
pagenum_t header_page::get_next_free_page_number(page_t* page_t){
    pagenum_t next_free_page_number;
    get_data(&next_free_page_number,&page_t->data[header_next_free_page_number]);
    return next_free_page_number;
}
pagenum_t header_page::get_number_of_free_page(page_t* page_t){
    pagenum_t number_of_free_page;
    get_data(&number_of_free_page,&page_t->data[header_number_of_free_page]);
    return number_of_free_page;
}
pagenum_t header_page::get_root_page_num(page_t* page_t){
    pagenum_t root_page_num;
    get_data(&root_page_num,&page_t->data[header_root_page_number]);
    return root_page_num;
}
void header_page::set_magic_number(page_t* page_t,pagenum_t magic_number){
    put_data(&page_t->data[header_magic_number],magic_number);
}
void header_page::set_next_free_page_number(page_t* page_t, pagenum_t next_free){
    put_data(&page_t->data[header_next_free_page_number],next_free);
}
void header_page::set_number_of_free_page(page_t* page_t, pagenum_t num_of_free){
    put_data(&page_t->data[header_number_of_free_page],num_of_free);
}
void header_page::set_root_page_num(page_t* page_t, pagenum_t root_page_num){
    put_data(&page_t->data[header_root_page_number],root_page_num);
}

pagenum_t free_page::get_next_free_page_number(page_t* page_t){
    pagenum_t next_free_page_number;
    get_data(&next_free_page_number,&page_t->data[free_page_next_free_page_number]);
    return next_free_page_number;
}

void free_page::set_next_free_page_number(page_t* page_t, pagenum_t next_free_page_num){
    put_data(&page_t->data[free_page_next_free_page_number],next_free_page_num);
}

pagenum_t page_header::get_parent_page_num(page_t* page_t){
    pagenum_t parent_page_num;
    get_data(&parent_page_num,&page_t->data[page_header_parent_page_number]);
    return parent_page_num;
}
// leaf = 1 internal = 0
int_4bit page_header::get_leaf_check(page_t* page_t){
    int_4bit leaf_check;
    std::memcpy(&leaf_check, &page_t->data[page_header_leaf_check],4);
    return leaf_check;
}
int_4bit page_header::get_number_of_keys(page_t* page_t){
    int_4bit num_of_keys;
    std::memcpy(&num_of_keys,&page_t->data[page_header_number_of_keys],4);
    return num_of_keys;
}

// set as 0 if it is root page
void page_header::set_parent_page_num(page_t* page_t, pagenum_t parent_page_num){
    put_data(&page_t->data[page_header_parent_page_number],parent_page_num);
}
void page_header::set_leaf_check(page_t* page_t, int_4bit leaf_check){
    std::memcpy(page_t->data+page_header_leaf_check,&leaf_check,4);
}
void page_header::set_number_of_keys(page_t* page_t, int_4bit number_of_keys){
    put_data(&page_t->data[page_header_number_of_keys],number_of_keys);
}

pagenum_t leaf_page::get_amount_of_free_space(page_t* page_t){
    pagenum_t amount_of_free;
    get_data(&amount_of_free,&page_t->data[leaf_page_amount_of_free_space]);
    return amount_of_free;
}
pagenum_t leaf_page::get_right_sibling_page_number(page_t* page_t){
    pagenum_t right_sibling_page_num;
    get_data(&right_sibling_page_num,&page_t->data[leaf_page_right_sibling_page_number]);
    return right_sibling_page_num;
}
slot_t* leaf_page::get_nth_slot(page_t* page_t, int n){
    slot_t* nth_slot =(slot_t*)malloc(sizeof(slot_t));
    std::memcpy(nth_slot,page_t->data+leaf_page_first_slot_start+n*SLOT_SIZE,SLOT_SIZE);//0번째부터
    
    //printf("getted %dth slot\n",n);
    return nth_slot;
}
char* leaf_page::get_value(page_t* page_t, ushortint_t offset ,size_t size){
    char* value = (char*)malloc(sizeof(char)*200);
    std::memcpy(value,page_t->data+offset,size);
    //get_data_size(&value,&page_t->data[offset],size);
    return value;
}

void leaf_page::set_amount_of_free_space(page_t* page_t, pagenum_t amount_of_free){
    put_data(&page_t->data[leaf_page_amount_of_free_space],amount_of_free);
}
void leaf_page::set_right_sibling_page_number(page_t* page_t, pagenum_t right_sibling_page_number){
    put_data(&page_t->data[leaf_page_right_sibling_page_number],right_sibling_page_number);
}
void leaf_page::set_nth_slot(page_t* page_t, int n, slot_t* nth_slot){
    std::memcpy(page_t->data+leaf_page_first_slot_start+n*SLOT_SIZE,nth_slot,SLOT_SIZE); //0번쨰부터
}
void leaf_page::set_value(page_t* page_t, ushortint_t offset,const char* value,size_t size){
    std::memcpy(page_t->data + offset,value,size);
}

pagenum_t internal_page::get_leftmost_key(page_t* page_t){
    pagenum_t leftmost_key;
    get_data(&leftmost_key,&page_t->data[internal_page_leafmost_key]);
    return leftmost_key;
}
branching_factor_t* internal_page::get_nth_branching_factor(page_t* page_t, int n){
    branching_factor_t* nth_branching_factor = (branching_factor_t*)malloc(sizeof(branching_factor_t));
    std::memcpy(nth_branching_factor,page_t->data+internal_page_branching_factor_start+(n-1)*BRANCHING_FACTOR_SIZE,BRANCHING_FACTOR_SIZE);
    return nth_branching_factor;
}
void internal_page::set_leftmost_key(page_t* page_t, pagenum_t leftmost_key){
    put_data(&page_t->data[internal_page_leafmost_key],leftmost_key);
}

//1번쨰부터 248번째까지(248개)
void internal_page::set_nth_branching_factor(page_t* page_t, int n, branching_factor_t* nth_branching_factor){
    //printf("secces\n");
    std::memcpy(page_t->data+internal_page_branching_factor_start+(n-1)*BRANCHING_FACTOR_SIZE,nth_branching_factor,BRANCHING_FACTOR_SIZE);
}
