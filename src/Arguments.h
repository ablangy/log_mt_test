// SPDX-License-Identifier: Unlicense

#ifndef TEST_APP_ARGUMENTS_H
#define TEST_APP_ARGUMENTS_H

#include <string>
#include <optional>

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

	const std::string& getProgname() const { return m_progname; }
	const std::string& getLogFilenamePrefix() const { return m_logFilenamePrefix; }
	const std::string& getLogDirectory() const { return m_logDirectory; }
	int64_t getFlushPolicy() const { return m_flushPolicy; }
	int64_t getMaxArchiveLogCount() const { return m_maxArchiveLogCount; }
	int64_t getMaxLogSize() const { return m_maxLogSize; }
	std::optional<uint32_t> getThreadCount() const { return m_threadCount; }

private:
	Arguments();
	bool check() const;

	std::string m_progname;
	std::string m_logFilenamePrefix;
	std::string m_logDirectory;
	int64_t m_flushPolicy;
	int64_t m_maxArchiveLogCount;
	int64_t m_maxLogSize;
	std::optional<uint32_t> m_threadCount;
};
}

#endif /* TEST_APP_ARGUMENTS_H */
