#ifndef mysql_wrap_h__
#define mysql_wrap_h__

#define WIN32_LEAN_AND_MEAN
#include <include/mysql.h>
#include <tuple>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include "query_result.hpp"
#include "utility.hpp"

#if _MSC_VER
#pragma warning(disable:4984)
#else
#pragma GCC system_header
#endif
namespace wheel {
	namespace mysql {
		class mysql_wrap {
		public:
			static mysql_wrap& get_intance() {
				if (instance_ == nullptr) {
					static std::once_flag falg;
					std::call_once(falg, []() {
						instance_.reset(new mysql_wrap);
						});

				}

				return *instance_;
			}

			~mysql_wrap() {
				disconnect();
			}

			void disconnect() {
				if (con_ != nullptr) {
					mysql_close(con_);
					con_ = nullptr;
				}
			}

			std::string get_laster_error()const {
				return err_;
			}

			/****** const char *host,
					 const char *user,
					 onst char *passwd,
					 const char *db,
					 unsigned int port
					 time_out,
			***********/
			template<typename...Args>
			bool connect(Args&&... args) {
				static_assert(sizeof...(args) >= 4, "args >=4");

				if (con_ != nullptr) {
					mysql_close(con_);
				}

				con_ = mysql_init(nullptr);
				if (con_ == nullptr) {
					return false;
				}

				auto tp = std::make_tuple(con_, std::forward<Args>(args)...);
				int timeout = 0;
				if constexpr (sizeof...(args) == 6) {
					timeout = std::get<6>(tp);
				}

				if (timeout > 0) {
					if (mysql_options(con_, MYSQL_OPT_CONNECT_TIMEOUT, &timeout) != 0) {
						return false;
					}
				}

				char value = 1;
				mysql_options(con_, MYSQL_OPT_RECONNECT, &value);
				mysql_options(con_, MYSQL_SET_CHARSET_NAME, "utf8");

				int port = 0;
				if constexpr (sizeof...(args) >= 5) {
					port = std::get<5>(tp);
				}

				if (port == 0) {
					port = 3306;
				}

				if (mysql_real_connect(con_, std::get<1>(tp), std::get<2>(tp), std::get<3>(tp), std::get<4>(tp), port, NULL, 0) == nullptr) {
					err_ = mysql_error(con_);
					disconnect();
					return false;
				}

				return true;
			}

			bool commit() {
				if (con_ == nullptr) {
					return false;
				}

				return mysql_query(con_, "commit") == 0 ? true : false;
			}

			bool rollback() {
				if (con_ == nullptr) {
					return false;
				}

				return mysql_query(con_, "ROLLBACK") == 0 ? true : false;
			}

			bool begin() {
				return mysql_query(con_, "BEGIN") == 0 ? true : false;
			}

			//excute  sql
			bool execute(const std::string& sql) {
				if (con_ == nullptr) {
					err_ = "SQL:=" + sql + ", ";
					err_+= mysql_error(con_);
					return false;
				}

				return mysql_query(con_, sql.data()) == 0 ? true : false;
			}

			//根据SQL查询结果集去映射
			  int query(std::string dml, query_result &qresult) {
				  if (con_ == nullptr){
					  return-1;
				  }

				if (mysql_query(con_, dml.c_str()) != 0){
					err_ = "SQL:=" + dml + ", ";
					err_ = mysql_error(con_);
					return -1;
				}

				do
				{
					MYSQL_RES* result = mysql_store_result(con_);
					if (result == nullptr){
						continue;
					}

					//指向  mysql 的查询字段集

					//取得各字段名和类型
					for (MYSQL_FIELD* fieldptr = mysql_fetch_field(result); fieldptr != nullptr; fieldptr = mysql_fetch_field(result)){
						qresult.fieldtype_.emplace_back(set_field_type(fieldptr->type), fieldptr->length, qresult.fieldtype_.size(), fieldptr->name);
					}

					for (MYSQL_ROW currrow = mysql_fetch_row(result); currrow != nullptr; currrow = mysql_fetch_row(result))
					{
						unsigned long* lengths = mysql_fetch_lengths(result);

						//读行的记录
						const unsigned int colcount = mysql_num_fields(result);
						row_t rows(colcount);
						for (unsigned int i = 0; i < colcount; ++i){
							if (qresult.fieldtype_[i].type_ == BIN)
							{
								if (lengths[i] > 0)
								{
									rows[i].resize(lengths[i]);
									memcpy(&(*rows[i].begin()), currrow[i], lengths[i]);
								}
							}
							else
							{
								rows[i] = currrow[i] ? currrow[i] : "";
							}
						}

						qresult.recordset_.emplace_back(rows);
						qresult.affected_rows_++;
					}

					mysql_free_result(result);

					result = nullptr;
				} while (!mysql_next_result(con_));

				return 1;
			}

