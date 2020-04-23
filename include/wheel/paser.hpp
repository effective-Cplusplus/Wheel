#ifndef paser_h__
#define paser_h__
#include <memory>
#include "stream_format.hpp"

namespace wheel {
	enum parser_type { BIN_PARSER}; //0:二进制流解析

	enum parser_status {
		STATE_PARSER_HEAD_AREA,
		STATE_PARSER_DATA_AREA
	};

	class IProtocol_parser
	{
	public:
		static const std::int32_t ERROR_FORMAT_INVALID = -1;
		static const std::int32_t ERROR_PACKET_IMPERFECT = -2;
		static const std::int32_t ERROR_HEAD_FLAGS_INVALID = -3;

		virtual ~IProtocol_parser() {

		}

		virtual std::shared_ptr<stream_format>get_read_parser()const {
			return nullptr;
		}

		virtual std::shared_ptr<stream_format>get_write_parser()const {
			return nullptr;
		}
		//成功读到一个包或一个以上的包，返回TRUE
		virtual std::int32_t read_stream(const char*, const std::size_t, streams&) = 0;
	protected:
		IProtocol_parser(){

		}
	};

}
#endif // paser_h__
