#ifndef tcp_handle_h__
#define tcp_handle_h__

#include<memory>
#include <functional>
#include <list>
#include <iostream>
#include "native_stream.hpp"
#include "send_buffer.hpp"
#include "bin_parser.hpp"
#include"unit.hpp"
#include "picohttpparser.hpp"
#include "io_service_poll.hpp"
#include "traits.hpp"

namespace wheel {
	namespace tcp_socket {
		const int g_client_reconnect_seconds = 3;
		using TCP = boost::asio::ip::tcp;
		using ADDRESS = boost::asio::ip::address;
		class tcp_handle;
		typedef std::function<void(std::shared_ptr<tcp_handle >, std::shared_ptr<native_stream>) > MessageEventObserver;

		typedef std::function<void(std::shared_ptr<tcp_handle >, const boost::system::error_code&) > CloseEventObserver;

		typedef std::function<void(std::shared_ptr<tcp_handle >) > ConnectEventObserver;
		enum
		{
			disconnect = -1,
			connectinged = 0,
		};

		class tcp_handle :public std::enable_shared_from_this<tcp_handle>
		{
		public:
			tcp_handle(const std::shared_ptr<boost::asio::io_service::strand>&strand,std::size_t header_size,std::size_t packet_size_offset, std::size_t packet_cmd_offset)
				:strand_(strand)
				, connect_status_(-1)
				, header_size_(header_size)
				, packet_size_offset_(packet_size_offset)
				, packet_cmd_offset_(packet_cmd_offset)
			{
				try
				{
					socket_ = std::make_shared<boost::asio::ip::tcp::socket>(*io_service_poll::get_instance().get_io_service());
					timer_ = wheel::traits ::make_unique<boost::asio::steady_timer>(*io_service_poll::get_instance().get_io_service());
				}catch (std::exception &ex){
					std::cout << ex.what() << std::endl;
					socket_ = nullptr;
					timer_ = nullptr;
				}
			}

			~tcp_handle() {
				set_connect_status(disconnect);
				timer_ = nullptr;
				socket_ = nullptr;
				protocol_parser_ = nullptr;
			}

			void set_protocol_parse_type(int type) {
				parser_type_ = type;
				if (protocol_parser_ == nullptr) {
					try
					{
						protocol_parser_ = create_object(type, header_size_, packet_size_offset_, packet_cmd_offset_);
					}	catch (const std::exception&ex)
					{
						std::cout << ex.what() << std::endl;
						protocol_parser_ = nullptr;
					}

				}
			}

			std::shared_ptr<TCP::socket>get_socket() const {
				return socket_;
			}

			void activate() {
				set_connect_status(connectinged);
				connect_observer_(shared_from_this());
				init();
				to_read();
			}

			int close_send_endpoint() {
				if (socket_ == nullptr || !socket_->is_open()) {
					return -1;
				}

				boost::system::error_code ec;
				socket_->shutdown(TCP::socket::shutdown_send, ec);
				return ec.value();
			}

			int close_rs_endpoint() {
				if (socket_ == nullptr || !socket_->is_open()) {
					return -1;
				}

				boost::system::error_code ec;
				socket_->shutdown(TCP::socket::shutdown_both, ec);
				return ec.value();
			}

			int close_recv_endpoint() {
				if (socket_ == nullptr || !socket_->is_open()) {
					return -1;
				}

				//执行操作之后，会模拟进入客户端关闭socket的操作
				boost::system::error_code ec;
				socket_->shutdown(TCP::socket::shutdown_receive, ec);
				return ec.value();
			}

			int close_socket() {
				if (socket_ == nullptr || !socket_->is_open()) {
					return -1;
				}

				boost::system::error_code e;
				socket_->close(e);
				return e.value();
			}


			int to_send(const native_stream& stream) {
				return to_send(stream.buffer_.c_str(), stream.get_size());
			}

			int to_send(const char* data, const std::size_t count) {
				if (socket_ == nullptr || !socket_->is_open()) {
					return -1;
				}

				if (data == nullptr || count ==0){
					return -1;
				}

				//如果下一个包来，就可以放在末尾发，可以利用当前的内存，达到写多少，发多少的效果
				//最好枷锁
				std::shared_ptr<send_buffer>ptr = nullptr;
				while (data_lock_.test_and_set(std::memory_order_acquire));
				if (!send_buffers_.empty()) {
					if (!send_buffers_.back()->write(data, count))
					{
						try
						{
							ptr = std::make_shared<send_buffer>(data, count);
						}catch (const std::exception&ex)
						{
							std::cout << ex.what() << std::endl;
						}

						if (ptr != nullptr) {
							send_buffers_.push_back(ptr);
						}
					}
				}
				else {
					try
					{
						ptr = std::make_shared<send_buffer>(data, count);
					}catch (const std::exception&ex)
					{
						std::cout << ex.what() << std::endl;
					}
				
					if (ptr != nullptr) {
						send_buffers_.push_back(ptr);
					}
				}

				data_lock_.clear(std::memory_order_release);
				if (write_count_ == 0) {
					++write_count_;//1:等于0就相加，2:若此变量为1，说明有错误 

					//自由函数boost::asio::async_write 如果指定buffer的length没有写完或出错会在run loop中一直执行
					socket_->async_send(boost::asio::buffer(send_buffers_.front()->data(), send_buffers_.front()->size()), std::bind(&tcp_handle::on_write, this,
						std::placeholders::_1, std::placeholders::_2));
				}

				return 0;
			}

