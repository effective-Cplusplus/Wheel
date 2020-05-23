#ifndef unit_h__
#define unit_h__

#include <string>
#include <unordered_set>
#include <random>
#include <boost/asio.hpp>
#include <memory>
#include <regex>
#include <iomanip>
#include <sstream>
#include <tuple>
#include <fstream>
#include "traits.hpp"
#include "md5.hpp"

#if _MSC_VER
#pragma warning(disable:4984)
#pragma warning(disable:4996)
#else
#pragma GCC system_header
#endif

namespace wheel {
	namespace unit {

#define PATTERN_IPV4   "^(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3}$"
		/* IPv6 pattern */
#define PATTERN_IPV6   "^((([0-9A-Fa-f]{1,4}:){7}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){1,7}:)|(([0-9A-Fa-f]{1,4}:)"  \
                                       "{6}:[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){5}(:[0-9A-Fa-f]{1,4}){1,2})|(([0-9A-Fa-f]{1,4}:)"  \
                                       "{4}(:[0-9A-Fa-f]{1,4}){1,3})|(([0-9A-Fa-f]{1,4}:){3}(:[0-9A-Fa-f]{1,4}){1,4})|(([0-9A-Fa-f]"  \
                                       "{1,4}:){2}(:[0-9A-Fa-f]{1,4}){1,5})|([0-9A-Fa-f]{1,4}:(:[0-9A-Fa-f]{1,4}){1,6})|(:(:[0-9A-Fa-f]"  \
                                       "{1,4}){1,7})|(([0-9A-Fa-f]{1,4}:){6}(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])(\\.(\\d|[1-9]\\d|1\\d{2}" \
                                       "|2[0-4]\\d|25[0-5])){3})|(([0-9A-Fa-f]{1,4}:){5}:(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])" \
                                       "(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3})|(([0-9A-Fa-f]{1,4}:){4}(:[0-9A-Fa-f]{1,4})" \
                                       "{0,1}:(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3})|" \
                                       "(([0-9A-Fa-f]{1,4}:){3}(:[0-9A-Fa-f]{1,4}){0,2}:(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])" \
                                       "(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3})|(([0-9A-Fa-f]{1,4}:){2}(:[0-9A-Fa-f]{1,4})" \
                                       "{0,3}:(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3})|" \
                                       "([0-9A-Fa-f]{1,4}:(:[0-9A-Fa-f]{1,4}){0,4}:(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])" \
                                       "(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3})|(:(:[0-9A-Fa-f]{1,4})" \
                                       "{0,5}:(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3}))$"

		static std::uint32_t float_to_uint32(float f) {
			return *(std::uint32_t*)(&f);
		}

		static float uint32_to_float(std::uint32_t value) {
			return  *(float*)(&value);
		}

		static float string_to_float(std::string str) {
			float res;
			std::stringstream stream(str);
			stream >> res;
			return res;
		}

		//判断奇偶性
		static bool is_odd(int value) {
			return (value & 1) == 0 ? true : false;
		}

		//判断是否带有符号位
		static bool checkout_singned(std::uint32_t value) {
			return ((value >> 31) & 0x01) == 1 ? true : false;
		}

		//计算文件的MD5值
		static std::string get_md5_from_file(const std::string& file_path) {
			//默认2M
			constexpr int md5_read_size = 1024 * 1024;
			std::string md5_str;
			try {
				//默认读到结尾
				std::ifstream file(file_path, std::ios::binary | std::ios::ate);
				if (file.is_open()) {
					//获取文件大小
					std::size_t file_size = file.tellg();
					//指针偏移最好跟上偏移的字节长度
					file.seekg(0, std::ios::beg);
					if (file_size < 2 * md5_read_size) {//小文件全部都计算
						md5_str = cmd5::md5file(file);
					}
					else {//大文件计算前后，各1M
						std::string data_buffer;

						data_buffer.resize(2 * md5_read_size);
						file.read(&data_buffer[0], md5_read_size);
						//从头开始偏移相应的字节
						file.seekg(file_size - md5_read_size, std::ios::beg);
						file.read(&data_buffer[0] + md5_read_size, md5_read_size);

						if (file.good()) {
							md5_str = cmd5::md5(data_buffer);
						}
					}

					file.close();
				}
			}
			catch (...) {
			}

			return std::move(md5_str);
		};

