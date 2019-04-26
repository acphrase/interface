#include"C_msg.h"

C_msg::C_msg()
{
	memset(_write_message, 0x00, sizeof(_write_message));
}

C_msg::~C_msg()
{
	_msg.close();
}

/* Message File Open */
char* C_msg::F_open_msg_file(char* _process, char* _msg_file)
{
	/* 1. Get Process Name */
	_process_name = _process;

	/* 2. Open Message File */
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

/* Write Message */
void C_msg::F_write_msg(const char* _message)
{
	/* 1. Write Buffer Init */
	memset(_write_message, 0x00, sizeof(_write_message));

	/* 2. Time Setting */
	_time = _date_time.F_get_time_edit();

	/* 3. Write Message Setting */
	sprintf(_write_message, "[%6.6s-%8.8s] %s", _process_name, _time, _message);

	/* 4. Mutual Exclusion Lock */
	lock_func.lock();

	/* 5. Write Message File */
	_msg.seekp(0, ios::end);			/* Message File의 맨 마지막에 위치 */
	_msg << _write_message << endl;		/* Messag File에 Setting 한 Message Write */
	if(_msg.sync() == -1)
	{
		lock_func.unlock();
		sprintf(_write_message, "[%.6s-%.8s] MSG File Write Error..", _process_name, _time);
		throw _write_message;
	}

	/* 6. Mutual Exclusion Unlock */
	lock_func.unlock();
}
