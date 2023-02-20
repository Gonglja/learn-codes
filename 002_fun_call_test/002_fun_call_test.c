// 打开在线查看汇编 https://godbolt.org/
#include <stdio.h>


void funb() {
        char str[]= "hello,world!";
        printf("%s\r\n", str);
}

int funa() {
        funb();
        return 0;
}

int fun() {
        int a=10,b=12,c=0xff;
        funa();
        return 0;
}


int main() {
        fun();
        return 0;
}