			void set_reconect_seconds(int seconds) {
				seconds_ = seconds;
			}

			int connect(std::string ip, int port, MessageEventObserver recv_observer, CloseEventObserver close_observer) {
				if (socket_ == nullptr){
					return -1;
				}

				boost::system::error_code err;
				boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(ip, err), port);
				socket_->connect(endpoint, err);

				connect_status_ = err.value() == 0 ? connectinged : disconnect;

				if (err.value() == 0) {
					register_close_observer(std::move(close_observer));
					register_recv_observer(std::move(recv_observer));
					to_read();
					return 0;
				}

				return err.value();
			}

			//客户端支持异步重连
			void reconect_server(std::string ip, int port, MessageEventObserver recv_observer, CloseEventObserver  close_observer) {
				if (timer_ == nullptr){
					return;
				}

				timer_->expires_from_now(std::chrono::seconds(seconds_));
				timer_->async_wait(strand_->wrap([this, ip, port, recv_observer, close_observer](const boost::system::error_code& ec) {
					if (ec) {
						return;
					}

					if (connect_status_ == connectinged) {
						return;
					}

					async_connect(ip, port, recv_observer, close_observer);
					reconect_server(ip, port, recv_observer, close_observer);
					}));
			}

			void register_connect_observer(ConnectEventObserver observer) {
				connect_observer_ = observer;
			}

			void register_close_observer(CloseEventObserver observer) {
				close_observer_ = observer;
			}

			void register_recv_observer(MessageEventObserver observer) {
				recv_observer_ = observer;
			}

			int get_connect_status()const {
				return connect_status_;
			}

			std::shared_ptr<stream_format>get_read_parser() {
				if (protocol_parser_ == nullptr){
					return nullptr;
				}

				return protocol_parser_->get_read_parser();
			}

			std::shared_ptr<stream_format>get_write_parser() {
				if (protocol_parser_ == nullptr){
					return nullptr;
				}

				return protocol_parser_->get_write_parser();
			}

			std::size_t get_base_receive_buffer_size() {
				if (socket_ == nullptr) {
					return 0;
				}

				boost::asio::socket_base::receive_buffer_size opt;
				socket_->get_option(opt);
				return opt.value();
			}

			std::size_t get_base_send_buffer_size() {
				if (socket_ == nullptr) {
					return 0;
				}

				boost::asio::socket_base::send_buffer_size opt;
				socket_->get_option(opt);
				return opt.value();
			}

			int set_base_receive_buffer_size(int size, bool force = false) {
				if (socket_ == nullptr){
					return -1;
				}

				boost::system::error_code ec;

				///net.core.rmem_max, 大于这个系数值的话，就得先修改这个系统参数了
				//net.core.rmem_default 默认大小
				//	/proc/sys/net/ipv4/tcp_window_scaling	"1"	启用 RFC 1323 定义的 window scaling；要支持超过 64KB 的窗口，必须启用该值。
				// 系统根据负载，在这三个值之间调整SOCKET窗口大小
				// net.ipv4.tcp_wmem = 4096	16384	4194304
				// net.ipv4.tcp_rmem = 4096	87380	4194304

				// 	/proc/sys/net/ipv4/tcp_wmem	"4096 16384 131072"	为自动调优定义每个 socket 使用的内存。
				// 		第一个值是为 socket 的发送缓冲区分配的最少字节数。
				// 		第二个值是默认值（该值会被 wmem_default 覆盖），缓冲区在系统负载不重的情况下可以增长到这个值。
				// 		第三个值是发送缓冲区空间的最大字节数（该值会被 wmem_max 覆盖）。

				if (force == true) {
					boost::asio::socket_base::receive_buffer_size  rs(size);
					socket_->set_option(rs, ec);
				}
				else {
					std::size_t recv_buffer_size = get_base_receive_buffer_size();
					if (ec.value() != 0 || recv_buffer_size < size)
					{
						boost::asio::socket_base::receive_buffer_size  rs(size);
						socket_->set_option(rs, ec);
					}
				}

				return ec.value();
			}

			int set_base_send_buffer_size(int send_buffer_size, bool force=false)
			{
				if (socket_ == nullptr) {
					return -1;

				}
				boost::system::error_code ec;

				///net.core.wmem_max, 大于这个系数值的话，就得先修改这个系统参数了
				//net.core.wmem_default 默认大小
				if (force == true){
					boost::asio::socket_base::send_buffer_size op(send_buffer_size);
					socket_->set_option(op, ec);
				}else{
					std::size_t buffer_size = get_base_send_buffer_size();
					if (ec.value() != 0 || buffer_size < send_buffer_size)
					{
						boost::asio::socket_base::send_buffer_size op(send_buffer_size);
						socket_->set_option(op, ec);
					}
				}

				return ec.value();
			}

