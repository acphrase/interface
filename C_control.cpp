#include"C_control.h"

C_control::C_control(char *argv[]) : _key(&argv[1]), _config_path(&argv[2])
{
	try
	{
		/* 1. read config */
		_config.F_read_config(*_key, *_config_path);	/* CONFIG에서 정보 가져오기 */

		/* 2. check socket infrom to socket */
        F_set_check_socket_information();
	}
	catch(const char* _message)
	{
		cout << _message << endl;
		exit(1);
	}

	/* 사전 처리 */
	F_open_file();				/* CONFIG에서 가져 온 정보로 관련파일들 Open */
	F_get_cnt();				/* Process상태, Link여부, 마지막 데이터 개수 */
	F_get_jang();				/* JANG File을 읽어와서 Variable Setting */
	F_update_cnt(START);		/* Count File에 Process Status 정상기동으로 수정 */
}

void C_control::F_set_check_socket_information()
{
	int _data_length = 0;
	_data_length = atoi(_config.F_get_message_length());
	char* _ip_number = _config.F_get_ip_number();
	char* _port_number = _config.F_get_port_number();
	char* _company_id = _config.F_get_company_id();
	char* _tr_code = _config.F_get_tr_code();

	if(_socket.F_set_config_information(_data_length, _ip_number, _port_number, _company_id, _tr_code, F_get_communicate_type()) != SUCCESS)
	{
		throw "Check Socket Setting Error..";
	}
}

void C_control::F_open_file()
{
	try
	{
		/* 1. LOG File Open */
		_log.F_open_log_file(_config.F_get_process_name(), _config.F_get_log_file_name());
		
		/* 2. MSG File Open */
		_log.F_write_log(_msg.F_open_msg_file(_config.F_get_process_name(), _config.F_get_msg_file_name()));

		/* 3. Program Start Message */
		_log.F_write_log("TCP/IP PROGRAM STARTING");
		_msg.F_write_msg("TCP/IP PROGRAM STARTING");

		/* 4. Data File Open */
		_log.F_write_log(_data.F_open_data_file(_config.F_get_data_file_name(), F_get_communicate_type()));

		/* 5. CNT File Open */
		_log.F_write_log(_cnt.F_open_cnt_file(_config.F_get_cnt_file_name(), _config.F_get_company_id(), _config.F_get_cnt_gubun()));

		/* 6. Jang File Open */
		_log.F_write_log(_jang.F_open_jang_file(_config.F_get_jang_file_name()));
	}
	catch(const char* r_message)
	{
		_log.F_write_log(r_message);
		_msg.F_write_msg(r_message);
		exit(1);
	}
}

void C_control::F_get_cnt()
{
	try
	{
		/* 1. 해당 회원사 Record Read */
		_cnt.F_read_cnt();

		/* 2. Last Data Count Status Check */
		memset(_message, 0x00, sizeof(_message));
		_log.F_write_log(_cnt.F_get_last_data());

		/* 3. Process Status Check */
		memset(_message, 0x00, sizeof(_message));
		_log.F_write_log(_cnt.F_get_process());

		/* 4. Link Status Check */
		memset(_message, 0x00, sizeof(_message));
		_log.F_write_log(_cnt.F_get_link());
	}
	catch(const char* r_message)
	{
		_log.F_write_log(r_message);
		_msg.F_write_msg(r_message);
		F_stop_process(FAIL);
	}
}

void C_control::F_get_jang()
{
	try
	{
		_jang_status = _jang.F_get_jang_status();
	}
	catch(const char* r_message)
	{
		_log.F_write_log(r_message);
		_msg.F_write_msg(r_message);
		F_stop_process(FAIL);
	}

}

void C_control::F_start()
{
    if(!F_get_link_status())
    {
        while(!F_get_link_status())
			F_create_socket();
    }
}

int C_control::F_create_socket()
{
	try
	{
		/* 1. Socket Create, Bind, Listen */
		_log.F_write_log(_socket.F_create_socket());

		/* 2. Socket Accept */
		memset(_message, 0x00, sizeof(_message));
		sprintf(_message, "%s", _socket.F_accept_socket());
		_log.F_write_log(_message);
		_msg.F_write_msg(_message);

		/* 3. Status Message */
		memset(_message, 0x00, sizeof(_message));
		sprintf(_message, "Link 대기 중...");
		_log.F_write_log(_message);
		_msg.F_write_msg(_message);
	}
	catch(const char* r_message)
	{
		_log.F_write_log(r_message);
		_msg.F_write_msg(r_message);
		F_stop_process(FAIL);
	}
}

void C_control::F_read_message()
{
	/* Variable Init */
	int _result = FAIL;
	_socket.F_put_retry_init();

	while(1)
	{
		try
		{
			/* 1. Read Message */
			_result = _socket.F_recv_message();

			/* 2. Message Status Check */
			if(_result == SUCCESS) 
			{ /* 정상 수신 */
				_log.F_write_log(_socket.F_put_log_recv_message());
				break;
			}
			else
			{ /* 재송횟수 3회 초과 */
				memset(_message, 0x00, sizeof(_message));
				sprintf(_message, "RECV RETRY %d Times Error..", _result);
				_log.F_write_log(_message);
				_log.F_write_log("TRY TCP/IP RECONNECT..");

				/* Socket recreate */
				F_start();
			}
		}
		catch(const char* r_message)
		{
			_log.F_write_log(r_message);
			_msg.F_write_msg(r_message);
			break;
		}
		catch(const int _fail)
		{
			memset(_message, 0x00, sizeof(_message));
			sprintf(_message, "SEND Retry Error..%d", _fail);
			F_stop_process(FAIL);
		}
	}
}

