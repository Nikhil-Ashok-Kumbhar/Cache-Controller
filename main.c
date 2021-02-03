#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>


#define DW 32                      //Data width
#define S 131072                   //Cache size
#define MAX_LINES 32768            //MAX_LINES
#define MAX_WAYS 16                //MAX_WAYS

int lru[MAX_LINES][MAX_WAYS];       //Cache implementation LRU
int tag_val[MAX_LINES][MAX_WAYS];   //TAG
bool v[MAX_LINES][MAX_WAYS];        //Valid bit
bool d[MAX_LINES][MAX_WAYS];        //Dirty bit

uint8_t N;                          //No of ways
int BL;                             //Burst Length
uint8_t B;                          //Block size
uint32_t L;                         //no of lines

uint32_t tag,tag_write;             //tag
uint32_t tag_bits;                  //tag bits
uint32_t line,Line,line_write;      //line
uint32_t add,word;                  //add/word

uint32_t rl_count,rl_miss_count,rl_hit_count,rlmd_count,rm_count;               //read counters
uint32_t wt_count,wm_count,wl_count,wlh_count,wlm_count,wlmd_count,wtm_count;   //write counters
uint32_t fcd_count,fc_count;                                                    //flush counters

uint8_t var_bl,var_n,wr_sta,count_comb;

bool miss_hit, miss_hit_write,wb, wtna, wta ;        //miss-hit write strategy

uint8_t WB = 0;
uint8_t WTNA = 0;
uint8_t WTA = 0;
// file init
FILE *fp;
//choldc-cholsl init
float a[256][256];
uint16_t n;
float p[256];
float b[256];
float x[256];

//Calculate Block-Size
void calc_block(uint8_t BL){
     B = (DW* BL)>> 3;
}

//Calculate No of Lines
void calc_line(uint8_t BL, uint8_t N){
   L = S/(N*4*BL);
}