			//for tuple and string with args...,传入tuple进行解析
			template<typename T, typename Arg, typename... Args>
			constexpr std::enable_if_t<!wheel::reflector::is_reflection_v<T>, std::vector<T>> query(const Arg& s, Args&&... args) {
				constexpr auto SIZE = std::tuple_size<T>::value;

				std::string sql = s;
				constexpr auto Args_Size = sizeof...(Args);
				if (Args_Size != 0) {
					if (Args_Size != std::count(sql.begin(), sql.end(), '?')) {
						return {};
					}

					sql = get_sql(sql, std::forward<Args>(args)...);
				}

				stmt_ = mysql_stmt_init(con_);
				if (!stmt_) {
					return {};
				}
				if (mysql_stmt_prepare(stmt_, sql.c_str(), (unsigned long)sql.size())) {
					err_ = mysql_error(con_);
					return {};
				}

				auto guard = guard_statment(stmt_);

				std::array<MYSQL_BIND, result_size<T>::value> param_binds = {};
				std::list<std::vector<char>> mp;

				std::vector<T> v;
				T tp{};

				size_t index = 0;
				wheel::reflector::for_each(tp, [&param_binds, &mp, &index](auto& item, auto I) {
					using U = std::remove_reference_t<decltype(item)>;
					if constexpr (std::is_arithmetic<U>::value) {
						param_binds[index].buffer_type = (enum_field_types)type_to_id(identity<U>{});
						param_binds[index].buffer = &item;
						index++;
					}
					else if constexpr (std::is_same<std::string, U>::value) {
						std::vector<char> tmp(65536, 0);
						mp.emplace_back(std::move(tmp));
						param_binds[index].buffer_type = MYSQL_TYPE_STRING;
						param_binds[index].buffer = &(mp.back()[0]);
						param_binds[index].buffer_length = 65536;
						index++;
					}
					else if constexpr (wheel::reflector::is_reflection_v<U>) {
						wheel::reflector::for_each(item, [&param_binds, &mp, &item, &index](auto& ele, auto i) {
							using U = std::remove_reference_t<decltype(std::declval<U>().*ele)>;
							if constexpr (std::is_arithmetic<U>::value) {
								param_binds[index].buffer_type = (enum_field_types)type_to_id(identity<U>{});
								param_binds[index].buffer = &(item.*ele);
							}
							else if constexpr (std::is_same<std::string, U>::value) {
								std::vector<char> tmp(65536, 0);
								mp.emplace_back(std::move(tmp));
								param_binds[index].buffer_type = MYSQL_TYPE_STRING;
								param_binds[index].buffer = &(mp.back()[0]);
								param_binds[index].buffer_length = 65536;
							}
							else if constexpr (wheel::traits::is_char_array_v<U>) {
								std::vector<char> tmp(sizeof(U), 0);
								mp.emplace_back(std::move(tmp));
								param_binds[index].buffer_type = MYSQL_TYPE_VAR_STRING;
								param_binds[index].buffer = &(mp.back()[0]);
								param_binds[index].buffer_length = (unsigned long)sizeof(U);
							}
							index++;
							});
					}
					else if constexpr (wheel::traits::is_char_array_v<U>) {
						param_binds[index].buffer_type = MYSQL_TYPE_VAR_STRING;
						std::vector<char> tmp(sizeof(U), 0);
						mp.emplace_back(std::move(tmp));
						param_binds[index].buffer = &(mp.back()[0]);
						param_binds[index].buffer_length = (unsigned long)sizeof(U);
						index++;
					}
					else {
						std::cout << typeid(U).name() << std::endl;
					}
					}, std::make_index_sequence<SIZE>{});

				if (mysql_stmt_bind_result(stmt_, &param_binds[0])) {
					err_ = mysql_error(con_);
					return {};
				}

				if (mysql_stmt_execute(stmt_)) {
					err_ = mysql_error(con_);
					return {};
				}

				while (mysql_stmt_fetch(stmt_) == 0) {
					auto it = mp.begin();
					wheel::reflector::for_each(tp, [&mp, &it](auto& item, auto i) {
						using U = std::remove_reference_t<decltype(item)>;
						if constexpr (std::is_same<std::string, U>::value) {
							item = std::string(&(*it)[0], strlen((*it).data()));
							it++;
						}
						else if constexpr (wheel::traits::is_char_array_v<U>) {
							memcpy(item, &(*it)[0], sizeof(U));
						}
						else if constexpr (wheel::reflector::is_reflection_v<U>) {
							wheel::reflector::for_each(item, [&it, &item](auto ele, auto i) {
								using V = std::remove_reference_t<decltype(std::declval<U>().*ele)>;
								if constexpr (std::is_same<std::string, V>::value) {
									item.*ele = std::string(&(*it)[0], strlen((*it).data()));
									it++;
								}
								else if constexpr (wheel::traits::is_char_array_v<V>) {
									memcpy(item.*ele, &(*it)[0], sizeof(V));
								}
								});
						}
						}, std::make_index_sequence<SIZE>{});

					if (index > 0)
						v.push_back(std::move(tp));
				}

				return v;
			}
			
