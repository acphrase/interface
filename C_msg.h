#ifndef __MSG_HEADER__
#define __MSG_HEADER__

#include"main.h"
#include"C_time.h"

class C_msg
{
	private :
		/* 1. Status Message Variable */
		char _write_message[150];

		/* 2. File Descriptor Variable */
		fstream _msg;

		/* 3. Time Class */
		C_time _date_time;

		/* 4. Mutex Variable */
		mutex lock_func;

		/* 5. ETC */
		char* _time;
		char* _process_name;

	public :
		C_msg();
		~C_msg();

		/* Message File Open */
		char* F_open_msg_file(char* _process, char* _msg_file);

		/* Write Message */
		void F_write_msg(const char* _message);
};

#endif
