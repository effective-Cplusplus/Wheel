#include "proxyEngine.h"
#include <wheel/tcp_hanlde.hpp>

ProxyEngine::ProxyEngine()
{

}

ProxyEngine::~ProxyEngine()
{

}

int ProxyEngine::OnMessage(std::shared_ptr<wheel::tcp_socket::tcp_handle> handler, std::shared_ptr<wheel::native_stream> streams)
{
	handler->get_read_parser()->set_stram_data(streams);
	int cmd = handler->get_read_parser()->get_cmd();
	int value = handler->get_read_parser()->read<int>();
	std::int16_t value1 = handler->get_read_parser()->read<std::int16_t>();
	std::string str = handler->get_read_parser()->read<std::string>();

	if (cmd ==4){
		handler->get_write_parser()->write_header(425);
		handler->get_write_parser()->write<std::int16_t>(102);
		handler->get_write_parser()->write(wheel::unit::float_to_uint32(102.1f));
		handler->get_write_parser()->write_string("102");
		handler->get_write_parser()->end();
		handler->to_send(*handler->get_write_parser()->get_native_stream());
	}
	return 0;
}

int ProxyEngine::OnMessage1(std::shared_ptr<wheel::websocket::ws_tcp_handle> handler, std::shared_ptr<wheel::native_stream> streams)
{
	return 0;
}
