//TCP
//#include <iostream>
//#include "proxyEngine.h"
//#include <wheel/client.hpp>
//#include <wheel/server.hpp>
//
//int main()
//{
//	ProxyEngine eng;
//	std::shared_ptr<boost::asio::io_service> ptr1 = std::make_shared<boost::asio::io_service>();
//	std::shared_ptr<wheel::server> ptr = std::make_shared<wheel::server>(std::bind(&ProxyEngine::OnMessage, &eng, std::placeholders::_1,std::placeholders::_2)); //偏移后的值
//
//	ptr->init(9000,4);
//
//	//std::shared_ptr<wheel::client> ptr = std::make_shared<wheel::client>(std::bind(&ProxyEngine::OnMessage,&eng,std::placeholders::_1,std::placeholders::_2),0);
//	//ptr->connect("127.0.0.1", 3333);
//	ptr->run();
//}


//#include <iostream>
//#include <wheel/picohttpparser.hpp>
//#include <boost/asio.hpp>
//
//int main()
//{
//	char data[1024] = {0};
//	const char* method = nullptr;
//	const char* path = nullptr;
//	int pret, minor_version;
//	struct phr_header headers[100];
//	size_t buflen = 0, prevbuflen = 0, method_len, path_len, num_headers;
//	boost::asio::io_service ios;
//	boost::asio::ip::tcp::acceptor ac(ios, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 9000));
//	boost::asio::ip::tcp::socket socket_(ios);
//	ac.accept(socket_);
//	std::size_t rret = socket_.receive(boost::asio::buffer(data, 1024));
//	prevbuflen = buflen;
//	buflen += rret;
//	pret = phr_parse_request(data, buflen, &method, &method_len, &path, &path_len,
//		&minor_version, headers, &num_headers, prevbuflen);
//
//	for (int i = 0; i != num_headers; ++i) {
//		printf("%.*s: %.*s\n", (int)headers[i].name_len, headers[i].name,
//			(int)headers[i].value_len, headers[i].value);
//	}
//
//}

//#include <iostream>
//#include "proxyEngine.h"
//#include <wheel/websocket_client.hpp>

//int main()
//{
	//ProxyEngine eng;

	//std::shared_ptr<wheel::websocket::webscoket_client> ptr = std::make_shared<wheel::websocket::webscoket_client>(std::bind(&ProxyEngine::OnMessage1,&eng,std::placeholders::_1,std::placeholders::_2));
	//ptr->connect("127.0.0.1", 8300);
	//ptr->run();
//}
//#include<wheel/json.hpp>
//struct person
//{
//	std::string  name;
//	int          age;
//	std::uint64_t ss;
//};
//
//REFLECTION(person, name, age, ss)
//struct one_t
//{
//	int id;
//};
//REFLECTION(one_t, id);
//struct composit_t
//{
//	int a;
//	std::vector<std::string> b;
//	int c;
//	std::map<int, int> d;
//	std::unordered_map<int, int> e;
//	double f;
//	std::list<one_t> g;
//};
//REFLECTION(composit_t, a, b, c, d, e, f, g);
//
//int main()
//{
//	auto const t = std::make_tuple(42, 'z', 3.14, 13, 0, "Hello, World!");
//
//	for (std::size_t i = 0; i < std::tuple_size<decltype(t)>::value; ++i) {
//		wheel::unit::tuple_switch(i, t, [](const auto& v) {
//			std::cout << v << std::endl;
//			});
//
//	}
//	person obj;
//	obj.age = 23;
//	obj.name = "yuh";
//	obj.ss = 12345;
//	const char* json = "{\"name\" : \"Boo\",\"age\" : 28,\"ss\" : 28234}";
//
//	wheel::json::from_json(obj, json);
//
//	wheel::json::string_stream sst;
//	wheel::json::to_json(sst, obj);
//	std::string str = sst.str();
//
//	one_t one = { 2 };
//	composit_t composit = { 1,{ "tom", "jack" }, 3,{ { 2,3 } },{ { 5,6 } }, 5.3,{ one } };
//	wheel::json::string_stream sst1;
//	wheel::json::to_json(sst1, composit);
//	str = sst1.str();
//
//	composit_t composit11;
//	wheel::json::from_json(composit11, str.c_str());
//
//	const char* str_comp = R"({"b":["tom", "jack"], "a":1, "c":3, "e":{"3":4}, "d":{"2":3,"5":6},"f":5.3,"g":[{"id":1},{"id":2}])";
//	composit_t comp;
//	wheel::json::from_json(comp, str_comp);
//	std::cout << comp.a << " " << comp.f << std::endl;
//
//	int i = 100;
//
//}

