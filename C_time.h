#ifndef __TIME_HEADER__
#define __TIME_HEADER__

#include"main.h"

class C_time
{
	private :
		time_t _now;
		struct tm *S_now;
		char now_time[7];
		char now_date[9];
		char edit_time[9];
		char edit_date[11];
	public :
		C_time();
		void F_update_date_time();
		char* F_get_time();
		char* F_get_date();
		char* F_get_time_edit();
		char* F_get_date_edit();
};

#endif
