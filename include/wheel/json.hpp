#ifndef json_h__
#define json_h__
#include <array>

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <type_traits>
#include <vector>
#include <unordered_map>
#include <ios>
#include "unit.hpp"
#include "reflection.hpp"

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif // _MSC_VER

namespace wheel {
	namespace json {
		namespace detail {
			struct string_ref {
				char const* str = nullptr;
				size_t        len = 0;
				bool operator == (string_ref const& rhs) const {
					if (len == rhs.len) {
						return std::memcmp(str, rhs.str, len) == 0;
					}

					return false;
				}


				bool operator != (std::string rhs) const{
					if (len == rhs.length()){
						return std::memcmp(str, rhs.data(), len) != 0;
					}

					return true;
				}
			};

			inline char const* char_table() {
				static char table[] = {
				  16, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 16, 17, 17, 16,
				  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
				  16, 16, 17, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
				  16, 16, 16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 16, 16, 16, 16, 16,
				  16, 16, 10, 11, 12, 13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16,
				  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
				  16, 16, 10, 11, 12, 13, 14, 15 };
				return table;
			}

			inline bool is_ws(char c) {
				if (c > ' ' || c <= 0)
					return false;
				return char_table()[(size_t)c] == 17;
			}

			inline char const* skip_ws(char const* str) {
				while (is_ws(*str))++str;
				return str;
			}
		}

		struct token{
			detail::string_ref str;
			enum{
				t_string,
				t_int,
				t_uint,
				t_number,
				t_ctrl,
				t_end,
			} type;

			union{
				int64_t   i64;
				uint64_t  u64;
				double    d64;
			} value;
			bool neg = false;
		};

#ifdef _MSC_VER
#define NOEXCEPT
#else
#define NOEXCEPT noexcept
#endif

		class exception : public std::exception
		{
			std::string msg_;
		public:
			exception(std::string const& msg)
				:msg_(msg)
			{}
			char const* what() const NOEXCEPT { return msg_.c_str(); }
		};

		class reader{
			token   cur_tok_;
			size_t  cur_col_ = 0;
			size_t  cur_line_ = 0;
			size_t  len_ = 0;
			size_t  cur_offset_ = 0;
			bool    end_mark_ = false;
			const char* ptr_ = nullptr;
			double decimal = 0.1;
			int    exp = 0;
			inline void decimal_reset() { decimal = 0.1; }

			inline char read() const{
				if (end_mark_)
					return 0;
				return ptr_[cur_offset_];
			}

			inline void take(){
				if (end_mark_ == false){
					++cur_offset_;
					char v = ptr_[cur_offset_];
					if (v != '\r')
						++cur_col_;
					if (len_ > 0 && cur_offset_ >= len_)
					{
						end_mark_ = true;
					}
					else if (v == 0)
					{
						end_mark_ = true;
					}
					if (v == '\n')
					{
						cur_col_ = 0;
						++cur_line_;
					}
				}
			}

			char skip(){
				auto c = read();
				while (c == ' ' || c == '\t' || c == '\r' || c == '\n'){
					take();
					c = read();
				}

				return c;
			}

			void parser_quote_string(){
				take();
				cur_tok_.str.str = ptr_ + cur_offset_;
				auto c = read();
				do
				{
					switch (c){
					case 0:
					case '\n':{
						error("not a valid quote string!");
						break;
					}
					case '\\':{
						take();
						c = read();
						break;
					}
					case '"':{
						cur_tok_.str.len = ptr_ + cur_offset_ - cur_tok_.str.str;
						take();
						return;
					}
					}
					take();
					c = read();
				} while (true);
			}

			void parser_string(){
				cur_tok_.str.str = ptr_ + cur_offset_;
				take();
				auto c = read();
				do{
					switch (c){
					case 0:{
						error("not a valid string!");
						break;
					}
					case ' ':
					case '\t':
					case '\r':
					case '\n':
					case ',':
					case '[':
					case ']':
					case ':':
					case '{':
					case '}':
					{
						cur_tok_.str.len = ptr_ + cur_offset_ - cur_tok_.str.str;
						return;
					}
					}
					take();
					c = read();
				} while (true);
			}

			void parser_exp_pos(){
				take();
				auto c = read();
				do{
					switch (c){
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
					{
						exp *= 10;
						exp += (c - '0');
						break;
					}
					default:
					{
						for (int i = 0; i < exp; ++i)
						{
							cur_tok_.value.d64 *= 10.0;
						}
						exp = 0;
						cur_tok_.str.len = ptr_ + cur_offset_ - cur_tok_.str.str;
						return;
					}
					}
					take();
					c = read();
				} while (1);
			}

