#include <cstdio>
#include <type_traits>

template <class C>
struct Parent {};

template <typename T>
struct IndexOf {
    static const int value = -1;
    IndexOf() {
        static_assert(false);
    }
};

#define RegisterClassIndex(type, index) \
template <> \
struct IndexOf <type<type>> { \
    static const int value = index; \
};

template <typename T>
struct TypeDetails {
    static const int index = IndexOf<T>::value;
};

template <typename T>
concept Encodable = TypeDetails<T>::index > 0;

//using std::remove_pointer<decltype(this)> =

// RegisterClassIndex(Parent, 1)

template <typename T>
struct IndexOf <Parent<T>> { 
    static const int value = 1; 
};

template <>
struct Parent <> {
    Parent() {}
    Parent(const char *s) {
        printf("parent ctor: %s\n", s);
    }
    void virtual foo() {
        printf("foo parent\n");
    }

    int virtual GetIndex() {
        printf("in Parent\n");
        return TypeDetails<Parent>::index;
    }
};

#include <vector>
int main() {

}