void C_control::F_check_message()
{
	try
	{
		/* 1. 마지막 데이터 갯수 확인 */
		long _last_data_count = 0;
		_last_data_count = _cnt.F_get_last_data_count();

		/* 2. 장 상태 확인 */
		F_get_jang();

		/* 3. Check Message */
		int _result_check = 0;
		_result_check = _socket.F_check_message(_jang_status, _last_data_count);
		if(_result_check == END)
		{
			F_stop_process(SUCCESS);
		}
	}
	catch(const char* r_message)
	{
		_log.F_write_log(r_message);
		_msg.F_write_msg(r_message);
		_error_code = _socket.F_get_error_code();
		if(_error_code != FAIL)
		{
			_cnt.F_setting_tcpip_error_code(_error_code);
			memset(_message, 0x00, sizeof(_message));
			sprintf(_message, "Interface Error Code..%d", _error_code);
			_log.F_write_log(_message);
			_msg.F_write_msg(_message);
			F_update_cnt(ERROR);
		}
		else
		{
			_log.F_write_log("TCPIP Error Code Check!!");
			_msg.F_write_msg("TCPIP Error Code Check!!");
		}
	}
}

void C_control::F_send_message()
{
	/* Variable Init */
	int _result = FAIL;

	while(1)
	{
		try
		{
			/* 1. Send Message */
			_result = _socket.F_send_message();

			/* 2. Message Status Check */
			if(_result == SUCCESS) 
			{ /* 정상 송신 */
				_log.F_write_log(_socket.F_put_log_send_message());
				F_update_cnt(NORMAL);
				break;
			}
			else
			{
				memset(_message, 0x00, sizeof(_message));
				sprintf(_message, "Message Send Error.. : %s", strerror(errno));
				_log.F_write_log(_message);
				_log.F_write_log(_message);
				F_stop_process(FAIL);
			}
		}
		catch(const char* r_message)
		{
			_log.F_write_log(r_message);
			_msg.F_write_msg(r_message);
			_log.F_write_log("TRY TCP/IP RECONNECT..");

			/* Socket recreate */
			F_start();
			break;
		}
	}
}

void C_control::F_set_send_message_type()
{
	try
	{
		if(_socket.F_get_connect_status() == DISCONNECT)
		{
			if(_socket.F_get_message_type() == MSG_0810_001)
				_socket.F_set_send_message_type(MSG_0810_001);

			F_send_message();
			F_set_send_message_type();
		}
		else
		{
			int _check_result;
			_check_result = _data.F_check_data(CHECK_DATA);

			if(_check_result == NONE)
				_socket.F_set_send_message_type(MSG_0800_301);
			else if(_check_result == SUCCESS)
				_socket.F_set_send_message_type(MSG_0200_000);
			else if(_jang.F_get_jang_status() == JANG_CLOSE)
				_socket.F_set_send_message_type(MSG_0800_040);
		}
	}
	catch(const char* r_message)
	{
		_log.F_write_log(r_message);
		_msg.F_write_msg(r_message);
	}
}

void C_control::F_update_cnt(int msg_type)
{
	int _update_result;
	try
	{
		switch(msg_type)
		{
			case START :
				_update_result = _cnt.F_update_cnt(START);
				break;
			case ERROR :
				_update_result = _cnt.F_update_cnt(ERROR);
				break;
			case NORMAL :
				_update_result = _cnt.F_update_cnt(_socket.F_get_message_type());
				break;
			default :
				memset(_message, 0x00, sizeof(_message));
				sprintf(_message, "Invalid Message Type..%d (in control)", msg_type);
				_log.F_write_log(_message);
				_msg.F_write_msg(_message);
				F_stop_process(FAIL);
				break;
		}

		if(_update_result !=  SUCCESS)
		{
			if(_update_result == MSG_0810_001)
			{
				_socket.F_set_connect_status(CONNECT);

				_log.F_write_log("Link Complete..");
				_msg.F_write_msg("Link Complete..");
			}
			else
			{
				_log.F_write_log("CNT File Update Error..");
				_msg.F_write_msg("CNT File Update Error..");
				F_stop_process(FAIL);
			}
		}
	}
	catch(const char* r_message)
	{
		_log.F_write_log(r_message);
		_msg.F_write_msg(r_message);
		F_stop_process(FAIL);
	}
}

char* C_control::F_get_communicate_type()
{
	char* _communicate_type = _config.F_get_communication_type();
	return _communicate_type;
}

int C_control::F_get_link_status()
{
	int _link_status = _socket.F_get_link_status();
	return _link_status;
}

void C_control::F_stop_process(int option)
{
	try
	{
		_socket.F_set_link_status(DISCONNECT);
		_socket.F_set_connect_status(DISCONNECT);

		if(_cnt.F_put_process_stop(option) == SUCCESS)
		{
			F_update_cnt(NORMAL);
			memset(_message, 0x00, sizeof(_message));
			sprintf(_message, "Interface PROGRAM NORMAL STOP");
			_log.F_write_log(_message);
			_msg.F_write_msg(_message);
			exit(0);
		}
		else
		{
			F_update_cnt(ERROR);
			memset(_message, 0x00, sizeof(_message));
			sprintf(_message, "Interface PROGRAM ABNORMAL STOP");
			_log.F_write_log(_message);
			_msg.F_write_msg(_message);
			exit(1);
		}
	}
	catch(const char* _message)
	{
		_log.F_write_log(_message);
		_msg.F_write_msg(_message);
		exit(1);
	}
}
