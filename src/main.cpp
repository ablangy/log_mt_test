// SPDX-License-Identifier: Unlicense

#include <cstdlib>
#include <iostream>

#include <memory>
#include <signal.h>
#include <string.h>

#include "TappThread.h"
#include "g3log/loglevels.hpp"
#include "g3log/logmessage.hpp"
#include "g3sinks/LogRotate.h"
#include "g3log/crashhandler.hpp"
#include "g3log/logworker.hpp"
#include "g3log/g3log.hpp"

#include "Arguments.h"

std::map<int, std::string_view> g_termSigs = {
	{ SIGTERM, "SIGTERM" },
	{ SIGQUIT, "SIGQUIT" },
};
sigset_t g_termSigSet;

static bool prepareTerminationSignal()
{
	struct sigaction termAction;

	memset(&termAction, 0, sizeof(termAction));
	termAction.sa_handler = SIG_IGN;
	sigemptyset(&g_termSigSet);

	for (auto sig : g_termSigs) {
		if (sigaction(sig.first, &termAction, nullptr) < 0) {
			std::cerr << "sigaction(" << sig.second << ") has failed: " << errno << std::endl;
			return false;
		}

		if (sigaddset(&g_termSigSet, sig.first) < 0) {
			std::cerr << "sigaddset(" << sig.second << ") has failed: " << errno << std::endl;
			return false;
		}
	}

	if (pthread_sigmask(SIG_BLOCK, &g_termSigSet, nullptr) < 0) {
		std::cerr << "pthread_sigmask() has failed: " << errno << std::endl;
		return false;
	}

	return true;
}

int main(int argc, char** argv)
{
	auto& args = tapp::Arguments::instance();
	if (args.set(argc, argv) != tapp::Arguments::eSetReturnCode::CONTINUE) {
		args.usage();
		std::exit(EXIT_FAILURE);
	}

	// We have to configure termination signals handling before creating the threads in order for them to inherit our signal mask
	prepareTerminationSignal();

	auto logWorker = g3::LogWorker::createLogWorker();
	if (not logWorker) {
		std::cerr << "Échec d'initialisation du module de trace (pas de worker)\n";
		std::exit(EXIT_FAILURE);
	}

	auto logRotateSink = std::make_unique<LogRotate>(args.getLogFilenamePrefix(), args.getLogDirectory());
	logRotateSink->setMaxArchiveLogCount(args.getMaxArchiveLogCount());
	logRotateSink->setMaxLogSize(args.getMaxLogSize());
	logRotateSink->setFlushPolicy(args.getFlushPolicy());

	auto logHandle = logWorker->addSink(std::move(logRotateSink), &LogRotate::save);
	if (not logHandle) {
		std::cerr << "Log system initialisation has failed\n";
		std::exit(EXIT_FAILURE);
	}

	g3::initializeLogging(logWorker.get());

	// SIGTERM default behavior restored
	g3::overrideSetupSignals({
	    { SIGABRT, "SIGABRT" },
	    { SIGFPE, "SIGFPE" },
	    { SIGILL, "SIGILL" },
	    { SIGSEGV, "SIGSEGV" },
	});

	LOG(INFO) << "Hello from test app !";

	std::vector<std::unique_ptr<tapp::TappThread>> thrVect;
	cpu_set_t cpu;

	CPU_ZERO(&cpu);
	CPU_SET(1, &cpu);
	CPU_SET(2, &cpu);
	CPU_SET(3, &cpu);
	CPU_SET(4, &cpu);
	CPU_SET(5, &cpu);
	CPU_SET(6, &cpu);

	if (tapp::Arguments::instance().getThreadCount()) {
		auto threadCount = tapp::Arguments::instance().getThreadCount().value();

		auto barrier = std::make_shared<pthread_barrier_t>();
		pthread_barrier_init(barrier.get(), NULL, threadCount + 1);

		for (uint32_t thrIdx = 0; thrIdx < threadCount; ++thrIdx) {
			thrVect.emplace_back(std::make_unique<tapp::TappThread>(SCHED_OTHER, 0, cpu, barrier, std::nullopt));
			thrVect.back()->start();
		}

		if (int bwRes = pthread_barrier_wait(barrier.get()); bwRes != 0 && bwRes != PTHREAD_BARRIER_SERIAL_THREAD) {
			std::cerr << "pthread_barrier_wait() has failed: " << strerror(bwRes);
		}
	}

	auto abortThread = std::make_unique<tapp::TappThread>(SCHED_OTHER, 0, cpu, nullptr, []() { abort(); });
	abortThread->start();

	auto signum = sigwaitinfo(&g_termSigSet, nullptr);
	if (signum < 0) {
		std::cerr << "Failed to wait for termination signal: " << errno;
		return EXIT_FAILURE;
	}

	auto found = g_termSigs.find(signum);
	if (found != g_termSigs.end()) {
		std::cout << found->second << " received, stop test app";
	} else {
		std::cout << "Signal: " << signum << " received, stop test app";
	}

	g3::internal::shutDownLogging();

	return EXIT_SUCCESS;
}
