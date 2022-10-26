// SPDX-License-Identifier: Unlicense

#ifndef TAPPTHREAD_H
#define TAPPTHREAD_H

#include <thread>
#include <mutex>
#include <condition_variable>

namespace tapp {

class TappThread {
public:
	TappThread(int schedPolicy, int schedPriority, cpu_set_t cpuAffinity);

	virtual ~TappThread() = default;

	virtual void run() = 0;

	void start();

	virtual void stop();

protected:
	bool m_isRunning;
	std::mutex m_threadLock;
	std::condition_variable m_threadCondVariable;
	std::thread m_pthread;

private:
	int m_threadPolicy;
	int m_threadPriority;
	cpu_set_t m_threadCpuAffinity;
};

}
#endif // TAPPTHREAD_H
