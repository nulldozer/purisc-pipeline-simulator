#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ANSI_COLOR_BLACK   "\x1b[30m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_BG_BLUE    "\x1b[44m"
#define ANSI_BG_WHITE    "\x1b[47m"
#define ANSI_COLOR_NEGATIVE   "\x1b[7m"

#define VERBOSE 1
#define MAX_MEMORY_ADDRESS 0x00011FFF
#define dontcare "x              "

#define NOCOLOR 0

typedef struct {
        int r_addr_a;
        int r_addr_b; 
        int r_addr_c;
        int r_data_a;
        int r_data_b;
        int r_data_c;
} stage_ri_t; 
void ri_shiftin(stage_ri_t * stage_ri, int pc, int pcp1, int pcp2, int a, int b, int c) {
        (*stage_ri).r_addr_a = pc;
        (*stage_ri).r_addr_b = pcp1;
        (*stage_ri).r_addr_c = pcp2;
        (*stage_ri).r_data_a = a;
        (*stage_ri).r_data_b = b;
        (*stage_ri).r_data_c = c;
}

typedef struct {
        int r_addr_0;
        int r_addr_1;
        int r_data_0;
        int r_data_1;
} stage_rd_t; 
void rd_shiftin(stage_rd_t * stage_rd, int a, int b, int da, int db){
        static int initCount = 0;
        if(initCount == 0) {
                //fill 1 with zeros (ignored later) and 0 with data
                stage_rd[1].r_addr_0 = 0;
                stage_rd[1].r_addr_1 = 0;
                stage_rd[1].r_data_0 = 0;
                stage_rd[1].r_data_1 = 0;
                
                stage_rd[0].r_addr_0 = a;
                stage_rd[0].r_addr_1 = b;
                stage_rd[0].r_data_0 = da;
                stage_rd[0].r_data_1 = db;
        } else {
                //move zero to one. move new data to zero
                stage_rd[1].r_addr_0 = stage_rd[0].r_addr_0;
                stage_rd[1].r_addr_1 = stage_rd[0].r_addr_1;
                stage_rd[1].r_data_0 = stage_rd[0].r_data_0;
                stage_rd[1].r_data_1 = stage_rd[0].r_data_1;
                
                stage_rd[0].r_addr_0 = a;
                stage_rd[0].r_addr_1 = b;
                stage_rd[0].r_data_0 = da;
                stage_rd[0].r_data_1 = db;
        }
        if(initCount != 1) 
                initCount = 1;
};


