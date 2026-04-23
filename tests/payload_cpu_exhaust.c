int main(void){
    volatile unsigned long long i;
    for(i=0;i<999999999ULL;i++)__asm__ volatile("");
    return 0;
}
