#ifndef http_tcp_handle_h__
#define http_tcp_handle_h__

#ifdef WHEEL_ENABLE_SSL
#include <boost/asio/ssl.hpp>
#endif
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include "unit.hpp"
#include "http_response.hpp"
#include "http_request.hpp"
#include "http_multipart_reader.hpp"
#include "uuid.h"
#include "io_service_poll.hpp"


namespace wheel {
	namespace http_servers {
		struct ssl_configure {
			std::string cert_file;          //private cert
			std::string key_file;           //private key
			std::string passp_hrase;        //password;//私有key，是否输入密码
			std::string pem_flie;           //*.pem文件
		};

		namespace fs = boost::filesystem;
		using http_handler = std::function<void(request&, response&)>;
		class http_tcp_handle :public std::enable_shared_from_this<http_tcp_handle> {
			typedef std::function<void(std::shared_ptr<http_tcp_handle >, const boost::system::error_code&) > CloseEventObserver;

			typedef std::function<void(std::shared_ptr<http_tcp_handle >) > ConnectEventObserver;

		public:
			http_tcp_handle() = delete;
			http_tcp_handle(const std::shared_ptr<boost::asio::io_service::strand>&strand,http_handler& handle,
				const std::string& static_dir, const ssl_configure& ssl_conf, const bool need_response_time) :http_handler_(handle)
				,static_dir_(static_dir), need_response_time_(need_response_time), strand_(strand){
#ifndef WHEEL_ENABLE_SSL 
				socket_ = std::make_shared<boost::asio::ip::tcp::socket>(*io_service_poll::get_instance().get_io_service());
#endif
				request_ = std::make_unique<request>();
				response_ = std::make_unique<response>();

#ifdef WHEEL_ENABLE_SSL
				is_ssl_ = true;
				if (!init_ssl_context(ssl_conf)) {
					exit(0);
				}
#endif
				init_multipart_parser();
			}

			boost::asio::ip::tcp::socket* get_socket()const {
#ifdef WHEEL_ENABLE_SSL
				return &ssl_socket_->next_layer();
#else
				return socket_.get();
#endif
			}

			~http_tcp_handle() {
				close();
			}

			void activate() {
				connect_observer_(shared_from_this());
				response_->enable_response_time(need_response_time_);
				init();
				do_read();
			}

			void register_connect_observer(ConnectEventObserver observer) {
				connect_observer_ = observer;
			}

			void register_close_observer(CloseEventObserver observer) {
				close_observer_ = observer;
			}
			void set_multipart_begin(std::function<void(request&, std::string&)> begin) {
				multipart_begin_ = std::move(begin);
			}
		private:
			bool check_argument() {
#ifdef WHEEL_ENABLE_SSL
				if (ssl_socket_ == nullptr) {
					return false;
				}
#else
				if (socket_ == nullptr) {
					return false;
				}
#endif
				return true;
			}

			auto& socket() {
#ifdef WHEEL_ENABLE_SSL
				return *ssl_socket_;
#else
				return *socket_;
#endif	
			}

#ifdef WHEEL_ENABLE_SSL
			bool init_ssl_context(const ssl_configure& ssl_conf) {
				unsigned long ssl_options = boost::asio::ssl::context::default_workarounds
					| boost::asio::ssl::context::no_sslv2
					| boost::asio::ssl::context::single_dh_use;
				try {
					boost::asio::ssl::context ssl_context(boost::asio::ssl::context::sslv23);
					ssl_context.set_options(ssl_options);
					if (!ssl_conf.passp_hrase.empty()) {
						ssl_context.set_password_callback([ssl_conf](size_t, boost::asio::ssl::context_base::password_purpose) {return ssl_conf.passp_hrase; });
					}

					boost::system::error_code ec;
					if (fs::exists(ssl_conf.cert_file, ec)) {
						ssl_context.use_certificate_chain_file(std::move(ssl_conf.cert_file));
					}
					else {
						std::cout << "server.crt is empty" << std::endl;
						return false;
					}

					if (fs::exists(ssl_conf.key_file, ec)) {
						ssl_context.use_private_key_file(std::move(ssl_conf.key_file), boost::asio::ssl::context::pem);
					}
					else {
						std::cout << "server.key is empty" << std::endl;
						return false;
					}

					if (fs::exists(ssl_conf.pem_flie, ec)) {
						ssl_context.use_tmp_dh_file(std::move(ssl_conf.pem_flie));
					}

					ssl_socket_ = std::make_unique<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(*io_service_poll::get_instance().get_io_service(), ssl_context);
				}
				catch (const std::exception & e) {
					std::cout << e.what() << std::endl;
					return false;
				}

				return true;
			}
#endif
			void do_read() {
				if (request_ == nullptr || response_ == nullptr) {
					release_session(boost::asio::error::make_error_code(
						static_cast<boost::asio::error::basic_errors>(boost::system::errc::invalid_argument)));
					return;
				}

				len_ = 0;
				last_transfer_ = 0;
				is_receve_all_chunked = false;
				req_content_type_ = content_type::unknown;
				request_->reset();
				response_->reset();

				if (is_ssl_ && !has_shake_) {
					async_handshake();
				}
				else {
					async_read_some();
				}
			}

