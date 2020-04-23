#ifndef htpp_define_h__
#define htpp_define_h__
#include <string>

namespace wheel {
	namespace http_servers {
		enum class status_type {
			init,
			switching_protocols = 101,
			ok = 200,
			created = 201,
			accepted = 202,
			no_content = 204,
			partial_content = 206,
			multiple_choices = 300,
			moved_permanently = 301,
			moved_temporarily = 302,
			not_modified = 304,
			temporary_redirect = 307,
			bad_request = 400,
			unauthorized = 401,
			forbidden = 403,
			not_found = 404,
			internal_server_error = 500,
			not_implemented = 501,
			bad_gateway = 502,
			service_unavailable = 503
		};

		enum class content_type {
			string,
			multipart,
			urlencoded,
			octet_stream,
			unknown,
		};

		enum class res_content_type {
			html,
			json,
			string,
			multipart,
			urlencoded,
			none
		};

		enum class transfer_type {
			normal =100,
			chunked
		};

		constexpr  auto HTML = res_content_type::html;
		constexpr  auto JSON = res_content_type::json;
		constexpr  auto TEXT = res_content_type::string;
		constexpr  auto NONE = res_content_type::none;

		const std::string STATIC_RESOURCE = "wheel_static_resource";

		enum class http_method {
			DEL,
			GET,
			HEAD,
			POST,
			PUT,
			CONNECT,
			OPTIONS,
			TRACE
		};
		constexpr  auto GET = http_method::GET;
		constexpr  auto POST = http_method::POST;
		constexpr  auto DEL = http_method::DEL;
		constexpr  auto HEAD = http_method::HEAD;
		constexpr  auto PUT = http_method::PUT;
		constexpr  auto CONNECT = http_method::CONNECT;
		constexpr  auto TRACE = http_method::TRACE;
		constexpr  auto OPTIONS = http_method::OPTIONS;


		enum class content_encoding {
			none,
			gzip
		};

		const std::string ok = "OK";
		const std::string created = "<html>"
			"<head><title>Created</title></head>"
			"<body><h1>201 Created</h1></body>"
			"</html>";

		const std::string accepted =
			"<html>"
			"<head><title>Accepted</title></head>"
			"<body><h1>202 Accepted</h1></body>"
			"</html>";
		const std::string no_content =
			"<html>"
			"<head><title>No Content</title></head>"
			"<body><h1>204 Content</h1></body>"
			"</html>";

		const std::string multiple_choices =
			"<html>"
			"<head><title>Multiple Choices</title></head>"
			"<body><h1>300 Multiple Choices</h1></body>"
			"</html>";

		const std::string moved_permanently =
			"<html>"
			"<head><title>Moved Permanently</title></head>"
			"<body><h1>301 Moved Permanently</h1></body>"
			"</html>";

		const std::string temporary_redirect =
			"<html>"
			"<head><title>Temporary Redirect</title></head>"
			"<body><h1>307 Temporary Redirect</h1></body>"
			"</html>";

		const std::string moved_temporarily =
			"<html>"
			"<head><title>Moved Temporarily</title></head>"
			"<body><h1>302 Moved Temporarily</h1></body>"
			"</html>";

		const std::string not_modified =
			"<html>"
			"<head><title>Not Modified</title></head>"
			"<body><h1>304 Not Modified</h1></body>"
			"</html>";

		const std::string bad_request =
			"<html>"
			"<head><title>Bad Request</title></head>"
			"<body><h1>400 Bad Request</h1></body>"
			"</html>";

		const std::string unauthorized =
			"<html>"
			"<head><title>Unauthorized</title></head>"
			"<body><h1>401 Unauthorized</h1></body>"
			"</html>";

		const std::string forbidden =
			"<html>"
			"<head><title>Forbidden</title></head>"
			"<body><h1>403 Forbidden</h1></body>"
			"</html>";

		const std::string not_found =
			"<html>"
			"<head><title>Not Found</title></head>"
			"<body><h1>404 Not Found</h1></body>"
			"</html>";

		const std::string internal_server_error =
			"<html>"
			"<head><title>Internal Server Error</title></head>"
			"<body><h1>500 Internal Server Error</h1></body>"
			"</html>";

		const std::string not_implemented =
			"<html>"
			"<head><title>Not Implemented</title></head>"
			"<body><h1>501 Not Implemented</h1></body>"
			"</html>";

		const std::string bad_gateway =
			"<html>"
			"<head><title>Bad Gateway</title></head>"
			"<body><h1>502 Bad Gateway</h1></body>"
			"</html>";

		const std::string service_unavailable =
			"<html>"
			"<head><title>Service Unavailable</title></head>"
			"<body><h1>503 Service Unavailable</h1></body>"
			"</html>";

