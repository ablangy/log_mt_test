// SPDX-License-Identifier: Unlicense

#include <unistd.h>
#include <getopt.h>
#include <vector>
#include <filesystem>

#include <iostream>

#include "Arguments.h"

tapp::Arguments::Arguments()
    : m_progname {}
    , m_logFilenamePrefix {}
    , m_logDirectory {}
    , m_syslogIdentity {}
    , m_flushPolicy { }
    , m_maxArchiveLogCount { }
    , m_maxLogSize { }
{
}

tapp::Arguments& tapp::Arguments::instance()
{
	static Arguments s_instance;
	return s_instance;
}

void tapp::Arguments::usage()
{
	std::cout << m_progname
	          << "-f prefix -d directory -s identity -p flushPolicy -m max_archives -l max_size[-h|--help]\n"
	             "\t -f|--logFilenamePrefix prefix\t préfixe du nom de fichier de log\n"
	             "\t -d|--logDirectory directory\t chemin complet du fichier de log\n"
	             "\t -p|--flushPolicy flushPolicy\t\t périodicité du flush des logs. 0: jamais (système auto flush), 1..N; toutes les N écritures dans le "
	             "fichier de log\n"
	             "\t -m|--maxArchiveLogCount max_archives\t Nombre maximum de rotation de fichiers de log. Après avoir avoir atteint 'maxArchiveLogCount', le "
	             "plus ancien fichier de log est détruit\n"
	             "\t -l|--maxLogSize max_size\t\t taille maximum d'un fichier de log en octets. Après avoir atteint 'maxLogSize' le fichier de log est "
	             "compressé et un nouveau fichier de log est créé et utilisé.\n"
	             "\n";
	return;
}

tapp::Arguments::eSetReturnCode tapp::Arguments::set(int argc, char* const* argv)
{
	if (argc < 1) {
		std::cerr << "prog name is missing\n";
		return eSetReturnCode::FAILURE;
	}

	m_progname = { argv[0] };

	int args_idx;
	int rc;
	const std::vector<option> args {
		{ "help", 0, nullptr, 'h' },
		{ "logFilenamePrefix", 1, nullptr, 'f' },
		{ "logDirectory", 1, nullptr, 'd' },
		{ "flushPolicy", 1, nullptr, 'p' },
		{ "maxArchiveLogCount", 1, nullptr, 'm' },
		{ "maxLogSize", 1, nullptr, 'l' },
		{ nullptr, 0, nullptr, 0 },
	};

	while ((rc = getopt_long(argc, argv, "hf:d:p:m:l:", args.data(), &args_idx)) != -1) {
		switch (rc) {
		case 'f':
			m_logFilenamePrefix = optarg;
			break;
		case 'd':
			m_logDirectory = optarg;
			break;
		case 'p':
			try {
				m_flushPolicy = std::stoi(optarg);
			} catch (const std::exception& ex) {
				std::cerr << "flushPolicy parameter (" << optarg << ") is badly formated : " << ex.what() << "\n\n";
			}
			break;
		case 'm':
			try {
				m_maxArchiveLogCount = std::stoi(optarg);

			} catch (const std::exception& ex) {
				std::cerr << "maxArchiveLogRotate parameter (" << optarg << ") is badly formated : " << ex.what() << "\n\n";
			}
			break;
		case 'l':
			try {
				m_maxLogSize = std::stoi(optarg);

			} catch (const std::exception& ex) {
				std::cerr << "maxLogSize parameter (" << optarg << ") is badly formated : " << ex.what() << "\n\n";
			}
			break;
		case '?':
		case 'h':
			return eSetReturnCode::FAILURE;
			break;
		default:
			std::cerr << "Argument indéfini";
			return eSetReturnCode::FAILURE;
			break;
		}
	}

	if (not check()) {
		return eSetReturnCode::FAILURE;
	}

	return eSetReturnCode::CONTINUE;
}

bool tapp::Arguments::check() const
{

	if (m_logFilenamePrefix.empty() || m_logDirectory.empty()) {
		std::cerr << "\nAt least one log parameter is missing\n\n";
		return false;
	}

	std::filesystem::path logDirectory { m_logDirectory };
	if (not std::filesystem::is_directory(logDirectory)) {
		std::cerr << "\nThe log directory: " << m_logDirectory << " doesn't exist\n\n";
		return false;
	}

	return true;
}
