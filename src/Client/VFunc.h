#pragma once
#include <bit>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace edu {

namespace details {
    template<typename>
    struct resolve_func;

    template<typename R, typename... Args>
    struct resolve_func<R(Args...)> {
        template<typename T>
        struct memfn : std::type_identity<R(T::*)(Args...)> {};

        template<typename T>
        struct memfn<const T> : std::type_identity<R(T::*)(Args...) const> {};

        template<typename T>
        using memfn_t = typename memfn<T>::type;
    };

    template<typename R, typename T, typename... Args>
    struct resolve_func<R(T::*)(Args...)> {
        template<typename>
        using memfn_t = R(T::*)(Args...);
    };

    template<typename R, typename T, typename... Args>
    struct resolve_func<R(T::*)(Args...) const> {
        template<typename>
        using memfn_t = R(T::*)(Args...) const;
    };
}

template<typename Fn, typename Inst, typename... Args>
decltype(auto) callMemberFunc(Inst* instance, const uintptr_t addr, Args&&... args) {
    using memfn_t = typename details::resolve_func<Fn>::template memfn_t<Inst>;
    const auto memfn = std::bit_cast<memfn_t>(addr);
    return (instance->*memfn)(std::forward<Args>(args)...);
}

template<size_t Index, typename Fn, typename Inst, typename... Args>
decltype(auto) callVFunc(Inst* instance, Args&&... args) {
    const auto vtable = *reinterpret_cast<uintptr_t* const*>(instance);
    return callMemberFunc<Fn>(instance, vtable[Index], std::forward<Args>(args)...);
}

} // namespace edu