			void async_handshake() {
#ifdef WHEEL_ENABLE_SSL     
				if (ssl_socket_ == nullptr) {
					release_session(boost::asio::error::make_error_code(
						static_cast<boost::asio::error::basic_errors>(boost::system::errc::invalid_argument)));
					return;
				}

				ssl_socket_->async_handshake(boost::asio::ssl::stream_base::server,strand_->wrap([self = shared_from_this()](const boost::system::error_code& error) {
					if (error) {
						self->release_session(boost::asio::error::make_error_code(
							static_cast<boost::asio::error::basic_errors>(error.value())));
						//std::cout << error.message() << std::endl;

						self->has_shake_ = false;
						return;
					}

					self->has_shake_ = true;
					self->async_read_some();
					}));
#endif
			}

			void async_read_some() {
				if (!check_argument()) {
					return;
				}

				socket().async_read_some(boost::asio::buffer(request_->buffer(), request_->buffer_size()),strand_->wrap([self = shared_from_this()](const boost::system::error_code& ec, std::size_t bytes_transferred) {
					if (ec) {
						self->release_session(boost::asio::error::make_error_code(
							static_cast<boost::asio::error::basic_errors>(ec.value())));
						//std::cout << ec.message() << std::endl;

						self->has_shake_ = false;
						return;
					}

					self->handle_read(bytes_transferred);
					}));
			}
			void close() {
				if (!check_argument()) {
					return;
				}

				boost::system::error_code ignored_ec;
#ifdef WHEEL_ENABLE_SSL
				if (ssl_socket_->lowest_layer().is_open()) {
					ssl_socket_->lowest_layer().shutdown(
						boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);

					ssl_socket_->lowest_layer().close(ignored_ec);
				}
#else
				if (socket_->is_open()) {
					socket_->shutdown(
						boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);

					socket_->close(ignored_ec);
				}
#endif
			}

			void release_session(const boost::system::error_code& ec) {
				//shared_from_this不能被调两次
				if (close_observer_ == nullptr) {
					return;
				}

				close_observer_(shared_from_this(), ec);
			}

			void do_read_head() {
				if (!check_argument()) {
					return;
				}

				socket().async_read_some(boost::asio::buffer(request_->buffer(), request_->left_size()),strand_->wrap([self = shared_from_this()](const boost::system::error_code& ec, std::size_t bytes_transferred) {
					if (ec) {
						self->release_session(boost::asio::error::make_error_code(
							static_cast<boost::asio::error::basic_errors>(ec.value())));
						return;
					}

					self->handle_read(bytes_transferred);
					}));
			}

			void handle_read(std::size_t bytes_transferred) {
				last_transfer_ = request_->current_size();
				bool at_capacity = request_->update_and_expand_size(bytes_transferred);
				//support 3M buffer
				if (at_capacity) {
					response_back(status_type::bad_request, "The request is too long, limitation is 3M");
					return;
				}

				int ret = request_->parse_header(len_);

				if (ret == parse_status::has_error) {
					response_back(status_type::bad_request);
					return;
				}

				check_keep_alive();
				if (ret == parse_status::not_complete) {
					do_read_head();
				}
				else {
					constexpr int offset = 4;
					if (bytes_transferred > ret + offset) {
						std::string str(request_->data() + ret, offset);
						if (str == "GET " || str == "POST") {
							handle_pipeline(ret, bytes_transferred);
							return;
						}
					}

					request_->set_last_len(len_);
					handle_request_on_respone(bytes_transferred);
				}
			}

