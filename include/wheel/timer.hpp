#ifndef timer_h__
#define timer_h__
#include <memory>
#include <thread>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include "traits.hpp"

namespace wheel {
	namespace unit {
		typedef std::function<void()> Trigger_type;

		class timer {
		public:
			timer(Trigger_type trigger)
			:trigger_(trigger){
				ios_ = wheel::traits::make_unique<boost::asio::io_service>();
				timer_ = wheel::traits::make_unique<boost::asio::steady_timer>(*ios_);
				thread_ = wheel::traits::make_unique<std::thread>([this] {ios_->run();});
			}
			
			~timer() {
				if (thread_ != nullptr){
					if (thread_->joinable()) {
						thread_->join(); //等待子线成退出
					}
				}

				timer_ = nullptr;
				thread_ = nullptr;
			}

			//定时器一个动作去执行
			void start_timer(int seconds) {
				timer_->expires_from_now(std::chrono::seconds(seconds));
				timer_->async_wait([this](const boost::system::error_code& ec) {
					if (ec){
						return;
					}

					trigger_(); //执行操作函数
					});
			}

			//取消定时器
			void cancel() {
				boost::system::error_code ec;
				timer_->cancel(ec);
			}

			//循环定时器
			void loop_timer(int seconds) {
				timer_->expires_from_now(std::chrono::seconds(seconds));
				timer_->async_wait([this,seconds](const boost::system::error_code& ec) {
					if (ec) {
						return;
					}

					trigger_(); //执行操作函数
					loop_timer(seconds);
					});
			}

		private:
			std::unique_ptr<boost::asio::io_service>ios_ = nullptr;
			std::unique_ptr<std::thread>thread_ = nullptr;
			std::unique_ptr<boost::asio::steady_timer>timer_;
			Trigger_type trigger_;
		};
	}
}
#endif // timer_h__