			template<typename T, typename... Args>
			constexpr std::enable_if_t<wheel::reflector::is_reflection_v<T>, std::vector<T>> query(Args&&... args) {
				std::string sql = generate_query_sql<T>(args...);
				constexpr auto SIZE = wheel::reflector::get_size<T>();

				if (con_ == nullptr) {
					return {};
				}

				stmt_ = mysql_stmt_init(con_);
				if (!stmt_) {
					err_ = mysql_error(con_);
					return {};
				}

				if (mysql_stmt_prepare(stmt_, sql.c_str(), (unsigned long)sql.size())) {
					err_ = mysql_error(con_);
					return {};
				}

				auto guard = guard_statment(stmt_);

				std::array<MYSQL_BIND, SIZE> param_binds = {};
				std::map<size_t, std::vector<char>> mp;

				std::vector<T> v;
				T t{};
				int index = 0;
				wheel::reflector::for_each_tuple_back(t, [&](auto item, auto i) {
					constexpr auto Idx = decltype(i)::value;
					using U = std::remove_reference_t<decltype(std::declval<T>().*item)>;
					if constexpr (std::is_arithmetic<U>::value) {
						param_binds[Idx].buffer_type = (enum_field_types)type_to_id(identity<U>{});
						param_binds[Idx].buffer = &(t.*item);
						index++;
					}
					else if constexpr (std::is_same<std::string, U>::value) {
						param_binds[Idx].buffer_type = MYSQL_TYPE_STRING;
						std::vector<char> tmp(65536, 0);
						mp.emplace(decltype(i)::value, tmp);
						param_binds[Idx].buffer = &(mp.rbegin()->second[0]);
						param_binds[Idx].buffer_length = (unsigned long)tmp.size();
						index++;
					}
					else if constexpr (wheel::traits::is_char_array_v<U>) {
						param_binds[Idx].buffer_type = MYSQL_TYPE_VAR_STRING;
						std::vector<char> tmp(sizeof(U), 0);
						mp.emplace(decltype(i)::value, tmp);
						param_binds[Idx].buffer = &(mp.rbegin()->second[0]);
						param_binds[Idx].buffer_length = (unsigned long)sizeof(U);
						index++;
					}
					});

				if (index == 0) {
					return {};
				}

				if (mysql_stmt_bind_result(stmt_, &param_binds[0])) {
					err_ = mysql_error(con_);
					return {};
				}

				if (mysql_stmt_execute(stmt_)) {
					err_ = mysql_error(con_);
					return {};
				}

				while (mysql_stmt_fetch(stmt_) == 0) {
					using TP = decltype(wheel::reflector::get(std::declval<T>()));

					wheel::reflector::for_each_tuple_back(t, [&mp, &t](auto item, auto i) {
						using U = std::remove_reference_t<decltype(std::declval<T>().*item)>;
						if constexpr (std::is_same<std::string, U>::value) {
							auto& vec = mp[decltype(i)::value];
							t.*item = std::string(&vec[0], strlen(vec.data()));
						}
						else if constexpr (wheel::traits::is_char_array_v<U>) {
							auto& vec = mp[decltype(i)::value];
							memcpy(t.*item, vec.data(), vec.size());
						}
						});

					for (auto& p : mp) {
						p.second.assign(p.second.size(), 0);
					}

					v.push_back(std::move(t));
					wheel::reflector::for_each_tuple_back(t, [&mp, &t](auto item, auto i) {
						using U = std::remove_reference_t<decltype(std::declval<T>().*item)>;
						if constexpr (std::is_arithmetic<U>::value) {
							memset(&(t.*item), 0, sizeof(U));
						}
						});
				}

				return v;
			}