			void handle_pipeline(int ret, std::size_t bytes_transferred) {
				response_->set_delay(true);
				request_->set_last_len(len_);
				handle_request_on_respone(bytes_transferred);
				last_transfer_ += bytes_transferred;

				(len_ == 0) ? (len_ = ret) : (len_ += ret);

				const std::string rep_str = response_->response_str();
				int result = 0;
				size_t left = ret;
				bool head_not_complete = false;
				bool body_not_complete = false;
				size_t left_body_len = 0;
				while (true) {
					result = request_->parse_header(len_);
					if (result == -1) {
						return;
					}

					if (result == -2) {
						head_not_complete = true;
						break;
					}

					auto total_len = request_->total_len();

					if (total_len <= (bytes_transferred - len_)) {
						request_->set_last_len(len_);
						handle_request_on_respone(bytes_transferred);
					}

					len_ += total_len;

					if (len_ == last_transfer_) {
						break;
					}
					else if (len_ > last_transfer_) {
						auto n = len_ - last_transfer_;
						len_ -= total_len;
						if (n < request_->header_len()) {
							head_not_complete = true;
						}
						else {
							body_not_complete = true;
							left_body_len = n;
						}

						break;
					}
				}

				response_->set_delay(false);
				boost::asio::async_write(socket(), boost::asio::buffer(rep_str.data(), rep_str.size()),
					[head_not_complete, body_not_complete, left_body_len, this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
						if (head_not_complete) {
							do_read_head();
							return;
						}

						if (body_not_complete) {
							request_->set_left_body_size(left_body_len);
							do_read_body();
							return;
						}

						handle_write(ec);
					});
			}

			void handle_request_on_respone(std::size_t bytes_transferred) {
				if (request_->has_body()) {
					req_content_type_ = get_content_type();
					request_->set_http_type(req_content_type_);
					if (request_->is_chunked()) {
						handle_chunked();
						return;
					}

					handle_no_chunked(bytes_transferred);
					return;
				}

				handle_no_body_respone();
			}

			void handle_no_chunked(std::size_t bytes_transferred) {
				switch (req_content_type_) {
				case content_type::string:
				case content_type::unknown:
					handle_string_body();
					break;
				case content_type::multipart:
					handle_multipart();
					break;
				case content_type::urlencoded:
					handle_form_urlencoded(bytes_transferred);
					break;
				case content_type::octet_stream:
					handle_octet_stream(bytes_transferred);
					break;
				default:
					response_back(status_type::bad_request, "content-type not support");
					break;
				}
			}

			//-------------octet-stream----------------//
			void handle_octet_stream(size_t bytes_transferred) {
				try {
					std::string name = static_dir_ + uuids::uuid_system_generator{}().to_short_str();
					request_->open_upload_file(name);
				}
				catch (const std::exception & ex) {
					response_back(status_type::internal_server_error, ex.what());
					return;
				}

				request_->set_state(data_proc_state::data_continue);//data
				size_t part_size = bytes_transferred - request_->header_len();
				if (part_size != 0) {
					request_->reduce_left_body_size(part_size);
					request_->set_part_data({ request_->current_part(), part_size });
					request_->write_upload_data(request_->current_part(), part_size);
				}

				if (request_->has_recieved_all()) {
					//on finish
					request_->set_state(data_proc_state::data_end);
					call_back();
					do_write();
				}
				else {
					request_->fit_size();
					request_->set_current_size(0);
					do_read_octet_stream_body();
				}
			}

			void do_read_octet_stream_body() {
				//加了这个buffer不会溢出
				boost::asio::async_read(socket(), boost::asio::buffer(request_->buffer(), request_->left_body_len()), boost::asio::transfer_exactly(request_->left_body_len()),
					strand_->wrap([self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred) {
						if (ec) {
							self->request_->set_state(data_proc_state::data_error);
							self->response_back(status_type::bad_request, "read octet_stream data error");
							return;
						}

						self->request_->set_part_data({ self->request_->buffer(), bytes_transferred });
						self->request_->write_upload_data(self->request_->buffer(), bytes_transferred);

						self->request_->reduce_left_body_size(bytes_transferred);

						if (self->request_->body_finished()) {
							self->request_->set_state(data_proc_state::data_end);
							self->call_back();
							self->do_write();
						}
						else {
							self->do_read_octet_stream_body();
						}
						}));
			}

