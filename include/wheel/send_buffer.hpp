#ifndef send_buffer_h__
#define send_buffer_h__
#include <string>
namespace wheel {

	const int g_default_buffer_size = 1024 * 16;
	class send_buffer {
	private:
		send_buffer() = delete;
		send_buffer(const send_buffer&) = delete;
		send_buffer& operator = (const send_buffer&) = delete;
	public:
		send_buffer(const char* data, std::size_t len)
		:read_pos_(0)
		,write_pos_(len){
			memcpy(buffer_, data, len);
		}

		~send_buffer() {

		}

		const char* data()
		{
			return buffer_ + read_pos_;
		}

		void read(const std::size_t to_read) {
			read_pos_ += to_read;
		}

		bool write(const char* data, std::size_t size) {
			if (size + write_pos_ <= g_default_buffer_size)
			{
				memcpy(buffer_ + write_pos_, data, size);
				write_pos_ += size;

				return true;
			}
			else
			{
				return false;
			}
		}

		std::size_t size()const {
			return write_pos_ - read_pos_;
		}
	private:

		std::size_t	read_pos_;
		std::size_t	write_pos_;

		char buffer_[g_default_buffer_size] = { 0 };
	};
}
#endif // send_buffer_h__
