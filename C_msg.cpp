#include"C_msg.h"

C_msg::C_msg()
{
	memset(_write_message, 0x00, sizeof(_write_message));
}

C_msg::~C_msg()
{
	_msg.close();
}

char* C_msg::F_open_msg_file(char* _process, char* _msg_file)
{
	_process_name = _process;

	_msg.open(_msg_file, ios::in | ios::app);
	if(!_msg.is_open())
	{
		memset(_write_message, 0x00, sizeof(_write_message));
		sprintf(_write_message, "MSG File Open Error(%s)..", _msg_file);
		cout << _write_message << endl;
		throw _write_message;
	}
	else
	{
		/* Start Msg Message */
		memset(_write_message, 0x00, sizeof(_write_message));
		sprintf(_write_message, "MSG File OPEN(%s)..", _msg_file);
		return _write_message;
	}
}

void C_msg::F_write_msg(const char* _message)
{
	/* 1. Write Buffer Init */
	memset(_write_message, 0x00, sizeof(_write_message));

	/* 2. Time Update */
	_date_time.F_update_date_time();
	_time = _date_time.F_get_time_edit();

	/* 3. Write Message Setting */
	sprintf(_write_message, "[%6.6s-%8.8s] %s", _process_name, _time, _message);

	/* 4. Write Msg File */
	_msg.seekp(0, ios::end);			/* Msg File의 맨 마지막에 위치 */
	_msg << _write_message << endl;		/* Msg File에 Setting 한 Message Write */
	if(_msg.bad())
	{
		sprintf(_write_message, "[%6.6s-%8.8s] MSG File Write Error..", _process_name, _time);
		throw _write_message;
	}
}
