#ifndef http_server_h__
#define http_server_h__

#include <iostream>
#include <thread>
#include <atomic>
#include <unordered_map>
#include "http_tcp_handle.hpp"
#include "http_router.hpp"

namespace wheel {
	namespace http_servers {
		class http_server {
		public:
			http_server(){
				init_conn_callback();
				strand_ = std::make_unique<boost::asio::io_service::strand>(*io_service_poll::get_instance().get_io_service());
			}

			~http_server() {
				connects_.clear();
			}

			void listen(int port,int connects=1) {
				if (port_in_use(port)){
					std::cout << "port reuse" << std::endl;
					return;
				}

				//accept_ = std::make_unique<boost::asio::ip::tcp::acceptor>(*ios_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
				boost::system::error_code ec;
				accept_ = std::make_unique<boost::asio::ip::tcp::acceptor>(*io_service_poll::get_instance().get_io_service());

				//一定要调用open否则会监听失败
				accept_->open(boost::asio::ip::tcp::v4());
				accept_->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
				accept_->bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port),ec);
				accept_->listen(boost::asio::socket_base::max_connections, ec);
				if (ec){
					std::cout << "服务器监听失败:"<<ec.message() << std::endl;
					return;
				}

				for (int i =0;i< connects;++i){
					make_session();
				}
			}

			//set http handlers
			template<http_method... Is, typename Function, typename... AP>
			void set_http_handler(std::string name, Function&& f, AP&&... ap) {
				http_router_.register_handler<Is...>(name, std::forward<Function>(f), std::forward<AP>(ap)...);
			}

			void run(size_t thread_num = std::thread::hardware_concurrency()) {
				io_service_poll::get_instance().run(thread_num);
			}

			//应答的时候是否需要加上时间
			void enable_response_time(bool enable) {
				need_response_time_ = enable;
			}

#ifdef WHEEL_ENABLE_SSL
			void set_ssl_conf(ssl_configure conf) {
				ssl_conf_ = std::move(conf);
			}
#endif
		private:
			bool port_in_use(unsigned short port) {
				boost::asio::io_service svc;
				boost::asio::ip::tcp::acceptor acept(svc);

				boost::system::error_code ec;
				acept.open(boost::asio::ip::tcp::v4(), ec);
				acept.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
				acept.bind({ boost::asio::ip::tcp::v4(), port }, ec);

				return ec == boost::asio::error::address_in_use;
			}
		private:
			void make_session() {
				if (accept_ == nullptr || strand_ == nullptr) {
					return;
				}

				std::shared_ptr<http_tcp_handle > new_session = nullptr;
				try
				{
					new_session = std::make_shared<http_tcp_handle>(strand_,http_handler_, upload_dir_, ssl_conf_, need_response_time_);
				}
				catch (const std::exception & ex)
				{
					std::cout << ex.what() << std::endl;
					new_session = nullptr;
				}

				if (new_session == nullptr) {
					return;
				}

				//发一次数据接收一次
				accept_->async_accept(new_session->get_socket(),strand_->wrap([this, new_session](const boost::system::error_code& ec) {
					if (ec) {
						return;
					}

					new_session->register_close_observer(std::bind(&http_server::on_close, this, std::placeholders::_1, std::placeholders::_2));
					new_session->register_connect_observer(std::bind(&http_server::on_connect, this, std::placeholders::_1));
					new_session->activate();
					make_session();
					}));
			}

			void init_conn_callback() {
				http_handler_ = [this](request& req, response& res) {
					res.set_headers(req.get_headers());
					try {
						bool success = http_router_.route(req.get_method(), req.get_url(), req, res);
						if (!success) {
							res.set_status_and_content(status_type::bad_request, "the url is not right");
						}
					}
					catch (const std::exception & ex) {
						res.set_status_and_content(status_type::internal_server_error, ex.what() + std::string(" exception in business function"));
					}
					catch (...) {
						res.set_status_and_content(status_type::internal_server_error, "unknown exception in business function");
					}
				};
			}

			void on_connect(std::shared_ptr<wheel::http_servers::http_tcp_handle> handler) {
				if (handler == nullptr) {
					return;
				}

				auto iter_find = connects_.find(handler);
				if (iter_find != connects_.end()) {
					return;
				}

				std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> tp = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
				std::time_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
				while (lock_.test_and_set(std::memory_order_acquire));
				connects_.emplace(handler, timestamp);

				lock_.clear(std::memory_order_release);
			}

			void on_close(std::shared_ptr<wheel::http_servers::http_tcp_handle> handler, const boost::system::error_code& ec) {
				while (lock_.test_and_set(std::memory_order_acquire));
				auto iter_find = connects_.find(handler);
				if (iter_find != connects_.end()) {
					connects_.erase(iter_find);
				}

				lock_.clear(std::memory_order_release);
			}
		private:
			bool need_response_time_ = false;
			ssl_configure ssl_conf_;
			std::string upload_dir_ = fs::absolute("www").string(); //default
			http_handler http_handler_;
			http_router http_router_;
			std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
			std::unordered_map<std::shared_ptr<wheel::http_servers::http_tcp_handle>, std::time_t>connects_;
			std::unique_ptr<boost::asio::ip::tcp::acceptor> accept_{};
			std::shared_ptr<boost::asio::io_service::strand>strand_;
		};
	}
}
#endif // http_server_h__
