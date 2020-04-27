#ifndef http_response_h__
#define http_response_h__

#include <chrono>
#include <string>
#include "unit.hpp"
#include "nlohmann_json.hpp"
#include "htpp_define.hpp"
#include "picohttpparser.hpp"
#include "itoa.hpp"
#include "base64.hpp"
#ifdef WHEEL_ENABLE_GZIP
#include "gzip.hpp"
#endif

namespace wheel {
	namespace http_servers {
		class response {
		public:
			response() {
			}

			void enable_response_time(bool enable) {
				need_response_time_ = enable;
				if (need_response_time_) {
					std::string mbstr;
					mbstr.resize(50);
					std::time_t tm = std::chrono::system_clock::to_time_t(last_time_);
#ifdef _WINDOWS_
					setlocale(LC_TIME, "en_us.utf-8");
#endif		
					std::strftime(&mbstr[0], mbstr.size(), "%a, %d %b %Y %T GMT", std::localtime(&tm));
					mbstr.resize(strlen(mbstr.c_str()));
					last_date_str_ = mbstr;
				}
			}

			~response() = default;
			std::string response_str() {
				return std::move(rep_str_);
			}

			void build_response_str() {
				rep_str_.append(to_rep_string(status_));

				for (auto& header : headers_) {
					rep_str_.append(header.first).append(":").append(header.second).append(crlf);
				}

				std::string temp;
				temp.resize(64);
				itoa_fwd((int)content_.size(), &temp[0]);
				std::string buff(temp.c_str(), strlen(temp.c_str()));
				rep_str_.append("Content-Length: ").append(buff).append(crlf);
				if (rep_str_.empty()) {
					return;
				}

				if (res_type_ != res_content_type::none) {
					std::string content_type = get_content_type(res_type_);
					if (content_type.empty()){
						set_status_and_content(status_type::bad_gateway, "send content type is failure");
						return;
					}

					rep_str_.append(content_type);
				}

				rep_str_.append(rep_server);

				if (need_response_time_) {
					append_respone_date_time();
				}

				rep_str_.append(crlf);
				rep_str_.append(std::move(content_));
			}

			void build_chunked_response_str() {
				if (headers_.empty()){
					trans_type_ = transfer_type::normal;
					set_status_and_content(status_type::bad_gateway, "http package header is empty");
					return;
				}

				chukend_header_.append(to_rep_string(status_));

				for (auto& header : headers_) {
					chukend_header_.append(header.first).append(":").append(header.second).append(crlf);
				}

				std::string content_type = get_content_type(res_type_);
				if (content_type.empty()){
					trans_type_ = transfer_type::normal;
					headers_.clear();
					set_status_and_content(status_type::bad_gateway, "send content_type is failure");
					return;
				}

				chukend_header_.append(content_type);

				chukend_header_.append(rep_server);

				if (need_response_time_) {
					append_chunked_pass_respone_date_time();
				}

				chukend_header_.append(crlf);
			}

			void build_response_multipart_str() {
				if (headers_.empty()){
					set_status_and_content(status_type::bad_gateway, "http package header is empty");
					return;
				}

				rep_str_.append(to_rep_string(status_));

				for (auto& header : headers_) {
					rep_str_.append(header.first).append(":").append(header.second).append(crlf);
				}

				std::string r_str;
				unit::random_str(r_str, 16);
				std::string raw_key = base64_encode(r_str);
				boundary_.append(boundary_demil_).append(raw_key);

				rep_str_.append(get_content_type(res_type_));
				rep_str_.append(boundary_);
				rep_str_.append(crlf);

				for (auto& param : multipart_datas_) {
					content_.append(boundary_).append(crlf);
					content_.append(rep_multipart_lable);
					content_.append("\"");
					content_.append(param.first);
					content_.append("\"");
					content_.append(crlf).append(crlf);
					content_.append(param.second);
					content_.append(crlf);
				}

				content_.append(crlf);
				content_.append(boundary_);
				content_.append(crlf);

				std::string temp;
				temp.resize(64);
				itoa_fwd((int)content_.size(), &temp[0]);
				std::string buff(temp.c_str(), strlen(temp.c_str()));
				rep_str_.append("Content-Length: ").append(buff).append(crlf);
				rep_str_.append(rep_server);

				if (need_response_time_) {
					append_respone_date_time();
				}

				rep_str_.append(crlf);
				rep_str_.append(std::move(content_));
			}

