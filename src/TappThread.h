// SPDX-License-Identifier: Unlicense

#ifndef TAPPTHREAD_H
#define TAPPTHREAD_H

#include <thread>
#include <mutex>
#include <condition_variable>

namespace tapp {

class TappThread {
public:
	TappThread(int schedPolicy, int schedPriority, cpu_set_t cpuAffinity, const std::shared_ptr<pthread_barrier_t>& barrier);

	void run();

	void start();
	void stop();

protected:
	bool m_isRunning;
	std::mutex m_threadLock;
	std::condition_variable m_threadCondVariable;
	std::thread m_pthread;

private:
	int m_threadPolicy;
	int m_threadPriority;
	cpu_set_t m_threadCpuAffinity;
	std::shared_ptr<pthread_barrier_t> m_barrier;
};

}
#endif // TAPPTHREAD_H
