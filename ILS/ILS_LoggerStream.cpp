#include "ILS_LoggerStream.h"
#include <vector>


TLoggerStream::TLoggerStream(const ILogger* pLogger, TFuncPtr pFunc) : m_pLogger(pLogger), m_pFunc(pFunc) {
}

TLoggerStream::TLoggerStream(const ILogger* pLogger, TFuncPtr pFunc, const char* sect) : m_pLogger(pLogger), m_pFunc(pFunc), m_sSectId(sect) {
}

TLoggerStream::TLoggerStream(const ILogger* pLogger, TFuncPtr pFunc, const char* sect, unsigned int ind) : m_pLogger(pLogger), m_pFunc(pFunc), m_sSectId(sect + std::to_string(ind)) {
}

void TLoggerStream::LogString(const std::string& msg, va_list args) const {
	unsigned int max_msg_size = 1024;
	std::unique_ptr<char[]> str = std::make_unique<char[]>(max_msg_size);
	
	size_t pos = msg.find("%t");
	if (pos != std::string::npos) {
		std::string result(msg, 0, max_msg_size);

		while (pos != std::string::npos) {
			result[pos + 1] = 'f';
			pos = result.find("%t");
		}
		vsnprintf(str.get(), max_msg_size, result.c_str(), args);
	}
	else {
		vsnprintf(str.get(), max_msg_size, msg.c_str(), args);
	}
	out << str;
}

const TLoggerStream& TLoggerStream::operator()(const LogId& id, const char* msg, ...) const {
	try {
		va_list marker;
		va_start(marker, msg);
		LogString(msg, marker);
		va_end(marker);
	}
	catch (...) {}
	return *this;
}

const TLoggerStream& TLoggerStream::SectBegin(const char* msg, ...) const {
	try {
		out << "SectionBegin " << m_sSectId << " ";
		va_list marker;
		va_start(marker, msg);
		LogString(msg, marker);
		va_end(marker);
	}
	catch (...) {}
	return *this;
}

void TLoggerStream::SectCheck(const char* sect) const {
	if (m_sSectId != sect && m_pLogger) {
		m_pLogger->errOut("Ожидается окончание секции " + m_sSectId + " вместо указанной " + sect, id);
	}
}

void TLoggerStream::SectCheck(const char* sect, unsigned int ind) const {
	if (m_sSectId != (sect + std::to_string(ind)) && m_pLogger) {
		m_pLogger->errOut("Ожидается окончание секции " + m_sSectId + " вместо указанной " + (sect + std::to_string(ind)), id);
	}
}

const TLoggerStream& TLoggerStream::SectEnd(const char* msg, ...) const {
	try {
		out << "SectionEnd " << m_sSectId << " ";
		va_list marker;
		va_start(marker, msg);
		LogString(msg, marker);
		va_end(marker);
	}
	catch (...) {}
	m_sSectId = "";
	return *this;
}

void TLoggerStream::Flush() const {
	(m_pLogger->*m_pFunc)(out.str(), id);
	out.str("");
}

const char* TLoggerStream::SectId() const {
	return m_sSectId.c_str();
}

TLoggerStream::~TLoggerStream() {
	if (m_sSectId != "") {
		// Если m_sSectId!="" знаачит она не была начата, но не закончена, заканчиваем насильно
		out << "SectionEnd " << m_sSectId << " ";
	}
	else {
		(m_pLogger->*m_pFunc)(out.str(), id);
	}
}
