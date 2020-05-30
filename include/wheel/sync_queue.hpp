#ifndef sync_queue_h__
#define sync_queue_h__
#include<list>
#include<mutex>
#include<thread>
#include<condition_variable>

namespace wheel {
	namespace synchronization{
		template<typename T>
		class sync_queue {
		public:
			sync_queue(int max_size) :max_size_(max_size), need_stop_(false) {
			}

			~sync_queue() = default;

			void put(const T& x) {
				add(x);
			}

			void put(T&& x) {
				add(std::forward<T>(x));
			}

			void take(std::list<T>& list) {
				std::unique_lock<std::mutex> locker(mutex_);
				not_empty_.wait(locker, [this] {return need_stop_ || not_empty(); });

				if (need_stop_) {
					return;
				}

				list = std::move(queue_);
				not_full_.notify_one();
			}

			void take(T& t) {
				std::unique_lock<std::mutex> locker(mutex_);
				not_empty_.wait(locker, [this] {return need_stop_ || not_empty(); });

				if (need_stop_) {
					return;
				}

				t = queue_.front();
				queue_.pop_front();
				not_full_.notify_one();
			}

			void stop() {
				{
					std::lock_guard<std::mutex> locker(mutex_);
					need_stop_ = true;
				}
				not_full_.notify_all();
				not_empty_.notify_all();
			}

			bool empty() {
				std::lock_guard<std::mutex> locker(mutex_);
				return queue_.empty();
			}

			bool full() {
				std::lock_guard<std::mutex> locker(mutex_);
				return queue_.size() == max_size_;
			}

			size_t size() {
				std::lock_guard<std::mutex> locker(mutex_);
				return queue_.size();
			}

			int count() {
				return queue_.size();
			}
		private:
			bool not_full() const {
				bool full = queue_.size() >= max_size_;
				if (full) {
					std::cout << "full, waiting,thread id: " << this_thread::get_id() << std::endl;
				}

				return !full;
			}

			bool not_empty() const {
				bool empty = queue_.empty();
				if (empty) {
					std::cout << "empty,waiting,thread id: " << this_thread::get_id() << std::endl;
				}

				return !empty;
			}

			template<typename F>
			void add(F&& x) {
				std::unique_lock< std::mutex> locker(mutex_);
				not_full_.wait(locker, [this] {return need_stop_ || not_full(); });
				if (need_stop_) {
					return;
				}

				queue_.push_back(std::forward<F>(x));
				not_empty_.notify_one();
			}
		private:
			std::list<T> queue_; //缓冲区
			std::mutex mutex_; //互斥量和条件变量结合起来使用
			std::condition_variable not_empty_;//不为空的条件变量
			std::condition_variable not_full_; //没有满的条件变量
			int max_size_; //同步队列最大的size

			bool need_stop_; //停止的标志
		};
	}//synchronization
}//wheel

#endif // sync_queue_h__