			//无body应答
			void handle_no_body_respone() {
				if (request_->get_state() == data_proc_state::data_error) {
					response_back(status_type::bad_request, "data_proc_state: data_error");
					return;
				}

				bool r = handle_gzip();
				if (!r) {
					response_back(status_type::bad_request, "gzip uncompress error");
					return;
				}

				call_back();

				if (!response_->need_delay()) {
					do_write();
				}
			}

			content_type get_content_type() {
				const std::string content_type = request_->get_header_value("content-type");
				if (!content_type.empty()) {
					if (content_type.find("application/x-www-form-urlencoded") != std::string::npos) {
						return std::move(content_type::urlencoded);
					}
					else if (content_type.find("multipart/form-data") != std::string::npos) {
						auto size = content_type.find("=");
						auto bd = content_type.substr(size + 1, content_type.length() - size);
						if (bd[0] == '"' && bd[bd.length() - 1] == '"') {
							bd = bd.substr(1, bd.length() - 2);
						}
						std::string boundary(bd.data(), bd.length());
						multipart_parser_.set_boundary("\r\n--" + std::move(boundary));
						return std::move(content_type::multipart);
					}
					else if (content_type.find("application/octet-stream") != std::string::npos) {
						return std::move(content_type::octet_stream);
					}

					return std::move(content_type::string);
				}

				return std::move(content_type::unknown);
			}

			//-------------form urlencoded----------------//
			//TODO: here later will refactor the duplicate code
			void handle_form_urlencoded(size_t bytes_transferred) {
				if (request_->at_capacity()) {
					response_back(status_type::bad_request, "The request is too long, limitation is 3M");
					return;
				}

				if (request_->has_recieved_all()) {
					handle_url_urlencoded_body();
				}
				else {
					request_->expand_size();
					request_->reduce_left_body_size((bytes_transferred - request_->header_len()));
					do_read_form_urlencoded();
				}
			}

			void handle_chunked() {
				struct phr_chunked_decoder dec = { 0 };
				std::string body = request_->body();
				size_t size = body.size();
				auto ret = phr_decode_chunked(&dec, &body[0], &size);
				if (ret == -1) {
					response_back(status_type::internal_server_error, "not support yet");
					return;
				}

				//数据
				if (ret == CHUNKED_IN_CHUNK_DATA) {
					std::string context;
					context.resize(size);
					memcpy(&context[0], body.c_str(), size);
					request_->set_client_chunked_data(context.c_str());
					handle_chunked_response();
					return;
				}

				//imperfect chunked data
				if (dec._state >= 0 && dec.bytes_left_in_chunk > 0) {
					body = request_->body();
					request_->set_client_chunked_data(body.c_str());
					request_->set_current_size(0);
					read_chunk_data(dec.bytes_left_in_chunk);
					return;
				}

				response_back(status_type::bad_request, "chunked data parse failure");
			}

			void read_chunk_data(int64_t read_len) {
				//boost::asio::async_read:注意参数填写,会超出内存越界
				socket().async_read_some(boost::asio::buffer(request_->buffer(), read_len),strand_->wrap([self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred) {
					if (ec) {
						self->response_back(status_type::bad_request, "read chunked data failure");
						return;
					}

					std::string data;
					data.resize(bytes_transferred);
					memcpy(&data[0], self->request_->buffer(0), bytes_transferred);
					self->request_->set_client_chunked_data(data.c_str());

					struct phr_chunked_decoder dec = { 0 };
					std::string body = self->request_->get_client_chunked_data();
					size_t size = body.size();
					auto ret = phr_decode_chunked(&dec, &body[0], &size);
					if (ret == -1) {
						self->response_back(status_type::bad_request, "read chunked data failure");
						return;
					}

					//一直受到完整的数据位置
					if (ret == CHUNKED_IN_CHUNK_DATA) {
						std::string context;
						context.resize(size);
						memcpy(&context[0], body.c_str(), size);
						self->request_->reset_client_chunked_data(context.c_str());
						self->handle_chunked_response();
						return;
					}

					self->read_imcomplete_chunk_data(self->request_->buffer_size());
					}));
			}