			//build chunked header
			void build_chunkead_header()
			{
				if (headers_.empty()){
					trans_type_ = transfer_type::normal;
					set_status_and_content(status_type::bad_gateway, "http package header is empty");
					return;
				}

				chukend_header_.append(to_rep_string(status_));
				for (auto& header : headers_) {
					chukend_header_.append(header.first).append(":").append(header.second).append(crlf);
				}

				if (res_type_ == res_content_type::multipart) {
					std::string r_str;
					unit::random_str(r_str, 16);
					std::string raw_key = base64_encode(r_str);
					boundary_.append(boundary_demil_).append(raw_key);

					std::string content_type = get_content_type(res_type_);
					if (content_type.empty()){
						trans_type_ = transfer_type::normal;
						headers_.clear();
						set_status_and_content(status_type::bad_gateway, "send content_type is failure");
						return;
					}

					chukend_header_.append(get_content_type(res_type_));
					chukend_header_.append(boundary_).append(crlf);
				}

				chukend_header_.append(rep_server);

				if (need_response_time_) {
					append_chunked_pass_respone_date_time();
				}

				chukend_header_.append(crlf);
			}

			//build chuked multipart body
			void build_chunkead_multipart_body()
			{
				build_response_multipart_data();
			}

			void build_response_multipart_data()
			{
				for (auto& param : multipart_datas_) {
					std::string context;
					std::string str;

					str += boundary_;
					str.append(crlf);
					str += rep_multipart_lable;
					str += "\"";
					str += param.first;
					str += "\"";
					str.append(crlf);
					str.append(crlf);//语法格式中间可以多加一个

					size_t lable_size = str.size();
					context.append(unit::to_hex_string(lable_size)).append(crlf).append(str);

					str.clear();
					str.append(crlf);
					str.append(unit::to_hex_string(param.second.size())).append(crlf);
					str += param.second;
					str.append(crlf);

					str.append(unit::to_hex_string(2)).append(crlf);
					//多加两个\r\n\r\n
					str.append(crlf);
					str.append(crlf);
					context.append(str);
					chunked_data_.push_back(std::move(context));
				}

				std::string end_chunked;
				end_chunked.append(unit::to_hex_string(boundary_.size())).append(crlf).append(boundary_).append(crlf);
				chunked_data_.push_back(std::move(end_chunked));
				chunked_data_.push_back(std::move(build_chunked_end_data()));
			}

			void build_response_urlencoded_str(content_encoding encoding) {
				if (headers_.empty()){
					set_status_and_content(status_type::bad_gateway, "http package header is empty");
					return;
				}

				rep_str_.append(to_rep_string(status_));

				for (auto& header : headers_) {
					rep_str_.append(header.first).append(":").append(header.second).append(crlf);
				}

				size_t size = urlencoded_datas.size();
				size_t count = 0;
				for (int i = 0; i < size; ++i) {
					content_.append(urlencoded_datas[i].first).append("=").append(urlencoded_datas[i].second);
					if (++count < size) {
						content_.append("&");
					}
				}

#ifdef WHEEL_ENABLE_GZIP
				if (encoding == content_encoding::gzip) {
					std::string compress_str;
					bool r = compress(compress_str, content_);
					if (!r) {
						return;
					}

					content_.clear();
					content_ = std::move(compress_str);
				}
#endif
				std::string temp;
				temp.resize(64);
				itoa_fwd((int)content_.size(), &temp[0]);
				std::string buff(temp.c_str(), strlen(temp.c_str()));
				rep_str_.append("Content-Length: ").append(buff).append(crlf);
				rep_str_.append(rep_server);

				if (need_response_time_) {
					append_respone_date_time();
				}

				rep_str_.append(crlf);
				rep_str_.append(std::move(content_));
			}

