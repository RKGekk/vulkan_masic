#pragma once

#include <memory>

#include "string_tools.h"

#include <string>
#include <type_traits>

template <class Type>
std::shared_ptr<Type> MakeStrongPtr(std::weak_ptr<Type> pWeakPtr) {
    if (!pWeakPtr.expired())
        return std::shared_ptr<Type>(pWeakPtr);
    else
        return std::shared_ptr<Type>();
}

template<class T>
struct SortBy_SharedPtr_Content {
    bool operator()(const std::shared_ptr<T>& lhs, const std::shared_ptr<T>& rhs) const {
        return *lhs < *rhs;
    }
};

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif

template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
    return static_cast<typename std::underlying_type<E>::type>(e);
}