		const std::string switching_protocols = "HTTP/1.1 101 Switching Protocals\r\n";
		const std::string rep_ok = "HTTP/1.1 200 OK\r\n";
		const std::string rep_created = "HTTP/1.1 201 Created\r\n";
		const std::string rep_accepted = "HTTP/1.1 202 Accepted\r\n";
		const std::string rep_no_content = "HTTP/1.1 204 No Content\r\n";
		const std::string rep_partial_content = "HTTP/1.1 206 Partial Content\r\n";
		const std::string rep_multiple_choices = "HTTP/1.1 300 Multiple Choices\r\n";
		const std::string rep_moved_permanently = "HTTP/1.1 301 Moved Permanently\r\n";
		const std::string rep_temporary_redirect = "HTTP/1.1 307 Temporary Redirect\r\n";
		const std::string rep_moved_temporarily = "HTTP/1.1 302 Moved Temporarily\r\n";
		const std::string rep_not_modified = "HTTP/1.1 304 Not Modified\r\n";
		const std::string rep_bad_request = "HTTP/1.1 400 Bad Request\r\n";
		const std::string rep_unauthorized = "HTTP/1.1 401 Unauthorized\r\n";
		const std::string rep_forbidden = "HTTP/1.1 403 Forbidden\r\n";
		const std::string rep_not_found = "HTTP/1.1 404 Not Found\r\n";
		const std::string rep_internal_server_error = "HTTP/1.1 500 Internal Server Error\r\n";
		const std::string rep_not_implemented = "HTTP/1.1 501 Not Implemented\r\n";
		const std::string rep_bad_gateway = "HTTP/1.1 502 Bad Gateway\r\n";
		const std::string rep_service_unavailable = "HTTP/1.1 503 Service Unavailable\r\n";

		const std::string rep_html = "Content-Type: text/html; charset=UTF-8\r\n";
		const std::string rep_json = "Content-Type: application/json; charset=UTF-8\r\n";
		const std::string rep_string = "Content-Type: text/plain; charset=UTF-8\r\n";
		const std::string rep_multipart = "Content-Type: multipart/form-data; boundary=";
		const std::string rep_urlencoded = "Content-Type:application/x-www-form-urlencoded\r\n";

		const std::string rep_keep = "Connection: keep-alive\r\n";
		const std::string rep_close = "Connection: close     \r\n";
		const std::string rep_len = "Content-Length: ";
		const std::string rep_crcf = "\r\n";
		const std::string rep_server = "Server: http_server\r\n";
		const std::string rep_multipart_lable = "Content-Disposition: form-data; name=";

		static const char name_value_separator[] = { ':', ' ' };

		//static const char crlf[] = { '\r', '\n' }; //渣渣程序员搞了调了一天
		//static const char last_chunk[] = { '0', '\r', '\n' };
		static const std::string crlf= "\r\n";
		static const std::string last_chunk= "0\r\n";

		static const std::string http_chunk_header =
			"HTTP/1.1 200 OK\r\n"
			"Transfer-Encoding: chunked\r\n";

		static const std::string http_range_chunk_header =
			"HTTP/1.1 206 Partial Content\r\n"
			"Transfer-Encoding: chunked\r\n";

		namespace detail {
			template<unsigned... digits>
			struct to_chars {
				static constexpr std::array<char, sizeof...(digits) + 18> value = { 'C','o','n','t','e','n','t','-','L','e','n','g',
																				 't','h',':',' ',('0' + digits)... , '\r', '\n' };
			};

			template<unsigned rem, unsigned... digits>
			struct explode : explode<rem / 10, rem % 10, digits...> {};

			template<unsigned... digits>
			struct explode<0, digits...> : to_chars<digits...> {};
		}

		template<unsigned num>
		struct num_to_string : detail::explode<num / 10, num % 10> {};

		static boost::asio::const_buffer to_buffer(status_type status) {
			switch (status) {
			case status_type::switching_protocols:
				return boost::asio::buffer(switching_protocols.data(), switching_protocols.length());
			case status_type::ok:
				return boost::asio::buffer(rep_ok.data(), rep_ok.length());
			case status_type::created:
				return boost::asio::buffer(rep_created.data(), rep_created.length());
			case status_type::accepted:
				return boost::asio::buffer(rep_accepted.data(), rep_created.length());
			case status_type::no_content:
				return boost::asio::buffer(rep_no_content.data(), rep_no_content.length());
			case status_type::partial_content:
				return boost::asio::buffer(rep_partial_content.data(), rep_partial_content.length());
			case status_type::multiple_choices:
				return boost::asio::buffer(rep_multiple_choices.data(), rep_multiple_choices.length());
			case status_type::moved_permanently:
				return boost::asio::buffer(rep_moved_permanently.data(), rep_moved_permanently.length());
			case status_type::temporary_redirect:
				return boost::asio::buffer(rep_temporary_redirect.data(), rep_temporary_redirect.length());
			case status_type::moved_temporarily:
				return boost::asio::buffer(rep_moved_temporarily.data(), rep_moved_temporarily.length());
			case status_type::not_modified:
				return boost::asio::buffer(rep_not_modified.data(), rep_not_modified.length());
			case status_type::bad_request:
				return boost::asio::buffer(rep_bad_request.data(), rep_bad_request.length());
			case status_type::unauthorized:
				return boost::asio::buffer(rep_unauthorized.data(), rep_unauthorized.length());
			case status_type::forbidden:
				return boost::asio::buffer(rep_forbidden.data(), rep_forbidden.length());
			case status_type::not_found:
				return boost::asio::buffer(rep_not_found.data(), rep_not_found.length());
			case status_type::internal_server_error:
				return boost::asio::buffer(rep_internal_server_error.data(), rep_internal_server_error.length());
			case status_type::not_implemented:
				return boost::asio::buffer(rep_not_implemented.data(), rep_not_implemented.length());
			case status_type::bad_gateway:
				return boost::asio::buffer(rep_bad_gateway.data(), rep_bad_gateway.length());
			case status_type::service_unavailable:
				return boost::asio::buffer(rep_service_unavailable.data(), rep_service_unavailable.length());
			default:
				return boost::asio::buffer(rep_internal_server_error.data(), rep_internal_server_error.length());
			}
		}