			void build_response_chunked_urlencoded_str(content_encoding encoding) {
				if (headers_.empty()){
					trans_type_ = transfer_type::normal;
					set_status_and_content(status_type::bad_gateway, "http package is empty");
				}

				chukend_header_.append(to_rep_string(status_));
				for (auto& header : headers_) {
					chukend_header_.append(header.first).append(":").append(header.second).append(crlf);
				}

				size_t size = urlencoded_datas.size();
				size_t count = 0;
				for (size_t i = 0; i < size; ++i) {
					content_.append(urlencoded_datas[i].first).append("=").append(urlencoded_datas[i].second);
					if (++count < size) {
						content_.append("&");
					}
				}

#ifdef WHEEL_ENABLE_GZIP
				if (encoding == content_encoding::gzip) {
					std::string compress_str;
					bool r = compress(compress_str, content_);
					if (!r) {
						return;
					}

					content_.clear();
					content_ = std::move(compress_str);
				}
#endif
				std::string temp;
				temp.resize(64);
				itoa_fwd((int)content_.size(), &temp[0]);
				std::string buff(temp.c_str(), strlen(temp.c_str()));
				buff = unit::to_hex_string(unit::stringDec_to_int(buff.c_str()));
				buff.append(crlf);
				content_.append(crlf).append("0").append(crlf).append(crlf);
				std::string data;
				data.append(buff).append(std::move(content_));
				chunked_data_.push_back(std::move(data));

				chukend_header_.append(rep_server);

				if (need_response_time_) {
					append_chunked_pass_respone_date_time();
				}

				chukend_header_.append(crlf);
			}

			std::vector<boost::asio::const_buffer> to_buffers() {
				std::vector<boost::asio::const_buffer> buffers;
				add_header("Host", "http_server");

				buffers.reserve(headers_.size() * 4 + 5);
				buffers.emplace_back(to_buffer(status_));
				for (auto const& h : headers_) {
					buffers.emplace_back(boost::asio::buffer(h.first));
					buffers.emplace_back(boost::asio::buffer(name_value_separator));
					buffers.emplace_back(boost::asio::buffer(h.second));
					buffers.emplace_back(boost::asio::buffer(crlf));
				}

				buffers.push_back(boost::asio::buffer(crlf));

				if (body_type_ == content_type::string) {
					buffers.emplace_back(boost::asio::buffer(content_.data(), content_.size()));
				}

				return buffers;
			}

			void add_header(std::string&& key, std::string&& value) {
				headers_.emplace_back(std::move(key), std::move(value));
			}

			void set_status(status_type status) {
				status_ = status;
			}

			status_type get_status() const {
				return status_;
			}

			void set_delay(bool delay) {
				delay_ = delay;
			}

			void set_status_and_content(status_type status) {
				status_ = status;
				set_content(to_string(status).data());
				build_response_str();
			}

			void set_status_and_content(status_type status, std::string&& content, res_content_type res_type = res_content_type::none,
				content_encoding encoding = content_encoding::none, transfer_type trans_type = transfer_type::normal) {
				status_ = status;
				res_type_ = res_type;

				if (trans_type == transfer_type::chunked) {
					add_header("Transfer-Encoding", "chunked");
					trans_type_ = transfer_type::chunked;
				}

				std::string encode_str;
#ifdef WHEEL_ENABLE_GZIP
				if (encoding == content_encoding::gzip) {
					bool r = gzip_codec::compress(std::string(content.data(), content.length()), encode_str, true);
					if (!r) {
						set_status_and_content(status_type::internal_server_error, "gzip compress error");
						return;
					}
					else {
						add_header("Content-Encoding", "gzip");
					}

					if (trans_type_ == transfer_type::chunked) {
						std::string len;
						len.resize(64);
						itoa_fwd((int)encode_str.size(), &len[0]);
						std::string buff(len.c_str(), strlen(len.c_str()));

						std::string data;
						data.append(unit::to_hex_string(unit::stringDec_to_int(len.c_str()))).append(crlf).append(content).append(crlf).append(last_chunk).append(crlf);
						chunked_data_.push_back(std::move(data));
						build_chunked_response_str();
						return;
			       }

					set_content(std::move(encode_str));
				}
#else
				if (trans_type_ == transfer_type::chunked) {
					std::string len;
					len.resize(64);
					itoa_fwd((int)content.size(), &len[0]);
					std::string buff(len.c_str(), strlen(len.c_str()));

					std::string data;
					data.append(unit::to_hex_string(unit::stringDec_to_int(len.c_str()))).append(crlf).append(content).append(crlf).append(last_chunk).append(crlf);
					chunked_data_.push_back(std::move(data));
					build_chunked_response_str();
					return;
				}

				set_content(std::move(content));
#endif          
				build_response_str();
			}