			void parser_exp_neg()
			{
				take();
				auto c = read();
				do
				{
					switch (c)
					{
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
					{
						exp *= 10;
						exp += (c - '0');
						break;
					}
					default:
					{
						for (int i = 0; i < exp; ++i)
						{
							cur_tok_.value.d64 *= 0.1;
						}
						exp = 0;
						cur_tok_.str.len = ptr_ + cur_offset_ - cur_tok_.str.str;
						return;
					}
					}
					take();
					c = read();
				} while (1);
			}

			void parser_exp(){
				take();
				auto c = read();
				do{
					switch (c){
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':{
						exp *= 10;
						exp += (c - '0');
						parser_exp_pos();
						return;
					}
					case '-':{
						parser_exp_neg();
						return;
					}
					default:{
						exp = 0;
						cur_tok_.str.len = ptr_ + cur_offset_ - cur_tok_.str.str;
						return;
					}
					}
				} while (1);
			}

			void parser_number(){
				cur_tok_.str.str = ptr_ + cur_offset_;
				take();
				auto c = read();
				do{
					switch (c){
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':{
						if (cur_tok_.type == token::t_int){
							cur_tok_.value.i64 *= 10;
							cur_tok_.value.i64 += c - '0';
						}
						else if (cur_tok_.type == token::t_uint){
							cur_tok_.value.u64 *= 10;
							cur_tok_.value.u64 += c - '0';
						}
						else if (cur_tok_.type == token::t_number){
							cur_tok_.value.d64 += decimal * (c - '0');
							decimal *= 0.1;
						}
						break;
					}
					case '.':{
						if (cur_tok_.type == token::t_int){
							cur_tok_.type = token::t_number;
							cur_tok_.value.d64 = (double)cur_tok_.value.i64;
							decimal_reset();
						}
						else if (cur_tok_.type == token::t_uint){
							cur_tok_.type = token::t_number;
							cur_tok_.value.d64 = (double)cur_tok_.value.u64;
							decimal_reset();
						}
						else if (cur_tok_.type == token::t_number){
							error("not a valid number!");
						}
						break;
					}
					case 'e':
					case 'E':{
						if (cur_tok_.type == token::t_int){
							cur_tok_.type = token::t_number;
							cur_tok_.value.d64 = (double)cur_tok_.value.i64;
						}
						else if (cur_tok_.type == token::t_uint){
							cur_tok_.type = token::t_number;
							cur_tok_.value.d64 = (double)cur_tok_.value.u64;
						}
						parser_exp();
					}
					default:{
						cur_tok_.str.len = ptr_ + cur_offset_ - cur_tok_.str.str;
						return;
					}
					}
					take();
					c = read();
				} while (1);
			}
		public:
			reader(const char* ptr = nullptr, size_t len = -1)
				:len_(len), ptr_(ptr){
				if (ptr == nullptr){
					end_mark_ = true;
				}
				else if (len == 0){
					end_mark_ = true;
				}
				else if (ptr[0] == 0){
					end_mark_ = true;
				}
				next();
			}

			static inline char* itoa_native(size_t val, char* buffer, size_t len){
				buffer[len] = 0;
				size_t pos = len - 1;
				if (val == 0){
					buffer[pos--] = '0';
				}
				while (val){
					buffer[pos--] = (char)((val % 10) + '0');
					val = val / 10;
				}
				++pos;
				return &buffer[0] + pos;
			}

			inline void error(const char* message) const{
				char buffer[20];
				std::string msg = "error at line :";
				msg += itoa_native(cur_line_, buffer, 19);
				msg += " col :";
				msg += itoa_native(cur_col_, buffer, 19);
				msg += " msg:";
				msg += message;
				throw exception(msg);
			}

			inline token const& peek() const{
				return cur_tok_;
			}

			void next(){
				auto c = skip();
				bool do_next = false;
				cur_tok_.neg = false;
				switch (c){
				case 0:
					cur_tok_.type = token::t_end;
					cur_tok_.str.str = ptr_ + cur_offset_;
					cur_tok_.str.len = 1;
					break;
				case '{':
				case '}':
				case '[':
				case ']':
				case ':':
				case ',':{
					cur_tok_.type = token::t_ctrl;
					cur_tok_.str.str = ptr_ + cur_offset_;
					cur_tok_.str.len = 1;
					take();
					break;
				}
				case '/':{
					take();
					c = read();
					if (c == '/'){
						take();
						c = read();
						while (c != '\n' && c != 0)
						{
							take();
							c = read();
						}
						do_next = true;
						break;
					}
					else if (c == '*'){
						take();
						c = read();
						do{
							while (c != '*')
							{
								if (c == 0)
								{
									return;
								}
								take();
								c = read();
							}
							take();
							c = read();
							if (c == '/')
							{
								take();
								do_next = true;
								break;
							}
						} while (true);
					}
					//error parser comment
					error("not a comment!");
				}
				case '"':{
					cur_tok_.type = token::t_string;
					parser_quote_string();
					break;
				}
				default:{
					if (c >= '0' && c <= '9')
					{
						cur_tok_.type = token::t_uint;
						cur_tok_.value.u64 = c - '0';
						parser_number();
					}
					else if (c == '-'){
						cur_tok_.type = token::t_int;
						cur_tok_.value.i64 = 0;
						cur_tok_.neg = true;
						parser_number();
					}
					else
					{
						cur_tok_.type = token::t_string;
						parser_string();
					}
				}
				}
				if (do_next == false)
					return;
				next();
			}

