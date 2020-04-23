#ifndef utility_h__
#define utility_h__
#include "unit.hpp"
#include "entity.hpp"
#include "reflection.hpp"

#if _MSC_VER
#pragma warning(disable:4984)
#else
#pragma GCC system_header
#endif

namespace wheel {
	namespace mysql {
		template<typename T>
		inline constexpr auto to_str(T&& t) {
			if constexpr (std::is_arithmetic<std::decay_t<T>>::value) {
				return std::to_string(std::forward<T>(t));
			}
			else {
				return std::string("'") + t + std::string("'");
			}
		}

		template<typename... Args>
		inline std::string get_sql(const std::string& str, Args&&... args) {
			using expander = int[];
			std::string sql = str;

			(void)expander {
				((get_str(sql, to_str(std::forward<Args>(args)))), false)...
			};

			return sql;
		}

		inline std::string get_sql(const std::string& str) {
			std::string sql = str;

			return sql;
		}

		inline void get_str(std::string& sql, const std::string& s) {
			auto pos = sql.find_first_of('?');
			sql.replace(pos, 1, " ");
			sql.insert(pos, s);
		}

		template<typename... Args>
		inline auto sort_tuple(const std::tuple<Args...>& tp) {
			if constexpr (sizeof...(Args) == 2) {
				auto [a, b] = tp;
				if constexpr (!std::is_same<decltype(a), wheel_sql_key>::value && !std::is_same<decltype(a), wheel_sql_auto_key>::value) {
					return std::make_tuple(b, a);
				}
			}

			return tp;
		}

		template<typename T, typename... Args >
		std::string generate_createtb_sql(Args&&... args) {
			constexpr auto SIZE = wheel::reflector::get_size<T>();
			std::vector<std::string>type_name_arr;
			type_name_arr.resize(SIZE);

			wheel::reflector::for_each_tuple_front(T{}, [&type_name_arr](auto& item, auto index) {
				constexpr auto Idx = decltype(index)::value;
				using U = std::remove_reference_t<decltype(wheel::reflector::get<Idx>(std::declval<T>()))>;
				std::string str = type_to_name(identity<U>{});
				type_name_arr[Idx] = str;
				});

			using U = std::tuple<std::decay_t <Args>...>;
			constexpr auto ARGS_SIZE = sizeof... (Args);
			if constexpr (ARGS_SIZE > 0) {
				static_assert(!(wheel::traits::has_type<wheel_sql_key, U>::value && wheel::traits::has_type<wheel_sql_auto_key, U>::value), "should only one key");
			}

			const auto name = wheel::reflector::get_name<T>();
			std::string sql = std::string("CREATE TABLE IF NOT EXISTS ") + name.data() + "(";
			auto arr = wheel::reflector::get_array<T>();
			auto tp = sort_tuple(std::make_tuple(std::forward<Args>(args)...));
			const size_t arr_size = arr.size();
			for (size_t i = 0; i < arr_size; ++i) {
				//����������������
				auto field_name = arr[i];
				bool has_add_field = false;
				wheel::unit::for_each0(tp, [&sql, &i, &has_add_field, field_name, type_name_arr, name](auto item) {
					if constexpr (std::is_same<decltype(item), wheel_sql_not_null>::value) {
						if (item.fields.find(field_name.data()) == item.fields.end())
							return;
					}
					else {
						if (item.fields != field_name.data())
							return;
					}

					if constexpr (std::is_same<decltype(item), wheel_sql_not_null>::value) {
						if (!has_add_field) {
							wheel::unit::append(sql, field_name.data(), " ", type_name_arr[i]);
						}
						wheel::unit::append(sql, " NOT NULL");
						has_add_field = true;
					}
					else if constexpr (std::is_same<decltype(item), wheel_sql_key>::value) {
						if (!has_add_field) {
							wheel::unit::append(sql, field_name.data(), " ", type_name_arr[i]);
						}
						wheel::unit::append(sql, " PRIMARY KEY");
						has_add_field = true;
					}
					else if constexpr (std::is_same<decltype(item), wheel_sql_auto_key>::value) {
						if (!has_add_field) {
							wheel::unit::append(sql, field_name.data(), " ", type_name_arr[i]);
						}
						wheel::unit::append(sql, " AUTO_INCREMENT");
						wheel::unit::append(sql, " PRIMARY KEY");
						has_add_field = true;
					}
					else if constexpr (std::is_same<decltype(item), wheel_sql_unique>::value) {
						if (!has_add_field) {
							wheel::unit::append(sql, field_name.data(), " ", type_name_arr[i]);
						}

						wheel::unit::append(sql, ", UNIQUE(", item.fields, ")");
						has_add_field = true;
					}
					else {
						wheel::unit::append(sql, field_name.data(), " ", type_name_arr[i]);
					}
					}, std::make_index_sequence<ARGS_SIZE>{});

				if (!has_add_field) {
					wheel::unit::append(sql, field_name.data(), " ", type_name_arr[i]);
				}

				if (i < arr_size - 1) {
					sql += ", ";
				}
			}

			sql += ")";

			return std::move(sql);
		}


		template<typename T, typename... Args>
		inline std::string generate_query_sql(Args&&... args) {
			std::ostringstream os;
			os << "select ";

			const auto name = wheel::reflector::get_name<T>();
			auto arr = wheel::reflector::get_array<T>();
			int count = arr.size();
			for (int index = 0;index < count;index++)
			{
				os << arr[index];
				if (index < count - 1) {
					os << ",";
				}
			}

			os << " from ";
			std::string sql = os.str();
			wheel::unit::append(sql, name.data());
			get_sql_conditions(sql, std::forward<Args>(args)...);
			return sql;
		}

		inline void get_sql_conditions(std::string& sql) {
		}

		template<typename... Args>
		inline void get_sql_conditions(std::string& sql, const std::string& arg, Args&&... args) {
			if (arg.find("select") != std::string::npos) {
				sql = arg;
			}
			else {
				wheel::unit::append(sql, "where", arg, std::forward<Args>(args)...);
			}
		}

		template<typename T, typename... Args>
		inline std::string generate_delete_sql(Args&&... where_conditon) {
			std::string sql = "delete from ";
			const auto SIZE = wheel::reflector::get_size<T>();
			const auto name = wheel::reflector::get_name<T>();
			wheel::unit::append(sql, name.data());
			wheel::unit::append(sql, " where ", std::forward<Args>(where_conditon)...);

			return sql;
		}

		template<typename T>
		inline std::string generate_insert_sql(bool update = false) {
			std::string sql = update ? "replace into " : "insert into ";
			const auto SIZE = wheel::reflector::get_size<T>();
			const auto name = wheel::reflector::get_name<T>();
			wheel::unit::append(sql, name.data(), " values(");
			for (auto i = 0; i < SIZE; ++i) {
				sql += "?";
				if (i < SIZE - 1)
					sql += ", ";
				else
					sql += ");";
			}

			return sql;
		}

	}
}
#endif // utility_h__
