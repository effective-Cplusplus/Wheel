#ifndef read_tream_format_h__
#define read_tream_format_h__
#include "stream_format.hpp"

namespace wheel {
	class read_tream_format : public stream_format{
	public:
		read_tream_format() :read_pos_(0) {

		}

		virtual ~read_tream_format() {
		}

		virtual void set_stram_data(std::shared_ptr< native_stream > data) {
			data_ = data;
		}

		virtual std::int16_t get_cmd(void) {
			std::int16_t cmd = 0;
			read(reinterpret_cast<char*>(&cmd), sizeof(cmd), get_cmd_offset());
			return cmd;
		}

		virtual std::int16_t get_data_area_size(void) {
			std::int16_t size = 0;
			read(reinterpret_cast<char*>(&size), sizeof(size), get_packet_size_offset());
			return size;
		}

		virtual std::int16_t get_data_area_size(const char* buffer, int offset) {
			std::int16_t size = 0;
			memcpy(&size, buffer + offset, sizeof(size));
			return size;
		}

		virtual std::string read(identity<std::string >) {
			std::string value;

			read(value);

			return value;
		}

		virtual void read(std::string& value)
		{
			std::uint32_t size = 0;
			read(size);

			if (size == 0 || size > data_->get_size() - get_read_pos()) {
				value.clear();
				return;
			}

			value.resize(size);

			read(&*value.begin(), size);
		}

		template<class T>
		void read(T& value) {
			static_assert(std::is_integral<T>::value, "T must is_integral ");
			read(reinterpret_cast<char*>(&value), sizeof(value));
		}

		virtual void reset_read_body() {
			read_pos_ = 0;
		}

	private:
		std::size_t get_read_pos()const {
			return read_pos_;
		}

		void read(char* buffer, std::size_t size, std::size_t read_pos) {
			assert(read_pos + size <= get_header_size());
			memcpy(buffer, &data_->buffer_[0] + read_pos, size);
		}

		virtual bool read(char* buffer, std::size_t size) {
			if ((size + read_pos_ + get_header_size()) > data_->get_size()) {
				return false;
			}

			memcpy(buffer, &data_->buffer_[0] + read_pos_ + get_header_size(), size);
			read_pos_ += size;
			return true;
		}
	private:
		std::shared_ptr<native_stream>data_;
		std::size_t read_pos_;
	};
}
#endif // read_tream_format_h__