			inline bool expect(char c)
			{
				return cur_tok_.str.str[0] == c;
			}
		};

		template <typename alloc_ty>
		struct json_string_stream
		{
		private:
			alloc_ty alloc;
		public:
			enum { good, read_overflow };

			char* m_header_ptr;
			char* m_read_ptr;
			char* m_write_ptr;
			char* m_tail_ptr;
			int							m_status;
			std::size_t			m_length;

			enum { INIT_BUFF_SIZE = 1024 };
			json_string_stream() :m_status(good), m_length(INIT_BUFF_SIZE){
				this->m_header_ptr = this->alloc.allocate(INIT_BUFF_SIZE);
				this->m_read_ptr = this->m_header_ptr;
				this->m_write_ptr = this->m_header_ptr;
				this->m_tail_ptr = this->m_header_ptr + m_length;
			}

			~json_string_stream(){
				this->alloc.deallocate(m_header_ptr, this->m_length);
			}

			inline std::size_t read(char* buffer, std::size_t len)	{
				if (this->m_read_ptr + len > this->m_tail_ptr){
					m_status = read_overflow;
					return 0;
				}
				std::memcpy(buffer, this->m_read_ptr, len);
				this->m_read_ptr += len;
				return len;
			}

			inline std::size_t growpup(std::size_t want_size){
				std::size_t new_size = ((want_size + INIT_BUFF_SIZE - 1) / INIT_BUFF_SIZE) * INIT_BUFF_SIZE;
				std::size_t write_pos = this->m_write_ptr - this->m_header_ptr;
				std::size_t read_pos = this->m_read_ptr - this->m_header_ptr;
				char* temp = this->m_header_ptr;
				this->m_header_ptr = this->alloc.allocate(new_size);
				std::memcpy(this->m_header_ptr, temp, this->m_length);
				this->alloc.deallocate(temp, this->m_length);
				this->m_length = new_size;
				this->m_write_ptr = this->m_header_ptr + write_pos;
				this->m_read_ptr = this->m_header_ptr + read_pos;
				this->m_tail_ptr = this->m_header_ptr + m_length;
				return new_size;
			}

			inline std::size_t write(const char* buffer, std::size_t len){
				std::size_t writed_len = this->m_write_ptr + len - this->m_header_ptr;
				if (writed_len > this->m_length)
				{
					this->growpup(writed_len);
				}
				std::memcpy((void*)this->m_write_ptr, buffer, len);
				this->m_write_ptr += len;
				return len;
			}

			inline void put(char c){
				std::size_t writed_len = this->m_write_ptr + 1 - this->m_header_ptr;
				if (writed_len > this->m_length)
				{
					this->growpup(writed_len);
				}
				*this->m_write_ptr = c;
				++this->m_write_ptr;
			}


			inline bool bad()const { return m_status != good; }

			inline json_string_stream& seekp(int offset, int seek_dir){
				switch (seek_dir){
				case std::ios::beg:	{
					if (offset < 0)
					{
						offset = 0;
					}
					this->m_write_ptr = this->m_header_ptr + offset;
					break;
				}
				case std::ios::cur:{
					if (offset < 0)
					{
						offset = offset + int(this->m_write_ptr - this->m_header_ptr);
						if (offset < 0)
						{
							offset = 0;
						}
						this->m_write_ptr = this->m_header_ptr + offset;
					}
					else
					{
						if (this->m_write_ptr + offset > this->m_tail_ptr)
						{
							this->m_write_ptr = this->m_tail_ptr;
						}
					}

					break;
				}
				case std::ios::end:{
					if (offset < 0){
						offset = offset + int(this->m_write_ptr - this->m_header_ptr);
						if (offset < 0)
						{
							offset = 0;
						}
						this->m_write_ptr = this->m_header_ptr + offset;
					}
					break;
				}
				}
				return *this;
			}

			inline void clear(){
				this->m_read_ptr = this->m_header_ptr;
				this->m_write_ptr = this->m_header_ptr;
			}

			inline const char* data() const{
				return this->m_header_ptr;
			}

			std::basic_string<char, std::char_traits<char>, alloc_ty> str(){
				std::basic_string<char, std::char_traits<char>, alloc_ty> s(this->m_header_ptr, this->write_length());
				return s;
			}

			inline ::std::size_t read_length() const{
				return this->m_read_ptr - this->m_header_ptr;
			}

			inline ::std::size_t write_length() const{
				return this->m_write_ptr - this->m_header_ptr;
			}
		};

		typedef json_string_stream<std::allocator<char>> string_stream;

		inline bool is_true(token const& tok){
			char const* ptr = tok.str.str;
			if (tok.str.len == 4)
			{
				return (ptr[0] == 't' || ptr[0] == 'T') &&
					(ptr[1] == 'r' || ptr[1] == 'R') &&
					(ptr[2] == 'u' || ptr[2] == 'U') &&
					(ptr[3] == 'e' || ptr[3] == 'E');
			}
			return false;
		}

		inline char char_to_hex(char v)	{
			if (v >= 0 && v <= 'f')
			{
				v = detail::char_table()[(size_t)v];
			}
			else
			{
				v = 16;
			}
			return v;
		}

		inline uint64_t read_utf(const char* data, size_t){
			char v = char_to_hex(*data++);
			if (v >= 16)
				return 0;
			uint64_t utf = v;
			utf <<= 4;
			v = char_to_hex(*data++);
			if (v >= 16)
				return 0;
			utf += v;
			utf <<= 4;
			v = char_to_hex(*data++);
			if (v >= 16)
				return 0;
			utf += v;
			utf <<= 4;
			v = char_to_hex(*data++);
			if (v >= 16)
				return 0;
			utf += v;
			return utf;
		}

		template<typename string_ty>
		inline bool esacpe_utf8(string_ty& str, uint64_t utf1){
			if (utf1 < 0x80){
				str.append(1, (char)utf1);
			}
			else if (utf1 < 0x800){
				str.append(1, (char)(0xC0 | ((utf1 >> 6) & 0xFF)));
				str.append(1, (char)(0x80 | ((utf1 & 0x3F))));
			}
			else if (utf1 < 0x80000){
				str.append(1, (char)(0xE0 | ((utf1 >> 12) & 0xFF)));
				str.append(1, (char)(0x80 | ((utf1 >> 6) & 0x3F)));
				str.append(1, (char)(0x80 | ((utf1 & 0x3F))));
			}
			else{
				if (utf1 < 0x110000){
					return false;
				}
				str.append(1, (char)(0xF0 | ((utf1 >> 18) & 0xFF)));
				str.append(1, (char)(0x80 | ((utf1 >> 12) & 0x3F)));
				str.append(1, (char)(0x80 | ((utf1 >> 6) & 0x3F)));
				str.append(1, (char)(0x80 | ((utf1 & 0x3F))));
			}
			return true;
		}

		template<typename string_ty>
		bool escape_string(string_ty& str, const char* data, size_t len)
		{
			str.clear();
			str.reserve(len);
			if (len == 0)
				return true;
			do
			{
				auto c = *data++;
				--len;
				switch (c)
				{
				case '\\':
				{
					c = *data++;
					--len;
					switch (c)
					{
					case '\\':
					{
						c = '\\';
						break;
					}
					case '/':
					{
						c = '/';
						break;
					}
					case 'b':
					{
						c = '\b';
						break;
					}
					case 'f':
					{
						c = '\f';
						break;
					}
					case 'n':
					{
						c = '\n';
						break;
					}
					case 'r':
					{
						c = '\r';
						break;
					}
					case 't':
					{
						c = '\t';
						break;
					}
					case '"':
					{
						break;
					}
					case 'u':
					{
						if (len < 4)
							return false;
						uint64_t uft1 = read_utf(data, len);
						data += 4;
						len -= 4;
						if (uft1 == 0)
							return false;
						if (!esacpe_utf8(str, uft1))
							return false;
						continue;
					}
					default:
					{
						return false;
					}
					}
					break;
				}
				}
				str.append(1, c);
			} while (len > 0);
			return true;
		}


		inline void char_array_read(reader& rd, char* val, size_t N)
		{
			auto& tok = rd.peek();
			if (tok.type == token::t_string)
			{
				std::string str;
				if (!escape_string(str, tok.str.str, tok.str.len))
				{
					rd.error("not a valid string.");
				}
				size_t len = str.length();
				if (len > N)
					len = N;
				std::memcpy(val, str.data(), len);
				if (len < N)
					val[len] = 0;
			}
			else
			{
				rd.error("not a valid string.");
			}
			rd.next();
		}

		inline void skip_array(reader& rd);

		inline void skip_object(reader& rd);

		inline void skip(reader& rd)
		{
			auto& tok = rd.peek();
			switch (tok.type)
			{
			case token::t_string:
			case token::t_int:
			case token::t_uint:
			case token::t_number:
			{
				rd.next();
				return;
			}
			case token::t_ctrl:
			{
				if (tok.str.str[0] == '[')
				{
					rd.next();
					skip_array(rd);
					return;
				}
				else if (tok.str.str[0] == '{')
				{
					rd.next();
					skip_object(rd);
					return;
				}
			}
			case token::t_end:
			{
				return;
			}
			}
			rd.error("invalid json document!");
		}

		inline void skip_array(reader& rd)
		{
			auto tok = &rd.peek();
			while (tok->str.str[0] != ']')
			{
				skip(rd);
				tok = &rd.peek();
				if (tok->str.str[0] == ',')
				{
					rd.next();
					tok = &rd.peek();
					continue;
				}
			}
			rd.next();
		}

		inline void skip_key(reader& rd)
		{
			auto& tok = rd.peek();
			switch (tok.type)
			{
			case token::t_string:
			case token::t_int:
			case token::t_uint:
			case token::t_number:
			{
				rd.next();
				return;
			}
			default:
				break;
			}
			rd.error("invalid json document!");
		}

		inline void skip_object(reader& rd)
		{
			auto tok = &rd.peek();
			while (tok->str.str[0] != '}')
			{
				skip_key(rd);
				tok = &rd.peek();
				if (tok->str.str[0] == ':')
				{
					rd.next();
					skip(rd);
					//rd.next();
					tok = &rd.peek();
				}
				else
				{
					rd.error("invalid json document!");
				}
				if (tok->str.str[0] == ',')
				{
					rd.next();
					tok = &rd.peek();
					continue;
				}
			}
			rd.next();
		}
		
