#include"C_time.h"

C_time::C_time()
{
	_now = time(NULL);
	S_now = localtime(&_now);

	sprintf(now_time, "%02d%02d%02d", S_now->tm_hour, S_now->tm_min, S_now->tm_sec);
	sprintf(now_date, "%04d%02d%02d", 1900 + S_now->tm_year, 1 + S_now->tm_mon, S_now->tm_mday);
	sprintf(edit_time, "%02d:%02d:%02d", S_now->tm_hour, S_now->tm_min, S_now->tm_sec);
	sprintf(edit_date, "%04d/%02d/%02d", 1900 + S_now->tm_year, 1 + S_now->tm_mon, S_now->tm_mday);
}

void C_time::F_update_date_time()
{
	_now = time(NULL);
	S_now = localtime(&_now);

	sprintf(now_time, "%02d%02d%02d", S_now->tm_hour, S_now->tm_min, S_now->tm_sec);
	sprintf(now_date, "%04d%02d%02d", 1900 + S_now->tm_year, 1 + S_now->tm_mon, S_now->tm_mday);
	sprintf(edit_time, "%02d:%02d:%02d", S_now->tm_hour, S_now->tm_min, S_now->tm_sec);
	sprintf(edit_date, "%04d/%02d/%02d", 1900 + S_now->tm_year, 1 + S_now->tm_mon, S_now->tm_mday);
}

char* C_time::F_get_time()
{
	return now_time;
}

char* C_time::F_get_date()
{
	return now_date;
}

char* C_time::F_get_time_edit()
{
	return edit_time;
}

char* C_time::F_get_date_edit()
{
	return edit_date;
}