		static void split(std::vector<std::string>& result, const std::string& str, std::string demli) {
			char* ptr = strtok((char*)str.c_str(), demli.c_str());
			while (ptr != nullptr) {
				if (strcmp(ptr, str.c_str()) == -1) {
					result.push_back(std::move(ptr));
				}

				ptr = strtok(nullptr, demli.c_str());
			}
		}

		//uint8转换
		constexpr std::uint8_t little_swap8(std::uint8_t value) {
			return ((value & 0xf0) >> 4) | ((value & 0x0f) << 4);
		}
		//短整型转大小端
		constexpr std::uint16_t little_swap16(std::uint16_t value) {
			return ((value & 0xff00) >> 8) | ((value & 0x00ff) << 8);
		}

		//长整型转大小端
		constexpr std::uint32_t little_swap32(std::uint32_t value) {
			return ((value & 0xff000000) >> 24) |
				((value & 0x00ff0000) >> 8) |
				((value & 0x0000ff00) << 8) |
				((value & 0x000000ff) << 24);
		}

		//长长整型转大小端
		constexpr std::uint64_t little_swap64(std::uint64_t value) {
			return((value & 0xff00000000000000) >> 56) |
				((value & 0x00ff000000000000) >> 40) |
				((value & 0x0000ff0000000000) >> 24) |
				((value & 0x000000ff00000000) >> 8) |
				((value & 0x00000000ff000000) << 8) |
				((value & 0x0000000000ff0000) << 24) |
				((value & 0x000000000000ff00) << 40) |
				((value & 0x00000000000000ff) << 56);
		}

		//float的大端转换
		static std::uint32_t htonf(float f) {
			uint32_t p;
			uint32_t sign = 0;
			if (f < 0) {// Get sign.
				sign = 1;
				f = -f;
			}
			p = ((((uint32_t)f) & 0x7fff) << 16) | (sign << 31);// Get integer part.
			p |= (uint32_t)(((f - (int)f) * 65536.0f)) & 0xffff; // Get fraction part.
			return p;
		}

		//float小端转换
		static float ntohf(uint32_t p) {
			float f = static_cast<float>(((p >> 16) & 0x7fff));
			f += (p & 0xffff) / 65536.0f;
			if (((p >> 31) & 0x1) == 0x1) {
				f = -f;
			}
			return f;
		}

		//ipv4转int 类型
		static int inet4_pton(const char* cp, std::size_t& ap) {
			std::size_t acc = 0;
			std::size_t  dots = 0;
			std::size_t  addr = 0;

			do {
				char cc = *cp;
				if (cc >= '0' && cc <= '9') {
					acc = acc * 10 + (cc - '0');
				}
				else if (cc == '.' || cc == '\0') {
					if (++dots > 3 && cc == '.') {
						return 0;
					}
					/* Fall through */

					if (acc > 255) {
						return 0;
					}

					//addr += (acc << (index * 8));
					//从左往右，低位放
					addr = addr << 8 | acc; // 这句是精华,每次将当前值左移八位加上后面的值
					acc = 0;
				}
			} while (*cp++);

			// Normalize the address 
			if (dots < 3) {
				addr <<= 8 * (3 - dots);
			}

			ap = addr;
			return 1;
		}

		//ipv4转int
		static void inet4_ntop(std::size_t value, std::string& str) {

			constexpr int inet_addrlen = 20;
			str.resize(inet_addrlen);

			//Intel 机器是高位存高位，低位存低位，因此数组越大越是低位
			unsigned char* temp_addrptr = (unsigned char*)(&value);
			snprintf(&str[0], str.size(), "%d.%d.%d.%d",
				*(temp_addrptr + 3), *(temp_addrptr + 2), *(temp_addrptr + 1), *(temp_addrptr + 0));
		}