		static std::string to_rep_string(status_type status) {
			switch (status) {
			case status_type::switching_protocols:
				return switching_protocols;
				break;
			case status_type::ok:
				return rep_ok;
				break;
			case status_type::created:
				return rep_created;
				break;
			case status_type::accepted:
				return rep_accepted;
				break;
			case status_type::no_content:
				return rep_no_content;
				break;
			case status_type::partial_content:
				return rep_partial_content;
				break;
			case status_type::multiple_choices:
				return rep_multiple_choices;
				break;
			case status_type::moved_permanently:
				return rep_moved_permanently;
				break;
			case status_type::moved_temporarily:
				return rep_moved_temporarily;
				break;
			case status_type::not_modified:
				return rep_not_modified;
				break;
			case status_type::temporary_redirect:
				return rep_temporary_redirect;
				break;
			case status_type::bad_request:
				return rep_bad_request;
				break;
			case status_type::unauthorized:
				return rep_unauthorized;
				break;
			case status_type::forbidden:
				return rep_forbidden;
				break;
			case status_type::not_found:
				return rep_not_found;
				break;
			case status_type::internal_server_error:
				return rep_internal_server_error;
				break;
			case status_type::not_implemented:
				return rep_not_implemented;
				break;
			case status_type::bad_gateway:
				return rep_bad_gateway;
				break;
			case status_type::service_unavailable:
				return rep_service_unavailable;
				break;
			default:
				return rep_not_implemented;
				break;
			}
		}

		constexpr auto type_to_name(std::integral_constant<http_method, http_method::DEL>) noexcept { return "DELETE"; }
		constexpr auto type_to_name(std::integral_constant<http_method, http_method::GET>) noexcept { return "GET"; }
		constexpr auto type_to_name(std::integral_constant<http_method, http_method::HEAD>) noexcept { return "HEAD"; }

		constexpr auto type_to_name(std::integral_constant<http_method, http_method::POST>) noexcept { return "POST"; }
		constexpr auto type_to_name(std::integral_constant<http_method, http_method::PUT>) noexcept { return "PUT"; }

		constexpr auto type_to_name(std::integral_constant<http_method, http_method::CONNECT>) noexcept { return "CONNECT"; }
		constexpr auto type_to_name(std::integral_constant<http_method, http_method::OPTIONS>) noexcept { return "OPTIONS"; }
		constexpr auto type_to_name(std::integral_constant<http_method, http_method::TRACE>) noexcept { return "TRACE"; }

		template<http_method... Is>
		static constexpr auto get_method_arr() {
			std::array<char, 26> arr{ 0 };
			std::string s;
			using expander = int[];
			expander {
				((s =type_to_name(std::integral_constant<http_method, Is>{})), arr[s[0] - 65] = s[0])...
			};

			//((s = type_to_name(std::integral_constant<http_method, Is>{}), arr[s[0] - 65] = s[0]), ...);

			return arr;
		}

		static std::string to_string(status_type status) {
			switch (status) {
			case status_type::ok:
				return ok;
			case status_type::created:
				return created;
			case status_type::accepted:
				return accepted;
			case status_type::no_content:
				return no_content;
			case status_type::multiple_choices:
				return multiple_choices;
			case status_type::moved_permanently:
				return moved_permanently;
			case status_type::moved_temporarily:
				return moved_temporarily;
			case status_type::temporary_redirect:
				return temporary_redirect;
			case status_type::not_modified:
				return not_modified;
			case status_type::bad_request:
				return bad_request;
			case status_type::unauthorized:
				return unauthorized;
			case status_type::forbidden:
				return forbidden;
			case status_type::not_found:
				return not_found;
			case status_type::internal_server_error:
				return internal_server_error;
			case status_type::not_implemented:
				return not_implemented;
			case status_type::bad_gateway:
				return bad_gateway;
			case status_type::service_unavailable:
				return service_unavailable;
			default:
				return internal_server_error;
			}
		}
	}
}

#endif // htpp_define_h__