			void read_imcomplete_chunk_data(size_t read_len) {
				socket().async_read_some(boost::asio::buffer(request_->get_buffer(0), read_len),strand_->wrap([self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred) {
					if (ec) {
						self->response_back(status_type::bad_request, "read chunked data failure");
						return;
					}

					std::string data;
					data.resize(bytes_transferred);
					memcpy(&data[0], self->request_->buffer(0), bytes_transferred);
					self->request_->set_client_chunked_data(data.c_str());

					struct phr_chunked_decoder dec = { 0 };
					std::string body = self->request_->get_client_chunked_data();
					size_t size = body.size();
					auto ret = phr_decode_chunked(&dec, &body[0], &size);
					if (ret == -1) {
						self->response_back(status_type::bad_request, "read chunked data failure");
						return;
					}

					//一直受到完整的数据位置
					if (ret == CHUNKED_IN_CHUNK_DATA) {
						std::string context;
						context.resize(size);
						memcpy(&context[0], body.c_str(), size);
						self->request_->reset_client_chunked_data(context.c_str());
						self->handle_chunked_response();
						return;
					}

					self->read_imcomplete_chunk_data(self->request_->buffer_size());
					}));
			}

			void do_read_form_urlencoded() {
				//加了这个buffer不会溢出
				boost::asio::async_read(socket(), boost::asio::buffer(request_->buffer(), request_->left_body_len()), boost::asio::transfer_exactly(request_->left_body_len()),
					strand_->wrap([self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred) {
						if (ec) {
							self->response_back(status_type::bad_request, "read form_urlencoded data error");
							return;
						}

						self->request_->update_size(bytes_transferred);
						self->request_->reduce_left_body_size(bytes_transferred);

						if (self->request_->body_finished()) {
							self->handle_url_urlencoded_body();
						}
						else {
							self->do_read_form_urlencoded();
						}
						}));
			}

			void handle_url_urlencoded_body() {
				bool success = request_->parse_form_urlencoded();

				if (!success) {
					response_back(status_type::bad_request, "read form_urlencoded data error");
					return;
				}

				call_back();
				if (!response_->need_delay()) {
					do_write();
				}
			}

			/****************** begin handle http body data *****************/
			void handle_string_body() {
				if (request_->at_capacity()) {
					response_back(status_type::bad_request, "The request is too long, limitation is 3M");
					return;
				}

				if (request_->has_recieved_all()) {
					handle_body();
				}
				else {
					request_->expand_size();

					size_t part_size = request_->current_size() - request_->header_len();
					if (part_size == -1) {
						return;
					}

					request_->reduce_left_body_size(part_size);
					do_read_body();
				}
			}

			void handle_multipart() {
				if (upload_check_) {
					bool r = (*upload_check_)(*request_, *response_);
					if (!r) {
						release_session(boost::asio::error::make_error_code(boost::asio::error::invalid_argument));
						return;
					}
				}

				bool has_error = parse_multipart(request_->header_len(), request_->current_size() - request_->header_len());

				if (has_error) {
					response_back(status_type::bad_request, "mutipart error");
					return;
				}

				if (request_->has_recieved_all_part()) {
					call_back();
					do_write();
				}
				else {
					request_->set_current_size(0);
					do_read_multipart();
				}
			}

			void do_read_multipart() {
				request_->fit_size();
				//boost::asio::transfer_exactly,加了这个buffer不会溢出
				boost::asio::async_read(socket(), boost::asio::buffer(request_->buffer(), request_->left_body_len()), boost::asio::transfer_exactly(request_->left_body_len()),
					strand_->wrap([self = shared_from_this()](boost::system::error_code ec, std::size_t length) {
						if (ec) {
							self->request_->set_state(data_proc_state::data_error);
							self->response_back(status_type::bad_request, "read multipart data error");
							return;
						}

						bool has_error = self->parse_multipart(0, length);

						if (has_error) { //parse error
							self->response_back(status_type::bad_request, "read multipart data error");
							return;
						}

						if (self->request_->body_finished()) {
							self->call_back();
							self->do_write();
							return;
						}

						self->request_->set_current_size(0);
						self->do_read_part_data();
						}));
			}

