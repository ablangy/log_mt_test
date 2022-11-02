// SPDX-License-Identifier: Unlicense

#include <cstdlib>
#include <iostream>

#include <signal.h>
#include <string.h>

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
