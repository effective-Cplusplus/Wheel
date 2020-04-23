<h2 align="center">wheel</h2>


## 介绍
1. 基于boost库的asio异步编程
1. 支持TCP和websocket通信
1. 支持客户端定时重连机制
1. 支持任意二进制流的解析
1. websocket支持json和二进制流解析
1. 新增c++结构体映射机制
1. 支持nlohmann_json.hpp支持json解析([地址](https://nlohmann.github.io/json/ "地址"))
1. 支持json转结构体
1. 支持mysql增删查改和存储过程
1. 支持http和https业务
1. 支持utf8,utf16,utf32和string 转换
1. 支持chunked 传输 from-data、string、 www-form-urlencoded和解析
## 说明
1. 使用websocket代码需要设置包含third_paty下的库
1. 使用c++11以及以上编译(若c++11编译不过，请用c++14)
1. 有些库支持c++11编译
## 使用列子
```cpp
//TCP server
#include <iostream>
#include <wheel/client.hpp>
#include <wheel/server.hpp>

ProxyEngine()
{

}

~ProxyEngine()
{

}

int OnMessage(std::shared_ptr<wheel::tcp_socket::tcp_handle> handler, std::shared_ptr<wheel::native_stream> streams)
{
        //header =6,body_len偏移2 cmd偏移6 可自行设置，详解构造函数
	handler->get_read_parser()->set_stram_data(streams);
	int cmd = handler->get_read_parser()->get_cmd();
	int value = handler->get_read_parser()->read<int>();
	std::int16_t value1 = handler->get_read_parser()->read<std::int16_t>();
	std::string str = handler->get_read_parser()->read<std::string>();

	if (cmd ==4){
		handler->get_write_parser()->write_header(425);
		handler->get_write_parser()->write<std::int16_t>(102);
		//注意传递float数字二进流，要转换成uint32再转换，否则会丢位
		handler->get_write_parser()->write(wheel::unit::float_to_uint32(102.1f));
		handler->get_write_parser()->write_string("102");
		handler->get_write_parser()->end();
		handler->to_send(*handler->get_write_parser()->get_native_stream());
	}
	return 0;
}

int main()
{
	ProxyEngine eng;
	std::shared_ptr<wheel::tcp_socket::server> ptr = std::make_shared<wheel::tcp_socket::server>(std::bind(&ProxyEngine::OnMessage, &eng, std::placeholders::_1,std::placeholders::_2)); //偏移后的值

	ptr->init(9000,4);

	//std::shared_ptr<wheel::client> ptr = std::make_shared<wheel::client>(std::bind(&ProxyEngine::OnMessage,&eng,std::placeholders::_1,std::placeholders::_2),0);
	//ptr->connect("127.0.0.1", 3333);
	ptr->run();
}
```
```cpp
//TCP client
#include <iostream>
#include <wheel/client.hpp>

int main()
{
	ProxyEngine eng;
	std::shared_ptr<wheel::tcp_socket::client> ptr = std::make_shared<wheel::tcp_socket::client>(std::bind(&ProxyEngine::OnMessage,&eng,std::placeholders::_1,std::placeholders::_2),0);
	ptr->connect("127.0.0.1", 3333);
	ptr->run();
}
```
```cpp
//websocket client
#include <iostream>
#include <wheel/websocket_client.hpp>

int main()
{
	ProxyEngine eng;

	std::shared_ptr<wheel::websocket::webscoket_client> ptr = std::make_shared<wheel::websocket::webscoket_client>(std::bind(&ProxyEngine::OnMessage1,&eng,std::placeholders::_1,std::placeholders::_2));
	ptr->connect("127.0.0.1", 8300);
	ptr->run();
}
```
```cpp
#include <iostream>
#include "proxyEngine.h"
#include <wheel/websocket_server.hpp>

int main()
{
	ProxyEngine eng;

	std::shared_ptr<wheel::websocket::websocket_server> ptr = std::make_shared<wheel::websocket::websocket_server>(std::bind(&ProxyEngine::OnMessage1,&eng,std::placeholders::_1,std::placeholders::_2));
	ptr->init(9000,4);
	ptr->run();
}
```
```cpp
#include <wheel/reflection.hpp>
#include <tuple>
#include <iostream>
#include <string>
#include <array>

struct MyStruct
{
	std::string str;
	int a;
};

REFLECTION(MyStruct, str, a);

struct person
{
	std::string  name;
	int          age;
	std::int16_t         sex;
};
REFLECTION(person, name, age, sex)

int main()
{
	auto t = fun(std::make_index_sequence<3>());
	std::cout << std::get<0>(t) << std::endl;
	std::cout << std::get<1>(t) << std::endl;
	std::cout << std::get<2>(t) << std::endl;


	int i = 10;

	auto sss =wheel::reflector::get_array<MyStruct>();

	int size = wheel::reflector::get_size<MyStruct>();
	auto ssscc = wheel::reflector::get_index<MyStruct>("a");
	std::vector<MyStruct> v;

	person p;
	p.age = 10;
	p.name = "1111";
	auto sssss = wheel::reflector::get(p);

	//for_ech
	wheel::reflector::for_each(p, [p](const auto& v, const auto index) {
		std::string name = wheel::reflector::get_name<decltype(p)>(index);
		std::cout << p.*v << std::endl;
		});

}
```
```cpp
/******************mysql/
#include <iostream>
#include <wheel/mysql_wrap.hpp>


struct name {
	std::string user_name;
	int age;
};

REFLECTION(name, user_name, age)

int main()
{

	std::vector<name>vec;
	for (int i =0;i<2;++i){
		name ns;
		ns.age = 10;
		ns.user_name = "1245";
		vec.emplace_back(ns);
	}

	wheel::mysql::mysql_wrap::get_intance().connect("127.0.0.1", "root", "root", "test");
	//更新指定一条数据
	//wheel::mysql::mysql_wrap::get_intance().update(ns);
	//更新指定n条数据
	//wheel::mysql::mysql_wrap::get_intance().update(vec);
	//wheel::mysql::mysql_wrap::get_intance().insert(vec);

	//wheel::mysql::mysql_wrap::get_intance().delete_records<name>("age =20 and user_name =\"yph1111\"");
	//auto result1 = wheel::mysql::mysql_wrap::get_intance().query<name>("select age from name where age =10 and user_name =\"yph1111\"");
	//wheel::mysql::mysql_wrap::get_intance().query("select age from name where age =10 and user_name =\"yph1111\"");
	wheel::mysql::query_result result;
	//wheel::mysql::mysql_wrap::get_intance().query("select age from name where age =10 and user_name =\"yph1111\"", result);
	//存在过程
	wheel::mysql::mysql_wrap::get_intance().query("call test_proc(10)", result);
	std::string sss = result.get_item_string(0, "user_name");
	int age = result.get_item_int(0, "age");
	//auto result4 = wheel::mysql::mysql_wrap::get_intance().query<std::tuple<int>>("select count(1) from name");

	//auto result5 = wheel::mysql::mysql_wrap::get_intance().query<std::tuple<std::string,int>>("select user_name, age from name");
}
```
```cpp
//https和
#include <iostream>
#include <wheel/http_server.hpp>

using namespace wheel::http_servers;
wheel::http_servers::http_server server;

//校验的切面
struct check {
	bool before(request& req, response& res) {
		std::cout << "before check" << std::endl;
		return true;
	}

	bool after(request& req, response& res) {
		std::cout << "after check" << std::endl;
		return true;
	}
};


int main()
{

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
	server.listen(9090);
	server.set_http_handler<GET, POST>("/", [](request& req, response& res) {
		res.set_status_and_content(status_type::ok, "hello world");
		});


	server.set_http_handler<GET, POST, OPTIONS,PUT>("/test", [](request& req, response& res) {

		std::string str = req.get_header_value("session");//获取包头值
		                                                    
		std::string str1 = req.get_query_value("name");//获取包体值
		std::string str2 = req.get_query_value("age");//获取包体值
		std::string str3 = req.get_query_value("sex");//获取包体值
		std::string body = req.body();
		//跨域解决
		res.add_header("Access-Control-Allow-origin", "*");
		res.set_status_and_content(status_type::ok, body.c_str());
		}, check{});

	server.run();
}
```
```cpp
//chunked pass string form-data urlencoded
#include <iostream>
#include <wheel/http_server.hpp>
#include <wheel/encoding_conversion.hpp>

using namespace wheel::http_servers;
wheel::http_servers::http_server server;

int main()
{
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

	server.enable_response_time(true);
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
		std::string path = wheel::char_encoding::encoding_conversion::utf8_to_gbk(req.get_query_value("path1"));
		res.add_header("Access-Control-Allow-origin", "*");
		//res.set_status_and_content(status_type::ok,"hello world");
		//res.set_status_and_content(status_type::ok, "hello world",res_content_type::string);
		//std::string str4 = wheel::char_encoding::encoding_conversion::gbk_to_utf8("我是好人我問三十多萬多無多無多無多多abcdefg");
		//res.set_urlencoded_datas("name",std::move(str4));
		//res.set_urlencoded_datas("age", "20123454");
		//res.set_urlencoded_status_and_content(status_type::ok,content_encoding::none,transfer_type::chunked);

		//res.set_multipart_data("name","wshihaoren"); //一次性发出的数据
		//res.set_multipart_data("age", "123");
		//res.set_multipart_data("age1", "456");
		//res.set_multipart_data("name1",std::move(str4));
		//std::string str5 = wheel::char_encoding::encoding_conversion::gbk_to_utf8("我是好人我問三十多萬多無多無多無多多abcdefg!~*&^23455698321550/.,??><");
		//res.set_multipart_data("name2", std::move(str5));

		//res.set_mstatus_and_content(status_type::ok,transfer_type::chunked);
		//res.set_status_and_content(status_type::ok, std::move(str4), res_content_type::urlencoded);
		std::string str5 = "我是好人我問三十多萬多無多無多無多多abcdefg!~*&^23455698321550/.,??><";
		if (wheel::char_encoding::encoding_conversion::is_valid_gbk(str5.c_str())){
			std::cout << "yes gbk" << std::endl;
		}

		std::string str6 = wheel::char_encoding::encoding_conversion::gbk_to_utf8(str5);
		res.set_status_and_content(status_type::ok,std::move(str6),res_content_type::string,content_encoding::none,transfer_type::chunked);
		});

	server.run();

