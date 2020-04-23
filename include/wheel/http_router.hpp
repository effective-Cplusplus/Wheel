#ifndef http_router_h__
#define http_router_h__
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include "http_request.hpp"
#include "http_response.hpp"
#include "unit.hpp"
#include "htpp_define.hpp"
#include "function_traits.hpp"
#include "traits.hpp"

namespace wheel {
	namespace {
		const char DOT = '.';
	}

	namespace http_servers {
		class http_router {
		public:
			template<http_method... Is, typename Function, typename... Ap>
			std::enable_if_t<!std::is_member_function_pointer<Function>::value> register_handler(std::string name, Function&& f, const Ap&... ap) {
				if constexpr (sizeof...(Is) > 0) {
					auto arr = get_method_arr<Is...>();
					register_nonmember_func(name, arr, std::forward<Function>(f), ap...);
				}
				else {
					register_nonmember_func(name, { 0 }, std::forward<Function>(f), ap...);
				}
			}

			template <http_method... Is, class T, class Type, typename T1, typename... Ap>
			std::enable_if_t<std::is_same<T*, T1>::value> register_handler(std::string name, Type(T::* f)(request&, response&), T1 t, const Ap&... ap) {
				register_handler_impl<Is...>(name, f, t, ap...);
			}

			template <http_method... Is, class T, class Type, typename... Ap>
			void register_handler(std::string name, Type(T::* f)(request&, response&), const Ap&... ap) {
				register_handler_impl<Is...>(name, f, (T*)nullptr, ap...);
			}

			void remove_handler(std::string name) {
				this->map_invokers_.erase(name);
			}

			bool route(std::string method, std::string url, request& req, response& res) {
				auto it = map_invokers_.find(url);
				if (it != map_invokers_.end()) {
					auto& pair = it->second;
					if (method[0] < 'A' || method[0] > 'Z')
						return false;

					if (pair.first[method[0] - 65] == 0) {
						return false;
					}

					pair.second(req, res);
					return true;
				}
				else {
					if (url.rfind(DOT) != std::string::npos) {
						url = STATIC_RESOURCE;
						return route(method, url, req, res);
					}

					return get_wildcard_function(url, req, res);
				}
			}

		private:
			bool get_wildcard_function(std::string key, request& req, response& res) {
				for (auto& pair : wildcard_invokers_) {
					if (key.find(pair.first) != std::string::npos) {
						auto& t = pair.second;
						t.second(req, res);
						return true;
					}
				}
				return false;
			}

			template <http_method... Is, class T, class Type, typename T1, typename... Ap>
			void register_handler_impl(std::string name, Type T::* f, T1 t, const Ap&... ap) {
				if constexpr (sizeof...(Is) > 0) {
					auto arr = get_method_arr<Is...>();
					register_member_func(name, arr, f, t, ap...);
				}
				else {
					register_member_func(name, { 0 }, f, t, ap...);
				}
			}

			template<typename Function, typename... AP>
			void register_nonmember_func(std::string raw_name, const std::array<char, 26>& arr, Function f, const AP&... ap) {
				if (raw_name.back() == '*') {
					this->wildcard_invokers_[raw_name.substr(0, raw_name.length() - 1)] = { arr, std::bind(&http_router::invoke<Function, AP...>, this,
						std::placeholders::_1, std::placeholders::_2, std::move(f), ap...) };
				}
				else {
					this->map_invokers_[raw_name] = { arr, std::bind(&http_router::invoke<Function, AP...>, this,
						std::placeholders::_1, std::placeholders::_2, std::move(f), ap...) };
				}
			}

			template<typename Function, typename... AP>
			void invoke(request& req, response& res, Function f, AP... ap) {
				using result_type = std::result_of_t<Function(request&, response&)>;
				std::tuple<AP...> tp(std::move(ap)...);
				bool r = do_ap_before(req, res, tp);

				if (!r) {
					return;
				}

				if constexpr (std::is_void<result_type>::value) {
					//business
					f(req, res);
					//after
					do_void_after(req, res, tp);
				}
				else {
					//business
					result_type result = f(req, res);
					//after
					do_after(std::move(result), req, res, tp);
				}
			}

			template<typename Function, typename Self, typename... AP>
			void register_member_func(std::string raw_name, const std::array<char, 26>& arr, Function f, Self self, const AP&... ap) {
				if (raw_name.back() == '*') {
					this->wildcard_invokers_[raw_name.substr(0, raw_name.length() - 1)] = { arr, std::bind(&http_router::invoke_mem<Function, Self, AP...>, this,
						std::placeholders::_1, std::placeholders::_2, f, self, ap...) };
				}
				else {
					this->map_invokers_[raw_name] = { arr, std::bind(&http_router::invoke_mem<Function, Self, AP...>, this,
						std::placeholders::_1, std::placeholders::_2, f, self, ap...) };
				}
			}

			template<typename Function, typename Self, typename... AP>
			void invoke_mem(request& req, response& res, Function f, Self self, AP... ap) {
				using result_type = typename timax::function_traits<Function>::result_type;
				std::tuple<AP...> tp(std::move(ap)...);
				bool r = do_ap_before(req, res, tp);

				if (!r)
					return;
				using nonpointer_type = std::remove_pointer_t<Self>;
				if constexpr (std::is_void<result_type>::value) {
					//business
					if (self)
						(*self.*f)(req, res);
					else
						(nonpointer_type{}.*f)(req, res);
					//after
					do_void_after(req, res, tp);
				}
				else {
					//business
					result_type result;
					if (self)
						result = (*self.*f)(req, res);
					else
						result = (nonpointer_type{}.*f)(req, res);
					//after
					do_after(std::move(result), req, res, tp);
				}
			}

			template<typename Tuple>
			bool do_ap_before(request& req, response& res, Tuple& tp) {
				bool r = true;
				wheel::unit::for_each_l(tp, [&r, &req, &res](auto& item) {
					if (!r)
						return;

					constexpr bool has_befor_mtd = wheel::traits::has_before<decltype(item), request&, response&>::value;
					if constexpr (has_befor_mtd)
						r = item.before(req, res);
					}, wheel::traits::make_index_sequence<std::tuple_size<Tuple>::value>{});

				return r;
			}

			template<typename Tuple>
			void do_void_after(request& req, response& res, Tuple& tp) {
				bool r = true;
				wheel::unit::for_each_r(tp, [&r, &req, &res](auto& item) {
					if (!r)
						return;

					constexpr bool has_after_mtd = wheel::traits::has_after<decltype(item), request&, response&>::value;
					if constexpr (has_after_mtd)
						r = item.after(req, res);
					}, wheel::traits::make_index_sequence<std::tuple_size<Tuple>::value>{});
			}

			template<typename T, typename Tuple>
			void do_after(T&& result, request& req, response& res, Tuple& tp) {
				bool r = true;
				for_each_r(tp, [&r, result = std::move(result), &req, &res](auto& item){
					if (!r)
						return;

					constexpr bool has_after_mtd = wheel::traits::has_after<decltype(item), T, request&, response&>::value;
					if constexpr (has_after_mtd)
						r = item.after(std::move(result), req, res);
				}, wheel::traits::make_index_sequence<std::tuple_size<Tuple>::value>{});
			}

			typedef std::pair<std::array<char, 26>, std::function<void(request&, response&)>> invoker_function;
			std::map<std::string, invoker_function> map_invokers_;
			std::unordered_map<std::string, invoker_function> wildcard_invokers_; //for url/*
		};
	}
}


#endif // http_router_h__
