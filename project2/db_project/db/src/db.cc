
#include "db.h"
#include <malloc.h>
//#include "bpt.h"


std::vector<pagenum_t> opened_table_unique_id;

// Open an existing database file or create one if not exist.
int64_t open_table(const char* pathname){
    int64_t table_id;
    if(opened_table_unique_id.size() == 19){ //maximum number of tables is 20
        return -1;
    }
    table_id = file_open_table_file(pathname);
    opened_table_unique_id.push_back(table_id);
    return table_id;
}

//insertion

// Insert a record to the given table.
int db_insert(int64_t table_id, int64_t key, const char* value, uint16_t val_size){
    //check size
    if(val_size < leaf_min_value_size){
        printf("smaller than min= value_size: %d\n",val_size);
        printf("insert_fail\n");
        return -1;
    }
    if(val_size > leaf_max_value_size){
        printf("bigger than max= value_size: %d\n",val_size);
        printf("insert_fail\n");
        return -1;
    }
    char* tmp = (char*)malloc(sizeof(char)*200);
    uint16_t tmp_size=0;
    //check finded &&  check if key is inserted twice
    int check_same_key = db_find(table_id,key,tmp,&tmp_size);
    free(tmp);
    if( check_same_key == 0){
        printf("same key found\n");
        printf("insert_fail\n");
        return -1;
    }

    page_t header_page;
    file_read_page(table_id,0,&header_page);
    pagenum_t root_page_num = header_page::get_root_page_num(&header_page);
    page_t root_page;
    // if tree doesn't exist
    if(root_page_num == 0){
        file_write_page(table_id,0,&header_page);
        //unpin(table_id,0);
        start_new_tree_disk(table_id,key,value,val_size);
        return 0;
    }
    page_t key_exists_leaf_page;
    pagenum_t key_exists_leaf_pagenum = find_leaf(table_id,key);
    file_read_page(table_id,key_exists_leaf_pagenum,&key_exists_leaf_page);

    if(val_size + SLOT_SIZE <= leaf_page::get_amount_of_free_space(&key_exists_leaf_page)){
        //unpin(table_id,key_exists_leaf_pagenum);
        page_t inserted_page = insert_into_leaf(table_id,key_exists_leaf_pagenum,key,value,val_size); //leaf에 그냥 삽입
        return 0;
    }
    //unpin(table_id,key_exists_leaf_pagenum);
    page_t inserted_page = insert_into_leaf_after_splitting(table_id,key_exists_leaf_pagenum,key,value,val_size);
    return 0;
}
// leaf 면 조건에 맞추어서 internal이면 124
int cut(longlong_t table_id,pagenum_t leaf_pagenum){
    int i,A;
    page_t leaf_page;
    file_read_page(table_id,leaf_pagenum,&leaf_page);
    if(page_header::get_leaf_check(&leaf_page)==1){
            
        A = page_header::get_number_of_keys(&leaf_page);
        int sum_size=0;
        for(i=0;i<A;i++){
            //slot = leaf_page::get_nth_slot(&leaf_page,i);
            slot_t* ith_slot =leaf_page::get_nth_slot(&leaf_page,i);
            sum_size = sum_size + ith_slot->get_size() + SLOT_SIZE;
            free(ith_slot);
            if(sum_size >1983) break;
        }
        return i;
    }
    else{
        return INTERNAL_ORDER/2;
    }
}
page_t insert_into_parent(longlong_t table_id,longlong_t leaf_pagenum, pagenum_t new_leaf_pagenum,pagenum_t new_key){
    
    printf("insert_into_parent / new_key: %d\n",new_key);
    page_t parent_page, new_leaf_page, leaf_page;
    file_read_page(table_id,leaf_pagenum,&leaf_page);
    file_read_page(table_id,new_leaf_pagenum,&new_leaf_page);
    pagenum_t parent_pagenum = page_header::get_parent_page_num(&leaf_page);
    file_read_page(table_id,parent_pagenum,&parent_page);
    if(parent_pagenum==0){
        printf("insert_into_new_root\n");
        return insert_into_new_root(table_id,leaf_pagenum,new_key,new_leaf_pagenum);
    }
    printf("parent page 존재\n");
    pagenum_t parent_left_index= get_left_index(table_id,parent_pagenum,leaf_pagenum); //parent에서 right를 가르키는 page Number / 0이면 leftmost
    if(page_header::get_number_of_keys(&parent_page) < INTERNAL_ORDER){
        return insert_into_page(table_id,parent_pagenum,parent_left_index, new_key, new_leaf_pagenum);
    }
    return insert_into_page_after_splitting(table_id,parent_pagenum,parent_left_index, new_key,new_leaf_pagenum);

}
/*old_page : parent_page*/
page_t insert_into_page_after_splitting(longlong_t table_id,pagenum_t old_pagenum,pagenum_t left_index,pagenum_t new_key,pagenum_t right_pagenum){
    
    printf("insert_into_page_after_splitting\n");
    pagenum_t leftmost_key; //leftmost가 가르키는 곳
    int i,j,split,k_prime;
    branching_factor_t** temp_branch = (branching_factor_t**)malloc(250*sizeof(branching_factor_t*));
    page_t old_page;
    file_read_page(table_id,old_pagenum,&old_page);
    int_4bit A = page_header::get_number_of_keys(&old_page);//key의 개수 /1번째는 1번째 시작 / 248
    printf("key_number: %d\n",A);
    //lefmost는 따로 관리
    leftmost_key = internal_page::get_leftmost_key(&old_page); //leftmost pagenum
    //temp에 branch의 정보를 삽입
    for(i=1,j=1;i<=A;i++,j++){
        if(j==left_index+1) {
            j++;
        }
        
        temp_branch[j] = internal_page::get_nth_branching_factor(&old_page,i);
    }
    temp_branch[left_index+1] = (branching_factor_t*)malloc(sizeof(branching_factor_t));
    temp_branch[left_index+1]->set_keys(new_key);
    temp_branch[left_index+1]->set_pagenumber(right_pagenum);

    split = cut(table_id,old_pagenum); //124

    //old_page 초기화
    page_header::set_number_of_keys(&old_page,0);
    file_write_page(table_id,old_pagenum,&old_page);
    pagenum_t number_of_key = 0;
    for(i=0 ; i <= split ; i++){ //125회: 1회는 leftmost 세팅 124회는 key세팅
        if(i==0){ //i=0
            internal_page::set_leftmost_key(&old_page,leftmost_key);
            // printf("branch의 %d번째에 삽입\n",i);
            // printf("왼쪽 pagenum:%d\n",old_pagenum);
            // printf("삽입된 pagenum:%d\n",internal_page::get_leftmost_key(&old_page));
        }
        else{   //i=1~124 : 124회
            internal_page::set_nth_branching_factor(&old_page,i,temp_branch[i]);
            page_header::set_number_of_keys(&old_page,page_header::get_number_of_keys(&old_page)+1);
            //free(temp_branch[i]);
            
            // printf("branch의 %d번째에 삽입\n",i);
            // printf("왼쪽 pagenum:%d\n",old_pagenum);
            // branching_factor_t* ith_branch_factor = internal_page::get_nth_branching_factor(&old_page,i);
            // printf("page key 개수:%d\n",page_header::get_number_of_keys(&old_page));
            // printf("삽입된 key:%d\n",ith_branch_factor->get_keys());
            // printf("삽입된 pagenum:%d\n",ith_branch_factor->get_pagenumber());
            // free(ith_branch_factor);
        }
    }
    file_write_page(table_id,old_pagenum,&old_page);

    k_prime = temp_branch[split+1]->get_keys(); //125번째

    pagenum_t new_leftmost_key = temp_branch[split+1]->get_pagenumber();

    //make new page
    page_t new_page;
    pagenum_t new_pagenum = file_alloc_page(table_id);
    page_header::set_leaf_check(&new_page,0); //internal
    page_header::set_number_of_keys(&new_page,0);
    file_write_page(table_id,new_pagenum,&new_page);
    for(i=split+1,j=0;i<=A+1;i++,j++){ //125회: 1회는 leftmost 세팅
        if(j==0){ //i =split+1
            internal_page::set_leftmost_key(&new_page,new_leftmost_key);
            // printf("branch의 %d번째에 삽입\n",j);
            // printf("오른쪽 pagenum:%d\n",new_pagenum);
            // printf("삽입된 pagenum:%d\n",internal_page::get_leftmost_key(&new_page));
        }
        else{ // i= split+2=126 ~ i =A+1 / 249
            // branching_factor_t* one_temp_branch =(branching_factor_t*)malloc(sizeof(branching_factor_t));
            // one_temp_branch->set_keys(temp_branch[i]->get_keys());
            // one_temp_branch->set_pagenumber(temp_branch[i]->get_pagenumber());
            internal_page::set_nth_branching_factor(&new_page,j,temp_branch[i]);
            page_header::set_number_of_keys(&new_page,page_header::get_number_of_keys(&new_page)+1);

            // printf("branch의 %d번째에 삽입\n",j);
            // printf("오른쪽 pagenum:%d\n",new_pagenum);
            // printf("page key 개수:%d\n",page_header::get_number_of_keys(&new_page));
            // branching_factor_t* ith_branch_factor = internal_page::get_nth_branching_factor(&new_page,i);
            // printf("삽입된 key:%d\n",ith_branch_factor->get_keys());
            // printf("삽입된 pagenum:%d\n",ith_branch_factor->get_pagenumber());
            // free(ith_branch_factor);

        }
    }
    page_header::set_parent_page_num(&new_page,page_header::get_parent_page_num(&old_page));
    file_write_page(table_id,new_pagenum,&new_page);
    
    //vector free
    for(int k=1;k<=A+1;k++){
        free(temp_branch[k]);
    }
    free(temp_branch); 
    
    //parent update
    page_header::set_parent_page_num(&new_page, page_header::get_parent_page_num(&old_page));
    for(i=0;i<=page_header::get_number_of_keys(&new_page);i++){
        if(i==0){
            //make new child(leaf or internal)
            page_t new_child_page;
            file_read_page(table_id,internal_page::get_leftmost_key(&new_page),&new_child_page);
            page_header::set_parent_page_num(&new_child_page,new_pagenum);
            file_write_page(table_id,internal_page::get_leftmost_key(&new_page),&new_child_page);
        }
        else{
            //make new child(leaf or internal)
            page_t new_child_page;
            branching_factor_t* ith_branch_factor = internal_page::get_nth_branching_factor(&new_page,i);
            file_read_page(table_id,ith_branch_factor->get_pagenumber(),&new_child_page);
            page_header::set_parent_page_num(&new_child_page,new_pagenum);
            file_write_page(table_id,ith_branch_factor->get_pagenumber(),&new_child_page);
            free(ith_branch_factor);
        }
    }
    
    //free(one_temp_branch);
    return insert_into_parent(table_id, old_pagenum ,new_pagenum, k_prime);


}
// left_index 수정 

