#ifndef ILS_LoggerStreamH
#define ILS_LoggerStreamH

#include <cstring>
#include <sstream>

#include "ILS_Logger.h"

//------------------------------------------------------------------------------
/// Тривиальный класс, для возможности потокового формирования сообщения в макросе ILS_LOG.
class TLoggerStream
{
private:
	typedef std::string Msg;
	typedef std::string LogId;
	typedef void (ILogger::*TFuncPtr)(const Msg& msg, const LogId& id) const;
	mutable std::ostringstream out;  // Поток для накопления вывода.
	mutable std::string m_sSectId;
	mutable LogId id;
	const ILogger* m_pLogger;
	TFuncPtr m_pFunc;

	void LogString(const std::string& msg, va_list args) const;
public:
	/// Конструктор.
	TLoggerStream(const ILogger* pLogger, TFuncPtr pFunc);
	TLoggerStream(const ILogger* pLogger, TFuncPtr pFunc, const char* sect);
	TLoggerStream(const ILogger* pLogger, TFuncPtr pFunc, const char* sect, unsigned int ind);
	const TLoggerStream& operator()(const LogId& id, const char* msg, ...) const; 
	const TLoggerStream& SectBegin(const char* msg, ...) const;
	void SectCheck(const char* sect) const;
	void SectCheck(const char* sect, unsigned int ind) const;
	const TLoggerStream& SectEnd(const char* msg, ...) const;
	const char* SectId() const;
	void Flush() const;
	/// Вывод в поток.
	template<class T> inline const TLoggerStream& operator<<(const T& t) const {out<<t;return *this;}
	~TLoggerStream();
};

#endif  // ILS_LoggerStreamH
