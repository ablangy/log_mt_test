// SPDX-License-Identifier: Unlicense

#ifndef TEST_APP_ARGUMENTS_H
#define TEST_APP_ARGUMENTS_H

#include <string_view>

namespace tapp {

class Arguments {
public:
	enum class eSetReturnCode {
		FAILURE,
		STOP,
		CONTINUE,
	};

	static Arguments& instance();

	void usage();
	eSetReturnCode set(int argc, char* const* argv);

	const std::string_view& getProgname() const { return m_progname; }
	const std::string_view& getLogFilenamePrefix() const { return m_logFilenamePrefix; }
	const std::string_view& getLogDirectory() const { return m_logDirectory; }
	int64_t getFlushPolicy() const { return m_flushPolicy; }
	int64_t getMaxArchiveLogCount() const { return m_maxArchiveLogCount; }
	int64_t getMaxLogSize() const { return m_maxLogSize; }

private:
	Arguments();
	bool check() const;

	std::string_view m_progname;
	std::string_view m_logFilenamePrefix;
	std::string_view m_logDirectory;
	int64_t m_flushPolicy;
	int64_t m_maxArchiveLogCount;
	int64_t m_maxLogSize;
};
}

#endif /* TEST_APP_ARGUMENTS_H */
