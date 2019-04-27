#include"C_data.h"

C_data::C_data()
{
	memset(_write_message, 0x00, sizeof(_write_message));

	/* 1. EPOLL Init Setting */
	_ep_events = new struct epoll_event;
	_epfd = epoll_create(EPOLL_SIZE);
	_event_cnt = 0;
}

C_data::~C_data()
{
	fclose(_data);
}

/* Data File Open */
char* C_data::F_open_data_file(char* r_data_file, char* r_rs_gubun)
{
	/* 1. Data File Name */
	_data_file = r_data_file;

	/* 2. 송수신 구분 변수 저장 */
	_rs_gubun = r_rs_gubun;

	/* 3. Data File Open */
	_data = fopen(_data_file, "at");
	if(_data == NULL)
	{
		memset(_write_message, 0x00, sizeof(_write_message));
		sprintf(_write_message, "Data File Open Error(%s)..", _data_file);
		throw _write_message;
	}
	else
	{
		/* 4. File Descriptor 저장 */
		_data_fd = fileno(_data);

		/* 5. Modify "_data_fd" Non-Blocking Mode */
		F_set_non_blocking_mode(_data_fd);

		/* 6. EPOLL Register */
		_event.events = EPOLLIN | EPOLLET;
		_event.data.fd = _data_fd;
		epoll_ctl(_epfd, EPOLL_CTL_ADD, _data_fd, &_event);

		/* 7. Start Data File Open Message */
		memset(_write_message, 0x00, sizeof(_write_message));
		sprintf(_write_message, "Data File OPEN(%s)..", _data_file);
		return _write_message;
	}
}

/* Check Data File Status */
int C_data::F_check_data(int _time)
{
	/* 1. Data 존재 여부 확인 */
	/* _time이 -1일 경우 Event 발생 시점까지 대기, 1000 = 1초 대기 */
	_event_cnt = epoll_wait(_epfd, _ep_events, EPOLL_SIZE, _time);
	if(_event_cnt < 0) /* EPOLL Error */
	{
		memset(_write_message, 0x00, sizeof(_write_message));
		sprintf(_write_message, "Data EPOLL Error : %s", strerror(errno));
		throw _write_message;
	}
	else if(_event_cnt == 0) /* DATA NONE */
	{
		return NONE;
	}
	else if(_event_cnt > 0) /* Data File Event */
	{
		for(int i = 0; i < _event_cnt; i++)
		{
			if(_ep_events[i].data.fd == _data_fd)
			{
				return SUCCESS;
			}
		}
	}
}

/* Write Data To File */
char* C_data::F_write_data(const char* r_data)
{
	/* 1. Mutual Exclosion Lock */
	lock_func.lock();

	/* 2. Write Data To File */

	if(fputs(r_data, _data) != -1)
	{
		memset(_write_message, 0x00, sizeof(_write_message));
		sprintf(_write_message, "%s File Write..%ld", _data_file, sizeof(r_data));
		throw _write_message;
	}
	else
	{
		memset(_write_message, 0x00, sizeof(_write_message));
		sprintf(_write_message, "%s File Write Error..%ld", _data_file, sizeof(r_data));
		throw _write_message;
	}

	/* 3. Mutual Exclosion Unlock */
	lock_func.unlock();
}

/* Read Data File */
char* C_data::F_read_data()
{
	int _result = 0;
	/* 1. Data File Status Check */
	_result = F_check_data(CHECK_DATA);
	if(_result == SUCCESS)
	{
		char* _result;
		memset(_result, 0x00, sizeof(_result));

		/* 2. Data Read */
		if(strncmp(_data_file, "KSND", 4) == 0)
			_result = fgets(_ksnd_file.sequence_number, sizeof(_ksnd_file), _data);
		else if(strncmp(_data_file, "KRJT", 4) == 0)
			_result = fgets(_krjt_file.sequence_number, sizeof(_krjt_file), _data);

		if(_result == NULL)
		{
			memset(_write_message, 0x00, sizeof(_write_message));
			sprintf(_write_message, "%s File Read Error..%ld", _data_file, sizeof(_ksnd_file));
			return _write_message;
		}
	}
	else if(_result = TIMEOUT)
	{
		/* Data가 없을 경우 아무일도 하지 않음 */
	}

	/* 3. Return Data Read Message */
	memset(_write_message, 0x00, sizeof(_write_message));
	if(strncmp(_data_file, "KSND", 4) == 0)
		sprintf(_write_message, "%s File Read..%ld", _data_file, sizeof(_ksnd_file));
	else if(strncmp(_data_file, "KRJT", 4) == 0)
		sprintf(_write_message, "%s File Read..%ld", _data_file, sizeof(_krjt_file));
	return _write_message;
}

/* Modify FD Non Blocking Mode */
void C_data::F_set_non_blocking_mode(int _fd)
{
	int flag = fcntl(_fd, F_GETFL, 0);
	fcntl(_fd, F_SETFL, flag | O_NONBLOCK);
}
