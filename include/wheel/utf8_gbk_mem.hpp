#ifndef utf8_gbk_mem_h__
#define utf8_gbk_mem_h__

#if _MSC_VER
#pragma warning(disable:4267)
#else
#pragma GCC system_header
#endif

#include "unicode.h"

namespace wheel {
	namespace char_encoding {
		static size_t utf8_to_unicode_(char* utf8char, int* unicode, int* len, int leaveLen) {
			char cTmp = *utf8char;
			if ((cTmp & 0xF8) == 0xF0) { /*4位*/
				if (leaveLen < 4 || (((*(utf8char + 1)) & 0xC0) != 0x80) || 
					(((*(utf8char + 2)) & 0xC0) != 0x80) || (((*(utf8char + 3)) & 0xC0) != 0x80)) {
					return 2;/*剩余位数不够 */
				}
				*len = 4;
				*unicode |= (cTmp & 0x07);
				*unicode <<= 6;

				cTmp = *(utf8char + 1);
				*unicode |= (cTmp & 0x3F);
				*unicode <<= 6;

				cTmp = *(utf8char + 2);
				*unicode |= (cTmp & 0x3F);
				*unicode <<= 6;

				cTmp = *(utf8char + 3);
				*unicode |= (cTmp & 0x3F);
			}
			else if ((cTmp & 0xF0) == 0xE0) { /* 3位 */
				if (leaveLen < 3 || (((*(utf8char + 1)) & 0xC0) != 0x80) || (((*(utf8char + 2)) & 0xC0) != 0x80)) {
					return 2;/*剩余位数不够 */
				}
				*len = 3;
				*unicode |= (cTmp & 0x0F);
				*unicode <<= 6;

				cTmp = *(utf8char + 1);
				*unicode |= (cTmp & 0x3F);
				*unicode <<= 6;

				cTmp = *(utf8char + 2);
				*unicode |= (cTmp & 0x3F);
			}
			else if ((cTmp & 0xE0) == 0xC0) { /* 2位 */
				if (leaveLen < 2 || (((*(utf8char + 1)) & 0xC0) != 0x80)) {
					return 2;/*剩余位数不够 */
				}
				*len = 2;
				*unicode |= (cTmp & 0x1F);
				*unicode <<= 6;

				cTmp = *(utf8char + 1);
				*unicode |= (cTmp & 0x3F);
			}
			else if ((cTmp & 0x80) == 0x00) { /* 1位 */
				*len = 1;
				*unicode = cTmp;
			}
			else {
				return 1; /*第一位不合法*/
			}
			return 0;
		}

		static size_t utf8_to_gbk_(char* str_utf8, char* str_gbk) {
			
			size_t str_utf8_len = strlen(str_utf8);
			int index_utf8 = 0;
			size_t index_gbk = 0;
			while (str_utf8_len > index_utf8) {
				int unicode = 0; 
				int lenTmp = 0;
				if (utf8_to_unicode_(str_utf8 + index_utf8, &unicode, &lenTmp, str_utf8_len - index_utf8) != 0) {
					index_utf8++;
					continue;
				}

				int gbk = uni2gbk[unicode];

				if (lenTmp == 3) {
					str_gbk[index_gbk++] = gbk >> 8;
					str_gbk[index_gbk++] = gbk & 0xFF;
				}
				else if (lenTmp == 1) {
					str_gbk[index_gbk++] = gbk & 0xFF;
				}
				else if (lenTmp == 2) {
					str_gbk[index_gbk++] = gbk >> 8;
					str_gbk[index_gbk++] = gbk & 0xFF;
				}
				else if (lenTmp == 4) {
					str_gbk[index_gbk++] = gbk >> 8;
					str_gbk[index_gbk++] = gbk & 0xFF;
				}
				index_utf8 += lenTmp;
			}

			str_gbk[index_gbk] = 0;

			return index_gbk;
		}

		static size_t gbk_to_utf8_(char* strgbk, char* strutf8) {

			size_t str_gbk_len = strlen(strgbk);
			size_t index_utf8 = 0;  
			size_t index_gbk = 0;
			size_t utf8 =0;
	
			while (str_gbk_len > index_gbk) {
				int index = (*(strgbk + (index_gbk++))) & 0xFF;
				if (index < 0x80) { /*1位*/
					utf8 = gbk2utf8[index];
					strutf8[index_utf8++] = utf8;
				}
				else {
					if ((str_gbk_len - index_gbk) < 1) {
						break;
					}
					index <<= 8;
					index |= ((*(strgbk + (index_gbk++))) & 0xFF);
					utf8 = gbk2utf8[index];
					if (utf8 < 0xD192) { /*2位*/
						strutf8[index_utf8++] = utf8 >> 8;
						strutf8[index_utf8++] = utf8 & 0xFF;
					}
					else {/*3位 0xD192*/
						strutf8[index_utf8++] = utf8 >> 16;
						strutf8[index_utf8++] = (utf8 >> 8) & 0xFF;
						strutf8[index_utf8++] = utf8 & 0xFF;
					}
				}
			}
			strutf8[index_utf8] = '\0';

			return index_utf8;
		}
	}
}
#endif // utf8_gbk_mem_h__