#define MIN_NUMBER_VALUE 1e-8
		inline void read_json(reader& rd, bool& val){
			auto& tok = rd.peek();
			switch (tok.type)
			{
			case token::t_string:
			{
				char const* ptr = tok.str.str;
				if (tok.str.len == 4)
				{
					val = (ptr[0] == 't' || ptr[0] == 'T') &&
						(ptr[1] == 'r' || ptr[1] == 'R') &&
						(ptr[2] == 'u' || ptr[2] == 'U') &&
						(ptr[3] == 'e' || ptr[3] == 'E');
				}
				else
				{
					val = false;
				}
				break;
			}
			case token::t_int:
			{
				val = (tok.value.i64 != 0);
				break;
			}
			case token::t_uint:
			{
				val = (tok.value.u64 != 0);
				break;
			}
			case token::t_number:
			{
				val = fabs(tok.value.d64) > MIN_NUMBER_VALUE;
				break;
			}
			default:
			{
				rd.error("not a valid bool.");
			}
			}

			rd.next();
		}

		template <typename T, size_t N>
		inline void read_json(reader& rd, std::array<T, N>& val)
		{
			read_array(rd, val);
		}

		template<typename T>
		inline void read_array(reader& rd, T& val)
		{
			if (rd.expect('[') == false) {
				rd.error("array must start with [.");
			}
			rd.next();
			auto tok = &rd.peek();
			int index = 0;
			while (tok->str.str[0] != ']') {
				read_json(rd, val[index++]);
				tok = &rd.peek();
				if (tok->str.str[0] == ',') {
					rd.next();
					tok = &rd.peek();
					continue;
				}
				else if (tok->str.str[0] == ']') {
					break;
				}
				else {
					rd.error("no valid array!");
				}
			}
			rd.next();
		}

		inline void read_json(reader& rd, std::string& val) {
			auto& tok = rd.peek();
			if (tok.type == token::t_string) {
				val.assign(tok.str.str, tok.str.len);
			}
			else {
				rd.error("not a valid string.");
			}
			rd.next();
		}

		//read json to value
		template<typename T>
		inline std::enable_if_t<wheel::traits::is_signed_intergral_like<T>::value> read_json(reader& rd, T& val) {
			auto& tok = rd.peek();
			switch (tok.type) {
			case token::t_string: {
				int64_t temp = std::strtoll(tok.str.str, nullptr, 10);
				val = static_cast<T>(temp);
				break;
			}
			case token::t_int: {
				val = static_cast<T>(tok.value.i64);
				if (tok.neg)
					val = -val;
				break;
			}
			case token::t_uint: {
				val = static_cast<T>(tok.value.u64);
				break;
			}
			case token::t_number: {
				val = static_cast<T>(tok.value.d64);
				if (tok.neg)
					val = -val;
				break;
			}
			default: {
				rd.error("not a valid signed integral like number.");
			}
			}
			rd.next();
		}

		template<typename T>
		inline std::enable_if_t<wheel::traits::is_unsigned_intergral_like<T>::value> read_json(reader& rd, T& val) {
			auto& tok = rd.peek();
			switch (tok.type) {
			case token::t_string: {
				uint64_t temp = std::strtoull(tok.str.str, nullptr, 10);
				val = static_cast<T>(temp);
				break;
			}
			case token::t_int: {
				if (tok.value.i64 < 0) {
					rd.error("assign a negative signed integral to unsigned integral number.");
				}
				val = static_cast<T>(tok.value.i64);
				break;
			}
			case token::t_uint: {
				val = static_cast<T>(tok.value.u64);
				break;
			}
			case token::t_number: {
				if (tok.value.d64 < 0) {
					rd.error("assign a negative float point to unsigned integral number.");
				}
				val = static_cast<T>(tok.value.d64);
				break;
			}
			default: {
				rd.error("not a valid unsigned integral like number.");
			}
			}
			rd.next();
		}


		template<typename T>
		inline std::enable_if_t<std::is_floating_point<T>::value> read_json(reader& rd, T& val)
		{
			auto& tok = rd.peek();
			switch (tok.type)
			{
			case token::t_string:
			{
				double temp = std::strtold(tok.str.str, nullptr);
				val = static_cast<T>(temp);
				break;
			}
			case token::t_int:
			{
				val = static_cast<T>(tok.value.i64);
				if (tok.neg)
					val = -val;
				break;
			}
			case token::t_uint:
			{
				val = static_cast<T>(tok.value.u64);
				break;
			}
			case token::t_number:
			{
				val = static_cast<T>(tok.value.d64);
				if (tok.neg)
					val = -val;
				break;
			}
			default:
			{
				rd.error("not a valid float point number.");
			}
			}
			rd.next();
		}

		template<typename T>
		std::enable_if_t<wheel::traits::is_emplace_back_able<T>::value> emplace_back(T& val) {
			val.emplace_back();
		}

		template<typename T>
		std::enable_if_t<wheel::traits::is_template_instant_of<std::queue, T>::value> emplace_back(T& val) {
			val.emplace();
		}

		template<typename T>
		inline std::enable_if_t<wheel::traits::is_sequence_container<T>::value> read_json(reader& rd, T& val) {
			if (rd.expect('[') == false) {
				rd.error("array must start with [.");
			}
			rd.next();
			auto tok = &rd.peek();
			while (tok->str.str[0] != ']') {
				emplace_back(val);
				read_json(rd, val.back());
				tok = &rd.peek();
				if (tok->str.str[0] == ',') {
					rd.next();
					tok = &rd.peek();
					continue;
				}
				else if (tok->str.str[0] == ']') {
					break;
				}
				else {
					rd.error("no valid array!");
				}
			}
			rd.next();
		}

		template<typename T>
		inline std::enable_if_t<wheel::traits::is_associat_container<T>::value> read_json(reader& rd, T& val) {
			if (rd.expect('{') == false)
			{
				rd.error("object must start with {!");
			}
			rd.next();
			auto tok = &rd.peek();
			while (tok->str.str[0] != '}')
			{
				typename T::key_type key;
				read_json(rd, key);
				if (rd.expect(':') == false)
				{
					rd.error("invalid object!");
				}
				rd.next();
				typename T::mapped_type value;
				read_json(rd, value);
				val[key] = value;
				tok = &rd.peek();
				if (tok->str.str[0] == ',')
				{
					rd.next();
					tok = &rd.peek();
					continue;
				}
				else if (tok->str.str[0] == '}')
				{
					break;
				}
				else
				{
					rd.error("no valid object!");
				}
			}
			rd.next();
		}

		template <typename T, size_t N>
		inline void read_json(reader& rd, T(&val)[N]){
			read_array(rd, val);
		}

		template<typename T>
		inline constexpr std::enable_if_t<wheel::reflector::is_reflection_v<T>> do_read_aux(reader& rd, T&& t)
		{
			wheel::reflector::for_each_tuple_front(std::forward<T>(t), [&t, &rd](const auto& v, auto i)
				{
					using M = decltype(reflector_reflect_members(std::forward<T>(t)));
					constexpr auto Idx = decltype(i)::value;
					constexpr auto Count = M::size();
					static_assert(Idx < Count,"Idx > Count ");

					using type_v = decltype(std::declval<T>().*std::declval<decltype(v)>());
					if(!wheel::reflector::is_reflection<type_v>::value)
					{
						rd.next();
						if (rd.peek().str != wheel::reflector::get_name<T, Idx>().data()) {
							return;
						}

						rd.next();
						rd.next();
						read_json(rd, t.*v);
					}
					else
					{
						rd.next();
						rd.next();
						rd.next();
						do_read(rd, t);
						rd.next();
					}
				});
		}

		template<typename T, typename = std::enable_if_t<wheel::reflector::is_reflection<T>::value>>
		inline void read_json(reader& rd, T& val) {
			do_read_aux(rd, val);
			rd.next();
		}

		template<typename T, typename = std::enable_if_t<wheel::reflector::is_reflection<T>::value>>
		constexpr void do_read(reader& rd, T&& t)
		{
			using M = decltype(reflector_reflect_members(std::forward<T>(t)));

			auto tp = M::apply_impl();
			constexpr auto Size = M::size();
			size_t index = 0;
			while (rd.peek().type != token::t_end && index <= Size) {
				rd.next();
				auto& tk = rd.peek();

				std::string s(tk.str.str, tk.str.len);
				index = wheel::reflector::get_index<T>(s);
				if (index == Size) {
					if (tk.type == token::t_end)
						break;

					rd.next();
					rd.next();
					rd.next();
					continue;
				}

				using type_v = decltype(std::declval<T>().*std::declval<decltype(std::get<0>(tp))>());

				wheel::unit::tuple_switch(index, tp, [&t, &rd](auto& v) {
					using type_v = decltype(std::declval<T>().*std::declval<decltype(v)>());
					if(!wheel::reflector::is_reflection_v<type_v>){
						rd.next();
						rd.next();
						read_json(rd, t.*v);
					}else {
						rd.next();
						rd.next();
						do_read(rd, t);
						rd.next();
					}

					}, std::make_index_sequence<Size>{});

				index++;
			}
		}

		template<typename InputIt, typename T, typename F>
		T join(InputIt first, InputIt last, const T& delim, const F& f) {
			if (first == last)
				return T();

			T t = f(*first++);
			while (first != last) {
				t += delim;
				t += f(*first++);
			}
			return t;
		}

		template<typename Stream, typename InputIt, typename T, typename F>
		void join(Stream& ss, InputIt first, InputIt last, const T& delim, const F& f) {
			if (first == last)
				return;

			f(*first++);
			while (first != last) {
				ss.put(delim);
				f(*first++);
			}
		}

		template<typename Stream>
		void render_json_value(Stream& ss, std::nullptr_t) { ss.write("null"); }

		template<typename Stream>
		void render_json_value(Stream& ss, bool b) { ss.write(b ? "true" : "false"); };

		template<typename Stream, typename T>
		std::enable_if_t<!std::is_floating_point<T>::value && (std::is_integral<T>::value || std::is_unsigned<T>::value || std::is_signed<T>::value)>
			render_json_value(Stream& ss, T value){
			char temp[20] = {0};
			auto p = itoa_fwd(value, temp);
			ss.write(temp, p - temp);
		}

		template<typename Stream>
		void render_json_value(Stream& ss, int64_t value){
			char temp[65] = {0};
			auto p = xtoa(value, temp, 10, 1);
			ss.write(temp, p - temp);
		}

		template<typename Stream>
		void render_json_value(Stream& ss, uint64_t value){
			char temp[65] = {0};
			auto p = xtoa(value, temp, 10, 0);
			ss.write(temp, p - temp);
		}

		template<typename Stream, typename T>
		std::enable_if_t<std::is_floating_point<T>::value> render_json_value(Stream& ss, T value){
			char temp[20] = {0};
			sprintf(temp, "%f", value);
			int len = strlen(temp);
			ss.write(temp,len);
		}

		template<typename Stream>
		void render_json_value(Stream& ss, const std::string& s){
			ss.write(s.c_str(), s.size());
		}

		template<typename Stream>
		void render_json_value(Stream& ss, const char* s, size_t size){
			ss.write(s, size);
		}

		template<typename Stream, typename T>
		std::enable_if_t<std::is_arithmetic<T>::value> render_key(Stream& ss, T t) {
			ss.put('"');
			render_json_value(ss, t);
			ss.put('"');
		}

		template<typename Stream>
		void render_key(Stream& ss, const std::string& s) {
			render_json_value(ss, s);
		}

		template<typename Stream, typename T>
		auto render_json_value(Stream& ss, T&& t) -> std::enable_if_t<wheel::reflector::is_reflection<T>::value>{
			to_json(ss, std::forward<T>(t));
		}

		template<typename Stream, typename T>
		std::enable_if_t <std::is_enum<T>::value> render_json_value(Stream& ss, T val){
			render_json_value(ss, (std::underlying_type_t<T>&)val);
		}

		template <typename Stream, typename T, size_t N>
		void render_json_value(Stream& ss, const T(&v)[N]){
			render_array(ss, v);
		}

		template<typename Stream, typename T>
		void render_array(Stream& ss, const T& v){
			ss.put('[');
			join(ss, std::begin(v), std::end(v), ',',
				[&ss](const auto& jsv) {
					render_json_value(ss, jsv);
				});
			ss.put(']');
		}

		template <typename Stream, typename T, size_t N>
		void render_json_value(Stream& ss, const std::array<T, N>& v){
			render_array(ss, v);
		}

		template<typename Stream, typename T>
		std::enable_if_t <wheel::traits::is_associat_container<T>::value>
			render_json_value(Stream& ss, const T& o) {
			ss.put('{');
			join(ss, o.cbegin(), o.cend(), ',',
				[&ss](const auto& jsv) {
					render_key(ss, jsv.first);
					ss.put(':');
					render_json_value(ss, jsv.second);
				});
			ss.put('}');
		}

		template<typename Stream, typename T>
		std::enable_if_t <wheel::traits::is_sequence_container<T>::value> render_json_value(Stream& ss, const T& v) {
			ss.put('[');
			join(ss, v.cbegin(), v.cend(), ',',
				[&ss](const auto& jsv) {
					render_json_value(ss, jsv);
				});
			ss.put(']');
		}

		//c++17产量表达式不能拥有lamda
		//constexpr auto write_json_key = [](auto& s, auto i, auto& t) {
		 auto write_json_key = [](auto& s, auto i, auto& t) {
			s.put('"');
			const auto name = wheel::reflector::get_name<decltype(t), decltype(i)::value>(); //will be replaced by string_view later
			s.write(name.data(), name.length());
			s.put('"');
		};


		template<typename ty>
		inline void from_json(ty&& val, const char* buff, size_t len = -1){
			reader rd(buff, len);

			try
			{
				do_read(rd, val);
			}
			catch (const std::exception & ex) {
				std::cout << ex.what() << std::endl;
			}
			catch (...) {
				std::cout << "exception......" << std::endl;
			}
		}


		template<typename Stream, typename T>
		auto to_json(Stream& s, T&& v)->std::enable_if_t<wheel::traits::is_sequence_container<std::decay_t<T>>::value>
		{
			try
			{
				using U = typename std::decay_t<T>::value_type;
				s.put('[');
				const size_t size = v.size();
				for (size_t i = 0; i < size; i++)
				{
					if (wheel::reflector::is_reflection_v<U>) {
						to_json(s, v[i]);
					}
					else {
						render_json_value(s, v[i]);
					}

					if (i != size - 1)
						s.put(',');
				}
				s.put(']');
			}catch (const std::exception &ex){
				std::cout << ex.what() << std::endl;	
				s.put(' ');
			}catch (...) {
				std::cout <<"expection ...."<< std::endl;
				s.put(' ');
			}
		}

		template<typename T>
		inline std::enable_if_t<std::is_enum<T>::value> read_json(reader& rd, T& val) {
			typedef typename std::underlying_type<T>::type RAW_TYPE;
			read_json(rd, (RAW_TYPE&)val);
		}
		
		//linux下严禁constexptr抛异常
		template<typename Stream, typename T>
		auto to_json(Stream& s, T&& t)->std::enable_if_t<wheel::traits::is_tuple<std::decay_t<T>>::value> {
			try
			{
				using U = typename std::decay_t<T>;
				s.put('[');
				const size_t size = std::tuple_size<U>::value;
				wheel::reflector::for_each_tuple_front(std::forward<T>(t), [&s, size](auto& v, auto i) {
					render_json_value(s, v);

					if (i != size - 1)
						s.put(',');
					});
				s.put(']');
			}
			catch (const std::exception&ex){
				std::cout << ex.what() << std::endl;
				s.put(' ');
			}catch (...) {
				std::cout <<"exception..."<< std::endl;
				s.put(' ');
			}
		}

		template<typename Stream, typename T>
		auto to_json(Stream& s, T&& t) -> std::enable_if_t<wheel::reflector::is_reflection<T>::value>{
			try
			{
				s.put('{');
				wheel::reflector::for_each_tuple_front(std::forward<T>(t), [&t, &s](const auto& v, auto i)
					{
						using M = decltype(reflector_reflect_members(std::forward<T>(t)));
						constexpr auto Idx = decltype(i)::value;
						constexpr auto Count = M::size();
						static_assert(Idx < Count, "Idx >= Count");

						write_json_key(s, i, t);
						s.put(':');

						if (!wheel::reflector::is_reflection<decltype(v)>::value) {
							render_json_value(s, t.*v);
						}

						if (Idx < Count - 1)
							s.put(',');
					});
				s.put('}');
			}
			catch (const std::exception &ex)
			{
				std::cout << ex.what() << std::endl;
				s.put(' ');
			}catch (...){
				std::cout <<"exception...."<< std::endl;
				s.put(' ');
			}
		}
	}


}//wheel

#endif // json_h__
