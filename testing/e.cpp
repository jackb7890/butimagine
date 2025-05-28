#include <cstdio>

struct A {
    char a;
    A() {}
    virtual void foo() {
        printf("A\n");
    }

    void bar() {
        printf("b");
    }
};

struct B : public A {
    char b;
    B() {}
    virtual void foo() {
        printf("B\n");
    }
};

int main() {
    int a = sizeof(A);
    int b = sizeof(B);
    printf("a %d b %d\n", a, b);
}