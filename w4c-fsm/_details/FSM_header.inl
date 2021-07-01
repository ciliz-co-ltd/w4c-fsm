// (c) 2019-2021 Ciliz::W4
// Part of Ciliz W4 Game Creation SDK and Ciliz games codebase

template<typename T,  size_t idx, typename... List>
struct TypeList_IndexOf_impl;

template<typename T, size_t idx, typename... Tail >
struct TypeList_IndexOf_impl<T, idx, T, Tail...>
{
    static constexpr int value = idx;
};

template<typename T, size_t idx, typename Head, typename... Tail >
struct TypeList_IndexOf_impl<T, idx, Head, Tail...>
{
    static constexpr int value = TypeList_IndexOf_impl<T, idx + 1, Tail...>::value;
};

template<typename T, size_t idx>
struct TypeList_IndexOf_impl<T, idx>
{
    static constexpr int value = -1;
};

template<typename T, typename... List>
struct TypeList_IndexOf
{
    static constexpr int value = TypeList_IndexOf_impl<T, 0, List...>::value;
};

template<std::size_t... Is>
struct Indices {};

template<std::size_t N, std::size_t... Is>
struct BuildIndices
        : BuildIndices<N - 1, N - 1, Is...>
{};

template<std::size_t... Is>
struct BuildIndices<0, Is...>
        : Indices<Is...>
{};

#define COMMA_SEP(r, token, i, e) BOOST_PP_COMMA_IF(i) token<e>()
#define DECLVAL_WRAP(...) BOOST_PP_SEQ_FOR_EACH_I(COMMA_SEP, std::declval, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define DECLARE_FSM_HANDLER_TRAIT(NAME, ...)                                                                            \
    template <typename T, typename EventType>                                                                           \
    struct has_##NAME##_Short                                                                                           \
    {                                                                                                                   \
        template <typename C = T>                                                                                       \
        static auto test(int) -> decltype(std::declval<C>().NAME(                                                       \
                DECLVAL_WRAP(__VA_ARGS__)                                                                               \
            ), std::true_type()                                                                                         \
        );                                                                                                              \
        static auto test(...) -> std::false_type;                                                                       \
    public:                                                                                                             \
        enum { value = decltype(test(0))::value };                                                                      \
    };                                                                                                                  \
    template <typename T, typename EventType>                                                                           \
    struct has_##NAME##_Full                                                                                            \
    {                                                                                                                   \
        template <typename C = T>                                                                                       \
        static auto test(int) -> decltype(std::declval<C>().NAME(                                                       \
                std::declval<IFSM<EventType>&>(),                                                                       \
                DECLVAL_WRAP(__VA_ARGS__)                                                                               \
            ), std::true_type()                                                                                         \
        );                                                                                                              \
        static auto test(...) -> std::false_type;                                                                       \
    public:                                                                                                             \
        enum { value = decltype(test(0))::value };                                                                      \
    };


