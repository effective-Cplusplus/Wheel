#ifndef websocket_server_h__
#define websocket_server_h__
#include "ws_tcp_handle.hpp"
#include <unordered_map>
//#include <atomic>

namespace wheel {
	namespace websocket {
		enum wes_paser_type
		{
			json = 0,//默认是JSON
			bin = 1,
		};

		class websocket_server {
		public:
			//json 字符串
			websocket_server(const MessageEventObserver& recv_observer)
				:recv_observer_(recv_observer) {
			}

			//二进制流的格式
			websocket_server(const MessageEventObserver& recv_observer, int parser_type, std::size_t header_size,
				std::size_t packet_size_offset, std::size_t packet_cmd_offset)
				:recv_observer_(recv_observer), parser_type_(parser_type)
				, header_size_(header_size)
				, packet_size_offset_(packet_size_offset)
				, packet_cmd_offset_(packet_cmd_offset) {

			}

			~websocket_server() {
				connects_.clear();
			}

			void init(int port, int connect_pool=1) {
				if (port_in_use(port)) {
					std::cout << "port reuse" << std::endl;
					return;
				}

				//accept_ = std::make_unique<boost::asio::ip::tcp::acceptor>(*io_service_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));

				boost::system::error_code ec;
				accept_ = traits::make_unique<boost::asio::ip::tcp::acceptor>(*io_service_poll::get_instance().get_main_io_service());
				
				//一定要调用open否则会监听失败
				accept_->open(boost::asio::ip::tcp::v4(),ec);

				if (!accept_->is_open()) {
					return;
				}

				//端口复用
				accept_->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
				accept_->bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port), ec);
				accept_->listen(boost::asio::socket_base::max_connections, ec);
				if (ec) {
					std::cout << "服务器监听失败:" << ec.message() << std::endl;
					return;
				}

				for (int i = 0;i < connect_pool;i++) {
					make_session();
				}
			}

			void run() {
				io_service_poll::get_instance().run();
			}
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


			void make_session() {
				if (accept_ == nullptr) {
					return;
				}

				std::shared_ptr<ws_tcp_handle> new_session = nullptr;
				try
				{
					if (parser_type_ == json) {
						new_session = std::make_shared<ws_tcp_handle>(header_size_, packet_size_offset_, packet_cmd_offset_);
					}
					else if (parser_type_ == bin) {
						new_session = std::make_shared<ws_tcp_handle>();
					}
					else {
						return;
					}

				}catch (const std::exception&ex)
				{
					std::cout << ex.what() << std::endl;
				}

				if (new_session ==nullptr){
					return;
				}

				new_session->register_close_observer(std::bind(&websocket_server::on_close, this, std::placeholders::_1, std::placeholders::_2));
				new_session->register_connect_observer(std::bind(&websocket_server::on_connect, this, std::placeholders::_1));
				new_session->register_recv_observer(recv_observer_);

				//发一次数据接收一次
				accept_->async_accept(*new_session->get_socket(), [this, new_session](const boost::system::error_code& ec) {
					if (ec) {
						return;
					}

					new_session->activate();
					make_session();
					});
			}
		private:
			void on_connect(std::shared_ptr<ws_tcp_handle> handler) {
				if (handler == nullptr) {
					return;
				}

				auto iter_find = connects_.find(handler);
				if (iter_find != connects_.end()) {
					return;
				}

				std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> tp = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
				std::time_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();

				lock_.test_and_set(std::memory_order_acquire);
				connects_.emplace(handler, timestamp);
				lock_.clear(std::memory_order_release);
			}

			void on_close(std::shared_ptr<ws_tcp_handle> handler, const boost::system::error_code& ec) {
				//tcp主动断开 boost::asio::error::connection_reset
				//websocket主动断开  boost::asio::error::connection_aborted
				lock_.test_and_set(std::memory_order_acquire);
				auto iter_find = connects_.find(handler);
				if (iter_find != connects_.end()) {
					connects_.erase(iter_find);
				}

				lock_.clear(std::memory_order_release);
			}
		private:
			int parser_type_ = json;
			std::size_t header_size_ = 0;
			std::size_t packet_size_offset_ = 0;
			std::size_t packet_cmd_offset_ = 0;
			std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
			MessageEventObserver		recv_observer_;
			std::unique_ptr<TCP::acceptor>accept_{};
			std::unordered_map<std::shared_ptr<ws_tcp_handle>, std::time_t>connects_;
		};
  }
}

#endif // websocket_server_h__