		static std::uint32_t crc16(const char* data, unsigned short data_len_bit) {
			constexpr std::uint32_t crc16_poly = 0x1021; //   CRC_16校验方式的多项式.   
			std::uint32_t crc = 0;

			// initialize crc little endian
			*((char*)&crc) = *(data + 1);
			*((char*)&crc + 1) = *data;

			for (int i = 0; i < data_len_bit; i++) {
				unsigned char temp = 0;
				if (i < data_len_bit - 16) {
					temp = *(data + 2 + (i >> 3)); // calculate the position of bit in which byte 
				}

				unsigned short bit_shift = 7 - (i % 8); // calculate bit shift of bit in the specified byte

				if (crc >> 15)
				{
					crc = (crc << 1) | (temp >> bit_shift & 0x01); // shift crc one bit and fi ll in a data bit
					crc ^= crc16_poly; // xor with poly equations.
				}
				else {
					crc = (crc << 1) | (temp >> bit_shift & 0x01);
				}
			}

			return crc;
		}

		static int inet6_pton(const char* src, std::uint8_t* dst) {
			if (src == nullptr) {
				return 0;
			}

			constexpr  char  xdigits_l[] = "0123456789abcdef";
			constexpr  char  xdigits_u[] = "0123456789ABCDEF";
			const      char* xdigits = nullptr;
			const      char* curtok = nullptr;
			constexpr  int  NS_IN6ADDRSZ = 16;
			constexpr   int NS_INT16SZ = 2;
			std::uint8_t tmp[NS_IN6ADDRSZ] = { 0 };
			std::uint8_t* tp = tmp;
			std::uint8_t* endp = nullptr;
			std::uint8_t* colonp = nullptr;
			endp = tp + NS_IN6ADDRSZ;

			/* Leading :: requires some special handling. */
			if (*src == ':') {
				if (*++src != ':') {
					return 0;
				}
			}

			int              seen_xdigits = 0;
			std::size_t    val = 0;
			char  ch = 0;
			while ((ch = *src++) != '\0') {
				const char* pch = nullptr;

				if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL) {
					pch = strchr((xdigits = xdigits_u), ch);
				}

				if (pch != NULL) {
					val <<= 4;
					val |= (pch - xdigits);
					if (++seen_xdigits > 4) {
						return 0;
					}

					continue;
				}

				if (ch == ':') {
					curtok = src;
					if (!seen_xdigits) {
						if (colonp != nullptr) {
							return 0;
						}

						colonp = tp;
						continue;
					}
					else if (*src == 0) {
						return 0;
					}

					if (tp + NS_INT16SZ > endp) {
						return 0;
					}

					*tp++ = (u_char)(val >> 8) & 0xff;	//放在高位上
					*tp++ = (u_char)val & 0xff; //放在低位上
					seen_xdigits = 0;
					val = 0;
					continue;
				}

				if (ch == '.' && ((tp + 4) <= endp)) {
					std::size_t value = 0;
					if (inet4_pton(curtok, value)) {
						unsigned char* buf = (unsigned char*)&value;
						memcpy(tp, buf, 4);
						tp += 4;
						seen_xdigits = 0;
						break;  /*%< '\\' was seen by inet_pton4(). */
					}
				}

				return 0;
			}

			if (seen_xdigits) {
				if (tp + NS_INT16SZ > endp) {
					return 0;
				}

				*tp++ = (u_char)(val >> 8) & 0xff;
				*tp++ = (u_char)val & 0xff;
			}

			if (colonp != NULL) {
				if (tp == endp) {
					return 0;
				}

				const std::size_t n = tp - colonp;
				for (int i = 1; i <= n; i++) {
					endp[-i] = colonp[n - i];
					colonp[n - i] = 0;
				}

				tp = endp;
			}

			if (tp != endp) {
				return 0;
			}

			memcpy(dst, tmp, NS_IN6ADDRSZ);

			return 1;
		}

