#include <stdio.h>
#include <time.h>
#include <string.h>
#include "ILS_StdLog.h"

//=============================================================================
// BaseLogger - стандартая реализация основных механизмов Logger-а. 
// Рекомендуется наследоваться именно от этого класса при определении 
// собственного механизма регистрации хода процесса.
//-----------------------------------------------------------------------------
// Конструктор
BaseLogger::BaseLogger() {
	// По умолчанию ничего не выводим в заголовке
	show_info = 0;
	// Время начала
	bStarted = false;
	// флаг вывода лога в консоль
	bLogToConsole = false;
};
//-----------------------------------------------------------------------------
// Функции интерфейса
// Регистрация сообщений разных типов, каждая функция имеет праметры:
// id  - идентификатор сообщений
// msg - тело сообщения в формате функции printf
// ... - набор данных для вывода в сообщении по принципу printf
// Функции:
// Регистрация информационного сообщения
void BaseLogger::infOut(const Msg& msg, const LogId& id) const {
	lOut(iTitle() + msg);
}
//-----------------------------------------------------------------------------
// Функции интерфейса
// Регистрация сообщений разных типов, каждая функция имеет праметры:
// id  - идентификатор сообщений
// msg - тело сообщения в формате функции printf
// ... - набор данных для вывода в сообщении по принципу printf
// Функции:
// Регистрация информационного сообщения
void BaseLogger::logOut(const Msg& msg, const LogId& id) const {
	lOut(lTitle()+msg);
}
// Регистрация предупреждения (warning) и не фатальной ошибки
void BaseLogger::wrnOut(const Msg& msg, const LogId& id) const {
	++warnings;
	wOut(wTitle()+msg);
}
// Регистрация фатальной ошибки, 
// после которой результаты процесса не определены
void BaseLogger::errOut(const Msg& msg, const LogId& id) const {
	++errors;
	eOut(eTitle()+msg);
}
//-----------------------------------------------------------------------------
// Вспомогательные функции
// Создается общая для всех типов сообщений строка с заголовком
// на основе значений настроек
std::string BaseLogger::title() const {
	std::string res = "";
	char s[128];
	time_t ltime;
	time( &ltime );
	struct tm *today;
	today = localtime( &ltime );
	if(show_info & 1) {
		strftime(s, 128, "%Y/%m/%d ", today );
		res += s;
	}
	if((show_info & 2) || ((show_info & 4) && (!bStarted))) {
		strftime(s, 128, "%H:%M:%S ", today );
		res += s;
	}
	if (!bStarted) {
		start_time = std::chrono::steady_clock::now();
		bStarted = true;
	}
	else if (show_info & 4) {
		long fDurationMilli = long(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count());
		sprintf(s, "% 8.2f ", double(fDurationMilli) / 1000.0);
		res += s;
	}
	return res;
}
// Для кажного типа сообзения задается отдельная функция, которая 
// реализуется на основе общей
std::string BaseLogger::lTitle() const {
	return title()+"> ";
}
std::string BaseLogger::iTitle() const {
	return title() + "|INFO> ";
}
std::string BaseLogger::wTitle() const {
	return title()+"|WARNING> ";
}
std::string BaseLogger::eTitle() const {
	return title()+"|ERROR> ";
}

StdLogger::StdLogger(std::ostream& l_out, std::ostream& w_out, std::ostream& e_out) {
	log_out = &l_out; p_log_out = nullptr;
	wrn_out = &w_out; p_wrn_out = nullptr;
	err_out = &e_out; p_err_out = nullptr;
	BaseLogger::onLogStart(true, wrn_out != log_out, err_out != wrn_out && err_out != log_out);
}

