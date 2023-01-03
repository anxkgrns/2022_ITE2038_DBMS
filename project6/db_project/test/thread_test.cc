#include "db.h"
#include "buffer.h"
#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <random>
#include <algorithm>

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
TEST(thread, single_thread) {

    int db_exist1 = init_db(100);
    EXPECT_EQ(0,db_exist1);

    std::string pathname = "BF_test.db";
    remove(pathname.c_str());
    int64_t table_id = open_table(pathname.c_str());

    int key_num = 10000;

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
    //print_buffer();
    buffer_shut_down();

    int db_exist2 = init_db(100);
    EXPECT_EQ(0,db_exist2);

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
    print_buffer();
    free(key);
    shutdown_db();
    //EXPECT_EQ(1,2);
    int is_removed = remove(pathname.c_str());
}