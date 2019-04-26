#include"C_time.h"

C_time::C_time()
{
	_now = time(NULL);
	S_now = localtime(&_now);
}

/* Update Time */
void C_time::F_update_date_time()
{
	_now = time(NULL);
	S_now = localtime(&_now);
}

/* Return Date (YYYYMMDD) */
char* C_time::F_get_date()
{
	F_update_date_time();
	sprintf(now_date, "%04d%02d%02d", 1900 + S_now->tm_year, 1 + S_now->tm_mon, S_now->tm_mday);
	return now_date;
}

/* Return Time (HHMMSS) */
char* C_time::F_get_time()
{
	F_update_date_time();
	sprintf(now_time, "%02d%02d%02d", S_now->tm_hour, S_now->tm_min, S_now->tm_sec);
	return now_time;
}

/* Return Date (YYYY/MM/DD) */
char* C_time::F_get_date_edit()
{
	F_update_date_time();
	sprintf(edit_date, "%04d/%02d/%02d", 1900 + S_now->tm_year, 1 + S_now->tm_mon, S_now->tm_mday);
	return edit_date;
}

/* Return Time (HH:MM:SS) */
char* C_time::F_get_time_edit()
{
	F_update_date_time();
	sprintf(edit_time, "%02d:%02d:%02d", S_now->tm_hour, S_now->tm_min, S_now->tm_sec);
	return edit_time;
}
