#include <typeinfo>
#include <cstdio>

template <typename T>
struct Type_ {
    const int index;
};

template <typename T, typename... Rest>
int build() {

}

template <typename W>
struct GetTypeID {
    constexpr int operator()() {
        return typeid(W).hash_code();
    }
};

class Base {
    public:
    int id;
    Base() : id(1) {
        printf("in base cstor for obj\n");
    }

    virtual static int GetTypeID() {

    }
};

class Derived : public Base {
    public:
    int j;
    
    public:
    Derived() : Base() {
        printf("in derived cstor for obj\n");
        j = 2;
    }
};

#include <vector>
int main() {
    Base b;
    Derived d;
}