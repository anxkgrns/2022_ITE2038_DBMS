#ifndef __LOG_H__
#define __LOG_H__

#define RECOVER 0
#define REDO_CRASH 1
#define UNDO_CRASH 2

#define Log_Size 0
#define LSN 4
#define Prev_LSN 12
#define Transaction_ID 20
#define Type 24

#define Table_ID 28
#define Page Number 36
#define Offset 44
#define Data_Length 46 // N 을 의미
#define Old_Image 48
#define New_Image 48 // +N
#define Next_Undo_LSN 48+8 //+2N

struct log_record{
    char* data;
};

struct log_record log_record;


#endif /* __LOG_H__ */