typedef struct {
        int w_data;
        int w_addr;
        int we;
} stage_ex_t;
void ex_shiftin(stage_ex_t * stage_ex, int data, int addr, int en) {
        static int initCount = 0;
        if(initCount == 0) {
                //fill 1 and 2 with zeros (ignored later) and 0 with data
                stage_ex[2].w_data = 0;
                stage_ex[2].w_addr = 0;
                stage_ex[2].we     = 0;
                stage_ex[1].w_data = 0;
                stage_ex[1].w_addr = 0;
                stage_ex[1].we     = 0;
                stage_ex[0].w_data = data;
                stage_ex[0].w_addr = addr;
                stage_ex[0].we     = en;
        } else {
                //move one to two. move zero to one. move new data to zero.
                stage_ex[2].w_data = stage_ex[1].w_data;
                stage_ex[2].w_addr = stage_ex[1].w_addr;
                stage_ex[2].we     = stage_ex[1].we;
                
                stage_ex[1].w_data = stage_ex[0].w_data;
                stage_ex[1].w_addr = stage_ex[0].w_addr;
                stage_ex[1].we     = stage_ex[0].we;
                
                stage_ex[0].w_data = data;
                stage_ex[0].w_addr = addr;
                stage_ex[0].we     = en;
        }
//printf("ex shiftin: %d %d %d initCount = %d\n", stage_ex[0].w_data, stage_ex[0].w_addr, stage_ex[0].we, initCount);
        
        if(initCount != 1) 
                initCount = 1;
}
void print_csv_header() {
    printf("LS,pa,pb,pc,a,b,c,ra0,ra1,rd0,rd1,we,wad,wdat\n");
}
void print_csv_row(stage_ri_t* stage_ri, stage_rd_t* stage_rd, stage_ex_t* stage_ex, 
                int pc, int c, int fwd)
{
        printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                c,
                stage_ri[0].r_addr_a, stage_ri[0].r_addr_b, stage_ri[0].r_addr_c,
                stage_ri[0].r_data_a, stage_ri[0].r_data_b, stage_ri[0].r_data_c,
                stage_rd[1].r_addr_0, stage_rd[1].r_addr_1, 
                stage_rd[1].r_data_0, stage_rd[1].r_data_1,
                stage_ex[2].we,       stage_ex[2].w_data,   stage_ex[2].w_addr
        );
}
void print_table_header() {
        printf("  LS |   pa   pb   pc |    a    b    c |  ra 0 ra 1|   rd 0   r"
                        "d 1|   we  wad wdat\n\n");
}
void print_table_row(stage_ri_t* stage_ri, stage_rd_t* stage_rd, stage_ex_t* stage_ex, 
                int pc, int c, int fwd)
{
        char cc1[16], cc2[16], cc3[16], cc4[16]; //colour of each column in this particular row
        #if NOCOLOR == 0
                //set the colours of each row
                if( stage_ex[0].we == 0 )    
                        sprintf(cc1, ANSI_COLOR_RESET ANSI_COLOR_BLUE);
                else
                        sprintf(cc1, ANSI_COLOR_RESET ANSI_COLOR_WHITE); 
                if( stage_ex[1].we == 0 )    
                        sprintf(cc2, ANSI_COLOR_RESET ANSI_COLOR_BLUE);
                else 
                        sprintf(cc2, ANSI_COLOR_RESET ANSI_COLOR_WHITE);
                if( stage_ex[2].we == 0 )    
                        sprintf(cc3, ANSI_COLOR_RESET ANSI_COLOR_BLUE);
                else
                        sprintf(cc3, ANSI_COLOR_RESET ANSI_COLOR_WHITE);
                sprintf(cc4, ANSI_COLOR_RESET);
        #else 
                memset(cc1,0,16);memset(cc2,0,16);memset(cc3,0,16);memset(cc4,0,16);
        #endif
        char format[256];
        char str1[4], str2[4];
        if(fwd == 1) sprintf(str1, "fwd");
        else sprintf(str1, "   ");
        if(fwd == 2) sprintf(str2, "fwd");
        else sprintf(str2, "   ");
        //insert the colour codes and the "fwd" strings
        sprintf(format, "%%4d |%s %%4d %%4d %%4d | %%4d %%4d %%4d |%s %%4d %%4d | "
                        "%s%%3d" "%s%%3d" " | %s%%4d %%4d %%4d%s", cc1, cc2, 
                        str1, str2, cc3, cc4);
        char row[1024];
        sprintf(row, format,
                c,
                stage_ri[0].r_addr_a, stage_ri[0].r_addr_b, stage_ri[0].r_addr_c,
                stage_ri[0].r_data_a, stage_ri[0].r_data_b, stage_ri[0].r_data_c,
                stage_rd[1].r_addr_0, stage_rd[1].r_addr_1, 
                stage_rd[1].r_data_0, stage_rd[1].r_data_1,
                stage_ex[2].we,       stage_ex[2].w_data,   stage_ex[2].w_addr
        );
        printf("%s\n", row);
}
void run(int length, int* m, int limit, char* format) {
        stage_ri_t *stage_ri;    
        stage_rd_t *stage_rd; 
        stage_ex_t *stage_ex; 
        
        stage_ri = malloc( sizeof(stage_ri_t) );   //does not need to be delayed
        stage_rd = malloc( sizeof(stage_rd_t)*2 ); //needs to be delayed by one for output
        stage_ex = malloc( sizeof(stage_ex_t)*3 ); //needs to be delayed by two for output
        
        int we = 0;
        char ubranch_count = 0;
        int ubranch_addr;
        char cbranch_count = 0;
        int cbranch_addr;
        int fwd;
        if(strcmp("table", format) == 0) {
            print_table_header();
        } else if(strcmp("csv",format) == 0) {
            // Don't print the header, this just makes verification harder
            //print_csv_header();
        }
        //loop through instructions, count cycles w/ c
        for(int pc=0, count=0; pc < length && count < limit; count++) 
        {
                int a = m[pc], b = m[pc+1], c = m[pc+2];
                int da = m[a], db = m[b];
                
                if(cbranch_count > 0) cbranch_count--;
                if(ubranch_count > 0) ubranch_count--;
//printf("a:%d b:%d da:%d db:%d  \n", a, b, da, db);

                //determine if instruction contains a branch
//printf("c:%d pc+3:%d db:%d da:%d\n",c,pc+3, db, da);
                
                //don't ubranch to the next address, 
                //don't ubranch if cbranch in progress
                //don't ubranch if ubranch in progress
                if(a == b && c != pc+3 && cbranch_count == 0 && ubranch_count == 0) {
                        ubranch_count = 2;       //the next one instruction won't write
                        ubranch_addr = c;
                }
                //don't cbranch to the next address, 
                //don't cbranch if cbranch in progress
                //don't cbranch if already ubranching
                if((db-da) <= 0 && c!=pc+3 && cbranch_count == 0 && ubranch_count == 0) {
                        cbranch_count = 3; //the next two instructions won't write
                        cbranch_addr = c;
                }
//printf("ubranch_count:%d cbranch_count:%d\n", ubranch_count, cbranch_count);                
                if(ubranch_count == 1 || cbranch_count == 1 || cbranch_count==2) 
                        we = 0;
                else 
                        we = 1;
                
                //update the 0th element of each stage's array
                ri_shiftin(stage_ri, pc, pc+1, pc+2, a, b, c);
                rd_shiftin(stage_rd, a,b,da,db);
                ex_shiftin(stage_ex, b,db-da,we);
                //write result to memory
                if(we) m[b] = db-da;
                
                //determing if instruction reads using forwarding (DOES NOT SHOW INSTRUCTION FORWARDING)
                fwd = 0;
                if(     stage_ex[1].w_addr == stage_rd[0].r_addr_0 || 
                        stage_ex[2].w_addr == stage_rd[0].r_addr_0 || 
                        stage_ex[2].w_addr == stage_rd[1].r_addr_0 ) 
                { 
                        fwd = 1;
                        
                } 
                if(stage_ex[1].w_addr == stage_rd[0].r_addr_1 || 
                        stage_ex[2].w_addr == stage_rd[0].r_addr_1 || 
                        stage_ex[2].w_addr == stage_rd[1].r_addr_1 )
                {
                        fwd = 2;
                }
                if(strcmp("table", format) == 0) { 
                    print_table_row(stage_ri, stage_rd, stage_ex, pc, count, fwd);
                } else if (strcmp("csv", format) == 0) {
                    print_csv_row(stage_ri, stage_rd, stage_ex, pc, count, fwd);
                }
//printf(" %d %d", ubranch_count, cbranch_count);
//printf("\n");
                if(cbranch_count == 1) {
                        pc=cbranch_addr;
                        ubranch_count = 0;
                } else if(ubranch_count == 1) {
                        pc=ubranch_addr;
                } else {
                        pc+=3;
                }
                
        }
}
void print_usage(char * progname){
        fprintf(stderr, "usage: %s [options] filename \n", progname);
        fprintf(stderr, "\nOPTIONS:\n"
                        "\t-f FORMAT\n\t\teither csv or table\n"
                        "\t-l LIMIT\n\t\ta positive integer specifying the number of cycles to run for. default=20\n");
}
int main(int argc, char* argv[]) 
{
        int limit = 20; // default value
        char * format = NULL;
        char * fname = NULL;
        if(argc == 1) {
                print_usage(argv[0]);
                return(1);
        } else if(argc > 1) {
                for(int i = 1; i < argc; i++){
                        if(argv[i][0] != '-') {
                                fname=malloc(strlen(argv[i]));
                                strcpy(fname, argv[i]);
                        } else if(argv[i][1] == 'f') {
                                if(i+1 == argc || argv[i+1][0] == '-') {
                                        print_usage(argv[0]);
                                        return(1);
                                }
                                format=malloc(strlen(argv[i+1]));
                                strcpy(format, argv[i+1]);
                                i++;
                        } else if(argv[i][1] == 'l') {
                                if(i+1 == argc || argv[i+1][0] == '-') {
                                        print_usage(argv[0]);
                                        return(1);
                                }
                                limit = atoi(argv[i+1]);
                                i++;
                        } else {
                                print_usage(argv[0]);
                                return(1);
                        }
                }
        }
        if(fname == NULL) {
                print_usage(argv[0]);
                return(1);
        }
        if(format == NULL) {
                format=malloc(strlen("table")+1);
                strcpy(format,"table");
        } else if(strcmp("table",format) && strcmp("csv",format)) {
                print_usage(argv[0]);
                return(1);
        }
        FILE * slqFile = fopen(fname, "r");
        int * memory = malloc( sizeof(int)*MAX_MEMORY_ADDRESS );
        memset(memory,0,MAX_MEMORY_ADDRESS+1);
        int length = 0;
        int valueRead;
        for(int i = 0; fscanf(slqFile,"%i",&valueRead) != EOF; i++) {
                memory[i] = valueRead;
                length = i+1;
        }
        fclose(slqFile);
        
        run(length, memory, limit, format);
        
        if(format == NULL) free(format);
        if(fname == NULL) free(fname);

        return 0;
}










































