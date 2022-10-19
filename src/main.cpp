
#include <iostream>

#include "g3log/loglevels.hpp"
#include "g3log/logmessage.hpp"
#include "g3sinks/LogRotate.h"
#include "g3log/crashhandler.hpp"
#include "g3log/logworker.hpp"
#include "g3log/g3log.hpp"

int main(int /*argc*/, char ** /*argv*/)
{
	
	auto logWorker = g3::LogWorker::createLogWorker();
	if (not logWorker) {
		std::cerr << "Échec d'initialisation du module de trace (pas de worker)\n";
		std::exit(EXIT_FAILURE);
	}

	// LogRotate logRotateSink{"tapp_", "/tmp"};
	auto logRotateSink = std::make_unique<LogRotate> ("tapp_", "/tmp");
	logRotateSink->setMaxArchiveLogCount(1);
	logRotateSink->setMaxLogSize(1024 * 1024 * 3);
	logRotateSink->setFlushPolicy(1);

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

	g3::internal::shutDownLogging();

	return 0;
}
