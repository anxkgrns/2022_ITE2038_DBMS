#include "file.h"
#include "page.h"

#include <gtest/gtest.h>

#include <string>



/*******************************************************************************
 * The test structures stated here were written to give you and idea of what a
 * test should contain and look like. Feel free to change the code and add new
 * tests of your own. The more concrete your tests are, the easier it'd be to
 * detect bugs in the future projects.
 ******************************************************************************/

/*
 * Tests file open/close APIs.
 * 1. Open a file and check the descriptor
 * 2. Check if the file's initial size is 10 MiB
 */
TEST(FileInitTest, HandlesInitialization) {
  int fd;                                 // file descriptor
  std::string pathname = "init_test.db";  // customize it to your test file


  // Open a table file
  fd = file_open_table_file(pathname.c_str());
  // Check file size that is created
  int file_size = lseek(fd,(off_t)0,SEEK_END);
  EXPECT_EQ(file_size,INITIAL_DB_FILE_SIZE)
      << "The initial number of file size does not match the requirement:"
      << file_size;

  //Check the size of the initial file
  page_t header_page;
  file_read_page(fd,0,&header_page);
  pagenum_t header_number_of_page;
  memcpy(&header_number_of_page,&header_page.data[header_number_of_free_page],SIZE_TYPE);
  int num_pages = header_number_of_page; /* fetch the number of pages from the header page 2560;*/
  EXPECT_EQ(header_number_of_page, INITIAL_DB_FILE_SIZE / PAGE_SIZE)
      << "The initial number of pages does not match the requirement: "
      << num_pages;


  // Check if the file is opened
  ASSERT_TRUE(fd >= 0);  // change the condition to your design's behavior



  // Close all table files
  file_close_table_file();

  // Remove the db file
  int is_removed = remove(pathname.c_str());

  ASSERT_EQ(is_removed, 0 /*for success 0 //-1*/);
}

/*
 * TestFixture for page allocation/deallocation tests
 */
class FileTest : public ::testing::Test {
 protected:
  /*
   * NOTE: You can also use constructor/destructor instead of SetUp() and
   * TearDown(). The official document says that the former is actually
   * perferred due to some reasons. Checkout the document for the difference
   */
  FileTest() { fd = file_open_table_file(pathname.c_str()); }

  ~FileTest() {
    if (fd >= 0) {
      file_close_table_file();
    }
  }

  int fd;                // file descriptor
  std::string pathname;  // path for the file
};

/*
 * Tests page allocation and free
 * 1. Allocate 2 pages and free one of them, traverse the free page list
 *    and check the existence/absence of the freed/allocated page
 */
TEST_F(FileTest, HandlesPageAllocation) {
  int fd;                                 // file descriptor
  std::string pathname = "init_test2.db";
  fd = file_open_table_file(pathname.c_str());
  // Allocate the pages
  pagenum_t allocated_page, freed_page;
  allocated_page = file_alloc_page(fd);
  freed_page = file_alloc_page(fd);
  // Free one page
  file_free_page(fd, freed_page);
  // Traverse the free page list and check the existence of the freed/allocated
  // pages. You might need to open a few APIs soley for testing.
  page_t header_page, free_page1;
  file_read_page(fd,0,&header_page);
  pagenum_t header_number_of_page, next_free_page;
  memcpy(&header_number_of_page,&header_page.data[header_number_of_free_page],SIZE_TYPE);

  std::vector<pagenum_t> free_page_list;
  memcpy(&next_free_page,&header_page.data[header_next_free_page_number],SIZE_TYPE);
  while(next_free_page !=0){
    free_page_list.push_back(next_free_page);
    
    file_read_page(fd,next_free_page,&free_page1);
    memcpy(&next_free_page,&free_page1.data[free_page_next_free_page_number],SIZE_TYPE);
  } 
  /* 
  vector내에 freed_page이 존재함
  */
  EXPECT_NE(find(free_page_list.begin(), free_page_list.end(), freed_page) , free_page_list.end())
    <<"error: "
    <<freed_page;
  // vector내에 allocated_page가 존재하지 않음
  EXPECT_EQ(find(free_page_list.begin(), free_page_list.end(), allocated_page) ,free_page_list.end())
    <<"error: "
    <<allocated_page << find(free_page_list.begin(), free_page_list.end(), allocated_page) - free_page_list.begin()
    << free_page_list[1];

  // fredd_page가 그 위치에 존재
  // int link =find(free_page_list.begin(), free_page_list.end(), freed_page) - free_page_list.begin(); 
  // EXPECT_EQ(free_page_list[link],freed_page);
  file_close_table_file();

  int is_removed = remove(pathname.c_str());

  ASSERT_EQ(is_removed, 0 /*for success 0 //-1*/);
}

/*
 * Tests page read/write operations
 * 1. Write/Read a page with some random content and check if the data matches
 */
TEST_F(FileTest, CheckReadWriteOperation) {
  int fd;                                 // file descriptor
  std::string pathname = "init_test3.db";
  fd = file_open_table_file(pathname.c_str());
  page_t src;

  pagenum_t pagenum =file_alloc_page(fd);
  for(int i =0 ; i < PAGE_SIZE/sizeof('a'); i++){
    src.data[i]='a';
  }
  file_write_page(fd, pagenum, &src);
  page_t dest;
  file_read_page(fd, pagenum, &dest);
  for(int i =0 ; i < PAGE_SIZE/sizeof('a'); i++){
    EXPECT_EQ(src.data[i], dest.data[i])
      <<"error: IN " << i << " index "
      << src.data[i] << " != " << dest.data[i];
  }
  file_close_table_file();

  int is_removed = remove(pathname.c_str());

  ASSERT_EQ(is_removed, 0);
}

/*
  Check if file is successly doubled
*/

TEST_F(FileTest, CheckDoubledFileSize) {
  int fd;                                 // file descriptor
  std::string pathname = "init_test4.db";
  fd = file_open_table_file(pathname.c_str());
  page_t header_page1;
  file_read_page(fd,0,&header_page1);  
  pagenum_t page_number;
  memcpy(&page_number,&header_page1.data[header_number_of_free_page],SIZE_TYPE);
  for(int i =1 ; i <= page_number; i++){
    file_alloc_page(fd);
  }
  page_t header_page2;
  file_read_page(fd,0,&header_page2);
  pagenum_t header_page_size2;
  memcpy(&header_page_size2,&header_page2.data[header_number_of_free_page],SIZE_TYPE);

  EXPECT_EQ(page_number*2, lseek(fd,(off_t)0,SEEK_END)/PAGE_SIZE)//header_page_size2)
    <<"error: " 
    << page_number*2 << " != " << lseek(fd,(off_t)0,SEEK_END)/PAGE_SIZE;

  file_close_table_file();

  int is_removed = remove(pathname.c_str());

  ASSERT_EQ(is_removed, 0);
}