page_t insert_into_page(longlong_t table_id, pagenum_t parent_pagenum, pagenum_t left_index, pagenum_t new_key,pagenum_t right_pagenum){
    int i;
    printf("insert_into_page\n");
    page_t parent_page, right_page;
    file_read_page(table_id,parent_pagenum,&parent_page);
    file_read_page(table_id,right_pagenum,&right_page);
    printf("page_header::get_number_of_keys:%d\n",page_header::get_number_of_keys(&parent_page));
    for(i= page_header::get_number_of_keys(&parent_page);i>left_index;i--){

        branching_factor_t* temp_branch = internal_page::get_nth_branching_factor(&parent_page,i);
        internal_page::set_nth_branching_factor(&parent_page,i+1,temp_branch);
        // printf("shift:%d/ %d\n",temp_branch->get_keys(),temp_branch->get_pagenumber());
        // printf("branch.key:%d\n",temp_branch->get_keys());
        // printf("branch.pagenum%d\n",temp_branch->get_pagenumber());
        free(temp_branch);
    
    }
    
    branching_factor_t* temp_branch = (branching_factor_t*)malloc(sizeof(branching_factor_t));
    temp_branch->set_keys(new_key);
    temp_branch->set_pagenumber(right_pagenum);
    // printf("branch.key:%d\n",temp_branch->get_keys());
    // printf("branch.pagenum%d\n",temp_branch->get_pagenumber());
    internal_page::set_nth_branching_factor(&parent_page,left_index+1,temp_branch); //0일떄 leftmost / 1이면 branch
    free(temp_branch);
    page_header::set_number_of_keys(&parent_page,page_header::get_number_of_keys(&parent_page)+1);
    file_write_page(table_id,parent_pagenum,&parent_page);

    // branching_factor_t* ith_branch_factor = internal_page::get_nth_branching_factor(&parent_page,i);
    // printf("삽입된 page:%d\n",parent_pagenum);
    // printf("parent page key 개수:%d\n",page_header::get_number_of_keys(&parent_page));
    // printf("branch의 %d번째에 삽입\n",left_index+1);
    // printf("삽입된 key:%d\n",ith_branch_factor->get_keys());
    // printf("삽입된 pagenum:%d\n",ith_branch_factor->get_pagenumber());
    // free(ith_branch_factor);
    return parent_page;
}

page_t insert_into_new_root(longlong_t table_id,pagenum_t left_pagenum,pagenum_t key,pagenum_t right_pagenum){
    page_t new_root, header_page, left_page, right_page;
    //새로운 root 설정
    
    pagenum_t new_root_pagenum = file_alloc_page(table_id);
    //printf("새로운 노드의 주소: %d\n",new_root_pagenum);
    file_read_page(table_id,0,&header_page);
    header_page::set_root_page_num(&header_page,new_root_pagenum);
    file_write_page(table_id,0,&header_page);


    file_read_page(table_id,new_root_pagenum,&new_root);
    page_header::set_parent_page_num(&new_root,0); //set as 0 if root page
    page_header::set_leaf_check(&new_root,0); // internal
    page_header::set_number_of_keys(&new_root,1);
    internal_page::set_leftmost_key(&new_root,left_pagenum);
    branching_factor_t* branch = (branching_factor_t*)malloc(sizeof(branching_factor_t));
    branch->set_keys(key);
    branch->set_pagenumber(right_pagenum);
    file_read_page(table_id,left_pagenum,&left_page);
    file_read_page(table_id,right_pagenum,&right_page);
    page_header::set_parent_page_num(&left_page,new_root_pagenum);
    page_header::set_parent_page_num(&right_page,new_root_pagenum);
    internal_page::set_nth_branching_factor(&new_root,1,branch);
    free(branch);

    file_write_page(table_id,left_pagenum,&left_page);
    file_write_page(table_id,right_pagenum,&right_page);
    file_write_page(table_id,new_root_pagenum,&new_root);
    
    
    // branching_factor_t* nth_branch_factor = internal_page::get_nth_branching_factor(&new_root,1);
    // printf("새로운 왼쪽: %d\n",internal_page::get_leftmost_key(&new_root));
    // printf("key: %d\n",nth_branch_factor->get_keys());
    // printf("새로운 오른쪽: %d\n",nth_branch_factor->get_pagenumber());
    // free(nth_branch_factor);
    
    return new_root;
    

}

//insert된 parent의 left page를 가르키는 pagenum을 리턴하는 함수  (leaf_index: 0->leftmost / leaf_index: n -> nth_branch) 1번째 부터
longlong_t get_left_index(longlong_t table_id,pagenum_t parent_pagenum,pagenum_t left_pagenum){
    printf("get_left_index\n");
    pagenum_t left_index_address = 1;
    longlong_t left_index=0;
    page_t parent_page, left_page;
    file_read_page(table_id,left_pagenum,&left_page);
    file_read_page(table_id,parent_pagenum,&parent_page);
    left_index_address = internal_page::get_leftmost_key(&parent_page);
    while(left_index <= page_header::get_number_of_keys(&parent_page) &&  left_index_address != left_pagenum)
    {
        left_index +=1;
        branching_factor_t* ith_branch_factor = internal_page::get_nth_branching_factor(&parent_page,left_index);
        left_index_address = ith_branch_factor->get_pagenumber();
        free(ith_branch_factor);
    }
    printf("returned left_index: %d\n",left_index);
    return left_index; // pointer는 0번째부터 key는 1번쨰부터
}


