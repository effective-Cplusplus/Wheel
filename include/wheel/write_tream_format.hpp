#ifndef write_tream_format_h__
#define write_tream_format_h__
#include "stream_format.hpp"

namespace wheel {
	class write_tream_format:public stream_format {
	public:
		write_tream_format(){
			data_ = std::make_shared<native_stream>();//会调用之前的析构，再调用构造函数
			data_->rezie_buffer_size(get_header_size());
		}

		virtual ~write_tream_format() {
			is_end_ = false;
		}

		virtual void write_header(std::int16_t cmd){
			flush();

			write(protocol_head_flag.c_str(), 2, 0);
			write(reinterpret_cast<const char*>(&cmd), sizeof(std::int16_t), get_cmd_offset());
		}

		virtual void end() {
			is_end_ = true;
			const std::int16_t size = static_cast<std::int16_t>(data_->get_size() - get_header_size());
			write(reinterpret_cast<const char*>(&size), sizeof(size), get_packet_size_offset());
		}

		virtual const native_stream* get_native_stream()const {
			return &(*data_);
		}

	private:
		void flush(void) {
			is_end_ = false;

			data_->set_data_size(get_header_size());
		}

		void write(const char* buffer, std::size_t size, std::size_t write_pos) {
			assert(write_pos + size <= get_header_size());

			if (!is_end_) {
				data_->rezie_buffer_size(data_->get_size() + size);
			}

			memcpy(&data_->buffer_[0] + write_pos, buffer, size);
		}
		
		virtual bool write(const char* buffer, std::size_t size) {
			if ((size + data_->get_size()) > native_stream::packer_buffer_size){
				return false;
			}

			if (!is_end_) {
				data_->rezie_buffer_size(data_->get_size() + size);
			}

			memcpy(&data_->buffer_[0] + data_->get_size(), buffer, size);
			std::size_t current_size = data_->get_size()+size;

			data_->set_data_size(current_size);
			return true;
		}
	private:
		bool is_end_ = false;
		const std::string protocol_head_flag="wl";
		std::shared_ptr< native_stream > data_;
	};
}
#endif // write_tream_format_h__
