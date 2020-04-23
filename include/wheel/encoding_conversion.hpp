#ifndef encoding_conversion_h__
#define encoding_conversion_h__

#include <codecvt>
#include <locale>
#include <utility>
#include <string>
#include "utf8_gbk_mem.hpp"

namespace wheel {
	namespace char_encoding {
		class encoding_conversion {
		public:
			encoding_conversion() = delete;
			encoding_conversion(const encoding_conversion&) = delete;
			encoding_conversion(encoding_conversion&&) = delete;
			~encoding_conversion() = delete;
			encoding_conversion& operator=(const encoding_conversion&) = delete;
			encoding_conversion& operator=(encoding_conversion&&) = delete;
			static std::string   to_string(const std::wstring& wstr)
			{
				setlocale(LC_ALL, "");
				//算出代转string字节
				std::int64_t size = wcstombs(NULL, wstr.c_str(), 0);
				std::string desrt;
				desrt.resize(size);
				wcstombs(&desrt[0], wstr.c_str(), size);
				return std::move(desrt);
			}

			static std::wstring   to_wstring(const std::string& str)
			{
				setlocale(LC_ALL, "");
				std::int64_t size = mbstowcs(NULL, str.c_str(), 0);
				std::wstring w_str;
				w_str.resize(size);

				//算出代转wstring字节
				mbstowcs(&w_str[0], str.c_str(), str.size());
				return std::move(w_str);
			}

			static std::string    gbk_to_utf8(const std::string& str)
			{
				constexpr size_t offset = 8;//8倍
				std::int64_t size = (str.size()* offset);
				std::string out_str;
				out_str.resize(size);
				size = gbk_to_utf8_((char*)&str[0], &out_str[0]);

				out_str.resize(size);
				return std::move(out_str);
			}

			static std::string  utf8_to_gbk(const std::string& str)
			{
				constexpr size_t offset = 8;//8倍
				std::int64_t size = (str.size() * offset);
				std::string out_str;
				out_str.resize(size);
				size = utf8_to_gbk_((char*)&str[0], &out_str[0]);

				out_str.resize(size);
				return std::move(out_str);
			}

			static std::u16string utf8_to_utf16(const std::string& str)
			{
#if defined(_MSC_VER)
				std::wstring_convert<std::codecvt_utf8_utf16<uint16_t>, uint16_t> convert;
				auto tmp = convert.from_bytes(str.data(), str.data() + str.size());
				return std::u16string(tmp.data(), tmp.data() + tmp.size());
#else
				std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
				return convert.from_bytes(str.data(), str.data() + str.size());
#endif
			}

			static std::u32string utf8_to_utf32(const std::string& str)
			{
#if defined(_MSC_VER)
				std::wstring_convert<std::codecvt_utf8<uint32_t>, uint32_t> convert;
				auto tmp = convert.from_bytes(str.data(), str.data() + str.size());
				return std::u32string(tmp.data(), tmp.data() + tmp.size());
#else
				std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
				return convert.from_bytes(str.data(), str.data() + str.size());
#endif
			}

			static std::string    utf16_to_utf8(const std::u16string& str)
			{
#if defined(_MSC_VER)
				std::wstring_convert<std::codecvt_utf8_utf16<uint16_t>, uint16_t> convert;
				return convert.to_bytes((uint16_t*)str.data(), (uint16_t*)str.data() + str.size());
#else
				std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
				return convert.to_bytes(str.data(), str.data() + str.size());
#endif
			}

			static std::u32string utf16_to_utf32(const std::u16string& str)
			{
				std::string bytes;
				bytes.reserve(str.size() * 2);

				for (const char16_t ch : str) {
					bytes.push_back((uint8_t)(ch>>8));
					bytes.push_back((uint8_t)(ch &0xFF));
				}

#if defined(_MSC_VER)
				std::wstring_convert<std::codecvt_utf16<uint32_t>, uint32_t> convert;
				auto tmp = convert.from_bytes(bytes);
				return std::move(std::u32string(tmp.data(), tmp.data() + tmp.size()));
#else
				std::wstring_convert<std::codecvt_utf16<char32_t>, char32_t> convert;
				return convert.from_bytes(bytes);
#endif
			}

