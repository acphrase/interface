#ifndef __LOG_HEADER__
#define __LOG_HEADER__

#include"main.h"
#include"C_time.h"

class C_log
{
	private :
		fstream _log;
		int _log_fd;
		C_time _date_time;
		char _write_message[150];
		char* _time;
		char* _process_name;

	public :
		C_log();
		~C_log();
		void F_open_log_file(char* _process, char* _log_file);
		void F_write_log(const char* _message);
};

#endif
