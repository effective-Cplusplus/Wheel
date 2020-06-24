#ifndef string_stream_h__
#define string_stream_h__

#include <string>
namespace wheel {
	namespace str_stream {
		template <typename alloc_ty>
		class basic_string_stream {
		public:
			basic_string_stream() {
				header_ptr_ = this->alloc_.allocate(INIT_BUFF_SIZE);
				memset(&header_ptr_[0], 0, INIT_BUFF_SIZE);
				this->read_ptr_ = this->header_ptr_;
				this->write_ptr_ = this->header_ptr_;
				this->tail_ptr_ = this->header_ptr_ + length_;
			}

			~basic_string_stream() {
				this->alloc_.deallocate(header_ptr_, this->length_);
				header_ptr_ = nullptr;
			}

			inline std::size_t write(const char* buffer) {
				return write(buffer, strlen(buffer));
			}

			inline std::size_t read(const char* buffer, std::size_t len) {
				if (this->read_ptr_ + len > this->tail_ptr_)
				{
					status_ = read_overflow;
					return 0;
				}
				std::memcpy(buffer, this->read_ptr_, len);
				this->read_ptr_ += len;
				return len;
			}

			inline void write_str(char const* str, size_t len) {
				put('"');
				char const* ptr = str;
				char const* end = ptr + len;
				while (ptr < end)
				{
					const char c = *ptr;
					if (c == 0)
						break;
					++ptr;
					if (escape_[(unsigned char)c])
					{
						char buff[6] = { '\\', '0' };
						size_t len = 2;
						buff[1] = escape_[(unsigned char)c];
						if (buff[1] == 'u')
						{
							if (ptr < end)
							{
								buff[2] = (hex_table_[((unsigned char)c) >> 4]);
								buff[3] = (hex_table_[((unsigned char)c) & 0xF]);
								const char c1 = *ptr;
								++ptr;
								buff[4] = (hex_table_[((unsigned char)c1) >> 4]);
								buff[5] = (hex_table_[((unsigned char)c1) & 0xF]);
							}
							else
							{
								buff[2] = '0';
								buff[3] = '0';
								buff[4] = (hex_table_[((unsigned char)c) >> 4]);
								buff[5] = (hex_table_[((unsigned char)c) & 0xF]);
							}
							len = 6;
						}
						write(buff, len);
					}
					else
					{
						put(c);
					}
				}
				put('"');
			}

			inline bool bad()const { return status_ != good; }

			inline const char* data() const {
				return this->header_ptr_;
			}

			std::basic_string<char, std::char_traits<char>, alloc_ty> str() {
				std::basic_string<char, std::char_traits<char>, alloc_ty> s(this->header_ptr_, this->write_length());
				return s;
			}

			inline void put(char c) {
				std::size_t writed_len = this->write_ptr_ + 1 - this->header_ptr_;
				if (writed_len > this->length_)
				{
					this->growpup(writed_len);
				}
				*this->write_ptr_ = c;
				++this->write_ptr_;
			}

			inline std::size_t write(const char* buffer, std::size_t len) {
				std::size_t writed_len = this->write_ptr_ + len - this->header_ptr_;
				if (writed_len > this->length_)
				{
					this->growpup(writed_len);
				}
				std::memcpy((void*)this->write_ptr_, buffer, len);
				this->write_ptr_ += len;
				return len;
			}

			inline void clear() {
				this->read_ptr_ = this->header_ptr_;
				this->write_ptr_ = this->header_ptr_;
			}
		private:
			inline ::std::size_t read_length() const {
				return this->read_ptr_ - this->header_ptr_;
			}

			inline ::std::size_t write_length() const {
				return this->write_ptr_ - this->header_ptr_;
			}

			inline std::size_t growpup(std::size_t want_size) {
				std::size_t new_size = ((want_size + INIT_BUFF_SIZE - 1) / INIT_BUFF_SIZE) * INIT_BUFF_SIZE;
				std::size_t write_pos = this->write_ptr_ - this->header_ptr_;
				std::size_t read_pos = this->read_ptr_ - this->header_ptr_;
				char* temp = this->header_ptr_;
				this->header_ptr_ = this->alloc_.allocate(new_size);
				std::memcpy(this->header_ptr_, temp, this->length_);
				this->alloc_.deallocate(temp, this->length_);
				this->length_ = new_size;
				this->write_ptr_ = this->header_ptr_ + write_pos;
				this->read_ptr_ = this->header_ptr_ + read_pos;
				this->tail_ptr_ = this->header_ptr_ + length_;
				return new_size;
			}
		private:
			alloc_ty alloc_;
			enum { good, read_overflow };
			enum { INIT_BUFF_SIZE = 1024 };


			char* header_ptr_;
			char* read_ptr_;
			char* write_ptr_;
			char* tail_ptr_;
			int			status_ = good;
			std::size_t	length_ = INIT_BUFF_SIZE;

			char const* hex_table_ = "0123456789ABCDEF";
			char const escape_[256] = {
		#define Z16 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
				//0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
				'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'b', 't', 'n', 'u', 'f', 'r', 'u', 'u', // 00
				'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', // 10
				0, 0, '"', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 20
				Z16, Z16,																		// 30~4F
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '\\', 0, 0, 0, // 50
				Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16								// 60~FF
		#undef Z16
			};
		};

		typedef basic_string_stream<std::allocator<char>> string_stream;
	}//str_stream
}//wheel


#endif // string_stream_h__
