#ifndef bin_parser_h__
#define bin_parser_h__

#include "paser.hpp"
#include "read_tream_format.hpp"
#include "write_tream_format.hpp"
#include "traits.hpp"

namespace wheel {

	const char g_protocol_head_flag[3] = "wl";
	const std::size_t g_packet_buffer_size = 1024 *100;


	class bin_parser : public IProtocol_parser
	{
	public:
		bin_parser(std::size_t header_size,std::size_t packet_size_offset,std::size_t packet_cmd_offset) 
		:packet_header_size_(header_size)
		,packet_size_offset_(packet_size_offset)
		,packet_cmd_offset_(packet_cmd_offset){
		}

		virtual ~bin_parser() {

		}

		std::shared_ptr<stream_format>get_read_parser()const {
			return read_parser_stream_;
		}

		virtual std::shared_ptr<stream_format>get_write_parser()const {
			return write_parser_stream_;
		}

		virtual std::int32_t read_stream(const char* data, const std::size_t data_size, streams& stream_vec) {
			buffer_ = wheel::traits::make_unique<char[]>(data_size);

			if (buffer_ == nullptr){
				return -1;
			}

			read_parser_stream_ = std::make_shared<read_tream_format>();
			if (read_parser_stream_ == nullptr){
				return -1;
			}

			read_parser_stream_->set_cmd_offset(packet_cmd_offset_);
			read_parser_stream_->set_header_size(packet_header_size_);
			read_parser_stream_->set_packet_size_offset(packet_size_offset_);

			write_parser_stream_ = std::make_shared<write_tream_format>();
			if (write_parser_stream_ == nullptr){
				return -1;
			}

			write_parser_stream_->set_cmd_offset(packet_cmd_offset_);
			write_parser_stream_->set_header_size(packet_header_size_);
			write_parser_stream_->set_packet_size_offset(packet_size_offset_);

			std::size_t readpos = 0;
			while (readpos < data_size) {
				if (status_ == STATE_PARSER_HEAD_AREA) {
					const auto err = read_header_area(data, data_size, readpos);
					if (err != 0) {
						return err;
					}
				}
				else if (status_ == STATE_PARSER_DATA_AREA) {
					if (read_data_area(data, data_size, readpos)) {
						std::shared_ptr<native_stream> stream(new native_stream(&buffer_[0], write_pos_));
						stream_vec.push_back(stream);

						clear();
					}
				}
			}

			return 0;
		}
	private:
		// 读取Packet头数据, 
		std::int32_t read_header_area(const char* data, const size_t length, size_t& ndx) {
			while (write_pos_ < packet_header_size_ && ndx < length) {
				buffer_[write_pos_++] = data[ndx++];

				if (((write_pos_ == 1) && (buffer_[0] != g_protocol_head_flag[0])) ||
					((write_pos_ == 2) && (buffer_[1] != g_protocol_head_flag[1]))) {
					write_pos_ = 0;

					//如果 1024个字节内未发现有效数据包，直接报错
					if (++err_count_ > 1024) {
						return ERROR_HEAD_FLAGS_INVALID;
					}
				}
			}

			//主要包体可以通过,校验包体长度
			if (write_pos_ < packet_header_size_) {
				return ERROR_PACKET_IMPERFECT;
			}

			data_area_size_ = read_parser_stream_->get_data_area_size(&buffer_[0], packet_size_offset_);

			if (data_area_size_ <= (g_packet_buffer_size - packet_header_size_ - 4u))
			{
				status_ = STATE_PARSER_DATA_AREA;
				return 0;
			}else{
				write_pos_ = 0;
				return ERROR_FORMAT_INVALID;
			}

		}

		bool read_data_area(const char* data, const std::size_t recv_data_size, std::size_t& read_pos) {
			std::size_t need = (data_area_size_ + packet_header_size_) - write_pos_;
			if (need == 0) {
				//一个完整的数据包
				return true;
			}

			std::size_t buffed = recv_data_size - read_pos;
			if (buffed == 0) {
				//收到的数据都用完了
				return false;
			}

			std::size_t copy = buffed < need ? buffed : need;

			memcpy(&buffer_[0] + write_pos_, data + read_pos, copy);

			write_pos_ += copy;

			read_pos += copy;

			return write_pos_ == (data_area_size_ + packet_header_size_);
		}

		void clear(void) {
			status_ = STATE_PARSER_HEAD_AREA;
			write_pos_ = 0;
			data_area_size_ = 0;
		}

	private:
		// 当前处理状态
		int status_ = STATE_PARSER_HEAD_AREA;
		std::size_t packet_header_size_;
		std::size_t packet_size_offset_;
		std::size_t packet_cmd_offset_;

		size_t write_pos_ =0;

		size_t data_area_size_ = 0;
		std::unique_ptr<char[]>buffer_ = nullptr;
		std::shared_ptr<stream_format>read_parser_stream_ = nullptr;
		std::shared_ptr<stream_format>write_parser_stream_ = nullptr;
		//包解析错误计数
		std::int32_t err_count_ = 0;
	};
}

#endif // bin_parser_h__