page_t insert_into_leaf_after_splitting(longlong_t table_id,pagenum_t leaf_pagenum,longlong_t key, const char* value,ushortint_t val_size){
   
    //std::vector <slot_t*> temp_slots(125) ;
    slot_t** temp_slots= (slot_t**)malloc(67*sizeof(slot_t*)); //최대 64개
    char** temp_value=(char**)malloc(67*sizeof(char*)); //65
    int insertion_point, split, i, j;
    pagenum_t new_key;
    insertion_point = 0;
    page_t leaf_page;
    file_read_page(table_id,leaf_pagenum,&leaf_page);
    int_4bit A = page_header::get_number_of_keys(&leaf_page);//key의 개수 /번째는0번째 시작
    //어떤 slot 사이에 들어갈지 계산
    slot_t* ith_slot1 = leaf_page::get_nth_slot(&leaf_page,insertion_point);
    while(insertion_point < A && ith_slot1->get_key() < key){
        free(ith_slot1);
        insertion_point++;
        ith_slot1 = leaf_page::get_nth_slot(&leaf_page,insertion_point);
    }
    free(ith_slot1);
    printf("spliting insertion point : %d\n",insertion_point);
    //printf(" key_number: %d",A);
    //temp에 slot의 정보를 삽입
    for(i=0,j=0;i<A;i++,j++){
        if(j==insertion_point){
            j++;
        }
        temp_slots[j] = leaf_page::get_nth_slot(&leaf_page,i);
        temp_value[j] = leaf_page::get_value(&leaf_page,temp_slots[j]->get_offset(), temp_slots[j]->get_size()) ;//const_cast<char*>(value);
        
        
        //printf("shifted_slot.key:%d\n",temp_slots[j]->get_key());
        //printf("slot.offset%d\n",temp_slots[j]->get_offset());
        //printf("slot.size:%d\n",temp_slots[j]->get_size());
        //printf("slot.value:%s\n",(char*)temp_value[j]);
        
    }
    temp_slots[insertion_point] = (slot_t*)malloc(sizeof(slot_t));
    temp_slots[insertion_point]->set_key(key);
    temp_slots[insertion_point]->set_offset(NULL);
    temp_slots[insertion_point]->set_size(val_size);
    temp_value[insertion_point] = const_cast<char*>(value);

    split = cut(table_id,leaf_pagenum);
    //printf("!!!!\n");
    
    //leaf의 offset 설정 && leaf_page 초기화
    leaf_page::set_amount_of_free_space(&leaf_page,leaf_page_inital_amount_of_free_space);
    page_header::set_number_of_keys(&leaf_page,0);
    file_write_page(table_id,leaf_pagenum,&leaf_page);
    ushortint_t offset = leaf_page_inital_offset;
    for(i=0 ; i < split ; i++){
        //printf("in for\n");
        pagenum_t free_space = leaf_page::get_amount_of_free_space(&leaf_page);
        leaf_page::set_amount_of_free_space(&leaf_page,free_space - temp_slots[i]->get_size() - SLOT_SIZE);
        offset = offset-temp_slots[i]->get_size();
        temp_slots[i]->set_offset(offset);

        leaf_page::set_value(&leaf_page,offset,temp_value[i],temp_slots[i]->get_size());
        page_header::set_number_of_keys(&leaf_page,page_header::get_number_of_keys(&leaf_page)+1);
        leaf_page::set_nth_slot(&leaf_page,i,temp_slots[i]);

        file_write_page(table_id,leaf_pagenum,&leaf_page);
    }
    page_t new_leaf;
    pagenum_t new_leaf_pagenum = file_alloc_page(table_id);
    file_read_page(table_id,new_leaf_pagenum,&new_leaf);
    
    page_header::set_leaf_check(&new_leaf,1);
    page_header::set_number_of_keys(&new_leaf,0);
    leaf_page::set_amount_of_free_space(&new_leaf,leaf_page_inital_amount_of_free_space);
    file_write_page(table_id,new_leaf_pagenum,&new_leaf);
    offset = leaf_page_inital_offset;

    for(i=split,j=0;i<A+1;i++,j++){
        //printf("%d\n",temp_slot->get_size());
        pagenum_t free_space = leaf_page::get_amount_of_free_space(&new_leaf);
        leaf_page::set_amount_of_free_space(&new_leaf,free_space - temp_slots[i]->get_size()-SLOT_SIZE);
        file_write_page(table_id,new_leaf_pagenum,&new_leaf);
        offset = offset-temp_slots[i]->get_size();;
        temp_slots[i]->set_offset(offset);


        leaf_page::set_value(&new_leaf,offset,temp_value[i],temp_slots[i]->get_size());
        page_header::set_number_of_keys(&new_leaf,page_header::get_number_of_keys(&new_leaf)+1);
        leaf_page::set_nth_slot(&new_leaf,j,temp_slots[i]);
        
        // printf("삽입되고 있는 page: %d/ i: %d\n",new_leaf_pagenum,j);
        // printf("slot.key:%d\n",temp_slots[i]->get_key());
        // printf("slot.offset:%d\n",temp_slots[i]->get_offset());
        // printf("slot.size:%d\n",temp_slots[i]->get_size());
    
        file_write_page(table_id,new_leaf_pagenum,&new_leaf);
    }

    //free vector
    for(int k=0;k<=A;k++){
        free(temp_slots[k]);

        if(k!=insertion_point){
            free(temp_value[k]);
        }
    }
    free(temp_slots);
    free(temp_value);

    //leaf_page && new_leaf linked list
    leaf_page::set_right_sibling_page_number(&new_leaf,leaf_page::get_right_sibling_page_number(&leaf_page));
    leaf_page::set_right_sibling_page_number(&leaf_page,new_leaf_pagenum);
    file_write_page(table_id,new_leaf_pagenum,&new_leaf);
    
    //parent update
    page_header::set_parent_page_num(&new_leaf,page_header::get_parent_page_num(&leaf_page));
    file_write_page(table_id,new_leaf_pagenum,&new_leaf);
    slot_t* ith_slot = leaf_page::get_nth_slot(&new_leaf,0);
    new_key = ith_slot->get_key();
    free(ith_slot);
    return insert_into_parent(table_id,leaf_pagenum ,new_leaf_pagenum, new_key);

}


page_t insert_into_leaf(longlong_t table_id,pagenum_t leaf_pagenum,longlong_t key, const char* value,ushortint_t val_size){
    int i, insertion_point;
    printf("insert_into_leaf/ key: %d\n",key);
    insertion_point = 0; //0번째부터 시작
    page_t leaf_page;
    file_read_page(table_id,leaf_pagenum,&leaf_page);
    int_4bit A = page_header::get_number_of_keys(&leaf_page);//key의 개수 /번째는0번째 시작
    //printf("key_num: %d\n",A);
    //slot 재배열
    slot_t* ith_slot = leaf_page::get_nth_slot(&leaf_page,insertion_point);
    while(insertion_point < A && ith_slot->get_key() < key){
        free(ith_slot);
        insertion_point++;
        ith_slot = leaf_page::get_nth_slot(&leaf_page,insertion_point);
    }
    free(ith_slot);
    
    printf("insertion_point: %d\n",insertion_point);
    for(i=A-1 ; i>=insertion_point ; i--){ //slot은 0번째 부터 시작 따라서 A개의 숫자는 0번째부터 A-1번째까지
        slot_t* right_slot=leaf_page::get_nth_slot(&leaf_page,i);
        char* i_value = leaf_page::get_value(&leaf_page,right_slot->get_offset(),right_slot->get_size());
        
        right_slot->set_offset(right_slot->get_offset()-val_size);// slot의 vlaue를 가르키는 offset을 옆으로 밀음
        leaf_page::set_value(&leaf_page,right_slot->get_offset(),i_value,right_slot->get_size());
        leaf_page::set_nth_slot(&leaf_page,i+1,right_slot);
        
        //printf("right_slot.key:%d\n",right_slot->get_key());
        //printf("slot.offset%d\n",right_slot->get_offset());
        //printf("slot.size:%d\n",right_slot->get_size());
        //printf("slot.value:%s\n",(char*)i_value);
        free(right_slot);
        free(i_value);
    }
    // key의 slot 만든 후 후 대입
    slot_t* slot = (slot_t*) malloc(sizeof(slot_t));
    slot->set_key(key);
    slot->set_size(val_size);
    ushortint_t offset;
    if(insertion_point==0){
        offset=leaf_page_inital_offset-val_size;
    }
    else{
        slot_t* ith_slot = leaf_page::get_nth_slot(&leaf_page,insertion_point-1);
        offset = ith_slot->get_offset() - val_size; //새로 삽입하는 slot offset을 왼쪽의 slot의 다음 offset에 삽입
        free(ith_slot);
    }
    slot->set_offset(offset);
    // printf("삽입되고 있는 page: %d\n",leaf_pagenum);
    // printf("slot.key:%d\n",slot->get_key());
    // printf("slot.offset%d\n",slot->get_offset());
    // printf("slot.size:%d\n",slot->get_size());

    //put new slot in the leaf page
    leaf_page::set_value(&leaf_page,offset,value,val_size);
    leaf_page::set_nth_slot(&leaf_page,insertion_point,slot);
    free(slot);
    //update leaf_page
    page_header::set_number_of_keys(&leaf_page,A+1);
    leaf_page::set_amount_of_free_space(&leaf_page,leaf_page::get_amount_of_free_space(&leaf_page)-SLOT_SIZE-val_size);
    
    file_write_page(table_id,leaf_pagenum,&leaf_page); // 써주는 행위는 각각의 함수가 관리
    return leaf_page;
}

