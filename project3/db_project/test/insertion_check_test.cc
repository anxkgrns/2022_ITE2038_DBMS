#include "db.h"
#include "buffer.h"
#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <random>
#include <algorithm>


#define key_num 10000
int* key = (int*)malloc(sizeof(int)*key_num); 
char* values[key_num];
uint16_t* value_size = (uint16_t*)malloc(sizeof(uint16_t)*key_num); // value의 길이


char get_random_char(){

    return 'a'+(rand()%26);
}

char* get_random_value(uint16_t size,int any){

    char* random_str = (char*)malloc(sizeof(char)*size);
    srand(time(0)*any);
    for(int i=0;i<size;i++){
        random_str[i] = get_random_char();
    }
    return random_str;
}

//test0
TEST(db_insert_Check, buffertest) {

    init_db(20000);

    std::string pathname = "BF_test.db";
    remove(pathname.c_str());
    int64_t table_id = open_table(pathname.c_str());

    //int key_num = 100000;

    int* key = (int*)malloc(sizeof(int)*key_num);
    char* values[key_num];

    //setting key
    for(int i=0;i<key_num;i++){
        key[i]=i+1;
    }
    //std::random_device rd;
    //std::mt19937 gen(rd());
    //std::shuffle(key+0,key+key_num,gen);

    uint16_t* value_size = (uint16_t*)malloc(sizeof(uint16_t)*key_num); // value의 길이
    //setting random value size
    for(int i=0;i<key_num;i++){
        value_size[i] = rand()%51+50;
        printf("길이:%d",value_size[i]);
    }

    //Do insertion key_num times
    for(int i=0;i<key_num;i++){

        values[i] = get_random_value(value_size[i],key[i]);
        int check_inserted = db_insert(table_id,key[i],values[i],value_size[i]);
        EXPECT_EQ(check_inserted,0)
            << "failed inserting " << key[i] << " in " << i+1 << "th insertion \n";
    }
    
    for(int i=0;i<key_num;i++){
        char* find_value=(char*)malloc(sizeof(char)*200);
        uint16_t find_value_size;
        //check_fouund
        EXPECT_EQ(db_find(table_id,key[i],find_value,&find_value_size),0)
            << i+1 << "th find failed\n";
        //check_size
        EXPECT_EQ(value_size[i],find_value_size);
        std::string str_value(values[i],value_size[i]);
        std::string found_value(find_value,find_value_size);
        //check string
        EXPECT_EQ(str_value,found_value);
        free(find_value);
        free(values[i]);
    }
    free(key);

    //shutdown_db();
    int is_removed = remove(pathname.c_str());
}

void* original(void* arg){
    int table_id = *(int*)arg;

    for(int i=0;i<key_num;i++){
        printf("slock!!!\n");
        char* find_value=(char*)malloc(sizeof(char)*200);
        uint16_t find_value_size;
        //check_fouund
        EXPECT_EQ(db_find(table_id,key[i],find_value,&find_value_size),0)
            << i+1 << "th find failed\n";
        //check_size
        EXPECT_EQ(value_size[i],find_value_size);
        std::string str_value(values[i],value_size[i]);
        std::string found_value(find_value,find_value_size);
        //check string
        EXPECT_EQ(str_value,found_value);
        free(find_value);
        free(values[i]);
    }
    return nullptr;
}