//#include <iostream>
//#include <wheel/http_server.hpp>
//
//int main()
//{
//	std::string str = "11";
//	if (!str.empty()) {
//		int i = 100;
//	}
//	using namespace wheel::http_servers;
//	wheel::http_servers::http_server server;
//	server.set_ssl_conf({ "server.crt", "server.key","1234561" });
//	server.listen(9090);
//	server.set_http_handler<GET, POST>("/", [](request& req, response& res) {
//		res.set_status_and_content(status_type::ok, "hello world");
//		});
//
//
//	server.set_http_handler<GET, POST, OPTIONS>("/test", [](request& req, response& res) {
//
//		std::string body = req.body();
//		std::string str1 = req.get_method();
//		std::string str = req.get_header_value("session");
//		std::string name = req.get_multipart_field_name("name");
//		std::string age = req.get_multipart_field_name("age");
//
//		res.set_status_and_content(status_type::ok, "hello world");
//		});
//
//	server.run();
//}
//
//
//#include <iostream>
//#include <wheel/http_server.hpp>
//
//int main()
//{
//
//	std::string test_str;
//	test_str.resize(1024);
//
//	memcpy(&test_str[0], "123", 3);
//
//	memcpy(&test_str[3], "456", 3);
//
//	memcpy(&test_str[6], "456", 3);
//
//	std::string str = "11";
//	if (!str.empty()) {
//		int i = 100;
//	}
//	using namespace wheel::http_servers;
//	wheel::http_servers::http_server server;
//	//server.set_ssl_conf({ "server.crt", "server.key","1234561" });
//	server.set_ssl_conf({ "www.wheellovecplus.xyz_bundle.crt", "www.wheellovecplus.xyz.key" });
//	server.listen(9090);
//	server.set_http_handler<GET, POST>("/", [](request& req, response& res) {
//		res.set_status_and_content(status_type::ok, "hello world");
//		});
//
//
//	server.set_http_handler<GET, POST, OPTIONS>("/test", [](request& req, response& res) {
//
//		std::string str = req.get_header_value("session");//获取包头值
//
//		std::string str1 = req.get_query_value("name");//获取包体值
//		std::string str2 = req.get_query_value("age");//获取包体值
//		std::string str3 = req.get_query_value("sex");//获取包体值
//		std::string body = req.body();
//		res.set_status_and_content(status_type::ok, body.c_str());
//		});
//
//	server.run();
//}
//
//#include <iostream>
//#include <wheel/http_server.hpp>
//
//using namespace wheel::http_servers;
//wheel::http_servers::http_server server;
//
////校验的切面
//struct check {
//	bool before(request& req, response& res) {
//		std::cout << "before check" << std::endl;
//		return true;
//	}
//
//	bool after(request& req, response& res) {
//		std::cout << "after check" << std::endl;
//		return true;
//	}
//};
//
//
//int main()
//{
//
//	std::string test_str;
//	test_str.resize(1024);
//
//	memcpy(&test_str[0], "123", 3);
//
//	memcpy(&test_str[3], "456", 3);
//
//	memcpy(&test_str[6], "456", 3);
//
//	std::string str = "11";
//	if (!str.empty()) {
//		int i = 100;
//	}
//
//	//server.set_ssl_conf({ "server.crt", "server.key","1234561" });
//	server.set_ssl_conf({ "www.wheellovecplus.xyz_bundle.crt", "www.wheellovecplus.xyz.key" });
//	server.listen(9090);
//	server.set_http_handler<GET, POST>("/", [](request& req, response& res) {
//		res.set_status_and_content(status_type::ok, "hello world");
//		});
//
//
//	server.set_http_handler<GET, POST, OPTIONS, PUT>("/test", [](request& req, response& res) {
//
//		std::string str = req.get_header_value("session");//获取包头值
//
//		std::string str1 = req.get_query_value("name");//获取包体值
//		std::string str2 = req.get_query_value("age");//获取包体值
//		std::string str3 = req.get_query_value("sex");//获取包体值
//		std::string body = req.body();
//		res.set_status_and_content(status_type::ok, body.c_str());
//		}, check{});
//
//	server.run();
//}
//

#define WHEEL_ENABLE_SSL
//#define WHEEL_ENABLE_GZIP
#include <iostream>
#include <wheel/http_server.hpp>
#include <wheel/encoding_conversion.hpp>

using namespace wheel::http_servers;
wheel::http_servers::http_server server;

//校验的切面
struct check {
	bool before(request& req, response& res) {
		std::cout << "before check" << std::endl;
		return true;
	}