			/*********************************form-data数据chunked发送接口******************************************************/
			void set_mstatus_and_content(status_type status, transfer_type trans_type = transfer_type::normal) {

				status_ = status;
				res_type_ = res_content_type::multipart;

				if (trans_type == transfer_type::chunked) {
					add_header("Transfer-Encoding", "chunked");
					trans_type_ = transfer_type::chunked;
				}

				if (trans_type_ == transfer_type::normal) {
					build_response_multipart_str();
					return;
				}

				build_chunkead_header();
				build_chunkead_multipart_body();
			}

			void set_urlencoded_status_and_content(status_type status, content_encoding encoding = content_encoding::none, transfer_type trans_type = transfer_type::normal) {

				status_ = status;
				res_type_ = res_content_type::urlencoded;

#ifdef WHEEL_ENABLE_GZIP
				if (encoding == content_encoding::gzip) {
					add_header("Content-Encoding", "gzip");
				}
#endif
				add_header("Content-Type", "application/x-www-form-urlencoded");
				if (trans_type == transfer_type::chunked) {
					add_header("Transfer-Encoding", "chunked");
					trans_type_ = transfer_type::chunked;
					build_response_chunked_urlencoded_str(encoding);
					return;
				}

				build_response_urlencoded_str(encoding);
			}

			std::string get_content_type(res_content_type type) {
				switch (type) {
				case res_content_type::html:
					return rep_html;
				case res_content_type::json:
					return rep_json;
				case res_content_type::string:
					return rep_string;
				case res_content_type::multipart:
					return rep_multipart;
				case res_content_type::urlencoded:
					return rep_urlencoded;
				case res_content_type::none:
				default:
					return "";
				}
			}

			bool need_delay() const {
				return delay_;
			}

			void set_continue(bool con) {
				proc_continue_ = con;
			}

			bool need_continue() const {
				return proc_continue_;
			}

			void set_content(std::string&& content) {
				body_type_ = content_type::string;
				content_ = std::move(content);
			}

			std::vector<boost::asio::const_buffer> to_chunked_buffers(const char* chunk_data, size_t length, bool eof) {
				std::vector<boost::asio::const_buffer> buffers;

				if (length > 0) {
					// Construct chunk based on rfc2616 section 3.6.1
					buffers.push_back(boost::asio::buffer(wheel::unit::to_hex_string(length)));
					buffers.push_back(boost::asio::buffer(crlf));
					buffers.push_back(boost::asio::buffer(chunk_data, length));
					buffers.push_back(boost::asio::buffer(crlf));
				}

				//append last-chunk
				if (eof) {
					buffers.push_back(boost::asio::buffer(last_chunk));
					buffers.push_back(boost::asio::buffer(crlf));
				}

				return buffers;
			}

			void set_domain(const std::string& domain) {
				domain_ = domain;
			}

			std::string get_domain() {
				return domain_;
			}

			void set_path(const std::string& path) {
				path_ = path;
			}

			std::string get_path() {
				return path_;
			}

			void set_url(const std::string& url)
			{
				raw_url_ = url;
			}

			std::string get_url(const std::string& url)
			{
				return raw_url_;
			}

			void set_headers(std::vector<std::pair<std::string, std::string>> headers) {
				req_headers_ = std::move(headers);
			}

			void render_json(const nlohmann::json& json_data,transfer_type trans_type = transfer_type::normal)
			{
#ifdef  WHEEL_ENABLE_GZIP
				set_status_and_content(status_type::ok, json_data.dump(), res_content_type::json, content_encoding::gzip, trans_type);
#else
				set_status_and_content(status_type::ok, json_data.dump(), res_content_type::json, content_encoding::none, trans_type);
#endif
			}

			void render_string(std::string&& content,transfer_type trans_type = transfer_type::normal)
			{
#ifdef  WHEEL_ENABLE_GZIP
				set_status_and_content(status_type::ok, std::move(content), res_content_type::string, content_encoding::gzip,trans_type);
#else
				set_status_and_content(status_type::ok, std::move(content), res_content_type::string, content_encoding::none,trans_type);
#endif
			}

			void redirect(const std::string& url, bool is_forever = false)
			{
				add_header("Location", url.c_str());
				is_forever == false ? set_status_and_content(status_type::moved_temporarily) : set_status_and_content(status_type::moved_permanently);
			}

			void redirect_post(const std::string& url) {
				add_header("Location", url.c_str());
				set_status_and_content(status_type::temporary_redirect);
			}