		static void inet6_ntop(const u_char* src, std::string& dst) {
			constexpr  int  NS_IN6ADDRSZ = 16;
			constexpr   int NS_INT16SZ = 2;
			char tmp[100] = { 0 };
			struct { int base, len; } best, cur;
			std::size_t words[NS_IN6ADDRSZ / NS_INT16SZ] = { 0 };

			memset(words, '\0', sizeof words);
			for (int i = 0; i < NS_IN6ADDRSZ; i += 2) {
				words[i / 2] = (src[i] << 8) | src[i + 1];
			}

			best.base = -1;
			cur.base = -1;
			best.len = 0;
			cur.len = 0;
			for (int i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
				if (words[i] == 0) {
					if (cur.base == -1) {
						cur.base = i, cur.len = 1;
					}
					else {
						cur.len++;
					}
				}
				else {
					if (cur.base != -1) {
						if (best.base == -1 || cur.len > best.len) {
							best = cur;
						}

						cur.base = -1;
					}
				}
			}
			if (cur.base != -1) {
				if (best.base == -1 || cur.len > best.len) {
					best = cur;
				}
			}
			if (best.base != -1 && best.len < 2) {
				best.base = -1;
			}

			/*
			 * Format the result.
			 */
			char* tp = tmp;
			for (int i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
				/* Are we inside the best run of 0x00's? */
				if (best.base != -1 && i >= best.base &&
					i < (best.base + best.len)) {
					if (i == best.base) {
						*tp++ = ':';
					}

					continue;
				}
				/* Are we following an initial run of 0x00s or any real hex? */
				if (i != 0) {
					*tp++ = ':';
				}

				/* Is this address an encapsulated IPv4? */
				if (i == 6 && best.base == 0 &&
					(best.len == 6 || (best.len == 5 && words[5] == 0xffff))) {
					std::string temp;
					temp.resize(20);
					memcpy(&temp[0], (char*)src + 12, 4);

					std::size_t value = 0;
					std::size_t* ptr = (std::size_t*)temp.data();
					value = *ptr;

					inet4_ntop(value, temp);
					std::size_t len = strlen(temp.c_str());
					memcpy(tp, temp.c_str(), len);
					tp += len;
					break;
				}

				std::stringstream sstream;
				sstream << std::hex << words[i];
				std::size_t len = strlen(sstream.str().c_str());
				memcpy(tp, sstream.str().c_str(), len);
				tp += len;
			}
			/* Was it a trailing run of 0x00's? */
			if (best.base != -1 && (best.base + best.len) == (NS_IN6ADDRSZ / NS_INT16SZ)) {
				*tp++ = ':';
			}

			*tp++ = '\0';

			std::size_t len = strlen(tmp);
			dst.resize(len);
			memcpy(&dst[0], tmp, len);
		}


		static std::string to_string_with_precision(const float a_value, int precison)
		{
			std::ostringstream out;
			out << std::fixed << std::setprecision(precison) << a_value;
			return out.str();
		}

		static std::string to_string_with_precision(const double a_value, int precison)
		{
			std::ostringstream out;
			out << std::fixed << std::setprecision(precison) << a_value;
			return out.str();

		}

		static void bubble_sort_big(int array[], int n)
		{
			if (array == nullptr || n <= 1) {
				return;
			}

			bool flag = false;
			for (int i = 1; i < n; i++) {
				flag = false;
				for (int j = 0; j < n - i; j++) {
					if (array[j] < array[j + 1]) {
						flag = true;
						array[j] ^= array[j + 1];
						array[j + 1] ^= array[j];
						array[j] ^= array[j + 1];
					}
				}

				if (!flag) {
					break;
				}
			}
		}

		//从小到大，插入排序
		static void insert_sort(int* arr, int n) {
			int temp = -1;
			for (int i = 1; i < n; ++i) {
				temp = arr[i];

				int j = i - 1;
				//从后往前搬动数据
				for (; j >= 0; --j) {
					if (arr[j] <= temp) {
						break;
					}

					arr[j + 1] = arr[j];
				}

				//当前的后一个位置，放入数据
				arr[j + 1] = temp;
			}
		}

