#ifndef mysql_define_h__
#define mysql_define_h__
#include "reflection.hpp"

namespace wheel {
	namespace mysql {
		template <class T>
		struct identity {
		};

#define REGISTER_TYPE(Type, Index)                                              \
    inline constexpr int type_to_id(identity<Type>) noexcept { return Index; } \
    inline constexpr auto id_to_type( std::integral_constant<std::size_t, Index > ) noexcept { Type res{}; return res; }

		REGISTER_TYPE(char, MYSQL_TYPE_TINY)
			REGISTER_TYPE(short, MYSQL_TYPE_SHORT)
			REGISTER_TYPE(int, MYSQL_TYPE_LONG)
			REGISTER_TYPE(float, MYSQL_TYPE_FLOAT)
			REGISTER_TYPE(double, MYSQL_TYPE_DOUBLE)
			REGISTER_TYPE(int64_t, MYSQL_TYPE_LONGLONG)

			inline int type_to_id(identity<std::string>) noexcept { return MYSQL_TYPE_VAR_STRING; }
		inline std::string id_to_type(std::integral_constant<std::size_t, MYSQL_TYPE_VAR_STRING >) noexcept { std::string res{}; return res; }

		inline constexpr auto type_to_name(identity<char>) noexcept { return "TINYINT"; }
		inline constexpr auto type_to_name(identity<short>) noexcept { return "SMALLINT"; }
		inline constexpr auto type_to_name(identity<int>) noexcept { return "INTEGER"; }
		inline constexpr auto type_to_name(identity<float>) noexcept { return "FLOAT"; }
		inline constexpr auto type_to_name(identity<double>) noexcept { return "DOUBLE"; }
		inline constexpr auto type_to_name(identity<int64_t>) noexcept { return "BIGINT"; }
		inline auto type_to_name(identity<std::string>) noexcept { return "TEXT"; }
		template<size_t N>
		inline constexpr auto type_to_name(identity<std::array<char, N>>) noexcept {
			std::string s = "varchar(" + std::to_string(N) + ")";
			return s;
		}

		template <typename ... Args>
		struct value_of;

		template <typename T>
		struct value_of<T> {
			static  constexpr auto value = (wheel::reflector::get_size<T>());
		};

		template < typename T, typename ... Rest >
		struct value_of < T, Rest ... > {
			static  constexpr auto value = (value_of<T>::value + value_of<Rest...>::value);
		};

		template<typename List>
		struct result_size;

		template<template<class...> class List, class... T>
		struct result_size<List<T...>> {
			constexpr static const size_t value = value_of<T...>::value;// (wheel::reflector::get_size<T>() + ...);
		};

		enum  filedtype_t
		{
			CHAR = 1, INT = 2, DATETIME = 3, DOUBLE = 4, DEC = 5, UNKNOWN = 6, BIN
		};

		filedtype_t set_field_type(enum_field_types fieldtype)
		{
			filedtype_t type;
			switch (fieldtype)
			{
			case MYSQL_TYPE_BLOB:
				type = BIN;
				break;
			case MYSQL_TYPE_STRING:
				//
			case MYSQL_TYPE_VAR_STRING:
				//
			//case MYSQL_TYPE_TEXT:
				//
			case MYSQL_TYPE_SET:
				//
			case MYSQL_TYPE_GEOMETRY:
				//
			case MYSQL_TYPE_NULL:
				type = CHAR;
				break;
			case MYSQL_TYPE_TINY:
				//
			case MYSQL_TYPE_SHORT:
				//
			case MYSQL_TYPE_LONG:
				//
			case MYSQL_TYPE_INT24:
				//
			case MYSQL_TYPE_BIT:
				//
			case MYSQL_TYPE_ENUM:
				//
			case MYSQL_TYPE_YEAR:
			case MYSQL_TYPE_LONGLONG:
				type = INT;
				break;
			case MYSQL_TYPE_DECIMAL:
				//
			case MYSQL_TYPE_NEWDECIMAL:
				type = DEC;
				break;
			case MYSQL_TYPE_FLOAT:
				//
			case MYSQL_TYPE_DOUBLE:
				type = DOUBLE;
				break;
			case MYSQL_TYPE_TIMESTAMP:
				//
			case MYSQL_TYPE_DATE:
				//
			case MYSQL_TYPE_TIME:
				type = DATETIME;
				break;
			default:
				type = UNKNOWN;
				break;
			}
			return type;
		}

		struct typeset_t
		{
		public:
			typeset_t() :type_(UNKNOWN), length_(0), index_(0) {}

			typeset_t(filedtype_t type, unsigned int length, unsigned int index, const std::string& name)
				:type_(type), length_(length), index_(index), name_(name)
			{

			}

			filedtype_t type_;

			unsigned int length_;

			//查询列表中的列位置
			unsigned int index_;

			std::string name_;
		};

		typedef std::vector<typeset_t> field_t;

		typedef std::vector<std::string> row_t;
	}
}//wheel

#endif // mysql_define_h__
