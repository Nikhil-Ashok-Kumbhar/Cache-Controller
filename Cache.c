#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define DW 32
#define S 131072
#define MAX_LINES 32768
#define MAX_WAYS 16

int lru[MAX_LINES][MAX_WAYS];
int tag_val[MAX_LINES][MAX_WAYS];
bool v[MAX_LINES][MAX_WAYS];
bool d[MAX_LINES][MAX_WAYS];

int N;
int BL;
uint8_t B;
uint32_t L;

uint32_t tag;
uint32_t line,Line;
uint32_t add;

uint16_t rl_count,rl_miss_count,rl_hit_count,rlmd_count,rm_count;

bool miss_hit;


void calc_block(uint8_t BL){
     B = (DW* BL)>> 3;
}

void calc_line(uint8_t BL, uint8_t N){
    int a = log2(B);
   // L = (S*N)>> a ;
   L = S/(N*4*BL);
}

bool cache_miss_hit(){

uint8_t found = 0;
miss_hit = false ;
uint32_t i, j;

for(i=0; i<L; i++){
    for(j=0; j<N; j++){
        if(tag_val[L][N] == tag)
            break;
            found = 1;
    }
}
if(found){
    miss_hit = false;
}
else{
    miss_hit = true;
}
return miss_hit;
}


uint32_t get_tag(uint32_t add){

    uint32_t tag_bits, Tag;
    uint8_t a;

    tag_bits = (DW - (log2(L)) - (log2(B)) );
    a =((log2(L)) + (log2(B)));

    Tag = add >> a;

    //printf("tag bits %d \n\r",tag_bits);

    //printf("Tag %d \n\r",Tag);

    return Tag;
}

uint32_t get_line(uint32_t add){

    uint32_t line_bits;
    uint8_t a;

    line_bits = ((log2(L)) );

    a =((log2(L)) + (log2(B)));

    Line = add<<line_bits>>a ;

 //   printf("Line bits %d \n",line_bits);
 //   printf("Line %d \n",Line);
 //   printf("  \n\r");

    return Line;
}

int search_lru(){
uint8_t found = 0;
uint8_t val =0;
uint8_t i;
for(i=0; i<N; i++){
    if(lru[line][i] == N-1)
        break;
    found =1;
}
if(found){

val = i;
}
return val;
}

void updatelru(uint32_t line, uint32_t i){
uint32_t r = lru[line][i];
uint32_t inc;
lru[line][i] = 0;
for (int j=0; j<N; j++){
    if (lru[line][j] < r){
        lru[line][j] += 1;
    }
}

}

void read_line(){
    rl_count++;
   // printf("read line count: %d \n",rl_count);
    int i;
    uint32_t old_tag;
    miss_hit = cache_miss_hit();

if(miss_hit){

    rl_miss_count++;
    //printf("read line miss count: %d \n",rl_miss_count);
    i = search_lru();
    old_tag = tag_val[line][i];

    if(d[line][i] == 1){

    rlmd_count++;
    v[line][i]  = 0;
    d[line][i] =  0;

   }
    tag_val[line][i]= tag;
    d[line][i] =  0;
    v[line][i]  = 1;
}
    rl_hit_count++;
   // tag_val[line][i]= tag;
    updatelru(line,i);
    //printf("read line count: %d \n",rl_hit_count);
}
void read_memory(void *address, int size){

    rm_count++;
   // printf("read count: %d \n",rm_count);
    add = (uint32_t)address;
    int old_line = -1;



    for(int i=0; i<size; i++){
    //printf("Address %X \n",add);
    tag = get_tag(add);
    line = get_line(add);

        if(line !=old_line){
            old_line = line;
            //printf("call read line \n");
            read_line();
        }
        add++;
    }


}



int main( ) {


    rm_count = 0;
    rl_count = 0;
    rl_miss_count = 0;
    rl_hit_count = 0;
    rlmd_count = 0;

    printf( "Enter a value of N :");
    scanf("%d",&N);

    printf( "Enter a value of BL :");
   scanf("%d",&BL);

    printf(" \n");

   // BL = 1;
   // N = 8;
    calc_block(BL);

    calc_line(BL,N);

    printf( "Value of N is %d \r\n",N);

    printf( "Value of BL is %d \r\n",BL);

    printf( "Value of B is %d \r\n",B);

    printf( "Value of L is %d \r\n",L);

    printf(" \n");

    uint32_t sum =0;
    uint32_t x[8192];
    uint32_t i;

    for(i=0; i<8192; i++){
        sum += x[i];
        read_memory(&x[i], sizeof(uint32_t));
    }



    printf("read memory count %d \n",rm_count);
    printf("read line count %d \n",rl_count);
    printf("read miss count %d \n",rl_miss_count);
    printf("read hit count %d \n",rl_hit_count);
    printf("read line miss dirty count %d \n",rlmd_count);
   return 0;
}