			void do_read_part_data() {
				boost::asio::async_read(socket(), boost::asio::buffer(request_->buffer(), request_->left_body_size()), boost::asio::transfer_exactly(request_->left_body_size()),
					strand_->wrap([self = shared_from_this()](boost::system::error_code ec, std::size_t length) {
						if (ec) {
							self->response_back(status_type::bad_request, "read multipart data error");
							return;
						}

						bool has_error = self->parse_multipart(0, length);

						if (has_error) {
							self->response_back(status_type::bad_request, "read multipart data error");
							return;
						}

						if (!self->request_->body_finished()) {
							self->do_read_part_data();
						}
						else {
							self->call_back();
							self->do_write();
						}
						}));
			}

			bool parse_multipart(size_t size, std::size_t length) {
				request_->set_part_data(std::string(request_->buffer(size), length));
				std::string multipart_body = request_->get_part_data();
				size_t bufsize = multipart_body.length();

				size_t fed = 0;
				do {
					size_t ret = multipart_parser_.feed(multipart_body.data() + fed, multipart_body.length() - fed);
					fed += ret;
				} while (fed < bufsize && !multipart_parser_.stopped());

				if (multipart_parser_.has_error()) {
					request_->set_state(data_proc_state::data_error);
					return true;
				}

				request_->reduce_left_body_size(length);
				return false;
			}

			void do_read_body() {
				//加了boost::asio::transfer_exactly 不会发生buffer数组越界
				boost::asio::async_read(socket(), boost::asio::buffer(request_->buffer(), request_->left_body_len()), boost::asio::transfer_exactly(request_->left_body_len()),
					strand_->wrap([self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred) {
						if (ec) {
							self->response_back(status_type::bad_request, "body read failure");
							return;
						}

						self->request_->update_size(bytes_transferred);
						self->request_->reduce_left_body_size(bytes_transferred);

						if (self->request_->body_finished()) {
							self->handle_body();
						}
						else {
							self->do_read_body();
						}
						}));
			}

			void init() {
				if (!check_argument()) {
					return;
				}

				boost::system::error_code ec;
				//关闭牛逼的算法(nagle算法),防止TCP的数据包在饱满时才发送过去
				boost::asio::ip::tcp::no_delay option(true);
#ifdef WHEEL_ENABLE_SSL
				ssl_socket_->lowest_layer().set_option(option);
#else
				socket_->set_option(option, ec);

#endif
				//有time_wait状态下，可端口短时间可以重用
				//默认是2MSL也就是 (RFC793中规定MSL为2分钟)也就是4分钟
				set_reuse_address();
			}

			void set_reuse_address() {
				boost::system::error_code ec;
				boost::asio::socket_base::reuse_address readdress(true);
#ifdef WHEEL_ENABLE_SSL
				ssl_socket_->lowest_layer().set_option(readdress, ec);
#else
				socket_->set_option(readdress, ec);
#endif
			}

			void response_back(status_type status, std::string&& content) {
				response_->set_status_and_content(status, std::move(content));
				do_write(); //response to client
			}

			void response_back(status_type status) {
				response_->set_status_and_content(status);
				do_write(); //response to client
			}

			void do_write() {
				transfer_type trans_type = response_->get_trans_type();
				if (trans_type == transfer_type::normal) {
					do_no_chunked_write();
					return;
				}

				do_chunked_write_data();
			}

			void do_chunked_write_data() {
				std::string chunked_header = response_->get_chunked_header();
				const auto chunked_datas = response_->get_chunked_data();
				for (const auto& str : chunked_datas) {
					chunked_header.append(str);
				}

				boost::asio::async_write(socket(), boost::asio::buffer(chunked_header.data(), chunked_header.size()), boost::asio::transfer_exactly(chunked_header.size()),
					strand_->wrap([self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred) {
						if (ec) {
							self->response_back(status_type::bad_gateway, "write chunked header failure");
							return;
						}

						self->do_read();
						}));
			}

			/*********************此接口暂时没有后面待扩展********************************/
			void do_chunked_write_body() {
				std::list<std::string> buffers = response_->get_chunked_data();
				if (buffers.empty()) {
					response_back(status_type::bad_gateway, "write chunked data failure");
					return;
				}

				const std::string data = buffers.front();
				buffers.pop_front();
				boost::asio::async_write(socket(), boost::asio::buffer(data.data(), data.size()), boost::asio::transfer_exactly(data.size()),
					strand_->wrap([self = shared_from_this(), buffer = std::move(buffers)](const boost::system::error_code& ec, size_t bytes_transferred) {
					if (ec) {
						self->response_back(status_type::bad_gateway, "write chunked data failure");
						return;
					}

					if (buffer.empty()) {
						self->do_read();
						return;
					}

					self->handler_chunked_write_body(buffer);
				}));
			}

