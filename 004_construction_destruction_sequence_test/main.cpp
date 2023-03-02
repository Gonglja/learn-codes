//
// Created by ubuntu on 3/2/23.
//

#include <iostream>

static int i=0;

class A {
private:
    static int i;
    int j{-1};
public:
    A(int j){
        printf("A: construct %d\r\n", j);
        this->j = j;
    }
    ~A() {
        printf("A: construct %d\r\n", this->j);
    }
};

__attribute__((destructor)) class B {
private:
    static int i;
    int j{-1};
public:
    B(int j){
        printf("B: construct %d\r\n", j);
        this->j = j;
    }
    ~B() {
        printf("B: destruct %d\r\n", this->j);
    }
};


__attribute__((constructor()))void before_main() {
    printf("before_main\r\n");
}

__attribute__((constructor(100)))void before_main100() {
    printf("before_main 100\r\n");
}

__attribute__((constructor(101)))void before_main101() {
    printf("before_main 101\r\n");
}

__attribute__((destructor()))void after_main() {
    printf("after_main\r\n");
}
__attribute__((destructor(100)))void after_main100() {
    printf("after_main 100\r\n");
}

__attribute__((destructor(101)))void after_main101() {
    printf("after_main 101\r\n");
}

//__attribute__((section(".init_array "), aligned (sizeof (void *)))) void section_before_main()  {
//    printf("section_before_main\r\n");
//}
//
//__attribute__((section(".fini_array "), aligned (sizeof (void *)))) void section_after_main() {
//    printf("section_after_main\r\n");
//}

A a(0);
static A aa(1);
B b(2);
static B bb(3);

__attribute__((constructor())) void test() {
    static B b(4);
    B bb(5);
}
// 执行顺序：
// 1. 带有优先级的(值越小，执行越靠前) __attribute__((constructor(n)))
// 2. 不带优先级的 __attribute__((constructor()))
// 3. 全局对象
// 4. main
// 5. 全局d
int main() {
    std::cout << "hello, world"<<std::endl;
    return 0;
}