void start_new_tree_disk(int64_t table_id, int key,const char* value, uint16_t val_size){
    
    page_t header_page;
    printf("make new tree\n");
    
    file_read_page(table_id,0,&header_page);
    
    pagenum_t root_page_num = file_alloc_page(table_id);
    file_read_page(table_id,0,&header_page);
    
    header_page::set_root_page_num(&header_page,root_page_num);
    page_t root_page;
    file_read_page(table_id,root_page_num,&root_page);
    page_header::set_leaf_check(&root_page,1);
    
    page_header::set_number_of_keys(&root_page,1);
    page_header::set_parent_page_num(&root_page,0);
    leaf_page::set_amount_of_free_space(&root_page,leaf_page_inital_amount_of_free_space);
    
    slot_t* slot=(slot_t*) malloc(sizeof(slot_t));
    slot->set_key(key);
    slot->set_size(val_size);
    ushortint_t offset;
    offset = leaf_page_inital_offset-val_size;
    slot->set_offset(offset);

    leaf_page::set_value(&root_page,offset,(const char*)value,val_size);
    leaf_page::set_amount_of_free_space(&root_page,leaf_page_inital_amount_of_free_space - val_size - SLOT_SIZE);
    leaf_page::set_nth_slot(&root_page,0,slot);
    free(slot);
    
    leaf_page::set_right_sibling_page_number(&root_page, 0);
    file_write_page(table_id,0,&header_page);
    file_write_page(table_id,root_page_num,&root_page);
    
    return;
}

// Find a record with the matching key from the given table.
// success return 0 else -1
int db_find(int64_t table_id, int64_t key, char* ret_val, uint16_t* val_size){
    int i=0;
    page_t key_exist_leaf_page;
    pagenum_t key_exist_leaf_pagenum;
    key_exist_leaf_pagenum = find_leaf(table_id,key);
    if(key_exist_leaf_pagenum==0){
        return -1;
    }
    file_read_page(table_id,key_exist_leaf_pagenum,&key_exist_leaf_page);

    for(i=0;i<page_header::get_number_of_keys(&key_exist_leaf_page);i++){// printf("free space : %d\n",leaf_page::get_amount_of_free_space(&key_exist_leaf_page));
        slot_t* ith_slot = leaf_page::get_nth_slot(&key_exist_leaf_page,i);
        if(ith_slot->get_key() == (pagenum_t)key) {
            free(ith_slot);
            break;
        }
        free(ith_slot);
    }
    
    if(i == page_header::get_number_of_keys(&key_exist_leaf_page)){
        return -1;
    }
    else{ //find sccess
        slot_t* ith_slot = leaf_page::get_nth_slot(&key_exist_leaf_page,i);
        ushortint_t size = ith_slot->get_size();
        std::memcpy(val_size,&size,2);
        char* value = leaf_page::get_value(&key_exist_leaf_page,ith_slot->get_offset(),ith_slot->get_size());
        std::memcpy(ret_val,value,size);
        free(ith_slot);
        free(value);
        return 0;
    }
    return -1;
}

/* Traces the path from the root to a leaf, searching
 * by key.  Displays information about the path
 * if the verbose flag is set.
 * Returns the leaf containing the given key.
 */
