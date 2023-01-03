
#include "file.h"
//#include "page.h"


std::vector<int> opened_table_id;

void put_data(void*dest, pagenum_t source){
    std::memcpy(dest,&source,SIZE_TYPE);
}

void get_data(pagenum_t *num, const void* source){
    std::memcpy(num,source,SIZE_TYPE);
}

// Open existing database file or create one if it doesn't exist
int64_t file_open_table_file(const char* pathname) {
    int64_t table_id = open(pathname, O_RDWR /*| O_SYNC ,0644*/);
    if( table_id <= 0){
        table_id =  open(pathname, O_RDWR | O_CREAT /*| O_SYNC,0644*/);
        page_t header_page; 
        put_data(&header_page.data[header_magic_number], 2022);
        put_data(&header_page.data[header_next_free_page_number], FREE_PAGE_NUMBER);
        put_data(&header_page.data[header_number_of_free_page], FREE_PAGE_NUMBER + 1);
        put_data(&header_page.data[header_root_page_number],0);
        file_write_page(table_id,0,&header_page);
        for (pagenum_t i=FREE_PAGE_NUMBER; i>0 ; i--){
            page_t free_page; 
            put_data(&free_page.data[free_page_next_free_page_number], i-1);
            file_write_page(table_id,i,&free_page);
        }
    }
    //returns a negative value for error handling
    // if( table_id < 0 )
    //     perror("table open failed");
    pagenum_t magic_number;
    page_t header_page2;
    
    file_read_page(table_id,0,&header_page2);
    get_data(&magic_number,&header_page2.data[header_magic_number]);
    if(magic_number !=2022){
        table_id=-1;
    }
    opened_table_id.push_back(table_id);
    return table_id; 
}

// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page(int64_t table_id) { 
    page_t header_page;
    file_read_page(table_id,0,&header_page);
    pagenum_t header_next_free_page; //header의 다음 프리 페이지
    get_data(&header_next_free_page,&header_page.data[header_next_free_page_number]);
    // free page가 없다면
    if(header_next_free_page==0){
        pagenum_t data_header_number_of_free_page;
        get_data(&data_header_number_of_free_page,&header_page.data[header_number_of_free_page]);
        for (pagenum_t i=data_header_number_of_free_page*2;i>data_header_number_of_free_page;i--){
            page_t free_page; 
            if(i==data_header_number_of_free_page+1){
                put_data(&free_page.data[free_page_next_free_page_number], 0 );
            }
            else{
                put_data(&free_page.data[free_page_next_free_page_number], i -2);
            }
            file_write_page(table_id,i-1,&free_page);
        }
        put_data(&header_page.data[header_number_of_free_page], data_header_number_of_free_page*2);
        put_data(&header_page.data[header_next_free_page_number], data_header_number_of_free_page*2-1);
        file_write_page(table_id,0,&header_page);
    }
    file_read_page(table_id,0,&header_page);
    get_data(&header_next_free_page,&header_page.data[header_next_free_page_number]);
    page_t free_page;
    
    file_read_page(table_id,header_next_free_page,&free_page);
    //pread(table_id,&free_page,PAGE_SIZE,header_next_free_page*PAGE_SIZE);
    pagenum_t free_next_free_page; //free page의 next page
    get_data(&free_next_free_page,&free_page.data[free_page_next_free_page_number]);
    //header page의 다음 페이지를 free page의 다음 페이지로 바꿈
    put_data(&header_page.data[header_next_free_page_number], free_next_free_page); 
    file_write_page(table_id,0,&header_page);
    //printf("%d\n",header_next_free_page);
    //put_data(&free_page.data[free_page_next_free_page_number],header_next_free_page);
    return header_next_free_page;
    // header_page.data[header_next_free_page_number];
}

// Free an on-disk page to the free page list
void file_free_page(int64_t table_id, pagenum_t pagenum) {
    page_t header_page;
    file_read_page(table_id,0,&header_page);
    
    pagenum_t header_next_free_page;
    get_data(&header_next_free_page,&header_page.data[header_next_free_page_number]);

    put_data(&header_page.data[header_next_free_page_number], pagenum);
    page_t free_page;
    file_read_page(table_id,pagenum,&free_page);
    free_page.data[0] ={0,};
    put_data(&free_page.data[free_page_next_free_page_number],  header_next_free_page);
    file_write_page(table_id,pagenum,&free_page);
    file_write_page(table_id,0,&header_page);

}

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(int64_t table_id, pagenum_t pagenum, struct page_t* dest) {
    pread(table_id,dest,PAGE_SIZE,pagenum*PAGE_SIZE);
}

// Write an in-memory page(src) to the on-disk page
void file_write_page(int64_t table_id, pagenum_t pagenum, const struct page_t* src) {
    pwrite(table_id,src,PAGE_SIZE,pagenum*PAGE_SIZE);
    //sync();
}

// Close the database file
void file_close_table_file() {
    while(opened_table_id.empty())
    {
        close(opened_table_id.back());
        opened_table_id.pop_back();
    }

}