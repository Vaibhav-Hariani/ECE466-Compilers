#include <stdio.h>
int womp_womp(int a){
    char* b;
    *(b + 1) = 5;

    int c;
    int d;
    return a;
}

int main(){
    int tmp = 0;
    int* a = &tmp;
    
    printf("%d\n", *a);
    (*a)++;
    printf("%d\n", *a);

    // int a;
    // int b;
    // womp_womp(a);
}