		//选择排序
		static void selection_sort(int* ptr, int len)
		{
			if (ptr == NULL || len <= 1) {
				return;
			}

			int minindex = -1;
			//i是次数，也即排好的个数;j是继续排
			for (int i = 0; i < len - 1; ++i) {
				minindex = i;
				for (int j = i + 1; j < len; ++j) {
					//从小到大
					if (ptr[j] < ptr[minindex]) {
						minindex = j;
					}
				}

				//这里一定要加上,比如(5,8,5,2,9,2,1,10)
				if (i == minindex) {
					continue;
				}

				ptr[i] ^= ptr[minindex];
				ptr[minindex] ^= ptr[i];
				ptr[i] ^= ptr[minindex];

			}
		}

		//二分法查找适用于数据量较大时，但是数据需要先排好顺序
		inline int binary_search(int ptr[], int len, int key) {
			int low = 0;
			int high = len - 1;
			while (low <= high) {
				int mid = ((low + high) >> 1);
				if (ptr[mid] == key) {
					return mid - 1;
				}
				else if (ptr[mid] < key) {
					low = mid + 1;
				}
				else {
					high = mid - 1;
				}
			}

			return -1;
		}

		static void trim(std::string& s)
		{
			if (s.empty()) {
				return;
			}

			s.erase(0, s.find_first_not_of(" "));
			s.erase(s.find_last_not_of(" ") + 1);
		}

		static bool iequal(const char* s, size_t l, const char* t) {
			std::size_t size = strlen(t);
			if (size != l) {
				return false;
			}

			for (size_t i = 0; i < l; i++) {
				if (std::tolower(s[i]) != std::tolower(t[i]))
					return false;
			}

			return true;
		}

		static std::string to_hex_string(std::size_t value) {
			std::ostringstream stream;
			stream << std::hex << value;
			return stream.str();
		}

		static int32_t stringHex_to_int(const char* hex_str)
		{
			int32_t value = -1;
			if (strlen(hex_str) ==0 || hex_str == nullptr){
				return value;
			}

			std::istringstream istr(hex_str);
			istr >> std::hex >> value;
			return value;
		}

		static int32_t stringDec_to_int(const char* str)
		{
			int32_t value = -1;
			if (strlen(str) == 0 || str == nullptr) {
				return value;
			}

			std::istringstream istr(str);
			istr >> value;

			return value;
		}

		static std::string find_substr(const std::string& str, const std::string key, const std::string& diml) {
			std::string value;
			std::size_t begin = str.find(key, 0);
			if (begin != std::string::npos) {
				++begin;
				std::size_t falg_pos = str.find(diml, begin);
				if (falg_pos != std::string::npos) {
					++falg_pos;
					std::size_t end = str.find("\r\n", falg_pos);
					if (end != std::string::npos) {
						value = str.substr(falg_pos, end - falg_pos);
					}
				}
			}

			trim(value);

			return value;
		}

		//c++14使用传入lambda表达式
		template <typename F, typename ...Args>
		void for_each_args(F&& func, Args...args) {
			int arr[] = { (std::forward<F>(func)(args),0)... };
		}

		//单个tuple去索引
		template <typename Tuple, typename F, std::size_t...Is>
		void tuple_switch(const std::size_t i, Tuple&& t, F&& f, wheel::traits::index_sequence<Is...>) {
			[](...) {}(
				(i == Is && (
				(void)std::forward<F>(f)(std::get<Is>(std::forward<Tuple>(t))), false))...
				);
		}

		template <typename Tuple, typename F>
		void tuple_switch(const std::size_t i, Tuple&& t, F&& f) {
			static constexpr auto N =
				std::tuple_size < wheel::traits::remove_reference_t<Tuple >>::value;

			tuple_switch(i, std::forward<Tuple>(t), std::forward<F>(f),
				wheel::traits::make_index_sequence<N>{});
		}

		/**********使用例子********/

		//auto const t = std::make_tuple(42, 'z', 3.14, 13, 0, "Hello, World!");

