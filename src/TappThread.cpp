// SPDX-License-Identifier: Unlicense

#include "TappThread.h"

#include <iostream>
#include <cerrno>
#include <cstring>
#include <mutex>

tapp::TappThread::TappThread(int schedPolicy, int schedPriority, cpu_set_t cpuAffinity, const std::shared_ptr<pthread_barrier_t>& barrier)
    : m_isRunning { false }
    , m_threadLock {}
    , m_pthread {}
    , m_threadPolicy { schedPolicy }
    , m_threadPriority { schedPriority }
    , m_threadCpuAffinity { cpuAffinity }
    , m_barrier { barrier }
{
}

void tapp::TappThread::start()
{
	std::unique_lock<std::mutex> lockThread(m_threadLock);

	m_pthread = std::thread([this]() { run(); });

	struct sched_param schedParameters;
	int policy;

	if (pthread_getschedparam(m_pthread.native_handle(), &policy, &schedParameters)) {
		std::cerr << "Unable to change thread scheduling policy\n";
	}
	schedParameters.sched_priority = m_threadPriority;
	if (pthread_setschedparam(m_pthread.native_handle(), m_threadPolicy, &schedParameters)) {
		std::cerr << "Unable to change thread scheduling policy\n";
	}

	if (pthread_setaffinity_np(m_pthread.native_handle(), sizeof(cpu_set_t), &m_threadCpuAffinity) != 0) {
		std::cerr << "Unable to change thread CPU affinity\n";
	}

	m_isRunning = true;
	lockThread.unlock();
	m_threadCondVariable.notify_one();
}

void tapp::TappThread::stop()
{
	{
		std::lock_guard<std::mutex> locking(m_threadLock);
		m_isRunning = false;
	}
	m_threadCondVariable.notify_one();

	if (not m_pthread.joinable()) {
		std::cerr << "Impossible de thread::join le thread :" << m_pthread.get_id();
		return;
	}
	m_pthread.join();
}

void tapp::TappThread::run()
{
	std::unique_lock guard { m_threadLock };

	if (int bwRes = pthread_barrier_wait(m_barrier.get()); bwRes != 0 && bwRes != PTHREAD_BARRIER_SERIAL_THREAD) {
		std::cerr << "CommandFairDispatcher n'a pas pu faire pthread_barrier_wait, code erreur : " << bwRes << std::endl;
	}

	m_threadCondVariable.wait(guard);
}
