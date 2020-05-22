#ifndef reflection_h__
#define reflection_h__
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <map>
#include <vector>
#include <array>
#include <type_traits>
#include <functional>
#include "unit.hpp"
#include "traits.hpp"
#include "itoa.hpp"

namespace wheel {
	namespace reflector {
#define place_holder 124,123,122,121,120,119,118,117,116,115,114,113,112,111,110,109,108,107,106,105,104,103,102,101,100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
#define expand_marco(...) __VA_ARGS__
#define concat_a_b_(a,b) a##b
#define concat_a_b(a,b) concat_a_b_(a,b)

#ifdef  _MSC_VER

#define inner_var(...) unused,__VA_ARGS__
#define get_count_(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65,_66,_67,_68,_69,_70,_71,_72,_73,_74,_75,_76,_77,_78,_79,_80,_81,_82,_83,_84,_85,_86,_87,_88,_89,_90,_91,_92,_93,_94,_95,_96,_97,_98,_99,_100,_101,_102,_103,_104,_105,_106,_107,_108,_109,_110,_111,_112,_113,_114,_115,_116,_117,_118,_119,_120,_121,_122,_123,_124,count,...) count
#define get_count_help(...) expand_marco(get_count_(__VA_ARGS__))
#define get_count(...) get_count_help(inner_var(__VA_ARGS__),place_holder)

#else //  _NOT_MSC_VER
#define get_count_(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65,_66,_67,_68,_69,_70,_71,_72,_73,_74,_75,_76,_77,_78,_79,_80,_81,_82,_83,_84,_85,_86,_87,_88,_89,_90,_91,_92,_93,_94,_95,_96,_97,_98,_99,_100,_101,_102,_103,_104,_105,_106,_107,_108,_109,_110,_111,_112,_113,_114,_115,_116,_117,_118,_119,_120,_121,_122,_123,_124,count,...) count
#define get_count_help(...) get_count_(__VA_ARGS__)
#define get_count(...) get_count_help(0,## __VA_ARGS__,place_holder)
#endif

#define For_1(method,ClassName,element,...) method(ClassName,element)
#define For_2(method,ClassName,element,...) method(ClassName,element),expand_marco(For_1(method,ClassName,__VA_ARGS__))
#define For_3(method,ClassName,element,...) method(ClassName,element),expand_marco(For_2(method,ClassName,__VA_ARGS__))
#define For_4(method,ClassName,element,...) method(ClassName,element),expand_marco(For_3(method,ClassName,__VA_ARGS__))
#define For_5(method,ClassName,element,...) method(ClassName,element),expand_marco(For_4(method,ClassName,__VA_ARGS__))
#define For_6(method,ClassName,element,...) method(ClassName,element),expand_marco(For_5(method,ClassName,__VA_ARGS__))
#define For_7(method,ClassName,element,...) method(ClassName,element),expand_marco(For_6(method,ClassName,__VA_ARGS__))
#define For_8(method,ClassName,element,...) method(ClassName,element),expand_marco(For_7(method,ClassName,__VA_ARGS__))
#define For_9(method,ClassName,element,...) method(ClassName,element),expand_marco(For_8(method,ClassName,__VA_ARGS__))
#define For_10(method,ClassName,element,...) method(ClassName,element),expand_marco(For_9(method,ClassName,__VA_ARGS__))
#define For_11(method,ClassName,element,...) method(ClassName,element),expand_marco(For_10(method,ClassName,__VA_ARGS__))
#define For_12(method,ClassName,element,...) method(ClassName,element),expand_marco(For_11(method,ClassName,__VA_ARGS__))
#define For_13(method,ClassName,element,...) method(ClassName,element),expand_marco(For_12(method,ClassName,__VA_ARGS__))
#define For_14(method,ClassName,element,...) method(ClassName,element),expand_marco(For_13(method,ClassName,__VA_ARGS__))
#define For_15(method,ClassName,element,...) method(ClassName,element),expand_marco(For_14(method,ClassName,__VA_ARGS__))
#define For_16(method,ClassName,element,...) method(ClassName,element),expand_marco(For_15(method,ClassName,__VA_ARGS__))
#define For_17(method,ClassName,element,...) method(ClassName,element),expand_marco(For_16(method,ClassName,__VA_ARGS__))
#define For_18(method,ClassName,element,...) method(ClassName,element),expand_marco(For_17(method,ClassName,__VA_ARGS__))
#define For_19(method,ClassName,element,...) method(ClassName,element),expand_marco(For_18(method,ClassName,__VA_ARGS__))
#define For_20(method,ClassName,element,...) method(ClassName,element),expand_marco(For_19(method,ClassName,__VA_ARGS__))
#define For_21(method,ClassName,element,...) method(ClassName,element),expand_marco(For_20(method,ClassName,__VA_ARGS__))
#define For_22(method,ClassName,element,...) method(ClassName,element),expand_marco(For_21(method,ClassName,__VA_ARGS__))
#define For_23(method,ClassName,element,...) method(ClassName,element),expand_marco(For_22(method,ClassName,__VA_ARGS__))
#define For_24(method,ClassName,element,...) method(ClassName,element),expand_marco(For_23(method,ClassName,__VA_ARGS__))
#define For_25(method,ClassName,element,...) method(ClassName,element),expand_marco(For_24(method,ClassName,__VA_ARGS__))
#define For_26(method,ClassName,element,...) method(ClassName,element),expand_marco(For_25(method,ClassName,__VA_ARGS__))
#define For_27(method,ClassName,element,...) method(ClassName,element),expand_marco(For_26(method,ClassName,__VA_ARGS__))
#define For_28(method,ClassName,element,...) method(ClassName,element),expand_marco(For_27(method,ClassName,__VA_ARGS__))
#define For_29(method,ClassName,element,...) method(ClassName,element),expand_marco(For_28(method,ClassName,__VA_ARGS__))
#define For_30(method,ClassName,element,...) method(ClassName,element),expand_marco(For_29(method,ClassName,__VA_ARGS__))
#define For_31(method,ClassName,element,...) method(ClassName,element),expand_marco(For_30(method,ClassName,__VA_ARGS__))
#define For_32(method,ClassName,element,...) method(ClassName,element),expand_marco(For_31(method,ClassName,__VA_ARGS__))
#define For_33(method,ClassName,element,...) method(ClassName,element),expand_marco(For_32(method,ClassName,__VA_ARGS__))
#define For_34(method,ClassName,element,...) method(ClassName,element),expand_marco(For_33(method,ClassName,__VA_ARGS__))
#define For_35(method,ClassName,element,...) method(ClassName,element),expand_marco(For_34(method,ClassName,__VA_ARGS__))
#define For_36(method,ClassName,element,...) method(ClassName,element),expand_marco(For_35(method,ClassName,__VA_ARGS__))
#define For_37(method,ClassName,element,...) method(ClassName,element),expand_marco(For_36(method,ClassName,__VA_ARGS__))
#define For_38(method,ClassName,element,...) method(ClassName,element),expand_marco(For_37(method,ClassName,__VA_ARGS__))
#define For_39(method,ClassName,element,...) method(ClassName,element),expand_marco(For_38(method,ClassName,__VA_ARGS__))
#define For_40(method,ClassName,element,...) method(ClassName,element),expand_marco(For_39(method,ClassName,__VA_ARGS__))
#define For_41(method,ClassName,element,...) method(ClassName,element),expand_marco(For_40(method,ClassName,__VA_ARGS__))
#define For_42(method,ClassName,element,...) method(ClassName,element),expand_marco(For_41(method,ClassName,__VA_ARGS__))
#define For_43(method,ClassName,element,...) method(ClassName,element),expand_marco(For_42(method,ClassName,__VA_ARGS__))
#define For_44(method,ClassName,element,...) method(ClassName,element),expand_marco(For_43(method,ClassName,__VA_ARGS__))
#define For_45(method,ClassName,element,...) method(ClassName,element),expand_marco(For_44(method,ClassName,__VA_ARGS__))
#define For_46(method,ClassName,element,...) method(ClassName,element),expand_marco(For_45(method,ClassName,__VA_ARGS__))
#define For_47(method,ClassName,element,...) method(ClassName,element),expand_marco(For_46(method,ClassName,__VA_ARGS__))
#define For_48(method,ClassName,element,...) method(ClassName,element),expand_marco(For_47(method,ClassName,__VA_ARGS__))
#define For_49(method,ClassName,element,...) method(ClassName,element),expand_marco(For_48(method,ClassName,__VA_ARGS__))
#define For_50(method,ClassName,element,...) method(ClassName,element),expand_marco(For_49(method,ClassName,__VA_ARGS__))
#define For_51(method,ClassName,element,...) method(ClassName,element),expand_marco(For_50(method,ClassName,__VA_ARGS__))
#define For_52(method,ClassName,element,...) method(ClassName,element),expand_marco(For_51(method,ClassName,__VA_ARGS__))
#define For_53(method,ClassName,element,...) method(ClassName,element),expand_marco(For_52(method,ClassName,__VA_ARGS__))
#define For_54(method,ClassName,element,...) method(ClassName,element),expand_marco(For_53(method,ClassName,__VA_ARGS__))
#define For_55(method,ClassName,element,...) method(ClassName,element),expand_marco(For_54(method,ClassName,__VA_ARGS__))
#define For_56(method,ClassName,element,...) method(ClassName,element),expand_marco(For_55(method,ClassName,__VA_ARGS__))
#define For_57(method,ClassName,element,...) method(ClassName,element),expand_marco(For_56(method,ClassName,__VA_ARGS__))
#define For_58(method,ClassName,element,...) method(ClassName,element),expand_marco(For_57(method,ClassName,__VA_ARGS__))
#define For_59(method,ClassName,element,...) method(ClassName,element),expand_marco(For_58(method,ClassName,__VA_ARGS__))
#define For_60(method,ClassName,element,...) method(ClassName,element),expand_marco(For_59(method,ClassName,__VA_ARGS__))
#define For_61(method,ClassName,element,...) method(ClassName,element),expand_marco(For_60(method,ClassName,__VA_ARGS__))
#define For_62(method,ClassName,element,...) method(ClassName,element),expand_marco(For_61(method,ClassName,__VA_ARGS__))
#define For_63(method,ClassName,element,...) method(ClassName,element),expand_marco(For_62(method,ClassName,__VA_ARGS__))
#define For_64(method,ClassName,element,...) method(ClassName,element),expand_marco(For_63(method,ClassName,__VA_ARGS__))
#define For_65(method,ClassName,element,...) method(ClassName,element),expand_marco(For_64(method,ClassName,__VA_ARGS__))
#define For_66(method,ClassName,element,...) method(ClassName,element),expand_marco(For_65(method,ClassName,__VA_ARGS__))
#define For_67(method,ClassName,element,...) method(ClassName,element),expand_marco(For_66(method,ClassName,__VA_ARGS__))
#define For_68(method,ClassName,element,...) method(ClassName,element),expand_marco(For_67(method,ClassName,__VA_ARGS__))
#define For_69(method,ClassName,element,...) method(ClassName,element),expand_marco(For_68(method,ClassName,__VA_ARGS__))
#define For_70(method,ClassName,element,...) method(ClassName,element),expand_marco(For_69(method,ClassName,__VA_ARGS__))
#define For_71(method,ClassName,element,...) method(ClassName,element),expand_marco(For_70(method,ClassName,__VA_ARGS__))
#define For_72(method,ClassName,element,...) method(ClassName,element),expand_marco(For_71(method,ClassName,__VA_ARGS__))
#define For_73(method,ClassName,element,...) method(ClassName,element),expand_marco(For_72(method,ClassName,__VA_ARGS__))
#define For_74(method,ClassName,element,...) method(ClassName,element),expand_marco(For_73(method,ClassName,__VA_ARGS__))
#define For_75(method,ClassName,element,...) method(ClassName,element),expand_marco(For_74(method,ClassName,__VA_ARGS__))
#define For_76(method,ClassName,element,...) method(ClassName,element),expand_marco(For_75(method,ClassName,__VA_ARGS__))
#define For_77(method,ClassName,element,...) method(ClassName,element),expand_marco(For_76(method,ClassName,__VA_ARGS__))
#define For_78(method,ClassName,element,...) method(ClassName,element),expand_marco(For_77(method,ClassName,__VA_ARGS__))
#define For_79(method,ClassName,element,...) method(ClassName,element),expand_marco(For_78(method,ClassName,__VA_ARGS__))
#define For_80(method,ClassName,element,...) method(ClassName,element),expand_marco(For_79(method,ClassName,__VA_ARGS__))
#define For_81(method,ClassName,element,...) method(ClassName,element),expand_marco(For_80(method,ClassName,__VA_ARGS__))
#define For_82(method,ClassName,element,...) method(ClassName,element),expand_marco(For_81(method,ClassName,__VA_ARGS__))
#define For_83(method,ClassName,element,...) method(ClassName,element),expand_marco(For_82(method,ClassName,__VA_ARGS__))
#define For_84(method,ClassName,element,...) method(ClassName,element),expand_marco(For_83(method,ClassName,__VA_ARGS__))
#define For_85(method,ClassName,element,...) method(ClassName,element),expand_marco(For_84(method,ClassName,__VA_ARGS__))
#define For_86(method,ClassName,element,...) method(ClassName,element),expand_marco(For_85(method,ClassName,__VA_ARGS__))
#define For_87(method,ClassName,element,...) method(ClassName,element),expand_marco(For_86(method,ClassName,__VA_ARGS__))
#define For_88(method,ClassName,element,...) method(ClassName,element),expand_marco(For_87(method,ClassName,__VA_ARGS__))
#define For_89(method,ClassName,element,...) method(ClassName,element),expand_marco(For_88(method,ClassName,__VA_ARGS__))
#define For_90(method,ClassName,element,...) method(ClassName,element),expand_marco(For_89(method,ClassName,__VA_ARGS__))
#define For_91(method,ClassName,element,...) method(ClassName,element),expand_marco(For_90(method,ClassName,__VA_ARGS__))
#define For_92(method,ClassName,element,...) method(ClassName,element),expand_marco(For_91(method,ClassName,__VA_ARGS__))
#define For_93(method,ClassName,element,...) method(ClassName,element),expand_marco(For_92(method,ClassName,__VA_ARGS__))
#define For_94(method,ClassName,element,...) method(ClassName,element),expand_marco(For_93(method,ClassName,__VA_ARGS__))
#define For_95(method,ClassName,element,...) method(ClassName,element),expand_marco(For_94(method,ClassName,__VA_ARGS__))
#define For_96(method,ClassName,element,...) method(ClassName,element),expand_marco(For_95(method,ClassName,__VA_ARGS__))
#define For_97(method,ClassName,element,...) method(ClassName,element),expand_marco(For_96(method,ClassName,__VA_ARGS__))
#define For_98(method,ClassName,element,...) method(ClassName,element),expand_marco(For_97(method,ClassName,__VA_ARGS__))
#define For_99(method,ClassName,element,...) method(ClassName,element),expand_marco(For_98(method,ClassName,__VA_ARGS__))
#define For_100(method,ClassName,element,...) method(ClassName,element),expand_marco(For_99(method,ClassName,__VA_ARGS__))
#define For_101(method,ClassName,element,...) method(ClassName,element),expand_marco(For_100(method,ClassName,__VA_ARGS__))
#define For_102(method,ClassName,element,...) method(ClassName,element),expand_marco(For_101(method,ClassName,__VA_ARGS__))
#define For_103(method,ClassName,element,...) method(ClassName,element),expand_marco(For_102(method,ClassName,__VA_ARGS__))
#define For_104(method,ClassName,element,...) method(ClassName,element),expand_marco(For_103(method,ClassName,__VA_ARGS__))
#define For_105(method,ClassName,element,...) method(ClassName,element),expand_marco(For_104(method,ClassName,__VA_ARGS__))
#define For_106(method,ClassName,element,...) method(ClassName,element),expand_marco(For_105(method,ClassName,__VA_ARGS__))
#define For_107(method,ClassName,element,...) method(ClassName,element),expand_marco(For_106(method,ClassName,__VA_ARGS__))
#define For_108(method,ClassName,element,...) method(ClassName,element),expand_marco(For_107(method,ClassName,__VA_ARGS__))
#define For_109(method,ClassName,element,...) method(ClassName,element),expand_marco(For_108(method,ClassName,__VA_ARGS__))
#define For_110(method,ClassName,element,...) method(ClassName,element),expand_marco(For_109(method,ClassName,__VA_ARGS__))
#define For_111(method,ClassName,element,...) method(ClassName,element),expand_marco(For_110(method,ClassName,__VA_ARGS__))
#define For_112(method,ClassName,element,...) method(ClassName,element),expand_marco(For_111(method,ClassName,__VA_ARGS__))
#define For_113(method,ClassName,element,...) method(ClassName,element),expand_marco(For_112(method,ClassName,__VA_ARGS__))
#define For_114(method,ClassName,element,...) method(ClassName,element),expand_marco(For_113(method,ClassName,__VA_ARGS__))
#define For_115(method,ClassName,element,...) method(ClassName,element),expand_marco(For_114(method,ClassName,__VA_ARGS__))
#define For_116(method,ClassName,element,...) method(ClassName,element),expand_marco(For_115(method,ClassName,__VA_ARGS__))
#define For_117(method,ClassName,element,...) method(ClassName,element),expand_marco(For_116(method,ClassName,__VA_ARGS__))
#define For_118(method,ClassName,element,...) method(ClassName,element),expand_marco(For_117(method,ClassName,__VA_ARGS__))
#define For_119(method,ClassName,element,...) method(ClassName,element),expand_marco(For_118(method,ClassName,__VA_ARGS__))
#define For_120(method,ClassName,element,...) method(ClassName,element),expand_marco(For_119(method,ClassName,__VA_ARGS__))
#define For_121(method,ClassName,element,...) method(ClassName,element),expand_marco(For_120(method,ClassName,__VA_ARGS__))
#define For_122(method,ClassName,element,...) method(ClassName,element),expand_marco(For_121(method,ClassName,__VA_ARGS__))
#define For_123(method,ClassName,element,...) method(ClassName,element),expand_marco(For_122(method,ClassName,__VA_ARGS__))
#define For_124(method,ClassName,element,...) method(ClassName,element),expand_marco(For_123(method,ClassName,__VA_ARGS__))
#define For_125(method,ClassName,element,...) method(ClassName,element),expand_marco(For_124(method,ClassName,__VA_ARGS__))

#define address_macro(Class,Element) &Class::Element
#define element_name_macro(Class,Element) #Element

#define GENERATOR_META(N,ClassName,...) \
static std::array<std::string,N> arr_##ClassName = { expand_marco(concat_a_b(For_,N)(element_name_macro,ClassName,__VA_ARGS__)) }; \
static auto reflector_reflect_members(ClassName const&) \
{ \
	struct reflect_members \
	{   \
        constexpr decltype(auto) static apply_impl(){\
            return std::make_tuple(expand_marco(concat_a_b(For_,N)(address_macro,ClassName,__VA_ARGS__)));\
        }\
       using type = void;\
        static std::string name() \
        { \
            return #ClassName ; \
        } \
        using size_type = std::integral_constant<size_t, get_count(__VA_ARGS__)>; \
        constexpr static std::size_t size() { return size_type::value; } \
        static std::array<std::string,size_type::value> arr()\
		 { \
             return  arr_##ClassName ; \
		 } \
    }; \
    return reflect_members{}; \
}

#define REFLECTION(ClassName,...) GENERATOR_META(get_count(__VA_ARGS__),ClassName,__VA_ARGS__)
		template<typename T>
		using Reflect_members = decltype(reflector_reflect_members(std::declval<T>()));