		private:
			void init() {
				boost::system::error_code ec;
				//关闭牛逼的算法(nagle算法),防止TCP的数据包在饱满时才发送过去
				socket_->set_option(boost::asio::ip::tcp::no_delay(true), ec);

				//有time_wait状态下，可端口短时间可以重用
				//默认是2MSL也就是 (RFC793中规定MSL为2分钟)也就是4分钟
				set_reuse_address();	
			}

			void set_reuse_address() {
				if (socket_ == nullptr) {
					return;
				}

				boost::system::error_code ec;
				socket_->set_option(boost::asio::socket_base::reuse_address(true), ec);
			}

			void to_read() {
				if (socket_ == nullptr){
					return;
				}

				try
				{
					recv_buffer_ = wheel::traits::make_unique<char[]>(g_packet_buffer_size);
				}catch (std::exception & ex) {
					recv_buffer_ = nullptr;
					std::cout << ex.what() << std::endl;
				}
				
				if (recv_buffer_ == nullptr){
					return;
				}

				socket_->async_read_some(boost::asio::buffer(&recv_buffer_[0],g_packet_buffer_size), strand_->wrap([this](const boost::system::error_code ec, size_t bytes_transferred) {
					if (ec){
						if (this->get_connect_status() == disconnect) {
							return;
						}

						set_connect_status(disconnect);
						close_socket();
						close_observer_(shared_from_this(), ec);
						return;
					}

					streams steams;
					protocol_parser_->read_stream(&recv_buffer_[0], bytes_transferred, steams);;
					for (const std::shared_ptr<native_stream> stream : steams) {
						recv_observer_(shared_from_this(), stream);
					}

					to_read();
					}));
			}
			void async_connect(std::string ip, int port, const MessageEventObserver& recv_observer, const CloseEventObserver& close_observer) {
				if (socket_ == nullptr){
					return;
				}

				socket_->async_connect(TCP::endpoint(ADDRESS::from_string(ip), port), strand_->wrap([this, recv_observer, close_observer](const boost::system::error_code& ec) {
					if (ec) {
						return;
					}

					set_connect_status(connectinged);
					register_close_observer(close_observer);
					register_recv_observer(recv_observer);
					to_read();
					}));
			}
			void on_write(const boost::system::error_code& ec, std::size_t size) {
				--write_count_;

				if (ec) {
					//这地方不能删除conencts,应该直接通知发送错误
					return;
				}

				//最好加锁着地方
				while (data_lock_.test_and_set(std::memory_order_acquire));
				if (send_buffers_.empty()) {
					data_lock_.clear(std::memory_order_release);
					return;
				}

				std::shared_ptr<send_buffer>ptr = send_buffers_.front();
				ptr->read(size);

				if (ptr->size() == 0) {
					send_buffers_.pop_front();
				}

				if (!send_buffers_.empty()) {
					//这样写，会让write_count_ 多加一个1
					//to_send(send_buffers.front()->data(), send_buffers.front()->size());

					//有包就继续发,不管来多少，发多少
					socket_->async_send(boost::asio::buffer(send_buffers_.front()->data(), send_buffers_.front()->size()), strand_->wrap(std::bind(&tcp_handle::on_write, this,
						std::placeholders::_1, std::placeholders::_2)));
					++write_count_;
				}

				data_lock_.clear(std::memory_order_release);
			}

			void set_connect_status(int status) {
				connect_status_ = status;
			}
		private:
			std::shared_ptr<IProtocol_parser>create_object(const int type,
				std::size_t header_size = 0, std::size_t packet_size_offset = 0, std::size_t packet_cmd_offset = 0) {
				if (type == 0) {
					if (header_size ==0 || packet_size_offset ==0 || packet_cmd_offset ==0){
						return std::shared_ptr<IProtocol_parser>(new bin_parser(wheel::PACKET_HEADER_SIZE, wheel::PACKET_SIZE_OFFSET, wheel::PACKET_CMD_OFFSET));
					}

					return std::shared_ptr<IProtocol_parser>(new bin_parser(header_size, packet_size_offset, packet_cmd_offset));
				}

				return nullptr;
			}
		private:
			std::atomic_flag data_lock_ = ATOMIC_FLAG_INIT;
			int connect_status_ = disconnect;
			int seconds_ = g_client_reconnect_seconds;//客户端设置重连
			int parser_type_ = 0; //后续扩展0:二进制流
			std::size_t header_size_;
			std::size_t packet_size_offset_;
			std::size_t packet_cmd_offset_;
			std::int32_t write_count_ = 0;
			ConnectEventObserver		connect_observer_;
			MessageEventObserver		recv_observer_;
			CloseEventObserver			close_observer_;
			std::unique_ptr<char[]>recv_buffer_{};
			std::shared_ptr<boost::asio::io_service::strand>strand_{};
			std::unique_ptr<boost::asio::steady_timer> timer_{};
			std::shared_ptr<TCP::socket> socket_{};
			std::shared_ptr<IProtocol_parser>protocol_parser_{};
			std::list<std::shared_ptr<wheel::send_buffer>> send_buffers_;
		};
	}
}

#endif // tcp_handle_h__