			template<typename T, typename... Args>
			constexpr int insert(const T& t, Args&&... args) {
				if (con_ == nullptr){
					return -1;
				}

				std::string sql = generate_insert_sql<decltype(t)>();

				return insert_aux(sql, t, std::forward<Args>(args)...);
			}


			template<typename T ,typename... Args>
			constexpr int insert(const std::vector<T>& t, Args&&...args) {
				if (con_ == nullptr) {
					return -1;
				}

				const std::size_t size= t.size();
				if (size ==0){
					return -1;
				}

				std::string sql = generate_insert_sql<decltype(t[0])>();

				return insert_aux(sql, t, std::forward<Args>(args)...);
			}

			template<typename T, typename... Args>
			constexpr bool delete_records(Args&&... where_conditon) {
				auto sql = generate_delete_sql<T>(std::forward<Args>(where_conditon)...);
				if (mysql_query(con_, sql.data())) {
					err_ = mysql_error(con_);
					return false;
				}

				return true;
			}

			//指定更新一条数据
			template<typename T, typename... Args>
			constexpr int update(const T& t, Args&&... args) {
				if(con_ == nullptr){
					return -1;
				}

				std::string sql = generate_insert_sql<T>(true);
				return insert_aux(sql, t, std::forward<Args>(args)...);
			}

			//指定更新n条数据
			template<typename T, typename... Args>
			constexpr int update(const std::vector<T>& t, Args&&... args) {
				if(con_ == nullptr){
					return -1;
				}
				
				std::string sql = generate_insert_sql<T>(true);
				return insert_aux(sql, t, std::forward<Args>(args)...);
			}
			//创建表
			template<typename T,typename...Args>
			constexpr bool create_table(Args&&... args) {
				std::string sql = generate_createtb_sql<T>(std::forward<Args>(args)...);
				sql += " DEFAULT CHARSET=utf8";
				if (mysql_query(con_, sql.data())) {
					err_ = mysql_error(con_);
					return false;
				}

				return true;
			}
		private:
			template<typename T, typename... Args>
			constexpr int insert_aux(const std::string& sql, const std::vector<T>& t, Args&&... args) {
				stmt_ = mysql_stmt_init(con_);
				if (!stmt_) {
					err_ = mysql_error(con_);
					return INT_MIN;
				}


				if (mysql_stmt_prepare(stmt_, sql.c_str(), (int)sql.size())) {
					err_ = mysql_error(con_);
					return INT_MIN;
				}

				auto guard = guard_statment(stmt_);

				//transaction
				//һ��һ������bengin,Ȼ��һ���Եݽ�
				bool b = begin();
				if (!b)
					return INT_MIN;

				for (auto& item : t) {
					int r = stmt_execute(item);
					if (r == INT_MIN) {
						rollback();
						return INT_MIN;
					}
				}
				b = commit();

				return b ? (int)t.size() : INT_MIN;
			}

