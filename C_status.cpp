#include"C_status.h"

C_status::C_status()
{
	memset(&_status_record, 0x00, sizeof(_status_record));
	memset(_write_message, 0x00, sizeof(_write_message));
	memset(_status_key, 0x00, sizeof(_status_key));
	memset(_process_status, 0x00, sizeof(_process_status));
	memset(_data_count, 0x00, sizeof(_data_count));
	memset(_link_status, 0x00, sizeof(_link_status));
	_process_status_num = -1;
	_data_count_num = 0;
	_link_status_num = -1;
	_line_temp = 0;
}

C_status::~C_status()
{
	_status.close();
}

/* Status File Open */
char* C_status::F_open_status_file(char* _status_file, char* r_company_id, char* r_status_gubun)
{
	/* 1. Status File Open */
	_status.open(_status_file, ios::in | ios::out);
	if(!_status.is_open())
	{
		memset(_write_message, 0x00, sizeof(_write_message));
		sprintf(_write_message, "STATUS File Open Error(%s)..", _status_file);
		throw _write_message;
	}
	else
	{
	
		memset(_status_key, 0x00, sizeof(_status_key)); 
		strncpy(_status_key, r_company_id, 3); 
		strncpy(&_status_key[3], r_status_gubun, 2); 

		/* Start STATUS File Open Message */
		memset(_write_message, 0x00, sizeof(_write_message));
		sprintf(_write_message, "STATUS File OPEN(%s)..", _status_file);
		return _write_message;
	}
}

/* Read Status File */
void C_status::F_read_status()
{
	/* 1. Get Record To Buffer */ 
	_status.seekg(_line_temp, ios::beg);
	_status.read(_status_record.company_id, STATUS_RECORD_LENGTH + 1);

	/* 2. Key Positioning & Get Record To Buffer */ 
	if(strncmp(_status_record.company_id, _status_key, 5) != 0)
	{
		_status.seekg(0, ios::beg); /* STATUS File의 맨 처음에 위치 */ 
		while(!_status.eof()) 
		{ 
			_line_temp = _status.tellg();
			_status.read(_status_record.company_id, STATUS_RECORD_LENGTH + 1);
			if(_status.bad())
			{
				memset(_write_message, 0x00, sizeof(_write_message));
				sprintf(_write_message, "STATUS File Read Error..%s", _status_key);
				throw _write_message;
			}

			if(strncmp(_status_record.company_id, _status_key, 5) == 0)
				break;
		}

		if(strncmp(_status_record.company_id, _status_key, 5) != 0)
		{
			memset(_write_message, 0x00, sizeof(_write_message));
			sprintf(_write_message, "STATUS File Key Positioning Error..%s", _status_key);
			throw _write_message;
		}
	}
}

/* Write Status File */
void C_status::F_write_status()
{
	/* 1. Positioning */
	_status.seekp(_line_temp, ios::beg); /* STATUS File의 해당 회원사 Record 위치 */

	/* 2. Mutual Exclusion Lock */
	lock_func.lock();

	/* 3. Buffer Write */
	//_status.write(_status_record.company_id, STATUS_RECORD_LENGTH);
	_status << _status_record.company_id;
	if(_status.sync() == -1)
	{
		lock_func.unlock();
		memset(_write_message, 0x00, sizeof(_write_message));
		sprintf(_write_message, "STATUS File Write Error..%s", _status_key);
		throw _write_message;
	}

	/* 4. Mutual Exclusion Unlock */
	lock_func.unlock();
}