//Cache hit miss logic
bool cache_miss_hit(uint32_t tag, uint32_t line){

uint8_t found = 0;
miss_hit = false ;
uint32_t i, j;

for(i=0; i<N; i++){
    j = tag_val[line][i];
     if(j == tag){          //if tag matches the given tag at that particular line
            found = 1;
            break;
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

//calculate tag from given address
uint32_t get_tag(uint32_t add){

    uint32_t Tag;
    uint8_t a;

    tag_bits = (DW - (log2(L)) - (log2(B)) );
    a =((log2(L)) + (log2(B)));

    Tag = add >> a;
    return Tag;
}

//calculate line from address
uint32_t get_line(uint32_t add){

    uint8_t a;

    a =(tag_bits + (log2(B)));

    Line = (add<<tag_bits)>>a ;
    return Line;
}

//LRU Logic implementation
uint8_t search_lru(uint32_t line){
uint8_t found = 0;
uint8_t val =0;
uint8_t i, test;
for(i=0; i<N; i++){
    test = lru[line][i];
    if(test == (N-1))          //finding the LRU value at particular line
       {
           found = 1;
     break;
       }

}
if(found){
val = i;
}
return val;
}

//Update LRU Logic
void updatelru(uint32_t line, uint32_t i){
uint32_t r = lru[line][i];
lru[line][i] = 0;
for (int j=0; j<N; j++){
    if (lru[line][j] < r){        //if the passed lRU is greater than the lru at that line then increment lru count
        lru[line][j] += 1;
    }
}

}

//Read line Logic
void read_line(uint32_t line, uint32_t tag){
    rl_count++;
    uint8_t lruval = 0;
    uint32_t test,test2;
    miss_hit = cache_miss_hit(tag,line);

if(miss_hit){

    rl_miss_count++;
    lruval = search_lru(line);

    if(d[line][lruval] == 1){

    rlmd_count++;
    v[line][lruval]  = 0;
    d[line][lruval] =  0;

   }
   test = tag_val[line][lruval];
    tag_val[line][lruval]= tag;
    test2 = tag_val[line][lruval];
    d[line][lruval] =  0;
    v[line][lruval]  = 1;

}
else{
      rl_hit_count++;
}

    updatelru(line,lruval);
}

//Read Memory Implementation
void read_memory(void *address, int size){

    rm_count++;
    add = (uint32_t)address;
    int old_line = -1;

    for(int i=0; i<size; i++){
    tag = get_tag(add);
    line = get_line(add);

        if(line !=old_line){
            old_line = line;
            read_line(line,tag);
        }
        add++;
    }


}

//Logic to get a 30bit word from address
uint32_t get_word(uint32_t add_write){
    uint32_t wt_word;
    wt_word = add_write >> 2;
    return wt_word;
}

//writing through count
void write_word(uint32_t add_write){

    if((wta)||(wtna)){
        wt_count++;
    }
}

//write through logic
void write_line(uint32_t line,uint32_t tag){

wl_count++;
uint8_t lruaval_write = 0;
miss_hit = cache_miss_hit(tag,line);
    if ((wtna) || (wta)){

        wtm_count++;
    }

    if(((wb) || (wta))&& (miss_hit)){

        lruaval_write = search_lru(line);

        if(d[line][lruaval_write] == 1){
            wlmd_count++;
            d[line][lruaval_write] =  0;
        }

            tag_val[line][lruaval_write]= tag;
            v[line][lruaval_write]  = 1;
            updatelru(line,lruaval_write);

    }

        if(wb){
        d[line][lruaval_write] =  1;
        }
    if(!miss_hit){
        wlh_count++;
        v[line][lruaval_write]  = 1;
        updatelru(line,lruaval_write);

    }
    else{

        wlm_count++;
    }
}

//write memory logic
void write_memory(void* address, int size){

    wm_count++;
    uint32_t add_write = (uint32_t)address;
    int old_line = -1;

    for(int i=0; i<size; i++){
    tag = get_tag(add_write);;
    line = get_line(add_write);
        if(line != old_line){
            old_line = line;
            write_line(line,tag);
        }
        add_write++;
    }
    if((wta)||(wtna)){

        int old_word = -1;
        for(int i=0; i<size; i++){
           word = get_word(add_write);
            if(word != old_word){
                old_word = word;
                write_word(add_write);
            }
                    add_write++;
    }

}
}

//clear cache
void clear_cache(){
int i, j ;

for(i=0; i<MAX_LINES; i++){
    for(j=0; j<MAX_WAYS; j++){
        tag_val[i][j]=0;
    }
}

for(i=0; i<MAX_LINES; i++){
    for(j=0; j<MAX_WAYS; j++){
        lru[i][j]=0;
    }
}

for(i=0; i<MAX_LINES; i++){
    for(j=0; j<MAX_WAYS; j++){
        v[i][j]=0;
    }
}

for(i=0; i<MAX_LINES; i++){
    for(j=0; j<MAX_WAYS; j++){
        d[i][j]=0;
    }
}
}

//clear all counters
void clear_counters(){

    rm_count = 0;
    rl_count = 0;
    rl_miss_count = 0;
    rl_hit_count = 0;
    rlmd_count = 0;

    wm_count= 0;
    wl_count= 0;
    wlm_count =0;
    wlh_count =0 ;
    wlmd_count =0;
    wt_count = 0;
    wtm_count = 0;

    fcd_count = 0;
    fc_count = 0;
}

//flush cache logic
void flush_cache(){
int i, j;

for(i=0; i<L; i++){
    for(j=0; j<N; j++){
       if(d[i][j] == 1){
            fcd_count++;
        }
fc_count++;
    }

}

}

//printing counters
void print_counters(){

    printf("  \n");
    printf("BL %d \n",BL);
    printf("N %d \n",N);
    if(wb){
    printf("Write-back \n");
    }
    else if(wtna){
    printf("Write Through Non Allocate \n");
    }
    else{
    printf("Write Through Allocate \n");
    }
    printf("rm_count %d \n",rm_count);
    printf("rl_count %d \n",rl_count);
    printf("rl_hit_count %d \n",rl_hit_count);
    printf("rl_miss_count %d \n",rl_miss_count);
    printf("rlmd_count %d \n",rlmd_count);
    printf("wm_count %d \n",wm_count);
    printf("wl_count %d \n",wl_count);
    printf("wlm_miss_count %d \n",wlm_count);
    printf("wl_hit_count %d \n",wlh_count);
    printf("wlmd_count %d \n",wlmd_count);
    printf("wt_count %d \n",wt_count);
    printf("wtm_count %d \n",wtm_count);
    printf("fc_count %d \n",fc_count);
    printf("fcd_count %d \n",fcd_count);
    printf("  \n");
}

void clear_str(char *str_1){
    uint8_t i = 0;
    for(i=0; i<strlen(str_1); i++){
        str_1[i] = '0';
    }
}

//csv file output
void csv_file_out(){

    fprintf(fp,"%d,",BL);
    fprintf(fp,"%d,",N);

   if(wb){
        char str_1[3]=("WB");
    fprintf(fp,"%s,",str_1);
    clear_str(str_1);
    }
    else if(wtna){
            char str_2[5]=("WTNA");
    fprintf(fp,"%s,",str_2);
    clear_str(str_2);
    }
    else{
        char str_3[4]=("WTA");
    fprintf(fp,"%s,",str_3);
    clear_str(str_3);
    }

    fprintf(fp,"%d,",rm_count);
    fprintf(fp,"%d,",rl_count);
    fprintf(fp,"%d,",rl_hit_count);
    fprintf(fp,"%d,",rl_miss_count);
    fprintf(fp,"%d,",rlmd_count);
    fprintf(fp,"%d,",wm_count);
    fprintf(fp,"%d,",wl_count);
    fprintf(fp,"%d,",wlh_count);
    fprintf(fp,"%d,",wlm_count);
    fprintf(fp,"%d,",wlmd_count);
    fprintf(fp,"%d,",wt_count);
    fprintf(fp,"%d,",wtm_count);
    fprintf(fp,"%d,",fcd_count);
    fprintf(fp,"%d,\n",fc_count);


}

//matrix implementation for choldc/cholsl
void identity_matrix(){
    int i, j, k,l,m;
        for (i=0;i<256;i++) {
        for (j=0;j<256;j++) {
                if(i == j){
                    a[i][j]= 1.0;

                   }
            else{
                a[i][j]= 0.0;
            }
    }

}

        for(k=0;k<256;k++){
            p[k] = 1;

        }

        for(l=0;l<256;l++){
        b[l] = 1;

        }

        for(m=0;m<256;m++){
        x[m] = 0;

        }
}

//choldc
void choldc(float a[256][256], int n ,float p[256])
{
    uint32_t i,j,k;
    float sum;
    write_memory(&i,sizeof(i));
    read_memory(&n,sizeof(n));
    for (i=1;i<=n;i++) {
    read_memory(&i,sizeof(i));
    write_memory(&j,sizeof(j));
    read_memory(&n,sizeof(n));
        for (j=i;j<=n;j++) {
            read_memory(&i,sizeof(i));
            read_memory(&n,sizeof(n));
            read_memory(&j,sizeof(j));
            write_memory(&k,sizeof(k));
            for (sum=a[i][j],k=i-1;k>=1;k--){
            read_memory(&i,sizeof(i));
            read_memory(&j,sizeof(j));
            read_memory(&k,sizeof(k));
            read_memory(&a[i][j],sizeof(a[i][j]));
            write_memory(&sum,sizeof(sum));
            sum -= a[i][k]*a[j][k];
            read_memory(&a[i][k],sizeof(a[j][k]));
            write_memory(&sum,sizeof(sum));
            }

            if (i == j) {
            read_memory(&i,sizeof(i));
            read_memory(&j,sizeof(j));
                if (sum <= 0.0){
            read_memory(&sum,sizeof(sum));
                    printf("choldc failed");
                }
                p[i]=sqrt(sum);
            read_memory(&sum,sizeof(sum));
            write_memory(&p[i],sizeof(p[i]));
            } else{
                    a[j][i]=sum/p[i];
               read_memory(&p[i],sizeof(p[i]));
               read_memory(&sum,sizeof(sum));
               write_memory(&a[j][i],sizeof(a[j][i]));
            }
        }
    }

}

//cholsl
void cholsl(float a[256][256], int n, float p[256], float b[256], float x[256])
{
    uint32_t i,k;
    float sum;
    write_memory(&i,sizeof(i));
    read_memory(&n,sizeof(n));
    for (i=1;i<=n;i++) {
    read_memory(&i,sizeof(i));
    write_memory(&k,sizeof(k));
    read_memory(&b[i],sizeof(b[i]));
    write_memory(&sum,sizeof(sum));
    for (sum=b[i],k=i-1;k>=1;k--) {
    read_memory(&k,sizeof(k));
    read_memory(&i,sizeof(i));
           sum -= a[i][k]*x[k];
    read_memory(&x[k],sizeof(x[k]));
    read_memory(&a[i][k],sizeof(a[i][k]));
    write_memory(&sum,sizeof(sum));
    write_memory(&k,sizeof(k));
    }
    read_memory(&i,sizeof(i));
    x[i]=sum/p[i];
    read_memory(&p[i],sizeof(p[i]));
    read_memory(&sum,sizeof(sum));
    write_memory(&x[i],sizeof(x[i]));
    write_memory(&i,sizeof(i));
    }
    write_memory(&i,sizeof(i));
    read_memory(&n,sizeof(n));
    for (i=n;i>=1;i--) {
    read_memory(&i,sizeof(i));
    write_memory(&k,sizeof(k));
    read_memory(&n,sizeof(n));
    read_memory(&x[i],sizeof(x[i]));
    write_memory(&sum,sizeof(sum));
    for (sum=x[i],k=i+1;k<=n;k++) {
            sum -= a[k][i]*x[k];
    read_memory(&x[k],sizeof(x[k]));
    read_memory(&a[k][i],sizeof(a[k][i]));
    write_memory(&sum,sizeof(sum));
    write_memory(&k,sizeof(k));
    }
    x[i]=sum/p[i];
    read_memory(&p[i],sizeof(p[i]));
    read_memory(&sum,sizeof(sum));
    write_memory(&x[i],sizeof(x[i]));
    write_memory(&i,sizeof(i));
    }
    }



int main( ) {

    uint32_t x[8192];
    uint32_t i;
    n = 255;
    printf(" \n");
    printf("CACHE CONTROLLER \n");

    wb = false;
    wtna = false;
    wta = false;
    count_comb = 0;

    fp=fopen("Counters.csv","w+");
    fprintf(fp,"BL,N,WS,RM_count,RL_count,RLH_count,RLM_count,RLMD_count,WM_count,WL_count,WLH_count,WLM_count,WLMD_count,WT_count,WTM_count,FCD_count,FC_count \n");

    for(BL=1; BL<=8; BL+= BL){
          for(N=1; N<=16; N+=N){
                for(wr_sta=1; wr_sta<=3; wr_sta++){
                   calc_block(BL);
                   calc_line(BL,N);
                   if(wr_sta == 1){
                        wb = true;
                        wtna = false;
                        wta = false;
                    }
                    else if(wr_sta ==2){
                        wb = false;
                        wtna = true;
                        wta = false;
                                    }
                    else{
                        wb = false;
                        wtna = false;
                        wta = true;
                    }

                        clear_cache();
                        clear_counters();
                      /* TEST CODE
                       write_memory(&i,sizeof(uint32_t));
                        for(i=0; i<8192; i++){
                        read_memory(&i, sizeof(uint32_t));
                        x[i] =0;
                        write_memory(&x[i], sizeof(uint32_t));
                        write_memory(&i, sizeof(uint32_t));
                        }
*/
                        //count_comb++;

                        identity_matrix();
                        choldc(a,n,p);
                        cholsl(a,n,p,b,x);
                        flush_cache();
                        print_counters();
                        csv_file_out();
                }

          }

    }

    fclose(fp);


   return 0;
}
