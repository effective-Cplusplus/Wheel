#ifndef websocket_client_h__
#define websocket_client_h__
#include "ws_tcp_handle.hpp"
#include <chrono>
#include "base64.hpp"

namespace wheel {
	namespace websocket {
		class webscoket_client
		{
		public:
			//json 对象解析
			webscoket_client(const MessageEventObserver& recv_observer)
				:recv_observer_(recv_observer) {
				try{
					strand_ = std::make_shared<boost::asio::io_service::strand>(*io_service_poll::get_instance().get_io_service());
					tcp_handler_ = std::make_shared<ws_tcp_handle>(strand_);
					timer_ = std::make_unique<boost::asio::steady_timer>(*io_service_poll::get_instance().get_io_service()));
				}catch (const std::exception&ex){
					std::cout << ex.what() << std::endl;
				}
			}

			//bin 解析
			webscoket_client(const MessageEventObserver& recv_observer,std::size_t header_size, std::size_t packet_size_offset, std::size_t packet_cmd_offse)
				:recv_observer_(recv_observer)
				, header_size_(header_size)
				, packet_size_offset_(packet_size_offset)
				, packet_cmd_offset_(packet_cmd_offse) {
				try
				{
					strand_ = std::make_shared<boost::asio::io_service::strand>(*io_service_poll::get_instance().get_io_service());
					tcp_handler_ = std::make_shared<wheel::websocket::ws_tcp_handle>(strand_, header_size_, packet_size_offset_, packet_cmd_offset_);
					timer_ = std::make_unique<boost::asio::steady_timer>(*io_service_poll::get_instance().get_io_service());
				}catch (const std::exception&ex){
					std::cout << ex.what() << std::endl;
				}

				if (ios_ ==nullptr || tcp_handler_ ==nullptr || timer_ ==nullptr){
					return;
				}

				tcp_handler_->set_protocol_parse_type(0);
			}

			~webscoket_client() {

			}

			void set_reconnect(bool flag) {
				reconnent_ = flag;
			}

			int connect(std::string ip, int port) {
				server_ip_ = std::move(ip);
				server_port_ = std::move(port);
				auto tuple_data = handshake(ip, "/");
				if (tcp_handler_ == nullptr){
					return -1;
				}

				return tcp_handler_->connect(server_ip_, server_port_, std::get<0>(tuple_data), std::get<1>(tuple_data),recv_observer_,std::bind(&webscoket_client::on_close, this, std::placeholders::_1, std::placeholders::_2));
			}

			int send(const char* data, const std::size_t count) {
				if (tcp_handler_ == nullptr){
					return -1;
				}

				return tcp_handler_->to_send(data, count);
			}

			void run(size_t thread_num = std::thread::hardware_concurrency()) {
				io_service_poll::get_instance().run(thread_num);
			}
		private:
			//向ws服务器进行握手
			std::tuple<std::string,std::string> handshake(std::string host, std::string path) {
				std::string msg = comm_header_[0];
				msg += comm_header_[1];
				msg += comm_header_[2];
				msg += "Sec-WebSocket-Key: ";

				std::string random_key;
				unit::random_str(random_key, 16);
				std::string raw_key = base64_encode(random_key);
				send_sec_ws_key_ = base64_encode((unsigned char const*)raw_key.c_str(), 16);
				msg += send_sec_ws_key_;
				msg += "\r\n";
				msg += "Origin: ";
				msg += host;
				msg += "\r\n";
				msg += "Sec-WebSocket-Version: 13";
				msg += "\r\n\r\n";

				return { msg,send_sec_ws_key_};
			}
		private:
			void on_close(std::shared_ptr<websocket::ws_tcp_handle> handler, const boost::system::error_code& ec) {
				if (handler == nullptr){
					return;
				}

				if (ec) {
					boost::system::error_code e;

					//注意close的地方，如果先close，再获取远端的地址的ip和端口，都会变成0
					handler->close_socket();
					auto tuple_data =handshake(server_ip_,"/");
					reconnect(handler, server_ip_, server_port_,std::get<0>(tuple_data),std::get<1>(tuple_data));
				}
			}

			void reconnect(std::shared_ptr<websocket::ws_tcp_handle> ptr,std::string ip, int port, const std::string& handshake_msg,const std::string & handshake_key) {
				if (timer_ == nullptr || ptr == nullptr){
					return;
				}

				timer_->expires_from_now(std::chrono::seconds(3));
				timer_->async_wait([this, ptr, ip, port,handshake_msg,handshake_key](const boost::system::error_code& ec) {
					if (ec) {
						return;
					}

					if (ptr->get_connect_status() == wheel::websocket::connectinged) {
						return;
					}

					ptr->connect(ip, port, handshake_msg, handshake_key,recv_observer_,std::bind(&webscoket_client::on_close, this, std::placeholders::_1, std::placeholders::_2));
					timer_->expires_from_now(std::chrono::seconds(3));
					reconnect(ptr, ip, port,handshake_msg, handshake_key);
					});
			}
		private:
			bool reconnent_;
			int parser_type_ = json;
			int server_port_;
			std::size_t header_size_ = 0;
			std::size_t packet_size_offset_ = 0;
			std::size_t packet_cmd_offset_ = 0;
			std::string send_sec_ws_key_;
			std::string comm_header_[3] = {
				"GET / HTTP/1.1\r\n",
				"Connection: upgrade\r\n",
				"Upgrade: websocket\r\n"
				};
			std::string server_ip_;
			MessageEventObserver recv_observer_;
			std::shared_ptr<boost::asio::io_service::strand>strand_;
			std::shared_ptr<ws_tcp_handle> tcp_handler_{};
			std::unique_ptr<boost::asio::steady_timer> timer_{};

		};
	}
}
#endif // websocket_client_h__
