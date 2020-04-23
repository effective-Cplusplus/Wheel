#ifndef websocket_handle_h__
#define websocket_handle_h__
#include "sha1.hpp"
#include "base64.hpp"
#include "ws_define.h"

namespace wheel {
	namespace websocket {
		class websocket_handle
		{
		public:
			websocket_handle() {

			}~websocket_handle() {

			}

			std::string handle_shark_respond(std::string server_key) {
				std::string msg = comm_header_[0];
				msg += comm_header_[1];

				server_key += ws_guid;
				unsigned int message_digest[5] = {0};
				sha1 sha;
				sha.reset();
				sha << server_key.c_str();
				sha.result(message_digest);
				for (int i = 0; i < 5; i++) {
					message_digest[i] = htonl(message_digest[i]);
				}

				msg += "Sec-WebSocket-Accept: ";
				server_key = base64_encode(reinterpret_cast<const unsigned char*>(message_digest), 20);
				server_key += "\r\n";
				msg += server_key;
				msg += comm_header_[2];
				msg += "\r\n\r\n";

				return msg;
			}

			bool compare_handle_shark_key(const std::string& accept_key, const std::string& send_key){
				std::string temp_send_key = std::move(send_key);
				temp_send_key += ws_guid;
				sha1 sha;
				sha.reset();
				sha << temp_send_key.c_str();
				unsigned int message_digest[5] = {0};
				sha.result(message_digest);
				for (int i = 0; i < 5; i++) {
					message_digest[i] = htonl(message_digest[i]);
				}

				std::string server_key = base64_encode(reinterpret_cast<const unsigned char*>(message_digest), 20);
				if (server_key == accept_key){
					return true;
				}

				return false;
			}

			bool parse_header(const char* buf, size_t size) {
				const unsigned char* inp = (const unsigned char*)(buf);

				msg_opcode_ = inp[read_pos_] & 0x0F;
				msg_fin_ = (inp[read_pos_] >> 7) & 0x01;
				++read_pos_;
				mask_masked_ = (inp[read_pos_] >> 7);

				int length_field = inp[read_pos_] & (~0x80);

				++read_pos_;
				if (length_field <= 125) {
					payload_length_ = length_field;
				}
				else if (length_field == 126)  //msglen is 16bit!
				{
					unsigned short int length = 0;
					memcpy(&length, buf + read_pos_, 2);
					read_pos_ += 2;
					payload_length_ = ntohs(length);
				}
				else if (length_field == 127)  //msglen is 64bit!
				{
					unsigned short int length = 0;
					memcpy(&length, buf + read_pos_, 4);
					read_pos_ += 4;
					payload_length_ = ntohs(length);
				}
				else {
					return false;
				}

				if (mask_masked_) {
					for (int i = 0; i < 4; i++) {
						masking_key_[i] = buf[read_pos_ + i];
					}
				}

				read_pos_ += 4;
				return true;
			}

			ws_frame_type parse_payload(const char* buf, size_t size, std::string& outbuf) {
				if (payload_length_ > size)
					return ws_frame_type::WS_INCOMPLETE_FRAME;

				if (payload_length_ > outbuf.size()) {
					outbuf.resize((size_t)payload_length_);
				}

				if (mask_masked_ != 1) {
					memcpy((void*)outbuf.c_str(), buf + read_pos_, payload_length_);
				}
				else {
					// unmask data:
					for (size_t i = 0; i < payload_length_; i++) {
						outbuf[i] = buf[read_pos_ + i] ^ masking_key_[i % 4];
					}
				}

				if (msg_opcode_ == 0x0) return (msg_fin_) ? ws_frame_type::WS_TEXT_FRAME : ws_frame_type::WS_INCOMPLETE_TEXT_FRAME; // continuation frame ?
				if (msg_opcode_ == 0x1) return (msg_fin_) ? ws_frame_type::WS_TEXT_FRAME : ws_frame_type::WS_INCOMPLETE_TEXT_FRAME;
				if (msg_opcode_ == 0x2) return (msg_fin_) ? ws_frame_type::WS_BINARY_FRAME : ws_frame_type::WS_INCOMPLETE_BINARY_FRAME;
				if (msg_opcode_ == 0x8) return ws_frame_type::WS_CLOSE_FRAME;
				if (msg_opcode_ == 0x9) return ws_frame_type::WS_PING_FRAME;
				if (msg_opcode_ == 0xA) return ws_frame_type::WS_PONG_FRAME;
				return ws_frame_type::WS_BINARY_FRAME;
			}

			std::uint64_t get_payload_length()const {
				return payload_length_;
			}

			std::string format_header(size_t length, opcode code) {
				size_t header_length = encode_header(length, code);
				return { msg_header_, header_length };
			}

			std::string format_close_payload(uint16_t code, char* message, size_t length) {
				std::string close_payload;
				if (code) {
					close_payload.resize(length + 2);
					code = htons(code);
					std::memcpy((void*)close_payload.data(), &code, 2);
					std::memcpy((void*)(close_payload.data() + 2), message, length);
				}
				return close_payload;
			}

		private:
			size_t encode_header(size_t length, opcode code) {
				size_t header_length;

				if (length < 126) {
					header_length = 2;
					msg_header_[1] = static_cast<char>(length);
				}
				else if (length <= UINT16_MAX) {
					header_length = 4;
					msg_header_[1] = 126;
					*((uint16_t*)&msg_header_[2]) = htons(static_cast<uint16_t>(length));
				}
				else {
					header_length = 10;
					msg_header_[1] = 127;
					*((uint64_t*)&msg_header_[2]) = htonl(length);
				}

				int flags = 0;
				msg_header_[0] = (flags & SND_NO_FIN ? 0 : 128);
				if (!(flags & SND_CONTINUATION)) {
					msg_header_[0] |= code;
				}

				return header_length;
			}
		private:
			std::string comm_header_[3] = {
				"HTTP/1.1 101 Switching Protocols\r\n",
				 "Connection: upgrade\r\n",
				 "Upgrade: websocket"
			};


			std::uint64_t payload_length_ = 0;
			unsigned int mask_masked_ = 0;
			unsigned char msg_opcode_ = 0;
			unsigned char msg_fin_ = 0;
			unsigned char  masking_key_[4] = { 0 };
			std::size_t read_pos_ = 0;
			char msg_header_[10] = { 0 };
		};
  }
}
#endif // websocket_handle_h__
