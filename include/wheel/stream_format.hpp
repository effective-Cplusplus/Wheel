#ifndef stream_format_h__
#define stream_format_h__
#include <type_traits>
#include "native_stream.hpp"
#include <assert.h>
#include <memory>

namespace wheel {
	const std::size_t PACKET_HEADER_SIZE = 6;//包头2个字节wheel+cmd+body_len
	const std::size_t PACKET_SIZE_OFFSET = 2U;//偏移的长度
	const std::size_t PACKET_CMD_OFFSET = 4;

	template<typename T>
	struct identity { typedef T type; };
	class stream_format
	{
	public:
		stream_format() 
			:header_size_(PACKET_HEADER_SIZE)
			, packet_size_offset_(PACKET_SIZE_OFFSET)
			, packet_cmd_offset_(PACKET_CMD_OFFSET)
		{
		}

		virtual~stream_format() {

		}

		virtual void set_stram_data(std::shared_ptr< native_stream > data) {

		}

		void set_header_size(std::size_t header_size) {
			header_size_ = header_size;
		}

		void set_cmd_offset(std::size_t offset) {
			packet_cmd_offset_ = offset;
		}

		void set_packet_size_offset(std::size_t offset) {
			packet_size_offset_ = offset;
		}

		std::size_t get_header_size()const {
			return header_size_;
		}

		std::size_t get_cmd_offset()const {
			return packet_cmd_offset_;
		}

		std::size_t get_packet_size_offset()const {
			return packet_size_offset_;
		}

		virtual std::int16_t get_cmd(void) {
			return 0;
		}

		virtual std::int16_t get_data_area_size(void) {
			return 0;
		}

		void write_string(const std::string& value) {
			std::int32_t size = static_cast<std::int32_t>(value.size());
			write<std::int32_t>(size);
			//这样写直接错误
			//write(reinterpret_cast<const char*>(&value), sizeof(value));
			write(value.c_str(), size);
		}


		template<class T>
		void write(const T& value) {
			static_assert(std::is_integral<T>::value, "T must is_integral ");
			write(reinterpret_cast<const char*>(&value), sizeof(value));
		}

		virtual void end() {

		}

		virtual void reset_read_body() {

		}

		template<typename T>
		T read(){
			return read(identity<T>());
		}

		template<class T>
		 T read(identity<T>) {
			T value = 0;
			static_assert(std::is_integral<T>::value, "T must is_integral ");
			read(reinterpret_cast<char*>(&value), sizeof(value));
			return value;
		}

		virtual std::string read(identity<std::string >){
			return "";
		}

		virtual std::int16_t get_data_area_size(const char* buffer, int offset) {
			return 0;
		}

		virtual void read(std::string& value){

		}

		virtual void write_header(std::int16_t cmd) {

		}

		virtual const native_stream* get_native_stream()const{
			return nullptr;
		}

	protected:
		virtual bool read(char* buffer, std::size_t size) {
			return false;
		}

		virtual bool write(const char* buffer, std::size_t size){
			return false;
		}

	private:
		std::size_t header_size_;
		std::size_t packet_size_offset_;
		std::size_t packet_cmd_offset_;
	};
}

#endif // stream_format_h__