			/*********************此接口暂时没有后面待扩展********************************/
			void handler_chunked_write_body(std::list<std::string> buffers) {
				const std::string data = buffers.front();
				buffers.pop_front();

				boost::asio::async_write(socket(), boost::asio::buffer(data.data(), data.size()), boost::asio::transfer_exactly(data.size()),
					strand_->wrap([self = shared_from_this(), buffers = std::move(buffers)](const boost::system::error_code& ec, size_t bytes_transferred) {
					if (ec) {
						self->response_back(status_type::bad_gateway, "write chunked data failure");
						return;
					}

					if (buffers.empty()) {
						self->do_read();
						return;
					}

					self->handler_chunked_write_body(buffers);
				}));
			}


			void do_no_chunked_write() {
				const std::string rep_str = response_->response_str();
				if (rep_str.empty()) {
					std::cout << "send fail:message is empty" << std::endl;
					response_back(status_type::bad_gateway, "send message is empty");
					return;
				}

				boost::asio::async_write(socket(), boost::asio::buffer(rep_str.data(), rep_str.size()),boost::asio::transfer_exactly(rep_str.size()),
					strand_->wrap([self = shared_from_this()](const boost::system::error_code& ec, std::size_t bytes_transferred) {
					self->handle_write(ec);
					}));
			}

			void handle_write(const boost::system::error_code& ec) {
				if (ec) {
					std::cout << "send message failed:" << ec.message() << std::endl;
					release_session(boost::asio::error::make_error_code(
						static_cast<boost::asio::error::basic_errors>(boost::asio::error::connection_reset)));
					return;
				}

				if (!keep_alive_) {
					release_session(boost::asio::error::make_error_code(
						static_cast<boost::asio::error::basic_errors>(boost::asio::error::connection_reset)));
					return;
				}

				do_read();
			}

			void check_keep_alive() {
				auto req_conn_hdr = request_->get_header_value("connection");
				if (request_->is_http11()) {
					keep_alive_ = req_conn_hdr.empty() || !wheel::unit::iequal(req_conn_hdr.data(), req_conn_hdr.size(), "close");
					return;
				}

				keep_alive_ = !req_conn_hdr.empty() && wheel::unit::iequal(req_conn_hdr.data(), req_conn_hdr.size(), "keep-alive");
			}

			void handle_body() {
				if (request_->at_capacity()) {
					response_back(status_type::bad_request, "The body is too long, limitation is 3M");
					return;
				}

				bool r = handle_gzip();
				if (!r) {
					response_back(status_type::bad_request, "gzip uncompress error");
					return;
				}

				if (request_->get_content_type() == content_type::multipart) {
					bool has_error = parse_multipart(request_->header_len(), request_->current_size() - request_->header_len());
					if (has_error) {
						response_back(status_type::bad_request, "mutipart error");
						return;
					}
					do_write();
					return;
				}

				call_back();

				if (!response_->need_delay()) {
					do_write();
				}
			}

			/****************** chunked pass http body data *****************/
			void handle_chunked_string_body() {
				if (request_->at_capacity()) {
					response_back(status_type::bad_request, "The request is too long, limitation is 3M");
					return;
				}

				bool r = handle_gzip();
				if (!r) {
					response_back(status_type::bad_request, "gzip uncompress error");
					return;
				}

				call_back();

				if (!response_->need_delay()) {
					do_write();
				}
			}

			/************chunked pass from-data****************/
			void handle_chunked_multipart() {
				std::string multipart_body = request_->get_client_chunked_data();
				size_t fed = 0;
				size_t bufsize = multipart_body.length();
				do {
					size_t ret = multipart_parser_.feed(multipart_body.data() + fed, multipart_body.length() - fed);
					fed += ret;
				} while (fed < bufsize && !multipart_parser_.stopped());

				if (multipart_parser_.has_error()) {
					response_back(status_type::bad_request, "chunked mutipart error");
					return;
				}

				call_back();

				if (!response_->need_delay()) {
					do_write();
				}
			}

