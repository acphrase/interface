#ifndef __LOG_HEADER__
#define __LOG_HEADER__

#include"common.h"
#include"C_time.h"

class C_log
{
	private :
		/* 1. Status Message Variable */
		char _write_message[150];

		/* 2. File Descriptor Variable */
		fstream _log;

		/* 3. Time Class */
		C_time _date_time;

		/* 4. Mutex Variable */
		mutex lock_func;

		/* 5. ETC */
		char* _time;
		char* _process_name;

	public :
		C_log();
		~C_log();

		/* Log File Open */
		void F_open_log_file(char* _process, char* _log_file);

		/* Write Log */
		void F_write_log(const char* _message);
};

#endif
