#include <cstdio>
#include <type_traits>
#include <concepts>

struct Parent;
struct Child;
struct Child2;

//using std::remove_pointer<decltype(this)> =

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

template <class T>
concept IndexRegisteredT = TypeDetails<T>::index > 0;

template<typename T>
class Encodable {
public:
    Encodable() { 
        static_assert(std::derived_from<T, Encodable>);
    }

    constexpr static int GetIndex() {
        return TypeDetails<T>::index;
    }
};

struct Parent {
    Parent() {
        static_assert(IndexRegisteredT<T>)
    }
    Parent(const char *s) {
        printf("parent ctor: %s\n", s);
    }
    void virtual foo() {
        printf("foo parent\n");
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
};

struct Child2 : public Parent {
    Child2() {}
    Child2(const char *s) {
        printf("child2 ctor: %s\n", s);
    }
    void virtual foo() {
        printf("foo child2\n");
    }
};

RegisterClassIndex(Parent, 1)
RegisterClassIndex(Child, 2) 
RegisterClassIndex(Child2, 4) 

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