		template <typename T, typename = void>
		struct is_reflection : std::false_type
		{
		};

		template< class ... >
		using void_t = void;

		template <typename T>
		struct is_reflection<T, void_t<typename Reflect_members<T>::type>> : std::true_type
		{
		};

		template<typename T>
		constexpr bool is_reflection_v = is_reflection<T>::value;

		template<typename T>
		constexpr auto get_array() {
			//std::declval 模板推导
			using M = decltype(reflector_reflect_members(std::declval<T>()));
			return M::arr();
		}

		//获取结构体的值
		template<size_t I, typename T>
		constexpr decltype(auto) get(T&& t) {
			using M = decltype(reflector_reflect_members(std::forward<T>(t)));

			return std::forward<T>(t).*(std::get<I>(M::apply_impl()));
		}

		template <typename T, size_t ... Is>
		constexpr auto get_impl(T const& t, std::index_sequence<Is...>) {
			return std::make_tuple(get<Is>(t)...);
		}

		template <typename T, size_t ... Is>
		constexpr auto get_impl(T& t, std::index_sequence<Is...>) {
			return std::make_tuple(std::ref(get<Is>(t))...);
		}

		template<typename T>
		constexpr auto get_impl(T&& t) {
			using M = decltype(reflector_reflect_members(std::forward<T>(t)));

			return M::apply_impl();
		}