			static std::string    utf32_to_utf8(const std::u32string& str)
			{
#if defined(_MSC_VER)
				std::wstring_convert<std::codecvt_utf8<uint32_t>, uint32_t> convert;
				return convert.to_bytes((uint32_t*)str.data(), (uint32_t*)str.data() + str.size());
#else
				std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
				return convert.to_bytes(str.data(), str.data() + str.size());
#endif
			}

			static std::u16string utf32_to_utf16(const std::u32string& str)
			{
#if defined(_MSC_VER)
				std::wstring_convert<std::codecvt_utf16<uint32_t>, uint32_t> convert;
				std::string bytes = convert.to_bytes((uint32_t*)str.data(), (uint32_t*)str.data() + str.size());
#else
				std::wstring_convert<std::codecvt_utf16<char32_t>, char32_t> convert;
				std::string bytes = convert.to_bytes(str.data(), str.data() + str.size());
#endif

				std::u16string result;
				result.reserve(bytes.size()>>1);

				for (size_t i = 0; i < bytes.size(); i += 2) {
					result.push_back((char16_t)((uint8_t)(bytes[i]) * 256 + (uint8_t)(bytes[i + 1])));
				}

				return std::move(result);
			}

			static bool is_valid_utf8(const char* string)
			{
				if (string == nullptr) {
					return false;
				}

				const unsigned char* bytes = (const unsigned char*)string;
				unsigned int cp = 0;
				int num = 0;

				while (*bytes != 0x00) {
					if ((*bytes & 0x80) == 0x00) {
						// U+0000 to U+007F 
						cp = (*bytes & 0x7F);
						num = 1;
					}
					else if ((*bytes & 0xE0) == 0xC0) {
						// U+0080 to U+07FF 
						cp = (*bytes & 0x1F);
						num = 2;
					}
					else if ((*bytes & 0xF0) == 0xE0) {
						// U+0800 to U+FFFF 
						cp = (*bytes & 0x0F);
						num = 3;
					}
					else if ((*bytes & 0xF8) == 0xF0) {
						// U+10000 to U+10FFFF 
						cp = (*bytes & 0x07);
						num = 4;
					}
					else {
						return false;
					}

					bytes += 1;
					for (int i = 1; i < num; ++i) {
						if ((*bytes & 0xC0) != 0x80)
							return false;
						cp = (cp << 6) | (*bytes & 0x3F);
						bytes += 1;
					}

					if ((cp > 0x10FFFF) ||
						((cp >= 0xD800) && (cp <= 0xDFFF)) ||
						((cp <= 0x007F) && (num != 1)) ||
						((cp >= 0x0080) && (cp <= 0x07FF) && (num != 2)) ||
						((cp >= 0x0800) && (cp <= 0xFFFF) && (num != 3)) ||
						((cp >= 0x10000) && (cp <= 0x1FFFFF) && (num != 4))) {
						return false;
					}

				}

				return true;
			}

			static bool is_valid_gbk(const char* str)
			{
				unsigned int nBytes = 0;//GBK可用1-2个字节编码,中文两个 ,英文一个
				unsigned char chr = *str;
				bool bAllAscii = true; //如果全部都是ASCII,
				for (unsigned int i = 0; str[i] != '\0'; ++i) {
					chr = *(str + i);
					if ((chr & 0x80) != 0 && nBytes == 0) {// 判断是否ASCII编码,如果不是,说明有可能是GBK
						bAllAscii = false;
					}
					if (nBytes == 0) {
						if (chr >= 0x80) {
							if (chr >= 0x81 && chr <= 0xFE) {
								nBytes = +2;
							}
							else {
								return false;
							}
							nBytes--;
						}
					}
					else {
						if (chr < 0x40 || chr>0xFE) {
							return false;
						}
						nBytes--;
					}//else end
				}
				if (nBytes != 0) {   //违返规则
					return false;
				}
				if (bAllAscii) { //如果全部都是ASCII, 也是GBK
					return true;
				}
				return true;
			}
		};
	}
}

#endif // encoding_conversion_h__
