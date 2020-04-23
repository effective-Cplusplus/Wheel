
#ifndef proxyEngine_h__
#define proxyEngine_h__
#include <wheel/tcp_hanlde.hpp>
#include <wheel/ws_tcp_handle.hpp>
#include <wheel/websocket_server.hpp>

class ProxyEngine {
public:
	ProxyEngine();
	~ProxyEngine();

	int OnMessage(std::shared_ptr<wheel::tcp_socket::tcp_handle> handler,std::shared_ptr<wheel::native_stream> streams);

	int OnMessage1(std::shared_ptr<wheel::websocket::ws_tcp_handle> handler, std::shared_ptr<wheel::native_stream> streams);

};
#endif // proxyEngine_h__