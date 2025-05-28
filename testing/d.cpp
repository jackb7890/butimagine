#include <cstdio>
#include <type_traits>

struct Parent;
struct Child;
struct Child2;

template <typename T>
struct IndexOf {
    static const int value = -1;
    IndexOf() {
        static_assert(false);
    }
};

#define RegisterClassIndex(type, index) \
template <> \
struct IndexOf <type> { \
    static const int value = index; \
};

template <typename T>
struct TypeDetails {
    static const int index = IndexOf<T>::value;
};

template <typename T>
concept Encodable = TypeDetails<T>::index > 0;

//using std::remove_pointer<decltype(this)> =

RegisterClassIndex(Parent, 1)
RegisterClassIndex(Child, 2) 

struct Parent {
    Parent() {}
    Parent(const char *s) {
        printf("parent ctor: %s\n", s);
    }
    void virtual foo() {
        printf("foo parent\n");
    }

    int virtual GetIndex() const {
        printf("in Parent\n");
        // using _t = std::remove_const<std::remove_pointer<decltype(this)>::type>::type;
        return TypeDetails<Parent>::index;
    }

};

struct Child : public Parent {
    Child() {}
    Child(const char *s) {
        printf("child ctor: %s\n", s);
    }
    void virtual foo() {
        printf("foo child\n");
    }

    int virtual GetIndex() const {
        printf("in child\n");
        return TypeDetails<Child>::index;
    }
};

struct Child2 : public Parent {
    Child2() {}
    Child2(const char *s) {
        printf("child2 ctor: %s\n", s);
    }
    void virtual foo() {
        printf("foo child2\n");
    }

    int virtual GetIndex() const {
        printf("in child2\n");
        return TypeDetails<Child2>::index;
    }
};

#include <vector>
int main() {
    Parent* p = new Parent();
    Child* c1 = new Child();
    Child2* c2 = new Child2();

    std::vector<Parent*> list;
    list.push_back(p);
    list.push_back(c1);
    list.push_back(c2);

    int pi = list[0]->GetIndex();
    int c1i = list[1]->GetIndex();
    int c2i = list[2]->GetIndex();
    printf("pi %d c1i %d c2i %d\n", pi, c1i, c2i);
    // Encode(p);
    // Encode(c1);
    // Encode(c2);
    return 1;
}