			template<typename T>
			constexpr void set_param_bind(std::vector<MYSQL_BIND>& param_binds, T&& value) {
				MYSQL_BIND param = {};

				using U = std::remove_const_t<std::remove_reference_t<T>>;
				if constexpr (std::is_arithmetic<U>::value) {
					param.buffer_type = (enum_field_types)type_to_id(identity<U>{});
					param.buffer = const_cast<void*>(static_cast<const void*>(&value));
				}
				else if constexpr (std::is_same<std::string, U>::value) {
					param.buffer_type = MYSQL_TYPE_STRING;
					param.buffer = (void*)(value.c_str());
					param.buffer_length = (unsigned long)value.size();
				}
				else if constexpr (std::is_same<const char*, U>::value || wheel::traits::is_char_array_v<U>) {
					param.buffer_type = MYSQL_TYPE_STRING;
					param.buffer = (void*)(value);
					param.buffer_length = (unsigned long)strlen(value);
				}
				param_binds.push_back(param);
			}

			template<typename T>
			int stmt_execute(const T& t) {
				std::vector<MYSQL_BIND> param_binds;
				wheel::reflector::for_each_tuple_front(t, [&t, &param_binds, this](const auto& v, auto i) {
					set_param_bind(param_binds, t.*v);
					});

				if (mysql_stmt_bind_param(stmt_, &param_binds[0])) {
					err_ = mysql_error(con_);
					return INT_MIN;
				}

				if (mysql_stmt_execute(stmt_)) {
					err_ = mysql_error(con_);
					return INT_MIN;
				}

				int count = (int)mysql_stmt_affected_rows(stmt_);
				if (count == 0) {
					err_ = mysql_error(con_);
					return INT_MIN;
				}

				return count;
			}

			template<typename T>
			inline std::string generate_update_sql() {
				std::string sql = "update ";
				const auto name = wheel::reflector::get_name<T>();
				wheel::unit::append(sql, name.data());

				return sql;
			}

			template<typename T, typename... Args>
			constexpr int insert_aux(const std::string& sql, const T& t, Args&&... args) {
				stmt_ = mysql_stmt_init(con_);
				if (!stmt_) {
					err_ = mysql_error(con_);
					return INT_MIN;
				}


				if (mysql_stmt_prepare(stmt_, sql.c_str(), (int)sql.size())) {
					err_ = mysql_error(con_);
					return INT_MIN;
				}

				auto guard = guard_statment(stmt_);

				if (stmt_execute(t) < 0) {
					err_ = mysql_error(con_);
					return INT_MIN;
				}

				return 1;
			}

			struct guard_statment {
				guard_statment(MYSQL_STMT* stmt) :stmt_(stmt) {}
				MYSQL_STMT* stmt_ = nullptr;
				int status_ = 0;
				~guard_statment() {
					if (stmt_ != nullptr) {
						status_ = mysql_stmt_close(stmt_);
					}


					if (status_)
						fprintf(stderr, "close statment error code %d\n", status_);
				}
			};

		private:
			mysql_wrap() = default;

			mysql_wrap(const mysql_wrap&) = delete;
			mysql_wrap& operator=(const mysql_wrap&) = delete;
			mysql_wrap(const mysql_wrap&&) = delete;
			mysql_wrap& operator=(const mysql_wrap&&) = delete;

		private:
			static std::unique_ptr<mysql_wrap> instance_;
			MYSQL* con_ = nullptr;
			MYSQL_STMT* stmt_ = nullptr;

			//错误原因
			std::string err_;
		};

		std::unique_ptr<mysql_wrap> mysql_wrap::instance_;
	}
}//wheel
#endif // mysql_wrap_h__
