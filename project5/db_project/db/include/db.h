#ifndef __DB_H__
#define __DB_H__

#include "buffer.h"
#include <cstdint>
#include <vector>
#include <queue>
#include <stdio.h>

#endif

#define INTERNAL_ORDER (PAGE_SIZE - internal_page_branching_factor_start) / BRANCHING_FACTOR_SIZE //248

#define THRESHOLD 2500//BIT

// Open an existing database file or create one if not exist.
int64_t open_table(const char* pathname);

// Insert a record to the given table.
int db_insert(int64_t table_id, int64_t key, const char* value,uint16_t val_size);

int cut(longlong_t table_id,pagenum_t leaf_pagenum);

page_t insert_into_parent(longlong_t table_id,longlong_t leaf_pagenum, pagenum_t new_leaf_pagenum,pagenum_t new_key);

page_t insert_into_page_after_splitting(longlong_t table_id,pagenum_t old_pagenum,pagenum_t left_index,pagenum_t new_key,pagenum_t right_pagenum);

page_t insert_into_page(longlong_t table_id, pagenum_t parent_pagenum, pagenum_t left_index, pagenum_t new_key,pagenum_t right_pagenum);

page_t insert_into_new_root(longlong_t table_id,pagenum_t left_pagenum,pagenum_t key,pagenum_t right_pagenum);

longlong_t get_left_index(longlong_t table_id,pagenum_t parent_pagenum,pagenum_t leaf_pagenum);

page_t insert_into_leaf_after_splitting(longlong_t table_id,pagenum_t leaf_pagenum,longlong_t key, const char* value,ushortint_t val_size);

page_t insert_into_leaf(longlong_t table_id,pagenum_t leaf_pagenum,longlong_t key, const char* value,ushortint_t val_size);

void start_new_tree_disk(int64_t table_id, int key,const char* value, uint16_t val_size);

// Find a record with the matching key from the given table.
//success return 0 else -1
int db_find(int64_t table_id, int64_t key, char* ret_val,uint16_t* val_size);

/* Traces the path from the root to a leaf, searching
 * by key.  Displays information about the path
 * if the verbose flag is set.
 * Returns the leaf containing the given key.
 */
pagenum_t find_leaf(int64_t table_id,int64_t key);

// Delete a record with the matching key from the given table.
int db_delete(int64_t table_id, int64_t key);


// root page 재설정 -> 미완성   
// root를 지우고 새 root를 만듬
// return new_root_pagenum (꽉차면 root_pagenum))
// header_page 세팅 -> root pagenum 세팅해야 한다.
pagenum_t adjust_root_page(int64_t table_id, pagenum_t root_pagenum);


//transation 관련 함수
//success return 0 else -1
int db_find(int64_t table_id, int64_t key, char* ret_val,uint16_t* val_size, int trx_id);

//success return 0 else -1
int db_update(int64_t table_id, int64_t key, char* value, uint16_t new_val_size,uint16_t* old_value_size, int trx_id);


/// @brief 
/// @param table_id 
/// @param key_pagenum key가 포함된 leaf pagenum
/// @param key slot이나 branch의 key
/// @param keys_right_pagenum branch의 key에 해당하는 pagenum -> 사용 X 나의 프로그램은 key가 slot & branch의 시작 지점이기 때문
/// @return 
pagenum_t delete_entry(int64_t table_id, pagenum_t key_pagenum, int64_t key);//, char* value){//pagenum_t keys_right_pagenum);

pagenum_t leaf_merge(int64_t table_id, pagenum_t key_pagenum, pagenum_t neighbor_pagenum, longlong_t neighbor_index, int k_prime);

pagenum_t internal_merge(int64_t table_id, pagenum_t key_pagenum, pagenum_t neighbor_pagenum, longlong_t neighbor_index, int k_prime);

pagenum_t leaf_redistribute(int64_t table_id, pagenum_t key_pagenum, pagenum_t neighbor_pagenum, longlong_t neighbor_index, longlong_t k_prime_index, int k_prime);

pagenum_t internal_redistribute(int64_t table_id, pagenum_t key_pagenum, pagenum_t neighbor_pagenum, longlong_t neighbor_index, longlong_t k_prime_index, int k_prime);

pagenum_t neighbor_used_space(int64_t table_id, pagenum_t neighbor_pagenum);

// panent에서 neighbor_index의 pagenum을 출력
// -1이면 2번째 pagenum를 리턴(정확히는 leftmost의 다음꺼)
// parent이기에 branch에서만 생각
// branch에서는 1번때 pagenum 
// 나머지는 그냥 리턴
pagenum_t get_neighbor_pagenum( int64_t table_id, pagenum_t key_pagenum, longlong_t neighbor_index);

// key_pagenum의 parent page에서의 key of k_prime_index(-1이면 0 / 나머지는 그대로)번째 key
// neighbor_pagenum & key_pagenum 사이에 있는 key값
int get_k_prime( int64_t table_id, pagenum_t key_pagenum, longlong_t k_prime_index);

// key_pagenum이 leftmost에 존재하면 -1 
// key_page가 0을 return하면 neighbor가 leftmost이다 
// key를 확인할때는 1번째
longlong_t get_neighbor_index(int64_t table_id, pagenum_t key_pagenum);

// FREE : 2500일때 INTERNAL : key_num < 124 일때 체크
// 만약 merge나 coalesce가 필요하다면 return 1 else return 0 
int need_merge_or_coalesce(int64_t table_id, pagenum_t key_pagenum);

//merge 해야 하는 i번째 return 미완성
int cut_merge(int64_t table_id, pagenum_t key_pagenum);

// delete key 포함된 slot & branch 
pagenum_t remove_entry_from_page(int64_t table_id, pagenum_t pagenum, int64_t key);


// Find records with a key between the range: begin_key ≤ key ≤ end_key
int db_scan(int64_t table_id, int64_t begin_key,int64_t end_key, std::vector<int64_t>* keys,std::vector<char*>* values,std::vector<uint16_t>* val_sizes);

// Initialize the database system.
int init_db(int num_buf);

// Shutdown the database system.
int shutdown_db();
