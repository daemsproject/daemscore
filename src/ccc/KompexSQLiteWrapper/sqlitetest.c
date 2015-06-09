#include <stdio.h>  
#include <stdlib.h>  
#include <sqlite3.h>  //头文件的引用  
int  main (int argc, char ** argv)  
{  
        int result = 0;  
        sqlite3* db = NULL;  
                  
        result = sqlite3_open("lester.db", &db);  
                  
        printf("Hello World\n, %d", result);  
        return 0;  
}  