StdLogger::StdLogger(const std::string& l_out_file, std::ostream& w_out, std::ostream& e_out, std::ios_base::openmode mode) {
	using namespace std;
	log_out = nullptr;
	if (l_out_file != "") {
		p_log_out = std::make_shared<std::ofstream>(l_out_file.c_str(), mode | std::ios_base::out);
		if (*p_log_out) log_out = p_log_out.get();
		else p_log_out = nullptr;
	}
	wrn_out = &w_out; p_wrn_out = nullptr;
	err_out = &e_out; p_err_out = nullptr;
	BaseLogger::onLogStart(true, wrn_out != log_out, err_out != wrn_out && err_out != log_out);
}

StdLogger::StdLogger(const std::string& l_out_file, const std::string& w_out_file, std::ostream& e_out, std::ios_base::openmode mode) {
	using namespace std;
	log_out = nullptr;
	if (l_out_file != "") {
		p_log_out = std::make_shared<std::ofstream>(l_out_file.c_str(), mode | std::ios_base::out);
		if (*p_log_out) log_out = p_log_out.get();
		else p_log_out = nullptr;
	}
	wrn_out = nullptr;
	if (w_out_file != "") {
		p_wrn_out = std::make_shared<std::ofstream>(w_out_file.c_str(), mode | std::ios_base::out);
		if (*p_wrn_out) wrn_out = p_wrn_out.get();
		else p_wrn_out = nullptr;
	}
	err_out = &e_out;
	BaseLogger::onLogStart(true, wrn_out != log_out, err_out != wrn_out && err_out != log_out);
}

StdLogger::StdLogger(const std::string& l_out_file, const std::string& w_out_file, const std::string& e_out_file, std::ios_base::openmode mode) {
	using namespace std;
	log_out = nullptr;
	if (l_out_file != "") {
		p_log_out = std::make_shared<std::ofstream>(l_out_file.c_str(), mode | std::ios_base::out);
		if (*p_log_out) log_out = p_log_out.get();
		else p_log_out = nullptr;
	}
	wrn_out = nullptr;
	if (w_out_file == l_out_file && w_out_file != "") {
		wrn_out = log_out;
	}
	else if (w_out_file != "") {
		p_wrn_out = std::make_shared<std::ofstream>(w_out_file.c_str(), mode | std::ios_base::out);
		if (*p_wrn_out) wrn_out = p_wrn_out.get();
		else p_wrn_out = nullptr;
	}
	err_out = nullptr;
	if (e_out_file == l_out_file && e_out_file != "") {
		err_out = log_out;
	}
	else if (e_out_file == w_out_file && e_out_file != "") {
		err_out = wrn_out;
	}
	else if (e_out_file != "") {
		p_err_out = std::make_shared<std::ofstream>(e_out_file.c_str(), mode | std::ios_base::out);
		if (*p_err_out) err_out = p_err_out.get();
		else p_err_out = nullptr;
	}
	BaseLogger::onLogStart(true, wrn_out != log_out, err_out != wrn_out && err_out != log_out);
}

StdLogger::StdLogger(const StdLogger& src) {
	p_log_out = src.p_log_out;
	log_out = src.log_out;
	p_wrn_out = src.p_wrn_out;
	wrn_out = src.wrn_out;
	p_err_out = src.p_err_out;
	err_out = src.err_out;
	BaseLogger::onLogStart(true, wrn_out != log_out, err_out != wrn_out && err_out != log_out);
}

StdLogger::~StdLogger() {
	BaseLogger::onLogFinish(true, wrn_out != log_out, err_out != wrn_out && err_out != log_out);
}

void StdLogger::ConsoleOut(const std::string& msg) const {
	if (bLogToConsole)
		std::cout << msg << std::endl;
}

void StdLogger::lOut(const std::string& msg) const {
	if (log_out) (*log_out) << msg << std::endl;
	ConsoleOut(msg);
}

void StdLogger::wOut(const std::string& msg) const {
	if (wrn_out) (*wrn_out) << msg << std::endl;
	ConsoleOut(msg);
}

void StdLogger::eOut(const std::string& msg) const {
	if (err_out) (*err_out) << msg << std::endl;
	ConsoleOut(msg);
}