	bool after(request& req, response& res) {
		const std::string str = res.response_str();
		std::cout << str << std::endl;
		std::cout << "after check" << std::endl;
		return true;
	}
};


int main()
{
	uint8_t bytes[2] = {0};
	unsigned short int test_data = 65518;
	bytes[0] =(uint8_t)(test_data >> 8);
	bytes[1] = (uint8_t)(test_data &0xFF);

	std::string str1234 = "我是好人我問三十多萬多無多無多無多多";

	std::wstring w_name = wheel::char_encoding::encoding_conversion::to_wstring(str1234);
	if (wheel::char_encoding::encoding_conversion::is_valid_gbk(str1234.c_str())){
		int i = 10;
	}
	std::cout << str1234 << std::endl;
	std::string wwwwww = wheel::char_encoding::encoding_conversion::to_string(w_name);


	//std::string zip_str;
	//wheel::gzip_codec::compress(str1234, zip_str);
	//uf8
	std::string s1123 = wheel::char_encoding::encoding_conversion::gbk_to_utf8(str1234);
	std::u16string u16_str = wheel::char_encoding::encoding_conversion::utf8_to_utf16(s1123);
	std::cout << "u16_str:"<<u16_str.c_str()<<std::endl;

	std::u32string u32_str1 = wheel::char_encoding::encoding_conversion::utf8_to_utf32(s1123);
	std::cout << "u32_str1:" << u32_str1.c_str() << std::endl;
	std::u32string u32_str2 = wheel::char_encoding::encoding_conversion::utf16_to_utf32(u16_str);
	std::cout << "u32_str2:" << u32_str2.c_str() << std::endl;




	std::string str12345 = wheel::char_encoding::encoding_conversion::utf8_to_gbk(s1123);
	std::string test_str;
	test_str.resize(1024);

	memcpy(&test_str[0], "123", 3);

	memcpy(&test_str[3], "456", 3);

	memcpy(&test_str[6], "456", 3);

	std::string str = "11";
	if (!str.empty()){
		int i = 100;
	}

	//server.set_ssl_conf({ "server.crt", "server.key","1234561" });
	server.set_ssl_conf({ "www.wheellovecplus.xyz_bundle.crt", "www.wheellovecplus.xyz.key"});
	//server.enable_response_time(true);
	server.listen(9090);
	server.set_http_handler<GET, POST>("/", [](request& req, response& res) {
		res.set_status_and_content(status_type::ok, "hello world");
		});


	server.set_http_handler<GET, POST, OPTIONS,PUT>("/test", [](request& req, response& res) {

	//	std::string str = req.get_header_value("Accept-Encoding");//获取包头值
	//	                                                    
	//	std::string str1 = req.get_query_value("name");//获取包体值
	//	std::string str23133 = wheel::char_encoding::encoding_conversion::utf8_to_gbk(str1);
	//	std::string str2 = req.get_query_value("age");//获取包体值

	//	std::string part_data = req.get_part_data();
	////	std::string str3 = req.get_query_value("sex");//获取包体值
	//	//std::string body = req.body();
	//	//跨域解决
	//	res.add_header("Access-Control-Allow-origin", "*");
	//	//res.set_status_and_content(status_type::ok,"hello world");
	//	//res.set_status_and_content(status_type::ok, "hello world",res_content_type::string);
	//	std::string str4 = wheel::char_encoding::encoding_conversion::gbk_to_utf8("我是好人我問三十多萬多無多無多無多多");
	//	res.set_status_and_content(status_type::ok,std::move(str4), res_content_type::string);

		std::string str1 = req.get_query_value("name");//获取包体值
		std::string str23133 = wheel::char_encoding::encoding_conversion::utf8_to_gbk(str1);
		std::string str2 = req.get_query_value("age");//获取包体值

		std::string part_data = req.get_part_data();
		std::string name1 = wheel::char_encoding::encoding_conversion::utf8_to_gbk(req.get_query_value("name1"));
		std::cout << name1 << std::endl;
		std::string path = wheel::char_encoding::encoding_conversion::utf8_to_gbk(req.get_query_value("path"));
		res.add_header("Access-Control-Allow-origin", "*");
		//res.set_status_and_content(status_type::ok,"hello world");
		//res.set_status_and_content(status_type::ok, "hello world",res_content_type::string);
		std::string str4 = wheel::char_encoding::encoding_conversion::gbk_to_utf8("我是好人我問三十多萬多無多無多無多多");
		res.set_status_and_content(status_type::ok, std::move(str4), res_content_type::string);
		}, check{});

	server.run();
}