#define STATE_HNDL_IMPL(NAME, RET_TYPE, RETURN, RET)                                                                    \
    template<typename Type, typename EventType, typename... Args>                                                       \
    RET_TYPE NAME(IFSM<EventType>& fsm, Args... args)                                                                   \
    {                                                                                                                   \
        if constexpr (has_##NAME##_Full<Type, EventType>::value)                                                        \
        {                                                                                                               \
            RETURN (static_cast<Type*>(this))->NAME(fsm, args...);                                                      \
        } else if constexpr (has_##NAME##_Short<Type, EventType>::value)                                                \
        {                                                                                                               \
            RETURN (static_cast<Type*>(this))->NAME(args...);                                                           \
        } else                                                                                                          \
        {                                                                                                               \
            RETURN RET;                                                                                                 \
        }                                                                                                               \
    }

#define FSM_VOID_HNDL(NAME) STATE_HNDL_IMPL(NAME, void, BOOST_PP_EMPTY(), BOOST_PP_EMPTY())
#define FSM_BOOL_HNDL(NAME) STATE_HNDL_IMPL(NAME, bool, return, false)

namespace detail
{
    template <template <typename> typename Check, typename ... L> class set;

    namespace set_impl {
        template <template <typename> typename Check, typename ... L>
        struct contains_detail;
        template <template <typename> typename Check, typename U>
        struct contains_detail<Check, U>
        {
            using type = typename Check<U>::type;
            constexpr static bool value = std::is_void_v<type>;
        };
        template <template <typename> typename Check, typename U, typename T, typename ... L>
        struct contains_detail<Check, U, T, L...> {
            using type = typename Check<U>::type;
            using head = typename Check<T>::type;
            constexpr static bool value =
                    std::is_void_v<type>
                    || std::is_same_v<type, head>
                    || contains_detail<Check, U, L...>::value;
        };

        template <template <typename> typename Check, typename ... L>
        struct insert_detail;
        template <template <typename> typename Check, typename T, typename ... L>
        struct insert_detail<Check, T, set<Check, L...>> {
            using head = typename Check<T>::type;
            using type = set<Check, head, L...>;
        };

        template <typename ... L>
        struct unchecked_list
        {};
        template <typename T>
        struct cracker_list;
        template <template <typename> typename Check, typename ... L>
        struct cracker_list<set<Check, L...>>
        {
            using type = unchecked_list<L...>;
        };

        template <typename T>
        struct cracker;
        template <template <typename> typename Check, typename ... L>
        struct cracker<set<Check, L...>>
        {
            using list = typename cracker_list<typename set<Check, L...>::unique>::type;
            constexpr static size_t size = set<Check, L...>::unique::size;
        };
        template <typename T>
        struct dummy_check
        {
            using type = T;
        };
        template <typename ... L>
        struct intersect_detail;
        template <typename ... T>
        struct intersect_detail<unchecked_list<>, unchecked_list<T...>>
        {
            constexpr static size_t size = 0;
        };
        template <typename V, typename ... L, typename ... T>
        struct intersect_detail<unchecked_list<V, L...>, unchecked_list<T...>>
        {
            constexpr static size_t size = intersect_detail<unchecked_list<L...>, unchecked_list<T...>>::size +
                    (not std::is_void_v<V> && contains_detail<dummy_check, V, T...>::value);
        };
        template <size_t Mine, size_t Cross, size_t Theirs>
        struct is_cross
        {
            constexpr static bool value = Cross > 0;
        };
        template <size_t Mine, size_t Cross, size_t Theirs>
        struct is_super
        {
            constexpr static bool value = Cross == Theirs;
        };
        template <size_t Mine, size_t Cross, size_t Theirs> struct is_same
        {
            constexpr static bool value = Mine == Cross && Cross == Theirs;
        };

        template <template <typename> typename Check, typename ... L>
        struct unique;
        template <template <typename> typename Check>
        struct unique<Check>
        {
            using type = set<Check>;
            constexpr static size_t size = 0;

            template <template <size_t, size_t, size_t> typename How, typename U, size_t Size>
            constexpr static bool compare = How<0, 0, Size>::value;

            template <typename U>
            constexpr static bool test = contains_detail<Check, U>::value;
        };
        template <template <typename> typename Check, typename T, typename ... L>
        struct unique<Check, T, L...> {
            using type = typename std::conditional<
                    contains_detail<Check, T, L...>::value,
                    typename unique<Check, L...>::type,
                    typename insert_detail<Check, T, typename unique<Check, L...>::type>::type
            >::type;
            constexpr static size_t size =
                    contains_detail<Check, T, L...>::value
                    ? unique<Check, L...>::size
                    : unique<Check, L...>::size + 1;
            template <template <size_t, size_t, size_t>
                    typename How, typename U, size_t Size>
            constexpr static bool compare =
                    How<size, intersect_detail<unchecked_list<T, L...>, U>::size,Size>::value;
            template <typename U>
            constexpr static bool test = contains_detail<Check, U, T, L...>::value;
        };
    }

    template <template <typename> typename Check,typename ... L>
    class set {
        using impl = set_impl::unique<Check, L...>;
    public:
        constexpr explicit set() = default;
        using unique = typename impl::type;

        constexpr static size_t size = impl::size;
        constexpr static bool empty = size == 0;

        template <typename T>
        constexpr static bool is_same =
                impl::template compare<set_impl::is_same,
                        typename set_impl::cracker<T>::list,
                        set_impl::cracker<T>::size>;

        template <typename T>
        constexpr static bool is_cross =
                impl::template compare<set_impl::is_cross,
                        typename set_impl::cracker<T>::list,
                        set_impl::cracker<T>::size>;

        template <typename T>
        constexpr static bool is_super =
                impl::template compare<set_impl::is_super,
                        typename set_impl::cracker<T>::list,
                        set_impl::cracker<T>::size>;

        template <typename T>
        constexpr static bool test =
                impl::template test<T>;

        template <typename T>
        using type =
        typename std::enable_if<(!std::is_void<T>::value
                                 && test<T>), typename Check<T>::type>::type;

        template <typename T>
        using insert = typename
        set_impl::unique<Check, T, L...>::type;
    };

    template<template<typename...> class T, typename>
    struct instantiate_with_set;

    template<template<typename...> class T, template <typename> typename Check, typename... Ts>
    struct instantiate_with_set<T, detail::set<Check, Ts...>>
    {
        using type = T<Ts...>;
    };
}