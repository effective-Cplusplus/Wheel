#ifndef traits_h__
#define traits_h__

#include <type_traits>
#include <vector>
#include <map>
#include <unordered_map>
#include <deque>
#include <queue>
#include <list>
#include <cstddef>

namespace wheel {
	namespace traits {
		template< class T >
		struct is_signed_intergral_like : std::integral_constant < bool,
			(std::is_integral<T>::value) &&
			std::is_signed<T>::value
		> {};

		template< class T >
		struct is_unsigned_intergral_like : std::integral_constant < bool,
			(std::is_integral<T>::value) &&
			std::is_unsigned<T>::value
		> {};

		template < template <typename...> class U, typename T >
		struct is_template_instant_of : std::false_type {};

		template < template <typename...> class U, typename... args >
		struct is_template_instant_of< U, U<args...> > : std::true_type {};

		template<typename T>
		struct is_stdstring : is_template_instant_of < std::basic_string, T >
		{};

		template<typename T>
		struct is_tuple : is_template_instant_of < std::tuple, T >
		{};

		template< class T >
		struct is_sequence_container : std::integral_constant < bool,
			is_template_instant_of<std::deque, T>::value ||
			is_template_instant_of<std::list, T>::value ||
			is_template_instant_of<std::vector, T>::value ||
			is_template_instant_of<std::queue, T>::value
		> {};

		template< class T >
		struct is_associat_container : std::integral_constant < bool,
			is_template_instant_of<std::map, T>::value ||
			is_template_instant_of<std::unordered_map, T>::value
		> {};

		template< class T >
		struct is_emplace_back_able : std::integral_constant < bool,
			is_template_instant_of<std::deque, T>::value ||
			is_template_instant_of<std::list, T>::value ||
			is_template_instant_of<std::vector, T>::value
		> {};

		template<class...> struct disjunction : std::false_type { };
		template<class B1> struct disjunction<B1> : B1 { };

		template< bool B, class T, class F >
		using conditional_t = typename std::conditional<B, T, F>::type;

		template<class B1, class... Bn>
		struct disjunction<B1, Bn...>
			: conditional_t<bool(B1::value), B1, disjunction<Bn...>> { };

		template <typename T, typename Tuple>
		struct has_type;

		template <typename T, typename... Us>
		struct has_type<T, std::tuple<Us...>> : disjunction<std::is_same<T, Us>...> {};

		struct nonesuch {
			nonesuch() = delete;
			~nonesuch() = delete;
			nonesuch(const nonesuch&) = delete;
			void operator=(const nonesuch&) = delete;
		};

		template<class Default, class AlwaysVoid,
			template<class...> class Op, class... Args>
		struct detector {
			using value_t = std::false_type;
			using type = Default;
		};

		template <typename ...Ts> struct make_void
		{
			using type = void;
		};
		template <typename ...Ts> using void_t = typename make_void<Ts...>::type;

		template<class Default, template<class...> class Op, class... Args>
		struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
			using value_t = std::true_type;
			using type = Op<Args...>;
		};

		template<template<class...> class Op, class... Args>
		using is_detected = typename detector<nonesuch, void, Op, Args...>::value_t;

		template<template<class...> class Op, class... Args>
		using detected_t = typename detector<nonesuch, void, Op, Args...>::type;

		template<class T, typename... Args>
		using has_before_t = decltype(std::declval<T>().before(std::declval<Args>()...));

		template<class T, typename... Args>
		using has_after_t = decltype(std::declval<T>().after(std::declval<Args>()...));
		template<typename T, typename... Args>
		using has_before = is_detected<has_before_t, T, Args...>;

		template<typename T, typename... Args>
		using has_after = is_detected<has_after_t, T, Args...>;

		//c++17写法
		//template <typename T, typename... Us>
		//struct has_type<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...> {}

		template<typename T>
		constexpr bool  is_int64_v = std::is_same<T, int64_t>::value || std::is_same<T, uint64_t>::value;

		template< class T >
		using remove_pointer_t = typename std::remove_pointer<T>::type;

		template< class T >
		using remove_reference_t = typename std::remove_reference<T>::type;

		template< class T >
		using decay_t = typename std::decay<T>::type;

		template<class T>
		constexpr bool is_char_array_v = std::is_array<T>::value && std::is_same<char,remove_pointer_t<decay_t<T>>>::value;

		template<typename T>
		struct is_string
			: public disjunction<std::is_same<char*, typename std::decay<T>::type>,
			std::is_same<const char*, typename std::decay<T>::type>,
			std::is_same<std::string, typename std::decay<T>::type>
			> {

		};

		/**************************make_integer_sequence*****************************************************/

		template<typename T, T... Ints>
		struct integer_sequence
		{
			static_assert(std::is_integral<T>::value, "integer_sequence<T, I...> requires T to be an integral type.");
			using value_type =T;
			static constexpr std::size_t size() { return sizeof...(Ints); }
		};

		template<std::size_t... Ints>
		using index_sequence = integer_sequence<std::size_t, Ints...>;

		template<typename T, std::size_t N, T... Is>
		struct make_integer_sequence : make_integer_sequence<T, N - 1, N - 1, Is...> {};

		template<typename T, T... Is>
		struct make_integer_sequence<T, 0, Is...> : integer_sequence<T, Is...> {};

		template<std::size_t N>
		using make_index_sequence = make_integer_sequence<std::size_t, N>;

		template<typename... T>
		using index_sequence_for = make_index_sequence<sizeof...(T)>;
		/******************************smart pointer***************************************************************/

// STRUCT TEMPLATE enable_if
		template <bool _Test, class _Ty = void>
		struct enable_if {}; // no member "type" when !_Test

		template <class _Ty>
		struct enable_if<true, _Ty> { // type is _Ty for _Test
			using type = _Ty;
		};

		template <bool _Test, class _Ty = void>
		using enable_if_t = typename enable_if<_Test, _Ty>::type;

		template< class T >
		using remove_extent_t = typename std::remove_extent<T>::type;

		// FUNCTION TEMPLATE make_unique
		template <class _Ty, class... _Types, enable_if_t<!std::is_array<_Ty>::value, int> = 0>
		std::unique_ptr<_Ty> make_unique(_Types&&... _Args) { // make a unique_ptr
			return std::unique_ptr<_Ty>(new _Ty(std::forward<_Types>(_Args)...));
		}

		template <class _Ty, enable_if_t<std::is_array<_Ty>::value && std::extent<_Ty>::value == 0, int> = 0>
		std::unique_ptr<_Ty> make_unique(size_t _Size) { // make a unique_ptr
			using _Elem = remove_extent_t<_Ty>;
			return std::unique_ptr<_Ty>(new _Elem[_Size]());
		}

		template <class _Ty, class... _Types, enable_if_t<std::extent<_Ty>::value != 0, int> = 0>
		void make_unique(_Types&&...) = delete;


	}//traits
}//wheel

#endif // traits_h__