#ifndef __MSG_HEADER__
#define __MSG_HEADER__

#include"main.h"
#include"C_time.h"

class C_msg
{
	private :
		fstream _msg;
		C_time _date_time;
		char _write_message[150];
		char* _time;
		char* _process_name;
		mutex lock_func;

	public :
		C_msg();
		~C_msg();
		char* F_open_msg_file(char* _process, char* _msg_file);
		void F_write_msg(const char* _message);
};

#endif