		//for (std::size_t i = 0; i < std::tuple_size<decltype(t)>::value; ++i) {
		//	wheel::unit::tuple_switch(i, t, [](const auto& v) {
		//		std::cout << v << std::endl;
		//		});


		template<typename F, typename...Ts, std::size_t...Is>
		void for_each_tuple_front(std::tuple<Ts...>&& tuple, F&& func, wheel::traits::index_sequence<Is...>) {
			constexpr auto SIZE = std::tuple_size<wheel::traits::remove_reference_t<decltype(tuple)>>::value;
#if (_MSC_VER >= 1700 && _MSC_VER <= 1900) //vs2012-vs2015
			if (constexpr(SIZE > 0)) {
				using expander = int[];
				(void)expander {
					((void)std::forward<F>(func)(std::get<Is>(tuple), std::integral_constant<size_t, Is>{}), false)...
				};
			}
#else
			if constexpr (SIZE > 0) {
				using expander = int[];
				(void)expander {
					((void)std::forward<F>(func)(std::get<Is>(tuple), std::integral_constant<size_t, Is>{}), false)...
				};
			}
#endif // _MSC_VER <=1923


		}

		template<typename F, typename...Ts>
		void for_each_tuple_front(std::tuple<Ts...>&& tuple, F&& func) {
			for_each_tuple_front(std::forward<std::tuple<Ts...>>(tuple), func, wheel::traits::make_index_sequence<sizeof...(Ts)>());
		}

		template<typename F, typename...Ts, std::size_t...Is>
		void for_each_tuple_back(std::tuple<Ts...>&& tuple, F&& func, wheel::traits::index_sequence<Is...>) {
			//匿名构造函数调用
			constexpr auto SIZE = std::tuple_size<wheel::traits::remove_reference_t<decltype(tuple)>>::value;
#if (_MSC_VER >= 1700 && _MSC_VER <= 1900) //vs2012-vs2015
			if (constexpr (SIZE > 0)) {
				[](...) {}(0,
					((void)std::forward<F>(func)(std::get<Is>(tuple), std::integral_constant<size_t, Is>{}), false)...
					);
			}
#else
			if constexpr (SIZE > 0) {
				[](...) {}(0,
					((void)std::forward<F>(func)(std::get<Is>(tuple), std::integral_constant<size_t, Is>{}), false)...
					);
			}
#endif // #ifdef _MSC_VER <=1900


		}

		template<typename F, typename...Ts>
		void for_each_tuple_back(std::tuple<Ts...>&& tuple, F&& func) {
			for_each_tuple_back(std::forward<std::tuple<Ts...>>(tuple), func, wheel::traits::make_index_sequence<sizeof...(Ts)>());
		}

		//单个参数传单个参数，没有index(tuple不能空)
		template <typename... Args, typename F, std::size_t... Idx>
		constexpr void for_each0(std::tuple<Args...>&& t, F&& f, wheel::traits::index_sequence<Idx...>) {
			constexpr auto N = std::tuple_size <wheel::traits::remove_reference_t<decltype(t)>>::value;

			//编译器编译时，会做判断
			if constexpr (N > 0) {
				using expander = int[];
				(void)expander {
					((std::forward<F>(f)(std::get<Idx>(t))), false)...
				};
			}

		}


		template <typename... Args, typename F, std::size_t... Idx>
		constexpr void for_each_l(std::tuple<Args...>&& t, F&& f, wheel::traits::index_sequence<Idx...>) {

			constexpr auto size = sizeof...(Idx);
			if constexpr (size > 0) {
				using expander = int[];
				(void)expander {
					((std::forward<F>(f)(std::get<Idx>(t))), false)...
				};
			}
		}

		template <typename... Args, typename F, std::size_t... Idx>
		constexpr void for_each_r(std::tuple<Args...>&& t, F&& f, wheel::traits::index_sequence<Idx...>) {
			constexpr auto size = sizeof...(Idx);
			if constexpr (size > 0) {
				using expander = int[];
				(void)expander {
					((std::forward<F>(f)(std::get<size - Idx - 1>(t))), false)...
				};
			}

		}


