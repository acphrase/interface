#include"C_log.h"

C_log::C_log()
{
	memset(_write_message, 0x00, sizeof(_write_message));
}

C_log::~C_log()
{
	_log.close();
}

void C_log::F_open_log_file(char* _process, char* _log_file)
{
	_process_name = _process;
	char _start_message[150];

	_log.open(_log_file, ios::in | ios::app);
	if(!_log.is_open())
	{
		memset(_start_message, 0x00, sizeof(_start_message));
		sprintf(_start_message, "Log File Open Error(%s)..", _log_file);
		cout << _start_message << endl;
		F_write_log(_start_message);
		exit(1);
	}
	else
	{
		/* Start Log Message */
		F_write_log("====================================================");
		F_write_log("TCP/IP PROGRAM");
		F_write_log("====================================================");

		memset(_start_message, 0x00, sizeof(_start_message));
		sprintf(_start_message, "Log File OPEN(%s)..", _log_file);
		F_write_log(_start_message);
	}
}

void C_log::F_write_log(const char* _message)
{
	/* 1. Write Buffer Init */
	memset(_write_message, 0x00, sizeof(_write_message));

	/* 2. Time Update */
	_date_time.F_update_date_time();
	_time = _date_time.F_get_time_edit();

	/* 3. Write Message Setting */
	sprintf(_write_message, "[%6.6s-%8.8s] %s", _process_name, _time, _message);


	/* 4. Write Log File */
	_log.seekp(0, ios::end);			/* Log File의 맨 마지막에 위치 */
	_log << _write_message << endl;	/* Log File에 Setting 한 Message Write */
	if(_log.bad())
	{
		sprintf(_write_message, "[%6.6s-%8.8s] Log File Write Error..", _process_name, _time);
		cout << _write_message << endl;
		exit(1);
	}
}