		template <typename T>
		constexpr auto get(T const& t) {
			using M = decltype(reflector_reflect_members(t));
			return get_impl(t, wheel::traits::make_index_sequence<M::size()>{});
		}

		template<typename T, size_t  I>
		constexpr const std::string get_name() {
			using M = decltype(reflector_reflect_members(std::declval<T>()));
			static_assert(I < M::size(), "out of range");
			return M::arr()[I];
		}

		template<typename T>
		constexpr const std::string get_name(size_t i) {
			using M = decltype(reflector_reflect_members(std::declval<T>()));
			return M::arr()[i];
		}

		template<typename T>
		constexpr const std::string get_name()
		{
			using M = decltype(reflector_reflect_members(std::declval<T>()));
			return M::name();
		}

		template<typename T>
		//等价c++17写法 std::enable_if_t<is_reflection<T>::value, size_t>::type
		constexpr typename std::enable_if<is_reflection<T>::value, size_t>::type get_size() {
			using M = decltype(reflector_reflect_members(std::declval<T>()));
			return M::size();
		}

		template<typename T>
		constexpr std::enable_if_t<!is_reflection<T>::value, size_t> get_size()
		{
			std::cout << "reflect is error" << std::endl;
			return 1;
		}

		template<typename T>
		constexpr auto get_index(const std::string& name) {
			using M = decltype(reflector_reflect_members(std::declval<T>()));
			const auto arr = M::arr();

			auto it = std::find_if(arr.begin(), arr.end(), [name](auto str) {
				return (str == name);
				});

			//迭代器转下表
			return std::distance(arr.begin(), it);
		}