// no root(find leaf fail) return 0 / success return not 0
// key 보다 작은거는 앞에 같거나 큰거는 뒤에  
pagenum_t find_leaf(int64_t table_id,int64_t key){
    page_t header_page;
    file_read_page(table_id,0,&header_page);
    pagenum_t root_pagenum = header_page::get_root_page_num(&header_page);
    if(root_pagenum == 0){ //root 가 없음
        printf("Is empty\n");
        return root_pagenum;
    }
    page_t root_page;
    file_read_page(table_id,root_pagenum,&root_page);
    pagenum_t key_exist_pagenum = root_pagenum;
    pagenum_t i=0;
    pagenum_t check_leaf = page_header::get_leaf_check(&root_page);
    pagenum_t A =page_header::get_number_of_keys(&root_page);
    while( check_leaf!= 1){
        i=0;
        while(i < A){
            branching_factor_t* ith_branch_factor = internal_page::get_nth_branching_factor(&root_page,i+1);
            
            if(key >= ith_branch_factor->get_keys()){   
                i++;  
                free(ith_branch_factor);
            }
            else {
                free(ith_branch_factor);
                break;
            }
        }

        if(i==0){
            key_exist_pagenum = internal_page::get_leftmost_key(&root_page);
        }
        else{
            branching_factor_t* ith_branch_factor = internal_page::get_nth_branching_factor(&root_page,i);
            key_exist_pagenum = ith_branch_factor->get_pagenumber();
            free(ith_branch_factor);
        }
        file_read_page(table_id,key_exist_pagenum,&root_page);
        check_leaf = page_header::get_leaf_check(&root_page);
        A = page_header::get_number_of_keys(&root_page);
    }
    return key_exist_pagenum;
}
/*

//deletion

// Delete a record with the matching key from the given table.
// success return 0 fail return -1
int db_delete(int64_t table_id, int64_t key){
    
    char* value =(char*)malloc(sizeof(char)*200);
    uint16_t value_size;
    int found = db_find(table_id,key,value,&value_size);
    printf("%d\n",found);
    pagenum_t found_pagenum = find_leaf(table_id,key);
    printf("%d\n",found_pagenum);
    if(found == 0 && found_pagenum != 0){
        //printf("found success\n");
        delete_entry(table_id,found_pagenum,key);//,value);
        free(value);
        return 0;
    }
    else{
        printf("finding key: %d failed\n",key);
        free(value);
        return -1;
    }
}

// root page 재설정 -> 미완성   
// root를 지우고 새 root를 만듬
// return new_root_pagenum (꽉차면 root_pagenum))
// header_page 세팅 -> root pagenum 세팅해야 한다.
pagenum_t adjust_root_page(int64_t table_id, pagenum_t root_pagenum){
    
    page_t header_page, root_page;
    file_read_page(table_id,0,&header_page);
    file_read_page(table_id,root_pagenum,&root_page);
    if(page_header::get_number_of_keys(&root_page) > 0){ // 개수가 남아있음 상관 X
        return root_pagenum;
    }   
    pagenum_t new_root_pagenum; // = file_alloc_page(table_id);

    if(page_header::get_leaf_check(&root_page) == 0){ //if internal page
        printf("root change\n");
        new_root_pagenum = internal_page::get_leftmost_key(&root_page);
        page_t new_root_page;
        file_read_page(table_id,new_root_pagenum,&new_root_page);
        page_header::set_parent_page_num(&new_root_page,0); 
        header_page::set_root_page_num(&header_page,new_root_pagenum);
        file_write_page(table_id,0,&header_page);
        file_write_page(table_id,new_root_pagenum,&new_root_page);
    }
    else{ // if leaf page
        header_page::set_root_page_num(&header_page,0);
        file_write_page(table_id,0,&header_page);
    }
    buffer_free_page(table_id,root_pagenum);

    return new_root_pagenum;

}

/// @brief 
/// @param table_id 
/// @param key_pagenum key가 포함된 leaf pagenum
/// @param key slot이나 branch의 key
/// @param keys_right_pagenum branch의 key에 해당하는 pagenum -> 사용 X 나의 프로그램은 key가 slot & branch의 시작 지점이기 때문
/// @return 
pagenum_t delete_entry(int64_t table_id, pagenum_t key_pagenum, int64_t key){//, char* value){//pagenum_t keys_right_pagenum){
    int64_t min_key;
    pagenum_t neighbor_pagenum;
    longlong_t neighbor_index, k_prime_index;
    int k_prime;
    //printf("key_pagenum: %d\n",key_pagenum);

    remove_entry_from_page(table_id,key_pagenum,key);
    page_t key_page, header_page;
    file_read_page(table_id,key_pagenum,&key_page);
    if( page_header::get_parent_page_num(&key_page) == 0 ){ //key_pagenum 이 root라면
        printf("adjust_root_page\n");
        return adjust_root_page(table_id,key_pagenum);
    }

    // min key 세팅 - num_key가 넘어가면 / node merge 또는 다른게 되는지 확인
    
    //min_key 세팅
    if( need_merge_or_coalesce(table_id,key_pagenum) == 0 ){ // merge or coalesce 필요 X
        return key_pagenum;
    }

    neighbor_index = get_neighbor_index(table_id, key_pagenum); // parent에서 key_pagenum의 왼쪽 번째를 받아온다
    k_prime_index =  neighbor_index == -1 ? 0 : neighbor_index; // 0이면 neighbor의 왼쪽 index / key는 +1로 계산
    k_prime = get_k_prime(table_id,key_pagenum,k_prime_index); // key_pagenum의 parent page에서 neighbor_pagenum & key_pagenum 사이에 있는 key값
    neighbor_pagenum = get_neighbor_pagenum(table_id,key_pagenum,neighbor_index); // neighbor pagenum을 가져온다. key_pagenum이 leftmost일 경우 1번째 branch의 pagneu 가져옴 
    //printf("delete entry: neighbor_index: %d k_prime_index: %d k_prime: %d neighbor_pagenum: %d\n",neighbor_index,k_prime_index,k_prime,neighbor_pagenum);
    
    page_t neighbor_page;
    file_read_page(table_id,neighbor_pagenum,&neighbor_page);
    
    if(page_header::get_leaf_check(&key_page) == 1) { //leaf page이면
        //cap
        if(leaf_page::get_amount_of_free_space(&key_page) >= neighbor_used_space(table_id,neighbor_pagenum)){
            printf("leaf_merge\n");
            return leaf_merge(table_id, key_pagenum, neighbor_pagenum, neighbor_index, k_prime);
        }
        else{
            printf("leaf_redistribute\n");
            return leaf_redistribute(table_id, key_pagenum, neighbor_pagenum, neighbor_index, k_prime_index ,k_prime);
        }
    }
    else{ //internal page
        if( page_header::get_number_of_keys(&neighbor_page) + page_header::get_number_of_keys(&key_page) < INTERNAL_ORDER -1 ){
            printf("internal_merge\n");
            return internal_merge(table_id, key_pagenum, neighbor_pagenum, neighbor_index, k_prime);
        }
        else{
            printf("internal_redistribute\n");
            return internal_redistribute(table_id, key_pagenum, neighbor_pagenum, neighbor_index, k_prime_index ,k_prime);
        }
    }

}
pagenum_t leaf_merge(int64_t table_id, pagenum_t key_pagenum, pagenum_t neighbor_pagenum, longlong_t neighbor_index, int k_prime){
    int i,j;
    longlong_t neighbor_insertion_index;
    int key_end; // key_num of new page of leaf merge
    pagenum_t tmp;
    

    if(neighbor_index == -1){ // key_pagenum = 0 이면 한칸 옆으로 밀고 다시 정의하기
        tmp = key_pagenum;
        key_pagenum = neighbor_pagenum;
        neighbor_pagenum = tmp;
    }
    
    printf("key_pagenum: %d, neighbor_pagenum: %d\n",key_pagenum,neighbor_pagenum);
    page_t key_page, header_page, neighbor_page;

    file_read_page(table_id,key_pagenum,&key_page);
    file_read_page(table_id,0,&header_page);
    file_read_page(table_id,neighbor_pagenum,&neighbor_page);

    neighbor_insertion_index = page_header::get_number_of_keys(&neighbor_page); // slot은 0 시작이기 때문에, 개수가 1개이면 그 자리로 선언해도 된다.

    //여기 위까지 공통
    // if(page_header::get_leaf_check(&key_page)) already checked
    slot_t* ith_offset_slot = leaf_page::get_nth_slot(&neighbor_page,page_header::get_number_of_keys(&neighbor_page)-1);
    
    int offset = ith_offset_slot->get_offset();
    for(i=neighbor_insertion_index,j=0; j < page_header::get_number_of_keys(&key_page);i++,j++){

        slot_t* ith_slot = leaf_page::get_nth_slot(&key_page,j);
        char* ith_value = leaf_page::get_value(&key_page,ith_slot->get_offset(),ith_slot->get_size());
        offset = offset - ith_slot->get_size();
        ith_slot->set_offset(offset);
        leaf_page::set_value(&neighbor_page,ith_slot->get_offset(),ith_value,ith_slot->get_size());
        leaf_page::set_amount_of_free_space(&neighbor_page,leaf_page::get_amount_of_free_space(&neighbor_page) - ith_slot->get_size() - SLOT_SIZE);
        leaf_page::set_nth_slot(&neighbor_page,i,ith_slot); 
        page_header::set_number_of_keys(&neighbor_page,page_header::get_number_of_keys(&neighbor_page)+1);
        file_write_page(table_id,neighbor_pagenum,&neighbor_page);
        free(ith_slot);
    }
    free(ith_offset_slot);

    page_header::set_number_of_keys(&key_page,0);
    file_write_page(table_id,key_pagenum,&key_page);
    //이 아래도 공유
    leaf_page::set_right_sibling_page_number(&neighbor_page,leaf_page::get_right_sibling_page_number(&key_page));
    file_write_page(table_id,neighbor_pagenum,&neighbor_page);
    neighbor_pagenum = delete_entry(table_id,page_header::get_parent_page_num(&key_page),k_prime);
    //buffer_free_page(table_id,key_pagenum);

    return neighbor_pagenum;
}

pagenum_t internal_merge(int64_t table_id, pagenum_t key_pagenum, pagenum_t neighbor_pagenum, longlong_t neighbor_index, int k_prime){
    
    int i,j,n_end;
    longlong_t neighbor_insertion_index;
    int key_end; // key_num of new page of leaf merge
    pagenum_t tmp;

    if(neighbor_index == -1){ // key_pagenum = 0 이면 한칸 옆으로 밀고 다시 정의하기
        tmp = key_pagenum;
        key_pagenum = neighbor_pagenum;
        neighbor_pagenum = tmp;
    }

    
    printf("key_pagenum: %d, neighbor_pagenum: %d\n",key_pagenum,neighbor_pagenum);
    page_t key_page, header_page, neighbor_page; //header page는 사용 X

    file_read_page(table_id,key_pagenum,&key_page);
    file_read_page(table_id,0,&header_page);    
    file_read_page(table_id,neighbor_pagenum,&neighbor_page);
    
    neighbor_insertion_index = page_header::get_number_of_keys(&neighbor_page) + 1; //branch는 1부터 시작하기 때문에 +1 해줘야 다음고스이 index이다.
    //여기 위까지 공통
    // if(page_header::get_leaf_check(&key_page)) already checked
    branching_factor_t* ith_branch = (branching_factor_t*)malloc(sizeof(branching_factor_t));
    ith_branch->set_keys(k_prime);
    ith_branch->set_pagenumber(internal_page::get_leftmost_key(&key_page));
    
    page_header::set_number_of_keys(&neighbor_page,page_header::get_number_of_keys(&neighbor_page)+1);
    internal_page::set_nth_branching_factor(&neighbor_page,neighbor_insertion_index,ith_branch);
    
    //neighbor_page update
    file_write_page(table_id,neighbor_pagenum,&neighbor_page);
    free(ith_branch);
    file_read_page(table_id,neighbor_pagenum,&neighbor_page);

    n_end = page_header::get_number_of_keys(&key_page);

    for(i=neighbor_insertion_index+1,j=1; j <= n_end ; i++,j++){ //branch는 1부터 시작 leftmost는 0이지만 이미 위에서 다룸

        branching_factor_t* ith_slot = internal_page::get_nth_branching_factor(&key_page,j);
        internal_page::set_nth_branching_factor(&neighbor_page,i,ith_slot); 
        
        //neighbor->num_keys++;
        page_header::set_number_of_keys(&neighbor_page,page_header::get_number_of_keys(&neighbor_page)+1);
        file_write_page(table_id,neighbor_pagenum,&neighbor_page);
        
        // n->num_keys--;
        page_header::set_number_of_keys(&key_page,page_header::get_number_of_keys(&key_page)-1);
        file_write_page(table_id,key_pagenum,&key_page);
    }

    for(i=0;i<page_header::get_number_of_keys(&neighbor_page)+1;i++){
        page_t tmp_page;
        if(i==0){
            tmp = internal_page::get_leftmost_key(&neighbor_page);
            file_read_page(table_id,tmp,&tmp_page);
            page_header::set_parent_page_num(&tmp_page,neighbor_pagenum);
            file_write_page(table_id,tmp,&tmp_page);
        }
        else{
            branching_factor_t* ith_branch1 = internal_page::get_nth_branching_factor(&neighbor_page,i);
            tmp = ith_branch1->get_pagenumber();
            file_read_page(table_id,tmp,&tmp_page);
            page_header::set_parent_page_num(&tmp_page,neighbor_pagenum);
            file_write_page(table_id,tmp,&tmp_page);
            free(ith_branch1); // 에러 날수도? tmp에 저장이 안되어서
        }
    }
    file_write_page(table_id,neighbor_pagenum,&neighbor_page);
    //return 0;
    neighbor_pagenum = delete_entry(table_id,page_header::get_parent_page_num(&key_page),k_prime);
    //buffer_free_page(table_id,key_pagenum);

    return neighbor_pagenum;
}

pagenum_t leaf_redistribute(int64_t table_id, pagenum_t key_pagenum, pagenum_t neighbor_pagenum, longlong_t neighbor_index, longlong_t k_prime_index, int k_prime){
    int i;
    
    printf("key_pagenum: %d, neighbor_pagenum: %d\n",key_pagenum,neighbor_pagenum);
    page_t key_page, header_page, neighbor_page, parent_page;

    file_read_page(table_id,key_pagenum,&key_page);
    file_read_page(table_id,0,&header_page);
    file_read_page(table_id,neighbor_pagenum,&neighbor_page);
    pagenum_t parent_pagenum = page_header::get_parent_page_num(&key_page);
    file_read_page(table_id,parent_pagenum,&parent_page);

    
    
    if(neighbor_index != -1){ // key_pagenum = 0 이면 한칸 옆으로 밀고 다시 정의하기
        slot_t* ith_offset_slot = leaf_page::get_nth_slot(&neighbor_page,page_header::get_number_of_keys(&neighbor_page)-1);
        int size = ith_offset_slot->get_size();
        for(i = page_header::get_number_of_keys(&key_page);i>0;i--){
            slot_t* ith_slot = leaf_page::get_nth_slot(&key_page,i-1);
            char* ith_value = leaf_page::get_value(&key_page,ith_slot->get_offset(),ith_slot->get_size());
            ith_slot->set_offset(ith_slot->get_offset()-size);
            leaf_page::set_value(&key_page,ith_slot->get_offset(),ith_value,ith_slot->get_size());
            leaf_page::set_nth_slot(&key_page,i,ith_slot);
            file_write_page(table_id,key_pagenum,&key_page);

            free(ith_slot);
            free(ith_value);
        }
        slot_t* ith_slot = leaf_page::get_nth_slot(&neighbor_page,page_header::get_number_of_keys(&neighbor_page)-1);
        char* ith_value = leaf_page::get_value(&neighbor_page,ith_slot->get_offset(),ith_slot->get_size());
        
        leaf_page::set_value(&key_page,leaf_page_inital_offset,ith_value,ith_slot->get_size());
        leaf_page::set_nth_slot(&key_page,0,ith_slot);

        branching_factor_t* ith_branch = internal_page::get_nth_branching_factor(&parent_page,k_prime_index+1);
        ith_branch->set_keys(ith_slot->get_key());
        internal_page::set_nth_branching_factor(&parent_page,k_prime_index+1,ith_branch);

        free(ith_slot);
        free(ith_value);
        free(ith_branch);
        free(ith_offset_slot);
    }
    else{ // key_pagenum이 leftmost일때 
        

        slot_t* ith_slot0 = leaf_page::get_nth_slot(&neighbor_page,0);
        int size = ith_slot0->get_size();
        char* ith_value0 = leaf_page::get_value(&neighbor_page,ith_slot0->get_offset(),size);

        slot_t* ith_offset_slot = leaf_page::get_nth_slot(&key_page,page_header::get_number_of_keys(&key_page)-1);
        int offset = ith_offset_slot->get_offset() - size;

        ith_slot0->set_offset(offset);
        leaf_page::set_value(&key_page,offset,ith_value0,size);
        leaf_page::set_nth_slot(&key_page,page_header::get_number_of_keys(&key_page),ith_slot0);
        
        free(ith_value0);

        slot_t* ith_slot1 = leaf_page::get_nth_slot(&neighbor_page,1);
        branching_factor_t* ith_branch = internal_page::get_nth_branching_factor(&parent_page,k_prime_index+1);
        ith_branch->set_keys(ith_slot1->get_key());
        internal_page::set_nth_branching_factor(&parent_page,k_prime_index+1,ith_branch);


        for(i=0;i<page_header::get_number_of_keys(&neighbor_page)-1;i++){
            slot_t* ith_slot2 = leaf_page::get_nth_slot(&neighbor_page,i+1);
            
            char* ith_value2 = leaf_page::get_value(&neighbor_page,ith_slot2->get_offset(),ith_slot2->get_size());
            ith_slot2->set_offset(ith_slot2->get_offset()+size);
            leaf_page::set_value(&neighbor_page,ith_slot2->get_offset(),ith_value2,ith_slot2->get_size());
            leaf_page::set_nth_slot(&neighbor_page,i,ith_slot2);

            free(ith_slot2);
            free(ith_value2);
            file_write_page(table_id,neighbor_pagenum,&neighbor_page);
        }   
        free(ith_slot0);

        free(ith_slot1);
        free(ith_branch);
    }
    file_write_page(table_id,key_pagenum,&key_page);
    file_write_page(table_id,parent_pagenum,&parent_page); // 사실 안 들어가도 될듯?

    
    page_header::set_number_of_keys(&key_page,page_header::get_number_of_keys(&key_page)+1);
    page_header::set_number_of_keys(&neighbor_page,page_header::get_number_of_keys(&neighbor_page)-1);
    
    file_write_page(table_id,key_pagenum,&key_page);
    file_write_page(table_id,neighbor_pagenum,&neighbor_page);
    return key_pagenum;
}

pagenum_t internal_redistribute(int64_t table_id, pagenum_t key_pagenum, pagenum_t neighbor_pagenum, longlong_t neighbor_index, longlong_t k_prime_index, int k_prime){
    int i;
    pagenum_t tmp_pagenum;
    page_t tmp_page;
    
    
    page_t key_page, header_page, neighbor_page, parent_page;

    printf("key_pagenum: %d, neighbor_pagenum: %d\n",key_pagenum,neighbor_pagenum);
    file_read_page(table_id,key_pagenum,&key_page);
    file_read_page(table_id,0,&header_page);
    file_read_page(table_id,neighbor_pagenum,&neighbor_page);
    pagenum_t parent_pagenum = page_header::get_parent_page_num(&key_page);
    file_read_page(table_id,parent_pagenum,&parent_page);

    if(neighbor_index != -1){ // key_pagenum = 0 이면 한칸 옆으로 밀고 다시 정의하기
        for(i = page_header::get_number_of_keys(&key_page);i>0;i--){
            if(i==1){ // leftmost 넘기기
                pagenum_t leftmost_pagenum = internal_page::get_leftmost_key(&key_page);
                branching_factor_t* ith_branch = internal_page::get_nth_branching_factor(&key_page,1);
                ith_branch->set_pagenumber(leftmost_pagenum);
                internal_page::set_nth_branching_factor(&key_page,1,ith_branch);
                file_write_page(table_id,key_pagenum,&key_page);
                free(ith_branch);
            }
            else{
                branching_factor_t* ith_branch = internal_page::get_nth_branching_factor(&key_page,i-1);
                internal_page::set_nth_branching_factor(&key_page,i,ith_branch);
                file_write_page(table_id,key_pagenum,&key_page);
                free(ith_branch);
            }
        }
        
        branching_factor_t* ith_branch = internal_page::get_nth_branching_factor(&neighbor_page,page_header::get_number_of_keys(&neighbor_page));
        internal_page::set_leftmost_key(&key_page,ith_branch->get_pagenumber());
        tmp_pagenum = ith_branch->get_pagenumber();
        file_read_page(table_id,tmp_pagenum,&tmp_page);
        page_header::set_parent_page_num(&tmp_page,key_pagenum);
        file_write_page(table_id,tmp_pagenum,&tmp_page);

        branching_factor_t* ith_branch1 = internal_page::get_nth_branching_factor(&key_page,1);
        ith_branch1->set_keys(k_prime);
        file_write_page(table_id,key_pagenum,&key_page); // 나중에 한번에 정리 가능
        //ith_branch       
        branching_factor_t* ith_branch2 = internal_page::get_nth_branching_factor(&parent_page,k_prime_index+1); // key 재설정이기 때문에 k_prime_index 다음꺼를 설정 
        ith_branch2->set_keys(ith_branch->get_keys()); 
        file_write_page(table_id,parent_pagenum,&parent_page);
        
        free(ith_branch);
        free(ith_branch1);
        free(ith_branch2);
    }
    else{ // key_pagenum이 leftmost일때
        branching_factor_t* ith_branch = (branching_factor_t*)malloc(sizeof(branching_factor_t));
        ith_branch->set_keys(k_prime);
        ith_branch->set_pagenumber(internal_page::get_leftmost_key(&neighbor_page));
        internal_page::set_nth_branching_factor(&key_page,page_header::get_number_of_keys(&key_page)+1,ith_branch);
        
        tmp_pagenum = ith_branch->get_pagenumber();
        file_read_page(table_id,tmp_pagenum,&tmp_page);
        free(ith_branch);
        page_header::set_parent_page_num(&tmp_page,key_pagenum);
        file_write_page(table_id,tmp_pagenum,&tmp_page);
               
        branching_factor_t* ith_branch1 = internal_page::get_nth_branching_factor(&neighbor_page,1);
        branching_factor_t* ith_branch2 = internal_page::get_nth_branching_factor(&parent_page,k_prime_index+1);
        ith_branch2->set_keys(ith_branch1->get_keys());
        internal_page::set_nth_branching_factor(&parent_page,k_prime_index+1,ith_branch2);
        file_write_page(table_id,parent_pagenum,&parent_page);

        for(i=0;i<=page_header::get_number_of_keys(&neighbor_page)-1;i++){
            branching_factor_t* ith_branch3 = internal_page::get_nth_branching_factor(&neighbor_page,i+1);
            if(i==0){
                internal_page::set_leftmost_key(&neighbor_page,ith_branch3->get_pagenumber());
                free(ith_branch3);
            }
            else{
                internal_page::set_nth_branching_factor(&neighbor_page,i,ith_branch3);
                free(ith_branch3);
            }
            file_write_page(table_id,parent_pagenum,&parent_page);
        }   
        free(ith_branch1);
        free(ith_branch2);
    }
    file_write_page(table_id,key_pagenum,&key_page);
    file_write_page(table_id,parent_pagenum,&parent_page); // 사실 안 들어가도 될듯?
    
    page_header::set_number_of_keys(&key_page,page_header::get_number_of_keys(&key_page)+1);
    page_header::set_number_of_keys(&neighbor_page,page_header::get_number_of_keys(&neighbor_page)-1);
    
    file_write_page(table_id,key_pagenum,&key_page);
    file_write_page(table_id,neighbor_pagenum,&neighbor_page);
    return key_pagenum;
}

//neighbor page가 사용한 공간을 return
pagenum_t neighbor_used_space(int64_t table_id, pagenum_t neighbor_pagenum){
    page_t neighbor_page;
    file_read_page(table_id,neighbor_pagenum,&neighbor_page);
    //pagenum_t used_space = 0;
    //for(int i=0;i<page_header::get_number_of_keys(&neighbor_page);i++){ //slot은 0번째부터 시작
    //    slot_t* ith_slot = leaf_page::get_nth_slot(&neighbor_page,i);
    //    used_space = used_space + SLOT_SIZE + ith_slot->get_size();
    //    free(ith_slot);
    //}
    //printf("used_spce: %d\n",used_space);
    //return used_space;
    return PAGE_SIZE - leaf_page_first_slot_start - leaf_page::get_amount_of_free_space(&neighbor_page);
     
}

// panent에서 neighbor_index의 pagenum을 출력
// -1이면 2번째 pagenum를 리턴(정확히는 leftmost의 다음꺼)
// parent이기에 branch에서만 생각
// branch에서는 1번때 pagenum 
// 나머지는 그냥 리턴
pagenum_t get_neighbor_pagenum( int64_t table_id, pagenum_t key_pagenum, longlong_t neighbor_index){
    page_t key_page, parent_page;
    file_read_page(table_id,key_pagenum,&key_page);
    pagenum_t parent_pagenum = page_header::get_parent_page_num(&key_page);
    file_read_page(table_id,parent_pagenum,&parent_page);
    pagenum_t neighbor_pagenum;
    
    if(neighbor_index == -1){ // key_pagenum이 parent page에서 0번째에 위치 따라서, 이것은 1번째 branch의 pagenum 출력
        branching_factor_t* ith_branch = internal_page::get_nth_branching_factor(&parent_page,1);
        neighbor_pagenum = ith_branch->get_pagenumber();
        free(ith_branch);
    }
    else if( neighbor_index == 0){ // key_pagenum이 1번째에 위치 따라서, 0번째 즉, leftmost 출력
        pagenum_t leftmost_pagenum = internal_page::get_leftmost_key(&parent_page);
        neighbor_pagenum = leftmost_pagenum;
    }
    else{ //평범한 경우 neighbor_index 번째
        branching_factor_t* ith_branch = internal_page::get_nth_branching_factor(&parent_page,neighbor_index);
        neighbor_pagenum = ith_branch->get_pagenumber();
        free(ith_branch);
    }
    return neighbor_pagenum;
}

// key_pagenum의 parent page에서의 key of k_prime_index(-1이면 0 / 나머지는 그대로)번째 key
// neighbor_pagenum & key_pagenum 사이에 있는 key값
int get_k_prime( int64_t table_id, pagenum_t key_pagenum, longlong_t k_prime_index){
    page_t key_page, parent_page;
    file_read_page(table_id,key_pagenum,&key_page);
    pagenum_t parent_pagenum = page_header::get_parent_page_num(&key_page);
    file_read_page(table_id,parent_pagenum,&parent_page);
    pagenum_t k_prime;
    branching_factor_t* ith_branch = internal_page::get_nth_branching_factor(&parent_page,k_prime_index+1);
    k_prime = ith_branch->get_keys();
    free(ith_branch);
    return k_prime;
}

// key_pagenum이 leftmost에 존재하면 -1 
// key_page가 0을 return하면 neighbor가 leftmost이다 
// key를 확인할때는 1번째
longlong_t get_neighbor_index(int64_t table_id, pagenum_t key_pagenum){
    page_t key_page, parent_page;
    file_read_page(table_id,key_pagenum,&key_page);
    pagenum_t parent_pagenum = page_header::get_parent_page_num(&key_page);
    file_read_page(table_id,parent_pagenum,&parent_page);
    pagenum_t leftmost_pagenum = internal_page::get_leftmost_key(&parent_page);
    int i;
    if(leftmost_pagenum = key_pagenum){
        return -1;
    }
    for(i=1;i <= page_header::get_number_of_keys(&parent_page);i++){ //branch_t 가 1번째에서 시작하기 때문
        branching_factor_t* ith_branch = internal_page::get_nth_branching_factor(&parent_page,i);
        if(ith_branch->get_pagenumber() == key_pagenum){
            free(ith_branch);
            printf("neighbor_index:%d\n",i-1);
            return i-1; // 0이면 leftmost
        }
        free(ith_branch);
    }
    printf("unexpected key to find\n");
    exit(EXIT_FAILURE);
}

// FREE : 2500일때 INTERNAL : key_num < 124 일때 체크
// 만약 merge나 coalesce가 필요하다면 return 1 else return 0 
int need_merge_or_coalesce(int64_t table_id, pagenum_t key_pagenum){
    page_t key_page,parent_page;
    file_read_page(table_id,key_pagenum,&key_page);
    pagenum_t parent_pagenum = page_header::get_parent_page_num(&key_page);
    file_read_page(table_id,parent_pagenum,&parent_page);
    printf("parent key 개수 : %d\n",page_header::get_number_of_keys(&parent_page));
    if(page_header::get_leaf_check(&key_page) == 1) { //leaf page이면
        //printf("amount free space: %d\n",leaf_page::get_amount_of_free_space(&key_page));
        if(leaf_page::get_amount_of_free_space(&key_page) < 2500 )//|| page_header::get_number_of_keys(&parent_page) == 1)
            return 0; // don't need
        else
            return 1; // need
        
    }
    else{ //internal page
        if(page_header::get_number_of_keys(&key_page) >= INTERNAL_ORDER/2 )
            return 0; // don't need
        else
            return 1; // need
    }
}


//merge 해야 하는 i번째 return 미완성
int cut_merge(int64_t table_id, pagenum_t key_pagenum){
    page_t key_page;
    file_read_page(table_id,key_pagenum,&key_page);
    int i;
    int A = page_header::get_number_of_keys(&key_page);
        int sum_size=0;
        for(i=0;i<A;i++){
            slot_t* ith_slot =leaf_page::get_nth_slot(&key_page,i);
            sum_size = sum_size + ith_slot->get_size() + SLOT_SIZE;
            free(ith_slot);
            if(sum_size >1983) break;
        }
        return i;
}


// delete key 포함된 slot & branch 
pagenum_t remove_entry_from_page(int64_t table_id, pagenum_t pagenum, int64_t key){
    int i;
    page_t page; //can be iternal or leaf
    printf("remove: %d / from : %d\n",key,pagenum);
    file_read_page(table_id,pagenum,&page);
    if(page_header::get_leaf_check(&page) == 1){ //leaf_page이면
        i=0;
        slot_t* key_slot = leaf_page::get_nth_slot(&page,i);
        while(key_slot->get_key() != key){
            free(key_slot);
            i++;
            key_slot = leaf_page::get_nth_slot(&page,i);
        } //key 값의 ith
        free(key_slot);
        
        slot_t* key_slot1 = leaf_page::get_nth_slot(&page,i);
        ushortint_t value_size = key_slot1->get_size();
        int offset = key_slot1->get_offset();
        leaf_page::set_amount_of_free_space(&page,leaf_page::get_amount_of_free_space(&page)+value_size + SLOT_SIZE);
        //leaf_page::get_value(&page,off)
        

        for(++i; i < page_header::get_number_of_keys(&page);i++){
            slot_t* ith_slot = leaf_page::get_nth_slot(&page,i);
            char* ith_value = leaf_page::get_value(&page,ith_slot->get_offset(),ith_slot->get_size());
            
            ith_slot->set_offset(ith_slot->get_offset()+value_size);
            leaf_page::set_value(&page,ith_slot->get_offset(),ith_value,ith_slot->get_size());

            leaf_page::set_nth_slot(&page,i-1,ith_slot);
            free(ith_value);
            free(ith_slot);
        }
        free(key_slot1);

        page_header::set_number_of_keys(&page,page_header::get_number_of_keys(&page)-1);
    }

    else{ //internal page 이면
        i=1;
        pagenum_t leftmost = internal_page::get_leftmost_key(&page);
        branching_factor_t* key_branch = internal_page::get_nth_branching_factor(&page,i);
        while(key_branch->get_keys() != key){
            free(key_branch);
            i++;
            key_branch = internal_page::get_nth_branching_factor(&page,i);
        } //key 값의 ith
        free(key_branch);
        int k=i;
        for(i=k+1; i <= page_header::get_number_of_keys(&page);i++){
            branching_factor_t* ith_branch = internal_page::get_nth_branching_factor(&page,i);
            internal_page::set_nth_branching_factor(&page,i-1,ith_branch);
            free(ith_branch);
        }
        //printf("delete_entry_leftmost: %d",internal_page::get_leftmost_key(&page));

        page_header::set_number_of_keys(&page,page_header::get_number_of_keys(&page)-1);
    }
    file_write_page(table_id,pagenum,&page);
    // set 나머지 NULL
    return pagenum;
}
*/
// Find records with a key between the range: begin_key ≤ key ≤ end_key
int db_scan(int64_t table_id, int64_t begin_key,int64_t end_key, std::vector<int64_t>* keys,std::vector<char*>* values,std::vector<uint16_t>* val_sizes){
    int i,num_found;
    num_found=0;
    page_t key_exist_leaf_page;
    pagenum_t key_exist_leaf_pagenum;
    slot_t *ith_slot;
    key_exist_leaf_pagenum = find_leaf(table_id,begin_key);
    file_read_page(table_id,key_exist_leaf_pagenum,&key_exist_leaf_page);
    if(key_exist_leaf_pagenum == 0) return -1; // root page가 비어있다
    ith_slot=leaf_page::get_nth_slot(&key_exist_leaf_page, i);
    for(i=0;i<page_header::get_number_of_keys(&key_exist_leaf_page) && ith_slot->get_key() < begin_key;i++){
        free(ith_slot);
        ith_slot=leaf_page::get_nth_slot(&key_exist_leaf_page, i);
    }
    free(ith_slot);
    if(i==page_header::get_number_of_keys(&key_exist_leaf_page)) return -1;
    while(leaf_page::get_right_sibling_page_number(&key_exist_leaf_page) != 0){
        ith_slot=leaf_page::get_nth_slot(&key_exist_leaf_page, i);
        for( ; i < page_header::get_number_of_keys(&key_exist_leaf_page) && ith_slot->get_key() <= end_key;i++){
            keys->push_back(ith_slot->get_key());
            ushortint_t val_size = ith_slot->get_size();
            val_sizes->push_back(val_size);
            char* ret_val;
            values->push_back(ret_val);
        }
        file_read_page(table_id,leaf_page::get_right_sibling_page_number(&key_exist_leaf_page),&key_exist_leaf_page);
        i = 0;
    }
    free(ith_slot);
    return 0;

}

// Initialize the database system.
int init_db(int num_buf){
    return 0;
    /*
    int i = buffer_init_db(num_buf);
    printf("buffer_init_db: %d\n",i);
    return i;
    */
    //destroy tree
}



// Shutdown the database system.
int shutdown_db(){
    //file_shut_down();
    file_close_table_file();
    opened_table_unique_id.clear();
    return 0;
}


//bpt 만들기
