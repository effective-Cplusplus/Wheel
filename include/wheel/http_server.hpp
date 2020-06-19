#ifndef http_server_h__
#define http_server_h__

#include <iostream>
#include <thread>
#include <atomic>
#include <fstream>
#include <unordered_map>
#include "http_tcp_handle.hpp"
#include "http_router.hpp"

namespace wheel {
	namespace http_servers {
		class http_server {
		public:
			http_server(){
				counts_ = 0;
				init_conn_callback();
			}

			~http_server() {
				connects_.clear();
				io_service_poll::get_instance().stop();
			}

			void listen(int port,int connects=1) {
				if (port_in_use(port)){
					std::cout << "port reuse" << std::endl;
					return;
				}

				//accept_ = std::make_unique<boost::asio::ip::tcp::acceptor>(*ios_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
				boost::system::error_code ec;
				accept_ =wheel::traits::make_unique<boost::asio::ip::tcp::acceptor>(*io_service_poll::get_instance().get_main_io_service());

				//一定要调用open否则会监听失败
				accept_->open(boost::asio::ip::tcp::v4(),ec);
				if (!accept_->is_open()){
					return;
				}

				int qlen = 5;
				accept_->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
				boost::asio::ip::tcp::no_delay delay_option(true);
				accept_->set_option(delay_option, ec);
				accept_->set_option(boost::asio::detail::socket_option::integer<IPPROTO_TCP, TCP_FASTOPEN>(qlen), ec);

				accept_->bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port),ec);
				if (ec){
					std::cout << "bind fail" << std::endl;
					return;
				}

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

			void run() {
				io_service_poll::get_instance().run();
			}

			//应答的时候是否需要加上时间
			void enable_response_time(bool enable) {
				need_response_time_ = enable;
			}

			ssl_configure_data read_cert_data(const ssl_configure& conf) {
				ssl_configure_data ssl_conf_data;

				boost::system::error_code ec;
				if (fs::exists(ssl_conf_.cert_file, ec)) {
					std::unique_ptr<std::ifstream>file_read_ = std::make_unique<std::ifstream>();

					file_read_->open(ssl_conf_.cert_file, std::ios::binary | std::ios::ate);
					if (file_read_->is_open()) {
						std::string data;
						std::size_t file_size = file_read_->tellg();
						data.resize(file_size);
						file_read_->seekg(0, std::ios::beg);

						file_read_->read(&data[0], file_size);
						file_read_->close();

						ssl_conf_data.cert_data = std::move(data);
					}
				}


				if (fs::exists(ssl_conf_.key_file, ec)) {
					std::unique_ptr<std::ifstream>file_read_ = std::make_unique<std::ifstream>();

					file_read_->open(ssl_conf_.key_file, std::ios::binary | std::ios::ate);
					if (file_read_->is_open()) {
						std::string data;
						std::size_t file_size = file_read_->tellg();
						data.resize(file_size);
						file_read_->seekg(0, std::ios::beg);

						file_read_->read(&data[0], file_size);
						file_read_->close();

						ssl_conf_data.key_data = std::move(data);
					}
				}


				if (fs::exists(ssl_conf_.pem_flie, ec)) {
					std::unique_ptr<std::ifstream>file_read_ = std::make_unique<std::ifstream>();

					file_read_->open(ssl_conf_.pem_flie, std::ios::binary | std::ios::ate);
					if (file_read_->is_open()) {
						std::string data;
						std::size_t file_size = file_read_->tellg();
						data.resize(file_size);
						file_read_->seekg(0, std::ios::beg);

						file_read_->read(&data[0], file_size);
						file_read_->close();

						ssl_conf_data.pem_data = std::move(data);
					}
				}

				return std::move(ssl_conf_data);
			}

#ifdef WHEEL_ENABLE_SSL
			void set_ssl_conf(ssl_configure conf) {
				ssl_conf_ = std::move(conf);

				ssl_conf_data_= read_cert_data(conf);
				
				unsigned long  ssl_options = boost::asio::ssl::context::default_workarounds
					| boost::asio::ssl::context::no_sslv3
					| boost::asio::ssl::context::no_sslv2
					| boost::asio::ssl::context::no_compression
					| boost::asio::ssl::context::single_dh_use;

				ssl_conf_data_.ssl_options = ssl_options;
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
				if (accept_ == nullptr) {
					return;
				}

				std::shared_ptr<http_tcp_handle > new_session = nullptr;
				try
				{
					new_session = std::make_shared<http_tcp_handle>(http_handler_, upload_dir_, ssl_conf_data_, need_response_time_);
				}
				catch (const std::exception & ex)
				{
					std::cout << ex.what() << std::endl;
					new_session = nullptr;
				}

				if (new_session == nullptr) {
					return;
				}

				auto socket_ptr = new_session->get_socket();
				if (socket_ptr == nullptr) {
					return;
				}

				new_session->register_close_observer(std::bind(&http_server::on_close, this, std::placeholders::_1, std::placeholders::_2));
				new_session->register_connect_observer(std::bind(&http_server::on_connect, this, std::placeholders::_1));
				//发一次数据接收一次
				accept_->async_accept(*socket_ptr,[this, new_session](const boost::system::error_code& ec) {
					if (ec) {
						return;
					}
					
					io_service_poll::get_instance().dispatch(std::bind(&http_tcp_handle::activate, new_session));
					make_session();
					});
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

				if (counts_ ==0){
					time_stamp_ =unit::get_time_stamp();
				}

				size_t time_stamp = unit::get_time_stamp();
				if (time_stamp - time_stamp_ >= 1) {
					std::cout << "qps:" << counts_ << std::endl;
					counts_ = 0;
					time_stamp_ = time_stamp;
				}

				counts_++;

				std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> tp = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
				std::time_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
				while (lock_.test_and_set(std::memory_order_acquire));
				auto iter_find = connects_.find(handler);
				if (iter_find == connects_.end()) {
					connects_.emplace(handler, timestamp);
				}

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
			ssl_configure_data ssl_conf_data_;
			std::atomic<int>counts_;
			std::string upload_dir_ = fs::absolute("www").string(); //default
			http_handler http_handler_;
			http_router http_router_;
			size_t time_stamp_ = 0;
			std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
			std::unordered_map<std::shared_ptr<wheel::http_servers::http_tcp_handle>, std::time_t>connects_;
			std::unique_ptr<boost::asio::ip::tcp::acceptor> accept_{};
		};
	}
}
#endif // http_server_h__
