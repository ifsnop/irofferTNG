#include <stdio.h>

struct a {

    int i;
    int j;

};

void copiar(struct a pri, struct a seg) {

    pri.i=seg.i;
    pri.j=seg.j;

    return;
}

int main(int argc, char *argv[]) {

struct a a1;
struct a a2;

    a1.i=0;
    a1.j=0;
    
    a2.i=10;
    a2.j=20;
    
    copiar(a1,a2);
    
    printf("%d %d\n", a1.i, a1.j);

    return 1;
}
