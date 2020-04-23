#ifndef native_stream_h__
#define native_stream_h__

#include <vector>
#include <cstring>

namespace wheel {
	class native_stream
	{
	public:
		const static std::size_t packer_buffer_size = 1024 * 100; //10M支持;
		native_stream() {
		}

		~native_stream() {

		}

		native_stream(const native_stream& stream) {
			packet_size_ = stream.packet_size_;
			buffer_.resize(packet_size_);
			memcpy(&buffer_[0], stream.buffer_.c_str(), stream.packet_size_);
		}

		native_stream(const char* buffer, std::size_t size)
			:packet_size_(size) {
			buffer_.resize(packet_size_);
			memcpy(&buffer_[0], buffer, size);
		}

		const char* get_data()const {
			return buffer_.c_str();
		}

		std::size_t get_size()const {
			return packet_size_;
		}
	
		void set_data_size(std::size_t size)	{
			packet_size_ = size;
		}

		void rezie_buffer_size(std::size_t size) {
			buffer_.resize(size);
		}

		std::string buffer_;//使用一个动态的buffer去存，解决内存动态分配问题,内存利用问题
	private:
		std::size_t packet_size_ = 0;
	};

	typedef std::vector< std::shared_ptr<native_stream> > streams;
}


#endif // native_stream_h__