			void reset() {
				rep_str_.clear();
				req_headers_.clear();

				trans_type_ = transfer_type::normal;
				res_type_ = res_content_type::none;
				status_ = status_type::init;
				proc_continue_ = true;
				delay_ = false;
				headers_.clear();
				content_.clear();
				multipart_datas_.clear();
				boundary_.clear();
				urlencoded_datas.clear();
			}

			void set_multipart_data(std::string&& name, std::string&& value)
			{
				multipart_datas_.emplace_back(name, value);
			}

			void set_urlencoded_datas(std::string&& name, std::string&& value)
			{
				urlencoded_datas.emplace_back(name, value);
			}

			transfer_type get_trans_type()const {
				return trans_type_;
			}

			std::string get_chunked_header() {
				return std::move(chukend_header_);
			}

			auto get_chunked_data() {
				return std::move(chunked_data_);
			}
		private:
			std::string get_header_value(std::string key) const {
				auto it = std::find_if(req_headers_.begin(), req_headers_.end(), [key](auto& pair) {
					if (wheel::unit::iequal(pair.first.data(), pair.first.size(), key.data())) {
						return true;
					}

					return false;
					});

				if (it != req_headers_.end()) {
					return (*it).second;
				}

				return {};
			}

			void append_respone_date_time() {
				//后面可以跟着秒
				using namespace std::chrono_literals;

				auto t = std::chrono::system_clock::now();
				if (t - last_time_ > 1s) {
					std::string mbstr;
					mbstr.resize(50);
					std::time_t tm = std::chrono::system_clock::to_time_t(t);
					std::strftime(&mbstr[0],mbstr.size(), "%a, %d %b %Y %T GMT", std::localtime(&tm));
					mbstr.resize(strlen(mbstr.c_str()));
					last_date_str_ = mbstr;
					rep_str_.append("Date: ").append(mbstr).append("\r\n");
					last_time_ = t;
				}
				else {
					rep_str_.append("Date: ").append(last_date_str_).append("\r\n");
				}
			}

			void append_chunked_pass_respone_date_time() {
				//后面可以跟着秒
				using namespace std::chrono_literals;

				auto t = std::chrono::system_clock::now();
				if (t - last_time_ > 1s) {
					std::string mbstr;
					mbstr.resize(50);
					std::time_t tm = std::chrono::system_clock::to_time_t(t);
#ifdef _WINDOWS_
					setlocale(LC_TIME, "en_us.utf-8");
#endif			
					std::strftime(&mbstr[0],mbstr.size(), "%a, %d %b %Y %T GMT", std::localtime(&tm));
					mbstr.resize(strlen(mbstr.c_str()));
					last_date_str_ = mbstr;
					chukend_header_.append("Date: ").append(mbstr).append("\r\n");
					last_time_ = t;
				}
				else {
					chukend_header_.append("Date: ").append(last_date_str_).append("\r\n");
				}
			}
		private:
			bool compress(std::string& dst, const std::string& src) {
				bool r = true;
#ifdef WHEEL_ENABLE_GZIP
				r = gzip_codec::compress(src, dst);
#endif
				return r;
			}

			std::string build_chunked_end_data()
			{
				std::string str;
				str.append(last_chunk);
				str.append(crlf);
				return std::move(str);
			}

		private:
			//piple传输，可以不发送
			bool delay_ = false;
			bool need_response_time_ = false;
			bool proc_continue_ = true;
			content_type body_type_ = content_type::unknown;
			status_type status_ = status_type::init;
			res_content_type res_type_;

			transfer_type trans_type_ = transfer_type::normal;
			std::string raw_url_;
			std::string content_;
			std::string domain_;
			std::string path_;
			std::string rep_str_;
			std::string last_date_str_;
			std::string boundary_;
			std::string chukend_header_;
			const std::string boundary_demil_ = "--------------------------";
			std::chrono::system_clock::time_point last_time_ = std::chrono::system_clock::now();
			std::vector<std::pair<std::string, std::string> > multipart_datas_;
			std::vector<std::pair<std::string, std::string>> req_headers_;
			std::vector<std::pair<std::string, std::string>> urlencoded_datas;
			std::vector<std::pair<std::string, std::string>> headers_;
			std::list<std::string>chunked_data_;
		};
	}
}
#endif // http_response_h__