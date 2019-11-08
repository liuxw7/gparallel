#pragma once
#include <iostream>
#include <string>
#include <cxxabi.h>
#include <memory>
#include <type_traits>
#include <functional>

template <class... type> struct type_list {};

template <class T, int I, class... AS>
struct find_index {
    typedef char type[I];
};
template <class T, int I, class A, class... AS>
struct find_index<T, I, A, AS...> {
    typedef char VT[I];
    static VT& test(T*);
    static typename find_index<T, I + 1, AS...>::type& test(...);
    typedef decltype(test((A*)0)) type;
    static const int index = sizeof(type);
};

template <int I, class T, class... TS>
struct n_th {
    typedef typename n_th<I - 1, TS...>::type type;

    static type value(T, TS ... ts) {
        return n_th<I - 1, TS...>::value(ts...);
    }
};

template <class T, class... TS>
struct n_th<0, T, TS...> {
    typedef T type;
    static type value(T t, ...) {
        return t;
    }
};


// followings handle parameter

template <class R, class F, class CS, class MS, int... IS>
struct parameter_match_invoke {};

template <class R, class F, class... CS, class... MS, int I, int... IS>
struct parameter_match_invoke<R, F, type_list<CS...>, type_list<MS...>, I, IS...> {
    typedef typename n_th<I, CS...>::type Match;
    static R process(F fn, CS... cs, MS ... matches) {
        return parameter_match_invoke<R, F, type_list<CS...>, type_list<MS..., Match>, IS...>::process(fn, cs..., matches..., n_th<I, CS...>::value(cs...));
    }
};
template <class R, class... CS, class... MS, class... AS>
struct parameter_match_invoke<R, R(*)(AS...), type_list<CS...>, type_list<MS...>> {
    static R process(R(*fn)(AS...), CS ... , MS ... matches) {
        return fn(matches...);
    }
};

// parameter_match
template <class R, class F, class AS, class CS, int... IS>
struct parameter_match {};

template <class R, class F, class A, class... AS, class... CS, int... IS>
struct parameter_match<R, F, type_list<A, AS...>, type_list<CS...>, IS...> {
    static const int index = find_index<A, 0, CS...>::index;
    static_assert(index != sizeof...(CS), "not match!");
    static R process(F fn, CS... cs) {
        return parameter_match<R, F, type_list<AS...>, type_list<CS...>, IS..., index>::process(fn, cs...);
    }
};

template <class R, class F, class... CS, int... IS>
struct parameter_match<R, F, type_list<>, type_list<CS...>, IS...> {
    static R process(F fn, CS... cs) {
        return parameter_match_invoke<R, F, type_list<CS...>, type_list<>, IS...>::process(fn, cs...);
    }
};


// invoke_ex
template <class R, class... AS, class... CS>
R invoke_ex(R(*fn)(AS...), CS... cs) {
    return parameter_match<R, R(*)(AS...), type_list<AS...>, type_list<CS...>>::process(fn, cs...);
};