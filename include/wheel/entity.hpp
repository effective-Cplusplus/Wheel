#ifndef entity_h__
#define entity_h__

#include <set>
#include "unit.hpp"
#include <string>
#include <iostream>
#include "traits.hpp"

namespace wheel {
	namespace mysql {
		struct wheel_sql_not_null {
			//代码故意不删除，血的教训，构造函数不能不能为变长参数，否则程序直接栈溢出
			//template<typename...Args>
			//wheel_sql_not_null(Args&&... args) {
			//	static_assert(sizeof...(args) > 0, "args >0");
			//	auto tp = std::make_tuple(std::forward<Args>(args)...);

			//	constexpr auto SIZE = std::tuple_size<decltype(tp)>::value;

			//	wheel::unit::for_each_tuple_back(tp, [this](auto& item, auto index) {
			//		if constexpr (wheel::traits::is_string<decltype(item)>::value) {
			//			fields.emplace(item);
			//		}
			//		}, std::make_index_sequence<SIZE>{});
			//}

			std::set<std::string> fields;
		};

		struct wheel_sql_key {
			wheel_sql_key(std::string key) :fields(key) {

			}

			std::string fields;
		};

		struct wheel_sql_auto_key {
			wheel_sql_auto_key(std::string key) :fields(key) {

			}

			std::string fields;
		};

		struct wheel_sql_unique {
			wheel_sql_unique(std::string key) :fields(key) {

			}

			std::string fields;
		};
	}
}

#endif // entity_h__
