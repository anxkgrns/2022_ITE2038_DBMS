#include "db.h"
#include "buffer.h"
#include "trx.h"
#include "lock_table_5.h"
#include <pthread.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <random>
#include <algorithm>

//int key_num = 10000;
#define key_num 20000
#define buf_size 640
#define trx_num 1000 //key_num/10

int* key = (int*)malloc(sizeof(int)*key_num); 
char* values[key_num]; 
uint16_t* value_size = (uint16_t*)malloc(sizeof(uint16_t)*key_num); // value의 길이
char* values_trx[key_num];

char get_random_char(){

    return 'a'+(rand()%26);
}

char* get_random_value(uint16_t size,int any){ //random record

    char* random_str = (char*)malloc(sizeof(char)*size);
    srand(time(0)*any);
    for(int i=0;i<size;i++){
        random_str[i] = get_random_char();
    }
    return random_str;
}

//test0

TEST(db_insert_Check, buffertest) {

    int db_exist1 = init_db(buf_size);
    EXPECT_EQ(0,db_exist1);

    std::string pathname = "BF_test.db";
    remove(pathname.c_str());
    int64_t table_id = open_table(pathname.c_str());

    //int key_num1 = 100000;

    int* key = (int*)malloc(sizeof(int)*key_num);
    char* values[key_num];

    //setting key
    for(int i=0;i<key_num;i++){
        key[i]=i+1;
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(key+0,key+key_num,gen);

    uint16_t* value_size = (uint16_t*)malloc(sizeof(uint16_t)*key_num); // value의 길이
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
    print_buffer();
    buffer_shut_down();
    int db_exist2 = init_db(buf_size);
    EXPECT_EQ(0,db_exist1);
    
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

TEST(db_find_Check, singleThread) {

    int db_exist1 = init_db(buf_size);
    EXPECT_EQ(0,db_exist1);

    std::string pathname = "BF_test.db";
    remove(pathname.c_str());
    int64_t table_id = open_table(pathname.c_str());

    int trx_id = trx_begin();

    //int key_num1 = 100000;

    int* key = (int*)malloc(sizeof(int)*key_num);
    char* values[key_num];

    //setting key
    for(int i=0;i<key_num;i++){
        key[i]=i+1;
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(key+0,key+key_num,gen);

    uint16_t* value_size = (uint16_t*)malloc(sizeof(uint16_t)*key_num); // value의 길이
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
    print_buffer();
    buffer_shut_down();

    int db_exist2 = init_db(buf_size);
    EXPECT_EQ(0,db_exist2);

    for(int i=0;i<key_num;i++){
        char* find_value=(char*)malloc(sizeof(char)*200);
        uint16_t find_value_size;
        //check_fouund
        EXPECT_EQ(db_find(table_id,key[i],find_value,&find_value_size,trx_id),0)
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
    trx_commit(trx_id);
    free(key);

    shutdown_db();
    int is_removed = remove(pathname.c_str());
}


void* original(void* arg){
    int table_id = *((int*)arg);

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

//test4
TEST(db_original_Check, single_trx) {

    int db_exist1 = init_db(buf_size);
    EXPECT_EQ(0,db_exist1);

    std::string pathname = "BF_test.db";
    remove(pathname.c_str());
    int *table_id = (int*)malloc(sizeof(int));
    *table_id = open_table(pathname.c_str());

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
        int check_inserted = db_insert(*table_id,key[i],values[i],value_size[i]);
        EXPECT_EQ(check_inserted,0)
            << "failed inserting " << key[i] << " in " << i+1 << "th insertion \n";
    }
    printf("!---------------\n");
    print_buffer();
    buffer_shut_down();

    int db_exist2 = init_db(buf_size);
    EXPECT_EQ(0,db_exist2);
    printf("single_thread\n");
    int pthread_num = 1;
    pthread_t pthread[pthread_num];//pthread_num];
    for(int i=0;i<pthread_num;i++){
        pthread_create(&pthread[i],NULL,original,table_id);
    }
    for(int i=0;i<pthread_num;i++){
        pthread_join(pthread[i],NULL);
    }


    shutdown_db();
    //EXPECT_EQ(1,2);
    int is_removed = remove(pathname.c_str());
}


void* slock(void* arg){
    int trx_id = trx_begin();
    EXPECT_NE(trx_id,0);
    printf("slock!!!\n");
    printf("%dth thread\n",trx_id);
    int table_id = *((int*)arg);
    for(int i=0;i<key_num;i++){
        printf("\n%dth thread %d slock_t\n\n",trx_id,i+1);
        char* find_value=(char*)malloc(sizeof(char)*200);
        uint16_t find_value_size;
        //check_fouund
        EXPECT_EQ(db_find(table_id,key[i],find_value,&find_value_size,trx_id),0)
            << i+1 << "th find failed\n";
        //check_size
        EXPECT_EQ(value_size[i],find_value_size);
        std::string str_value(values[i],value_size[i]);
        std::string found_value(find_value,find_value_size);
        //check string
        EXPECT_EQ(str_value,found_value);
        free(find_value);
        //free(values[i]);
    }
    trx_commit(trx_id);
    return nullptr;
}
//test0
TEST(db_trx_find_Check, single_trx_slock_only) {

    int db_exist1 = init_db(buf_size);
    EXPECT_EQ(0,db_exist1);

    std::string pathname = "BF_test.db";
    remove(pathname.c_str());
    int *table_id = (int*)malloc(sizeof(int));
    *table_id = open_table(pathname.c_str());
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
        int check_inserted = db_insert(*table_id,key[i],values[i],value_size[i]);
        EXPECT_EQ(check_inserted,0)
            << "failed inserting " << key[i] << " in " << i+1 << "th insertion \n";
    }
    printf("!---------------\n");
    //print_buffer();
    buffer_shut_down();
    int db_exist2 = init_db(buf_size);
    EXPECT_EQ(0,db_exist2);
    printf("single_thread\n");
    int pthread_num = 10;
    pthread_t pthread[pthread_num];//pthread_num];
    for(int i=0;i<pthread_num;i++){
        pthread_create(&pthread[i],NULL,slock,table_id);
    }
    for(int i=0;i<pthread_num;i++){
        pthread_join(pthread[i],NULL);
    }
    // for(int i=0;i<key_num;i++){
    //    free(values[i]);
    //}


    shutdown_db();
    //EXPECT_EQ(1,2);
    int is_removed = remove(pathname.c_str());
}

void* xlock(void* arg){
    int trx_id = trx_begin();
    EXPECT_NE(trx_id,0);
    printf("xlock!!!\n");
    printf("%dth thread\n",trx_id);
    int table_id = *((int*)arg);
    //for(int i=(trx_id-1)*trx_num;i<trx_id*trx_num;i++){
    for(int i=0;i<key_num;i++){  

        printf("\n%dth thread %d xlock_t\n\n",trx_id,i);
        char* find_value=(char*)malloc(sizeof(char)*200);
        find_value = get_random_value(value_size[i],key[i]+10);
        uint16_t* old_value_size = (uint16_t*)malloc(sizeof(uint16_t)*100);
        //check_fouund
        EXPECT_EQ(db_update(table_id,key[i],values_trx[i],value_size[i],old_value_size,trx_id),0)
            << i+1 << "th update failed\n";
        //check_size
        //EXPECT_EQ(value_size[i],find_value_size);
        //std::string str_value(values[i],value_size[i]);
        //std::string found_value(find_value,find_value_size);
        //check string
        //EXPECT_EQ(str_value,found_value);
        free(old_value_size);
        free(find_value);
        //free(find_value);
        //free(values[i]);
    }
    trx_commit(trx_id);
    return nullptr;
}
//test0
TEST(db_trx_find_Check, single_trx_xlock_only) {

    int db_exist1 = init_db(buf_size);
    EXPECT_EQ(0,db_exist1);

    std::string pathname = "BF_test.db";
    remove(pathname.c_str());
    int *table_id = (int*)malloc(sizeof(int));
    *table_id = open_table(pathname.c_str());
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
        values_trx[i] = get_random_value(value_size[i],key[i]+10);
        int check_inserted = db_insert(*table_id,key[i],values[i],value_size[i]);
        EXPECT_EQ(check_inserted,0)
            << "failed inserting " << key[i] << " in " << i+1 << "th insertion \n";
    }
    printf("!---------------\n");
    //print_buffer();
    buffer_shut_down();
    int db_exist2 = init_db(buf_size);
    EXPECT_EQ(0,db_exist2);
    printf("single_thread\n");
    int pthread_num = 10;
    pthread_t pthread[pthread_num];//pthread_num];
    for(int i=0;i<pthread_num;i++){
        pthread_create(&pthread[i],NULL,xlock,table_id);
    }
    for(int i=0;i<pthread_num;i++){
        pthread_join(pthread[i],NULL);
    }

    //int trx_id1 = trx_begin();
    //EXPECT_NE(trx_id1,0);
    printf("slock!!!\n");
    //printf("%dth thread\n",trx_id1);
    for(int i=0;i<key_num;i++){
        //printf("\n%dth thread %d slock_t\n\n",trx_id1,i+1);
        char* find_value=(char*)malloc(sizeof(char)*200);
        uint16_t find_value_size;
        //check_fouund
        EXPECT_EQ(db_find(*table_id,key[i],find_value,&find_value_size),0)
            << i+1 << "th find failed\n";
        //check_size
        EXPECT_EQ(value_size[i],find_value_size);
        std::string str_value_trx(values_trx[i],value_size[i]);
        std::string found_value(find_value,find_value_size);
        //check string
        EXPECT_EQ(str_value_trx,found_value)
        <<i<<"th find fail\n";
        free(find_value);
        //free(values[i]);
    }
    //trx_commit(trx_id1);

    shutdown_db();
    //EXPECT_EQ(1,2);
    int is_removed = remove(pathname.c_str());
}

