#ifndef __TIME_HEADER__
#define __TIME_HEADER__

#include"main.h"

class C_time
{
	private :
		/* 1. Time Variable */
		time_t _now;
		struct tm *S_now;

		/* 2. ETC */
		char now_time[7];
		char now_date[9];
		char edit_time[9];
		char edit_date[11];
	public :
		C_time();

		/* Update Time */
		void F_update_date_time();
	
		/* Return Date (YYYYMMDD) */
		char* F_get_date();

		/* Return Time (HHMMSS) */
		char* F_get_time();

		/* Return Date (YYYY/MM/DD) */
		char* F_get_date_edit();

		/* Return Time (HH:MM:SS) */
		char* F_get_time_edit();
};

#endif