		/***************使用列子*****************/
		//auto some = std::make_tuple("I am good", 255, 2.1);
		//for_each_tuple(some, [](const auto& x, auto index) {
		//	constexpr auto Idx = decltype(index)::value;
		//	std::cout << x << std::endl;
		//	}
		//);

		static 	int domain_name_query(const std::string& domain_name, std::vector< std::string >& ips, bool is_http = true)
		{
			boost::asio::io_service io_service;

			boost::asio::ip::tcp::resolver resolver(io_service);

			boost::system::error_code ec;
			boost::asio::ip::tcp::resolver::iterator endpoint_iterator;
			if (is_http) {
				boost::asio::ip::tcp::resolver::query q(boost::asio::ip::tcp::v4(), domain_name);
				endpoint_iterator = resolver.resolve(q, ec);
			}
			else {
				boost::asio::ip::tcp::resolver::query q(boost::asio::ip::tcp::v4(), domain_name, "https");
				endpoint_iterator = resolver.resolve(q, ec);
			}


			if (ec.value() == 0)
			{
				boost::asio::ip::tcp::resolver::iterator end;
				for (; endpoint_iterator != end; ++endpoint_iterator)
				{
					ips.push_back((*endpoint_iterator).endpoint().address().to_string());
				}
			}

			return ec.value();
		}

		static 	bool ip_v4_check(const std::string& ip_addr_dot_format)
		{

			std::regex expression(PATTERN_IPV4);
			return (std::regex_match(ip_addr_dot_format, expression));

			//std::regex expression("((?:(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d)\\.){3}(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d))");
			//return (std::regex_match(ip_addr_dot_format, expression));

			//std::regex pattern(("((([01]?\\d\\d?)|(2[0-4]\\d)|(25[0-5]))\\.){3}(([01]?\\d\\d?)|(2[0-4]\\d)|(25[0-5]))"));
			//return std::regex_match(ip_addr_dot_format, pattern);
		}

		static 	bool ip_v6_check(const std::string& ip_addr_dot_format)
		{
			std::regex expression(PATTERN_IPV6);
			return (std::regex_match(ip_addr_dot_format, expression));
		}

		template<typename T>
		inline void append_impl(std::string& sql, const T& str) {
			if constexpr (std::is_same<std::string, T>::value) {
				if (str.empty())
					return;
			}
			else {
				if (sizeof(str) == 0) {
					return;
				}
			}

			sql += str;
			sql += " ";
		}

		template<typename... Args>
		inline void append(std::string& sql, Args&&... args) {
			using expander = int[];
			(void)expander {
				((append_impl(sql, std::forward<Args>(args))), false)...
			};
		}

		static void random_str(std::string& random_str, int len) {
			std::unordered_set<char> set;
			for (int i = 0; i < len; i++) {
				static std::default_random_engine e((std::size_t)time(0));
				static std::uniform_int_distribution<unsigned int> u(1, 3);
				uint32_t ret = u(e);
				char c = 0;
				std::unordered_set<char>::const_iterator iter_find;
				switch (ret)
				{
				case 1:
					if (!set.empty()) {
						while (1) {
							c = 'A' + rand() % 26;
							iter_find = set.find(c);
							if (iter_find != set.end()) {
								break;
							}

							continue;
						}

					}
					else {
						c = 'A' + rand() % 26;
					}

					random_str.push_back(c);
					break;
				case 2:
					if (!set.empty()) {
						while (1) {
							c = 'a' + rand() % 26;
							iter_find = set.find(c);
							if (iter_find != set.end()) {
								break;
							}

							continue;
						}
					}
					else {
						c = 'a' + rand() % 26;
					}

					random_str.push_back(c);
					break;
				case 3:
					if (!set.empty()) {
						while (1) {
							c = 0 + rand() % 10;
							iter_find = set.find(c);
							if (iter_find != set.end()) {
								break;
							}

							continue;
						}
					}
					else {
						c = 0 + rand() % 10;
					}

					random_str.push_back(c);
					break;
				default:
					break;
				}
			}


		}
	}
}
#endif // unit_h__