			/************chunked pass rlencoded****************/
			void handle_chunked_rlencoded() {
				bool ret = request_->chunked_parse_form_urlencoded();

				if (!ret) {
					response_back(status_type::bad_request, "gzip uncompress error or data error");
					return;
				}

				call_back();

				if (!response_->need_delay()) {
					do_write();
				}
			}

			void handle_chunked_response() {
				switch (req_content_type_) {
				case content_type::string:
				case content_type::unknown:
					handle_chunked_string_body();
					break;
				case content_type::multipart:
					handle_chunked_multipart();
					break;
				case content_type::urlencoded:
					handle_chunked_rlencoded();
					break;
				default:
					response_back(status_type::bad_request, "content-type not support");
				}
			}

			void call_back() {
				if (http_handler_ == nullptr) {
					assert(http_handler_ != nullptr);
					return;
				}

				http_handler_(*request_, *response_);
			}
			//-------------multipart----------------------//
			void init_multipart_parser() {
				multipart_parser_.on_part_begin = [this](const multipart_headers& headers) {
					request_->set_multipart_headers(headers);
					auto filename = request_->get_multipart_field_name("filename");
					is_multi_part_file_ = request_->is_multipart_file();
					if (filename.empty() && is_multi_part_file_) {
						request_->set_state(data_proc_state::data_error);
						response_->set_status_and_content(status_type::bad_request, "mutipart error");
						return;
					}
					if (is_multi_part_file_) {
						auto ext = boost::filesystem::extension(filename);
						try {
							auto tp = std::chrono::high_resolution_clock::now();
							auto nano = tp.time_since_epoch().count();
							std::string name = static_dir_ + "/" + std::to_string(nano)
								+ std::string(ext.data(), ext.length()) + "_ing";
							if (multipart_begin_) {
								multipart_begin_(*request_, name);
							}

							request_->open_upload_file(name);
						}
						catch (const std::exception & ex) {
							request_->set_state(data_proc_state::data_error);
							response_->set_status_and_content(status_type::internal_server_error, ex.what());
							return;
						}
					}
					else {
						auto key = request_->get_multipart_field_name("name");
						request_->save_multipart_key_value(std::string(key.data(), key.size()), "");
					}
				};
				multipart_parser_.on_part_data = [this](const char* buf, size_t size) {
					if (request_->get_state() == data_proc_state::data_error) {
						return;
					}
					if (is_multi_part_file_) {
						request_->write_upload_data(buf, size);
					}
					else {
						auto key = request_->get_multipart_field_name("name");
						request_->update_multipart_value(std::move(key), buf, size);
					}
				};
				multipart_parser_.on_part_end = [this] {
					if (request_->get_state() == data_proc_state::data_error) {
						return;
					}

					if (is_multi_part_file_) {
						request_->close_upload_file();
						auto pfile = request_->get_file();
						if (pfile) {
							auto old_name = pfile->get_file_path();
							auto pos = old_name.rfind("_ing");
							if (pos != std::string::npos) {
								pfile->rename_file(old_name.substr(0, old_name.length() - 4));
							}
						}
					}
				};
				multipart_parser_.on_end = [this] {
					if (request_->get_state() == data_proc_state::data_error) {
						return;
					}

					request_->handle_multipart_key_value();
				};
			}


			bool handle_gzip() {
				if (request_->has_gzip()) {
					return request_->uncompress();
				}

				return true;
			}
		private:
			bool need_response_time_ = false;
			bool is_multi_part_file_;
			bool is_ssl_ = false;
			bool has_shake_ = false;
			bool keep_alive_ = false;
			bool is_receve_all_chunked = false;
			content_type req_content_type_;
			const std::string& static_dir_;
			const http_handler& http_handler_;
			ConnectEventObserver		connect_observer_;
			CloseEventObserver			close_observer_;
			size_t len_ = 0;
			size_t last_transfer_ = 0;
			multipart_reader multipart_parser_;
			std::unique_ptr<request>request_{};
			std::unique_ptr<response>response_;
			std::function<bool(request&, response&)>* upload_check_ = nullptr;
			std::function<void(request&, std::string&)> multipart_begin_ = nullptr;
#ifdef WHEEL_ENABLE_SSL
			std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> ssl_socket_{};
#else
			std::shared_ptr<boost::asio::ip::tcp::socket> socket_{};
#endif
			std::shared_ptr<boost::asio::io_service::strand>strand_;
		};
	}
}
#endif // http_tcp_handle_h__