		//可用数组展开循环
		//也可以函数递归的形式
		template<typename T, typename F>
		void  for_each_tuple_front(T&& t, F&& f) {
			using M = decltype(reflector_reflect_members(std::forward<T>(t)));
			unit::for_each_tuple_front(std::move(M::apply_impl()), std::forward<F>(f), wheel::traits::make_index_sequence<M::size()>{});
		}

		template<typename T, typename F>
		void  for_each_tuple_back(T&& t, F&& f) {
			using M = decltype(reflector_reflect_members(std::forward<T>(t)));
			unit::for_each_tuple_back(std::move(M::apply_impl()), std::forward<F>(f), wheel::traits::make_index_sequence<M::size()>{});
		}

		template <typename... Args, typename F, std::size_t... Idx>
		constexpr void for_each(std::tuple<Args...>& t, F&& f, wheel::traits::index_sequence<Idx...>) {
			using expander = int[];

			(void)expander {
				((std::forward<F>(f)(std::get<Idx>(t), std::integral_constant<size_t, Idx>{})), false)...
			};
		}

		template <typename... Args, typename F, std::size_t... Idx>
		constexpr void for_each(const std::tuple<Args...>& t, F&& f, wheel::traits::index_sequence<Idx...>) {
			using expander = int[];

			(void)expander {
				((std::forward<F>(f)(std::get<Idx>(t), std::integral_constant<size_t, Idx>{})), false)...
			};
		}

		/*********************c++17写法***********************************/
		//lambda函数
		//template <typename... Args, typename F, std::size_t... Idx>
		//constexpr void for_each(const std::tuple<Args...>& t, F&& f, std::index_sequence<Idx...>)
		//{
			 //(std::forward<F>(f)(std::get<Idx>(t), std::integral_constant<size_t, Idx>{}), ...);
		//}

		////加入函数
		//template<typename T, typename F>
		//constexpr void  for_each(T&& t, F&& f) {
		//	using M = decltype(reflector_reflect_members(std::forward<T>(t)));
		//	for_each(M::apply_impl(), std::forward<F>(f), std::make_index_sequence<M::size()>{});
		//}

	}
}
#endif // reflection_h__