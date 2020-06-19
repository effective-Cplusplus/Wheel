#ifndef io_service_poll_h__
#define io_service_poll_h__

#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <boost/asio.hpp>

/***********************在 event handler 中允许执行阻塞的操作(例如数据库查询操作)*****************************************/
namespace wheel {
	class io_service_poll {
	public:
		static io_service_poll& get_instance() {
			//c++11保证唯一性
			static io_service_poll instance;
			return instance;
		}

		~io_service_poll() = default;

		std::shared_ptr<boost::asio::io_service> get_main_io_service()const{
			auto ptr = io_services_[0];
			return std::move(ptr);
		}

		std::shared_ptr<boost::asio::io_service> get_io_service() {
			//while (lock_.test_and_set(std::memory_order_acquire));
			std::shared_ptr<boost::asio::io_service> ios;
			if (io_services_.size()>1){
				ios = io_services_[next_io_service_];
				++next_io_service_;
				if (next_io_service_ >= io_services_.size()) {
					next_io_service_ = 1;
				}
			}else {
				ios = io_services_[0];
			}

			//lock_.clear(std::memory_order_release);
			return std::move(ios);
		}

		void run(){
			std::vector<std::shared_ptr<std::thread> > threads;
			for (std::size_t i = 0; i < io_services_.size(); ++i) {
				threads.emplace_back(std::make_shared<std::thread>(
					[](io_service_ptr svr) {
						boost::system::error_code ec;
						svr->run(ec);
					}, io_services_[i]));
			}

			size_t count = threads.size();

			for (std::size_t i = 0; i < count; ++i) {
				threads[i]->join();
			}	
		}

		template <typename CompletionHandler>
		void post(BOOST_ASIO_MOVE_ARG(CompletionHandler) handler) {
			get_io_service()->post(handler);
		}

		template <typename CompletionHandler>
		void dispatch(BOOST_ASIO_MOVE_ARG(CompletionHandler) handler) {
			get_io_service()->dispatch(handler);
		}

		void stop() {
			works_.clear();

			size_t count = io_services_.size();
			for (size_t index = 0; index < count; ++index) {
				io_services_[index]->stop();
			}

			io_services_.clear();
		}

		io_service_poll(const io_service_poll&) = delete;
		io_service_poll& operator=(const io_service_poll&) = delete;
		io_service_poll(const io_service_poll&&) = delete;
		io_service_poll& operator=(const io_service_poll&&) = delete;
	private:
		io_service_poll() {
			try
			{
				size_t count= std::thread::hardware_concurrency();
				for (size_t index =0;index< count;++index){
					auto ios = std::make_shared<boost::asio::io_context>();
					auto work = traits::make_unique<boost::asio::io_context::work>(*ios);
					works_.emplace_back(std::move(work));
					io_services_.emplace_back(std::move(ios));
				}

			}catch (const std::exception & ex){
				std::cout << ex.what() << std::endl;
				exit(0);
			}
		}
	private:
		using io_service_ptr = std::shared_ptr<boost::asio::io_context>;
		using work_ptr = std::unique_ptr<boost::asio::io_context::work>;

		size_t next_io_service_ = 1;
		//std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
		std::vector<io_service_ptr>io_services_;
		std::vector<work_ptr>works_;
	};
}
#endif // io_service_poll_h__