//test0
TEST(db_original_Check, single_trx) {

    int db_exist1 = init_db(100);
    EXPECT_EQ(0,db_exist1);

    std::string pathname = "BF_test.db";
    remove(pathname.c_str());
    int64_t table_id = open_table(pathname.c_str());

    // int key_num = 10000;

    // int* key = (int*)malloc(sizeof(int)*key_num);
    // char* values[key_num];

    //setting key
    for(int i=0;i<key_num;i++){
        key[i]=i+1;
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(key+0,key+key_num,gen);

    //uint16_t* value_size = (uint16_t*)malloc(sizeof(uint16_t)*key_num); // value의 길이

    //setting random value size
    for(int i=0;i<key_num;i++){
        value_size[i] = rand()%51+50;
        //printf("길이:%d",value_size[i]);
    }

    //Do insertion key_num times
    for(int i=0;i<key_num;i++){

        values[i] = get_random_value(value_size[i],key[i]);
        int check_inserted = db_insert(table_id,key[i],values[i],value_size[i]);
        EXPECT_EQ(check_inserted,0)
            << "failed inserting " << key[i] << " in " << i+1 << "th insertion \n";
    }
    printf("!---------------\n");
    //print_buffer();
    buffer_shut_down();
    int db_exist3 = init_db(2000);
    EXPECT_EQ(0,db_exist3);
    printf("single_thread\n");
    int pthread_num = 3;
    pthread_t pthread[pthread_num];//pthread_num];
    for(int i=0;i<pthread_num;i++){
        pthread_create(&pthread[i],NULL,original,(void*)table_id);
    }
    for(int i=0;i<pthread_num;i++){
        pthread_join(pthread[i],NULL);
    }


    shutdown_db();
    //EXPECT_EQ(1,2);
    int is_removed = remove(pathname.c_str());
}


/*
//test1
TEST(db_insert_Check, RandomValueAndRandomKey) {

    init_db(10000);

    std::string pathname = "RVARK_test.db";
    remove(pathname.c_str());
    int64_t table_id = open_table(pathname.c_str());

    int key_num = 100000;
    uint16_t value_size[i] = 100; // value의 길이

    int* key = (int*)malloc(sizeof(int)*key_num);
    char* values[key_num];

    //setting key
    for(int i=0;i<key_num;i++){
        key[i]=i+1;
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(key+0,key+key_num,gen);
    
    //Do insertion key_num times
    for(int i=0;i<key_num;i++){
        values[i] = get_random_value(value_size[i],key[i]);
        int check_inserted = db_insert(table_id,key[i],values[i],value_size[i]);
        EXPECT_EQ(check_inserted,0)
            << "failed inserting " << key[i] << " in " << i+1 << "th insertion \n";
    }

    //Do find key_num times
    for(int i=0;i<key_num;i++){
        char* find_value=(char*)malloc(sizeof(char)*200);
        uint16_t find_value_size;
        //check_fouund
        EXPECT_EQ(db_find(table_id,key[i],find_value,&find_value_size),0)
            << i+1 << "th find failed\n";
        //check_size
        EXPECT_EQ(value_size[i],find_value_size);
        std::string str_value(values[i],value_size[i]);
        std::string found_value(find_value,find_value_size);
        //check string
        EXPECT_EQ(str_value,found_value);
        free(find_value);
        free(values[i]);
    }
    free(key);

    shutdown_db();
    int is_removed = remove(pathname.c_str());
}

//test2
TEST(db_insert_Check, RandomValueAndIncreaseKey) {

    init_db(10000);
    std::string pathname = "RVAIK_test.db";
    remove(pathname.c_str());
    int64_t table_id = open_table(pathname.c_str());
    
    int key_num = 100000;
    uint16_t value_size[i] = 100; // value의 길이

    int* key = (int*)malloc(sizeof(int)*key_num);
    char* values[key_num];

    //setting key
    for(int i=0;i<key_num;i++){
        key[i]=i+1;
    }

    //Do insertion key_num times
    for(int i=0;i<key_num;i++){
        values[i] = get_random_value(value_size[i],key[i]);
        int check_inserted = db_insert(table_id,key[i],values[i],value_size[i]);
        EXPECT_EQ(check_inserted,0)
            << "failed inserting " << key[i] << " in " << i+1 << "th insertion \n";
    }
    
    //Do find key_num times
    for(int i=0;i<key_num;i++){
        char* find_value=(char*)malloc(sizeof(char)*200);
        uint16_t find_value_size;
        //check_fouund
        EXPECT_EQ(db_find(table_id,key[i],find_value,&find_value_size),0)
            << i+1 << "th find failed\n";
        //check_size
        EXPECT_EQ(value_size[i],find_value_size);
        std::string str_value(values[i],value_size[i]);
        std::string found_value(find_value,find_value_size);
        //check string
        EXPECT_EQ(str_value,found_value);
        free(find_value);
        free(values[i]);
    }
    free(key);

    shutdown_db();
    int is_removed = remove(pathname.c_str());
}

//test3
TEST(db_insert_Check,SameKeyInsertedFail){
    
    init_db(10000);
    std::string pathname = "SKIF_test.db";
    remove(pathname.c_str());
    int64_t table_id = open_table(pathname.c_str());    
    int64_t key1 = 100;
    uint16_t value_size1 =100;
    EXPECT_EQ(db_insert(table_id,key1,get_random_value(value_size1,key1),value_size1),0)
            << "failed inserting " << key1 << " in 1th insertion \n";

    int64_t key2 = 100;
    uint16_t value_size2 =100;
    EXPECT_NE(db_insert(table_id,key2,get_random_value(value_size2,key2),value_size2),0) //must pass / if fail-> same key is inserted
            << "successed inserting " << key2 << " in 2th insertion \n";

    shutdown_db();

    int is_removed = remove(pathname.c_str());
}

//test4
TEST(db_insert_Check,OutOfSizeFail){
    
    init_db(10000);
    std::string pathname = "OOSF_test.db";
    remove(pathname.c_str());
    int64_t table_id = open_table(pathname.c_str());

    // need to fail insert
    int64_t key1 = 100;
    uint16_t value_size1 =49;
    EXPECT_NE(db_insert(table_id,key1,get_random_value(value_size1,key1),value_size1),0)
            << "success inserting " << value_size1 << " in 1th insertion \n";
    // success insert
    int64_t key2 = 101;
    uint16_t value_size2 =50;    
    EXPECT_EQ(db_insert(table_id,key2,get_random_value(value_size2,key2),value_size2),0) //must pass / if fail-> same key is inserted
            << "failed inserting " << value_size2 << " in 2th insertion \n";
      
    // need to fail insert
    int64_t key3 = 102;
    uint16_t value_size3 =113;
    
    EXPECT_NE(db_insert(table_id,key3,get_random_value(value_size3,key3),value_size3),0) //must pass / if fail-> same key is inserted
            << "successed inserting " << value_size3 << " in 3th insertion \n";
            
    // success insert
    int64_t key4 = 103;
    uint16_t value_size4 =112;
    EXPECT_EQ(db_insert(table_id,key1,get_random_value(value_size4,key4),value_size4),0)
            << "failed inserting " << value_size4 << " in 4th insertion \n";
            
    shutdown_db();
    int is_removed = remove(pathname.c_str());
} 

/*
//test5
TEST(db_delete_Check, RandomValueDelete) {

    std::string pathname = "RVD_test.db";
    remove(pathname.c_str());

    int64_t table_id = open_table(pathname.c_str());

    int key_num = 100000;
    uint16_t value_size[i] = 100; // value의 길이
    int delete_key = 100000;

    int* key = (int*)malloc(sizeof(int)*key_num);
    char* values[key_num];

    //setting key
    for(int i=0;i<key_num;i++){
        key[i]=key_num - i;//+1;
    }
    
    //std::random_device rd;
    //std::mt19937 gen(rd());
    //std::shuffle(key+0,key+key_num,gen);

    //Do insertion key_num times
    for(int i=0;i<key_num;i++){
        values[i] = get_random_value(value_size[i],key[i]);
        printf("%dth insertion\n",i+1);
        int check_inserted = db_insert(table_id,key[i],values[i],value_size[i]);
        EXPECT_EQ(check_inserted,0)
            << "failed inserting " << key[i] << " in " << i+1 << "th insertion \n";
    }
    
    //delete key[0]~key[delete_key]
    for(int i=0;i<delete_key;i++){
        printf("%dth deletion\n",i+1);
        int check_delete = db_delete(table_id,key[i]);
        EXPECT_EQ(check_delete,0)
            << " delete " << key[i] << " fail \n";
    }
    
    for(int i=0;i<delete_key;i++){
        char* find_value=(char*)malloc(sizeof(char)*200);
        uint16_t find_value_size;
        EXPECT_NE(db_find(table_id,key[i],find_value,&find_value_size),0) // 0이랑 다르다는 것은 delete 실패햇다는 것
            << i+1 << "th delete / deleting " << key[i] << " fail \n";
        free(find_value);
        free(values[i]);
    }

    //Do find key_num times
    for(int i=delete_key;i<key_num;i++){
        char* find_value=(char*)malloc(sizeof(char)*200);
        uint16_t find_value_size;
        //check_fouund
        EXPECT_EQ(db_find(table_id,key[i],find_value,&find_value_size),0)
            << i+1 << "th find failed\n";
        //check_size
        EXPECT_EQ(value_size[i],find_value_size);
        std::string str_value(values[i],value_size[i]);
        std::string found_value(find_value,find_value_size);
        //check string
        EXPECT_EQ(str_value,found_value);
        free(find_value);
        free(values[i]);
    }
    free(key);

    shutdown_db();
    int is_removed = remove(pathname.c_str());
}
*/
/*
//test6
TEST(db_delete_Check, RandomValueDeleteAll) {

    std::string pathname = "RVDA_test.db";
    remove(pathname.c_str());

    int64_t table_id = open_table(pathname.c_str());

    int key_num = 100000;
    uint16_t value_size[i] = 100; // value의 길이

    int* key = (int*)malloc(sizeof(int)*key_num);
    char* values[key_num];

    //setting key
    for(int i=0;i<key_num;i++){
        key[i]=i+1;
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(key+0,key+key_num,gen);

    //Do insertion key_num times
    for(int i=0;i<key_num;i++){
        values[i] = get_random_value(value_size[i],key[i]);
        int check_inserted = db_insert(table_id,key[i],values[i],value_size[i]);
        EXPECT_EQ(check_inserted,0)
            << "failed inserting " << key[i] << " in " << i+1 << "th insertion \n";
    }
    for(int i=0;i<key_num;i++){
        printf("%dth deletion\n",i+1);
        int check_delete = db_delete(table_id,key[i]);
        EXPECT_EQ(check_delete,0)
            << i+1 << "th delete / deleting " << key[i] << " fail \n";
    }
    
    for(int i=0;i<key_num;i++){
        char* find_value=(char*)malloc(sizeof(char)*200);
        uint16_t find_value_size;
        int check_delete = db_delete(table_id,key[i]);
        EXPECT_NE(db_find(table_id,key[i],find_value,&find_value_size),0)
            << i+1 << "th delete / deleting " << key[i] << " fail \n";
        free(find_value);
        free(values[i]);
    }

    free(key);

    shutdown_db();
    int is_removed = remove(pathname.c_str());
}
*/