/* Update Status File */
int C_status::F_update_status(int msg_type)
{
	/* 1. Read Record */
	F_read_status();

	/* 2. Buffer Time Setting */
	_time = _date_time.F_get_time();
	strncpy(&_status_record.date[0], _date_time.F_get_date(), 8);
	strncpy(&_status_record.time[0], _time, 6);

	/* 3. Update Variable Setting */
	switch(msg_type)
	{
		case START :
			strncpy(&_status_record.restart_time[0], _time, 6); /* 재시작 시간 Set */
			strncpy(&_status_record.process_status[0], "1", 1); /* "1"은 정상 기동 Set */
			break;
		case ERROR :
			break;
		default :
	        strncpy(&_status_record.error_code[0], "00", 2); /* TCPIP Error Code Set */

			switch(msg_type)
			{
				case MSG_0810_001 :
					strncpy(&_status_record.process_status[0], "1", 1);	/* 비정상 Stop 후 재기동 시 반영 */
					strncpy(&_status_record.link_status[0], "01", 2);		/* 01은 link 됨 */
					strncpy(&_status_record.link_time[0], _time, 6);		/* link 시간 기재 */
					strncpy(&_status_record.complete_yn[0], "0", 1);		/* 0 : 사용중, 1 : 사용완료 */
					break;

				case MSG_0800_301 :
					break;

				case MSG_0810_301 :
					break;

				case MSG_0800_040 :
					strncpy(&_status_record.link_status[0], "02", 2);			/* 02는 link 종료 송신 */
					break;

				case MSG_0810_040 :
					strncpy(&_status_record.link_status[0], "03", 2);			/* 03은 link 종료 수신 */
					break;

				case MSG_0200_000 :
				case MSG_0210_000 :
					long _temp_count;
					char _add_count[9];
					memset(_add_count, 0x00, sizeof(_add_count));
					
					_temp_count = F_get_last_data_count();
					_temp_count += 1;

					sprintf(_add_count, "%.8ld", _temp_count);

					strncpy(_status_record.data_count, _add_count, 8); /* 일련번호 세팅 */
					break;

				default :
					memset(_write_message, 0x00, sizeof(_write_message));
					sprintf(_write_message, "Invalid Message Type..%d (in message_set)", msg_type);
					throw _write_message;
			}
					break;
	}

	/* 4. Write Record */
	F_write_status();

	if(msg_type == MSG_0810_001)
		return MSG_0810_001;
	else
		return SUCCESS;
}

/* Update Process Stop To Status File */
int C_status::F_process_stop(int option)
{
	/* 1. Read Record */
	F_read_status();

	/* 2. Buffer Time Setting */
	_time = _date_time.F_get_time();
	strncpy(_status_record.date, _date_time.F_get_date(), sizeof(_status_record.date));
	strncpy(_status_record.time, _time, sizeof(_status_record.time));
	strncpy(_status_record.stop_time, _time, sizeof(_status_record.stop_time));

	/* 3. Buffer Status Setting */
	if(option == SUCCESS)
	{ /* 정상 종료 */
		strncpy(_status_record.process_status, NORMAL_STATUS, sizeof(_status_record.process_status));
		strncpy(_status_record.stop_gubun, NORMAL_STOP, sizeof(_status_record.stop_gubun));
		strncpy(_status_record.complete_yn, COMPLETE, sizeof(_status_record.complete_yn));
	}
	else
	{ /* 비정상 종료 */
		strncpy(_status_record.process_status, ABNORMAL_STATUS, sizeof(_status_record.process_status));
		strncpy(_status_record.stop_gubun, ABNORMAL_STOP, sizeof(_status_record.stop_gubun));
	}

	/* 4. Write Record */
	F_write_status();

	if(option == SUCCESS)
		return SUCCESS;
	else if(option == FAIL)
		return FAIL;
}

/* Return Last Data Count Number */
long C_status::F_get_last_data_count()
{
	/* Data Status 변수 Setting ("최종 수신 또는 송신 번호" -> "_data_count") */
	/* 마지막 데이터 수신 및 송신 개수 (일련번호) */
	strncpy(_data_count, _status_record.data_count, 8);
	_data_count_num = atol(_data_count);
	return _data_count_num;
}

/* Return Last Data Count String for Log */
char* C_status::F_get_last_data()
{
	memset(_write_message, 0x00, sizeof(_write_message));
	sprintf(_write_message, "Last Count : %ld", F_get_last_data_count());
	return _write_message;
}

/* Return Process Status String for Log */
char* C_status::F_get_process()
{
	memset(_write_message, 0x00, sizeof(_write_message));
	sprintf(_write_message, "Proc Status : %.1s", _status_record.process_status);
	return _write_message;
}

/* Return Link Status String for Log */
char* C_status::F_get_link()
{
	memset(_write_message, 0x00, sizeof(_write_message));
	sprintf(_write_message, "Link Status : %.2s", _status_record.link_status);
	return _write_message;
}

/* Get & Setting TCPIP Error Code */
void C_status::F_setting_tcpip_error_code(int r_error_code)
{
	char temp[3];
	sprintf(temp, "%.2d", r_error_code);
	strncpy(_status_record.error_code, temp, 2); /* TCPIP Error Code Set */
}
