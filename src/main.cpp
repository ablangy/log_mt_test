// SPDX-License-Identifier: Unlicense

#include <iostream>

#include "g3log/loglevels.hpp"
#include "g3log/logmessage.hpp"
#include "g3sinks/LogRotate.h"
#include "g3log/crashhandler.hpp"
#include "g3log/logworker.hpp"
#include "g3log/g3log.hpp"

#include "Arguments.h"

int main(int argc, char ** argv)
{
	auto& args = tapp::Arguments::instance();
	if (args.set(argc, argv) != tapp::Arguments::eSetReturnCode::CONTINUE) {
		args.usage();
		std::exit(EXIT_FAILURE);
	}
	
	auto logWorker = g3::LogWorker::createLogWorker();
	if (not logWorker) {
		std::cerr << "Échec d'initialisation du module de trace (pas de worker)\n";
		std::exit(EXIT_FAILURE);
	}

	auto logRotateSink = std::make_unique<LogRotate> (args.getLogFilenamePrefix(), args.getLogDirectory());
	logRotateSink->setMaxArchiveLogCount(args.getMaxArchiveLogCount());
	logRotateSink->setMaxLogSize(args.getMaxLogSize());
	logRotateSink->setFlushPolicy(args.getFlushPolicy());

	auto logHandle = logWorker->addSink(std::move(logRotateSink), &LogRotate::save);
	if (not logHandle) {
		std::cerr << "Log system initialisation has failed\n";
		std::exit(EXIT_FAILURE);
	}

	g3::initializeLogging(logWorker.get());

	LOG(INFO) << "Hello from test app !";

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
