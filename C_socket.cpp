#include"C_socket.h"

C_socket::C_socket()
{
	memset(_message, 0x00, sizeof(_message));

	memset(&_server_address, 0x00, sizeof(_server_address));
	memset(&_client_address, 0x00, sizeof(_client_address));
	_server_socket = 0;
	_client_socket = 0;
	_client_address_size = 0;

	_link_status = DISCONNECT;
	_connect_status = DISCONNECT;

    memset(_recv_buffer, 0x00, sizeof(_recv_buffer));
    memset(_send_buffer, 0x00, sizeof(_send_buffer));
    memset(&_recv_message, 0x00, sizeof(_recv_message));
    memset(&_send_message, 0x00, sizeof(_send_message));
	_recv_length = 0;
	_recv_message_type = UNDEFINED;
	_send_message_type = UNDEFINED;
	
	/* 1. EPOLL Init Setting */
	_ep_events = new struct epoll_event;
	_epfd = epoll_create(EPOLL_SIZE);
	_event_cnt = 0;
}

C_socket::~C_socket()
{
	delete _ep_events;
	close(_server_socket);
	close(_client_socket);
}

int C_socket::F_set_config_information(int r_data_length, char* r_ip_number, char* r_port_number, char* r_company_id, char* r_tr_code, char* r_communicate_type)
{
	_data_length = r_data_length;
	_ip_number = r_ip_number;
	_port_number = r_port_number;
	_company_id = r_company_id;
	_tr_code = r_tr_code;
	_communicate_type = r_communicate_type;

	return SUCCESS;
}

char* C_socket::F_create_socket()
{
	int state = 0;
	int _option = 0;
	int _check_option = 0;
	socklen_t _option_length;
	socklen_t _check_length;

	/* 1. Socket에 할당 할 정보 Setting */
	_server_address.sin_family = AF_INET; /* 주소 체계 : IPv4 */
	_server_address.sin_addr.s_addr = inet_addr(_ip_number); /* server ip(Network bytes 주소로 변환) */
	_server_address.sin_port = htons((unsigned short)atoi(_port_number)); /* server port number */

	/* 2. Create Socket */
	/* _server_socket : descriptor(handle). */
	/* PF_INET : Protocol Family. TCP/IP 일 경우. */
	/* SOCK_STREAM : Protocol Type. TCP. */
	/* IPPROTO_TCP : 앞의 SOCK_STREAM과 중복. 0을 넣어 앞의 값과 동일함을 알려도 됨 */
	_server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(_server_socket == -1)
	{
		memset(_message, 0x00, sizeof(_message));
		sprintf(_message, "Socket Create Error : %s", strerror(errno));
		throw _message;
	}

	/* 3. Add Option Socket */
	/* server 쪽에서 process를 종료하였을 경우 3분의 time wait 상태로 대기 중 */
	/* 주고 받는 데이터가 완료 되길 기다리는 시간으로 3분의 time wait 발생 */
	/* time wait을 무시하고 ip, port를 재사용 할 수 있도록 하는 socket option */
	_option_length = sizeof(_option);
	_option = 1;
	setsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR, (void*)&_option, _option_length);

	_check_length = sizeof(_check_option);
	state = getsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR, (void*)&_check_option, &_check_length);
	if(state)
	{
		memset(_message, 0x00, sizeof(_message));
		sprintf(_message, "Get Socket Option Error : %s", strerror(errno));
		throw _message;
	}
	if(_check_option != 1)
	{
		memset(_message, 0x00, sizeof(_message));
		sprintf(_message, "Set Socket Option Error : %s", strerror(errno));
		throw _message;
	}

	/* 4. Bind Socket */
	/* server socket에 _server_address 구조체에 저장 한 정보를 할당 */
	if(bind(_server_socket, (struct sockaddr*)&_server_address, sizeof(_server_address)) == -1)
	{
		memset(_message, 0x00, sizeof(_message));
		sprintf(_message, "Socket Bind Error : %s", strerror(errno));
		throw _message;
	}

	/* 5. Listen Socket */
	/* server socket에 connect를 요청하는지 대기 */
	/* 대기는 2개의 client까지만 허용. 나머지는 거부 안내 송신 */
	if(listen(_server_socket, 2) == 30000)
	{
		memset(_message, 0x00, sizeof(_message));
		sprintf(_message, "Socket Listen Error : %s", strerror(errno));
		throw _message;
	}
	else
	{
		memset(_message, 0x00, sizeof(_message));
		sprintf(_message, "Socket(NO = %d) Listening..", _server_socket);
		return _message;
	}
}

char* C_socket::F_accept_socket()
{
	/* 1. Accept Socket */
	/* server socket에 대기중인 client의 정보를 _client_address에 저장 */
	/* 접속 허용하여 데이터 송수신 시작 */
	F_set_non_blocking_mode(_server_socket); /* non-blocking socket 설정 */
	_event.events = EPOLLIN;
	_event.data.fd = _server_socket;
	epoll_ctl(_epfd, EPOLL_CTL_ADD, _server_socket, &_event);

	if(F_check_socket(WAIT_TIMEOUT) == SUCCESS) /* 연결 될 때 까지 무한 대기 */ 
	{
		memset(_message, 0x00, sizeof(_message));
		sprintf(_message, "TCP/IP Connected ( IP:%s port:%d )", inet_ntoa(_client_address.sin_addr), ntohs(_client_address.sin_port));
		return _message;
	}
	else
	{
		throw "Socket Check Error..";
	}
}

void C_socket::F_put_retry_init()
{
	_retry_check = 0;
}

int C_socket::F_recv_message()
{
	while(1)
	{
		/* 1. 재송횟수 확인 */
		/* 재송횟수 3번 이하 일 경우 */
		if(_retry_check < 3) 
		{
			int _header_length = 0;
			int _message_length = 0;
			int _check_result = 0;

			/* 2. 수신 가능한 상태인지 _client_socket 확인 */
			/* 개시 전 일 경우 수신 무한 대기, 개시 이 후는 설정 Time에 따른 대기 */
			if(_link_status == CONNECT && _connect_status == DISCONNECT)
				_check_result = F_check_socket(WAIT_TIMEOUT);	/* 데이터 수신 될 때 까지 무한 대기 */ 
			else if(_link_status == CONNECT && _connect_status == CONNECT)
				_check_result = F_check_socket(RECV_TIMEOUT);	/* TIMEOUT 설정 */

			/* 3. _client_socket 상태에 따른 행동 */
			switch(_check_result)
			{
				/* "SUCCESS"는 수신 할 데이터가 존재 */
				case SUCCESS :
					_header_length = F_read_socket(HEADER_LENGTH);
					if(_header_length > 0)
					{
						strncpy(_recv_message.message_length, _recv_buffer, HEADER_LENGTH);

						_message_length = F_read_socket(atoi(_recv_buffer));
						if(_message_length > 0)
						{
							strncpy(_recv_message.tr_code, _recv_buffer, sizeof(_recv_message) - HEADER_LENGTH);
							if(strncmp(_recv_message.err_code, "00", 2) != 0)
							{
								memset(_message, 0x00, sizeof(_message));
								sprintf(_message, "RECV Error Code : %.2s", _recv_message.err_code);
								throw _message;
							}
						}
						else
						{
    		    	    				throw	"Message Read Error From Socket..";
						}
					}
					else
					{
    		    	    			throw "Message Length Read Error From Socket..";
					}

					_retry_check = 0;

					return SUCCESS;

				/* "TIMEOUT"은 설정 시간 초과 */
				case TIMEOUT :
					/* 재송횟수 카운트 후 TIMEOUT에 따른 이벤트 진행 */
					_retry_check++;
					F_event_timeout();
					break;

				/* "TIMEOUT" 이외의 Error */
				default :
					memset(_message, 0x00, sizeof(_message));
					sprintf(_message, "RECV Socket EPOLL Error..%d", _check_result);
					throw _message;
			}
		}
		/* 재송횟수 3번 초과 일 경우 */
		else
		{
			/* Socket Close */
			_link_status = DISCONNECT;
			_connect_status = DISCONNECT;
			epoll_ctl(_epfd, EPOLL_CTL_DEL, _server_socket, NULL);
			epoll_ctl(_epfd, EPOLL_CTL_DEL, _client_socket, NULL);
			close(_client_socket);
			close(_server_socket);
			int _retry_temp = 0;
			_retry_temp = _retry_check;
			F_put_retry_init();

			return _retry_temp;
		}
	}
}

int C_socket::F_send_message()
{
	/* 1. Variable Init */
	int _send_length = 0;
	int _send_message_length = 0;
	_send_message_length = strlen(_send_message.message_length);

	/* 2. Send Message Setting */
	F_set_message();

	/* 3. Send Message from Buffer */
	_send_length = F_write_socket();
	return SUCCESS;
	//if(_send_length == _send_message_length)
	//{
	//	return SUCCESS;
	//}
	//else
	//{
	//	/* Socket Close */
	//	_link_status = DISCONNECT;
	//	_connect_status = DISCONNECT;
	//	epoll_ctl(_epfd, EPOLL_CTL_DEL, _server_socket, NULL);
	//	epoll_ctl(_epfd, EPOLL_CTL_DEL, _client_socket, NULL);
	//	close(_client_socket);
	//	close(_server_socket);
	//	int _retry_temp = 0;
	//	_retry_temp = _retry_check;
	//	F_put_retry_init();

	//	throw	"Message Send Error To Socket..";
	//}
}

void C_socket::F_event_timeout()
{
	if(strncasecmp(_communicate_type, &_send_gubun, 1) == 0)
	{
		/* 1. Variable Init */
		int _result = FAIL;
		memset(&_send_message, 0x00, sizeof(_send_message));

		/* 2. Retry Setting to Buffer */
		char temp[3];
		sprintf(temp, "%2.2d", _retry_check); 
		strncpy(_send_message.retry_cnt, temp, 2);
		
		/* 3. Send Message */
		_result = F_send_message();
		if(_result != SUCCESS)
		{
			char error_code[3];
			int error_no;
			error_no = atoi(strncpy(error_code, _send_message.err_code, 2));
			throw error_no;
		}
	}
	else
	{
		memset(_message, 0x00, sizeof(_message));
		sprintf(_message, "RECV Time Out...Keep..%d", _retry_check);
		throw _message;
	}
}

int C_socket::F_read_socket(int _message_length)
{
	/* 1. Buffer Init */
	memset(_recv_buffer, 0x00, sizeof(_recv_buffer));
	int _recv_length = 0;
	int _remain_length = 0;
	_remain_length = _message_length - _recv_length;

    /* 2. Message Read from Socket */
    while(_recv_length < _message_length)
    {
		_recv_length = recv(_client_socket, &_recv_buffer[_recv_length], _remain_length, 0);
	 
		if(_recv_length < 0)
			if(errno = EAGAIN)
				break;
	}

	/* 3. Recv Message Length Return */
	return _recv_length;
}

int C_socket::F_write_socket()
{
	/* 1. Buffer Init */
	memset(_send_buffer, 0x00, sizeof(_send_buffer));
	int _send_message_length = 0;
	_send_message_length = strlen(_send_message.message_length);
	int _send_length = 0;
	int _remain_length = 0;
	_remain_length = _send_message_length - _send_length;

	/* 2. Buffer Setting */
	strncpy(_send_buffer, _send_message.message_length, _send_message_length);

    /* 3. Message write to Socket */
	while(_send_length < _send_message_length)
	{
		_send_length = send(_client_socket, &_send_buffer[_send_length], _remain_length, 0);

		if(_send_length < 0)
			if(errno = EAGAIN)
				break;
	}

	/* 3. Send Message Length Return */
	return _send_length;
}

int C_socket::F_check_message(int r_jang_status, long r_last_data_count)
{
	/* 1. 장시간 확인 및 마지막 수신 데이터 확인 */
	_jang_status = r_jang_status;
	_last_data_count = r_last_data_count;

	/* 2. Message 유형 구분 */
	_recv_message_type = UNDEFINED;

	if((strncmp(_recv_message.msg_type, "0800", 4) == 0) && (strncmp(_recv_message.opr_type, "001", 3) == 0))
		_recv_message_type = MSG_0800_001;
	else if((strncmp(_recv_message.msg_type, "0800", 4) == 0) && (strncmp(_recv_message.opr_type, "301", 3) == 0))
		_recv_message_type = MSG_0800_301;
	else if((strncmp(_recv_message.msg_type, "0810", 4) == 0) && (strncmp(_recv_message.opr_type, "301", 3) == 0))
		_recv_message_type = MSG_0810_301;
	else if((strncmp(_recv_message.msg_type, "0800", 4) == 0) && (strncmp(_recv_message.opr_type, "040", 3) == 0))
		_recv_message_type = MSG_0800_040;
	else if((strncmp(_recv_message.msg_type, "0810", 4) == 0) && (strncmp(_recv_message.opr_type, "040", 3) == 0))
		_recv_message_type = MSG_0810_040;
	else if((strncmp(_recv_message.msg_type, "0200", 4) == 0) && (strncmp(_recv_message.opr_type, "000", 3) == 0))
		_recv_message_type = MSG_0200_000;
	else if((strncmp(_recv_message.msg_type, "0210", 4) == 0) && (strncmp(_recv_message.opr_type, "000", 3) == 0))
		_recv_message_type = MSG_0210_000;
	else
	{
		memset(_message, 0x00, sizeof(_message));
		sprintf(_message, "Rcv Message type error : %.4s..%.3s", _recv_message.msg_type, _recv_message.opr_type);
		throw _message;
	}

	/* 3. Parsing & 포맷 Error CHECK */
	if(_recv_message_type == MSG_0200_000)
	{
		if(strncmp(_tr_code, _recv_message.data_tr_code, 2) != 0)
		{
			F_set_error_code(PARSING_ERROR);
			throw "[Header] Parsing Error..";
		}

		char _length[5];
		sprintf(_length, _recv_message.message_length, HEADER_LENGTH);
		int _header_message_length = atoi(_length);

		if(strlen(_recv_buffer) != (HEADER_LENGTH + _header_message_length + _data_length))
		{
			F_set_error_code(FORMAT_ERROR);
			throw "[Header] Format Error..";
		}
	}

	/* 4. JANG File CHECK(0200/000, 0800/301 수신만) */
	if(_recv_message_type == MSG_0200_000)
	{
		if(_jang_status == JANG_BEFORE)
		{
			F_set_error_code(MARKET_BEF_ERROR);
			throw "[Header] Market is Not Started..";
		}
		if(_jang_status == JANG_CLOSE)
		{
			F_set_error_code(MARKET_AFT_ERROR);
			throw "[Header] Market is Aleady End..";
		}
	}

	if(_recv_message_type == MSG_0800_301)
	{
		if(_jang_status == JANG_CLOSE)
		{
			F_set_error_code(MARKET_AFT_ERROR);
			throw "[Header] Market is Aleady End..";
		}
	}

	/* 5. HEADER TR-CODE CHECK */
	if(strncasecmp(_recv_message.tr_code, &_header_tr_code, 1) != 0)
	{
		memset(_message, 0x00, sizeof(_message));
		sprintf(_message, "[Header] TR_CODE error..%.3s", _recv_message.tr_code);
		F_set_error_code(TR_CODE_INVALID);
		throw _message;
	}

	/* 6. 기관 ID CHECK */
	if(strncmp(_recv_message.gigwan_id, _company_id, 3) != 0)
	{

		memset(_message, 0x00, sizeof(_message));
		sprintf(_message, "[Header] Gigwan ID error..%.3s", _recv_message.gigwan_id);
		F_set_error_code(GIGWAN_ID_INVALID);
		throw _message;
	}

	/* 7. 전문 & 운용 type CHECK */
	/* (1) 허용 MSG type CHECK */
	if(strncasecmp(_communicate_type, &_recv_gubun, 1) == 0)
	{ /* 수신 프로세스인 경우 허용 MSG type Filtering */
		if((_recv_message_type == MSG_0810_001) || 
		   (_recv_message_type == MSG_0810_301) ||
	 	   (_recv_message_type == MSG_0810_040) ||
	 	   (_recv_message_type == MSG_0210_000))
		{
			memset(_message, 0x00, sizeof(_message));
			sprintf(_message, "Invalid Recv[R] MSG TYPE : %.4s.%.3s", _recv_message.msg_type, _recv_message.opr_type);
			F_set_error_code(MSG_TYPE_INVALID);
			throw _message;
		}
	}
	else
	{ /* 송신 프로세스인 경우 허용 MSG type Filtering */
		if((_recv_message_type == MSG_0810_001) ||
		   (_recv_message_type == MSG_0800_301) ||
		   (_recv_message_type == MSG_0800_040) ||
		   (_recv_message_type == MSG_0200_000))
		{
			memset(_message, 0x00, sizeof(_message));
			sprintf(_message, "Invalid Recv[S] MSG TYPE : %.4s.%.3s", _recv_message.msg_type, _recv_message.opr_type);
			F_set_error_code(MSG_TYPE_INVALID);
			throw _message;
		}

		/* 0800/040(마감요청)을 보내지 않았는데 0810/040(마감요청응답)이 들어오는 경우 */
		if(_recv_message_type == MSG_0810_040)
		{
			if(_send_message_type != MSG_0800_040)
			{
				memset(_message, 0x00, sizeof(_message));
				sprintf(_message, "Invalid Recv[S] OPR TYPE : %.4s.%.3s", _recv_message.msg_type, _recv_message.opr_type);
				F_set_error_code(OPR_TYPE_INVALID);
				throw _message;
			}
		}
	}

	/* (2) (개시 전) 전문/운용 type CHECK */
	if(_connect_status == DISCONNECT)
	{
		if((_recv_message_type == MSG_0800_301) ||
		   (_recv_message_type == MSG_0810_301) ||
		   (_recv_message_type == MSG_0800_040) ||
		   (_recv_message_type == MSG_0810_040) ||
		   (_recv_message_type == MSG_0200_000) ||
		   (_recv_message_type == MSG_0210_000))
		{
			memset(_message, 0x00, sizeof(_message));
			sprintf(_message, "RCV data before 0800/001 : %.4s %.3s", _recv_message.msg_type, _recv_message.opr_type);
			F_set_error_code(MSG_TYPE_INVALID);
			throw _message;
		}
	}

	/* 8. (Header부) DATA Sequence Number Check */
	char _data_no[9];
	long _recv_data_no;

	memset(_data_no, 0x00, sizeof(_data_no));
	strncpy(_data_no, _recv_message.data_no, 8);
	_recv_data_no = atol(_data_no);

	if((_recv_message_type != MSG_0800_301) && (_recv_message_type != MSG_0810_301))
	{
		if(strncasecmp(_communicate_type, &_send_gubun, 1) == 0)
		{ /* 송신 프로세스인 경우 */
			if(_recv_data_no != _last_data_count)
			{ /* 마지막 count와 같아야 함 */
				memset(_message, 0x00, sizeof(_message));
				sprintf(_message, "RCV [S header] data no : %ld..%ld", _recv_data_no, _last_data_count);
				F_set_error_code(DATA_NO_INVALID);
				throw _message;
			}
		}
		else
		{ /* 수신 프로세스인 경우 */
			if((_recv_message_type == MSG_0800_001) || (_recv_message_type == MSG_0800_040))
			{ /* 개시와 종료인 경우, 마지막 count와 같아야 함 */
				if(_recv_data_no != _last_data_count)
				{
					memset(_message, 0x00, sizeof(_message));
					sprintf(_message, "RCV [R header] data no : %ld..%ld", _recv_data_no, _last_data_count);
					F_set_error_code(DATA_NO_INVALID);
					throw _message;
				}
			}
			else
			{ /* 일반 수신인 경우, 마지막 전송 "count + 1" 과 같아야 함 */
				if(_recv_data_no != (_last_data_count + 1))
				{
					memset(_message, 0x00, sizeof(_message));
					sprintf(_message, "RCV [R header] data no : %ld..%ld", _recv_data_no, _last_data_count + 1);
					F_set_error_code(DATA_NO_INVALID);
					throw _message;
				}
			}
		}
	}
	
	/* 9. (Header부) DATA Count check */
	/* _recv_message.data_cnt = Data Block Count */ 
	/* 데이터가 없을 경우 Block Count는 0, 있을 경우 1 */
	char _data_cnt[3];
	int _recv_data_cnt;

	memset(_data_cnt, 0x00, sizeof(_data_cnt));
	strncpy(_data_cnt, _recv_message.data_cnt, 2);
	_recv_data_cnt = atoi(_data_cnt);

	if((_recv_message_type != MSG_0200_000) && (_recv_message_type != MSG_0210_000))
	{ /* 0200 이외에는 Block Count 는 '0' 이어야 함 */
		if(_recv_data_cnt != 0)
		{
			memset(_message, 0x00, sizeof(_message));
			sprintf(_message, "RCV Data Block Cnt Error : %d..", _recv_data_cnt);
			F_set_error_code(DATA_CNT_INVALID);
			throw _message;
		}
	}
	else
	{ /* 0200의 Block Count 는 '1' 이어야 함 */
		if(_recv_data_cnt != BLOCK_COUNT)
		{
			memset(_message, 0x00, sizeof(_message));
			sprintf(_message, "RCV Data Block Cnt Error : %d..", _recv_data_cnt);
			F_set_error_code(DATA_CNT_INVALID);
			throw _message;
		}
	}

	/* 10. INNER SEQUENCE CHECK */
    char _data_seq[9];
	long _recv_data_seq;

	memset(_data_seq, 0x00, sizeof(_data_seq));
	strncpy(_data_seq, _recv_message.data_seq, 8);
	_recv_data_seq = atol(_data_seq);

	if(_recv_message_type == MSG_0200_000)
	{ /* 0200/000인 경우만 check */
		if(_recv_data_no != _recv_data_seq)
		{
			memset(_message, 0x00, sizeof(_message));
			sprintf(_message, "RCV [Data] Data SEQ Error : %ld, %ld", _recv_data_no, _recv_data_seq);
			F_set_error_code(SEQ_ERROR);
			throw _message;
		}
	}

	F_set_error_code(NO_ERROR);

	if(strncasecmp(_communicate_type, &_send_gubun, 1) == 0)
	{ /* 송신 통신 일 경우 */
		if(F_get_error_code() == FAIL)
		{ /* Message Check시, 정상 인 경우 (NO_ERROR일 경우 FAIL 반환) */
			if(_recv_message_type == MSG_0810_040)
			{ /* 종료 수신 일 경우 */
				return END;
			}
		}
	}

	return SUCCESS;
}

int C_socket::F_set_message()
{
	/* 1. Message_length */
	strncpy(_send_message.message_length, _recv_message.message_length, 4);

	/* 2. TR-Code Set */
    strncpy(_send_message.tr_code, "N        ", 9);

	/* 3. Time Setting */
	_date_time.F_update_date_time();
	_date = _date_time.F_get_date();
	_time = _date_time.F_get_time();
	strncpy(_send_message.time, &_date[2], 6);
	strncpy(&_send_message.time[6], _time, 6);

    /* 4. Retry Set */
	char _temp_retry[3];
	memset(_temp_retry, 0x00, sizeof(_temp_retry));
	sprintf(_temp_retry, "%.2d", _retry_check);
    strncpy(_send_message.retry_cnt, _temp_retry, 2);

	if(strncasecmp(_communicate_type, &_recv_gubun, 1) == 0)
	{
		/* 5. Error Code Set */
		switch(_error_code)
    	{
    	    case NO_ERROR :
    	        strncpy(_send_message.err_code, "00", 2);
    	        break;
    	    case SEQ_ERROR : /* DATA SEQ Error */
    	        strncpy(_send_message.err_code, "01", 2);
    	        break;
    	    case CNT_ERROR : /* DATA 건수 Error */
    	        strncpy(_send_message.err_code, "02", 2);
    	        break;
    	    case MARKET_BEF_ERROR : /* 장개시 전 Error */
    	        strncpy(_send_message.err_code, "03", 2);
    	        break; 
    	    case MARKET_AFT_ERROR : /* 장마감 Error */
    	        strncpy(_send_message.err_code, "04", 2);
    	        break; 
    	    case FORMAT_ERROR : /* 전문 길이 Error */
    	        strncpy(_send_message.err_code, "14", 2);
    	        break; 
    	    case PARSING_ERROR : /* 전문TR Error */
    	        strncpy(_send_message.err_code, "18", 2);
    	        break; 
    	    case TR_CODE_INVALID :
    	        strncpy(_send_message.err_code, "91", 2);
    	        break;
    	    case GIGWAN_ID_INVALID :
    	        strncpy(_send_message.err_code, "92", 2);
    	        break;
    	    case MSG_TYPE_INVALID :
    	        strncpy(_send_message.err_code, "93", 2);
    	        break;
    	    case DATA_NO_INVALID : /* DATA Number Error */
    	        strncpy(_send_message.err_code, "98", 2);
    	        break; 
    	    case DATA_CNT_INVALID : /* DATA 개수 Error */
    	        strncpy(_send_message.err_code, "99", 2);
    	        break; 
    	    default :
				memset(_message, 0x00, sizeof(_message));
				sprintf(_message, "Invalid Error Code..%d(In Message Set)", _error_code);
				throw _message;
    	}

		/* 6. 기관 ID Set */
    	strncpy(_send_message.gigwan_id, _recv_message.gigwan_id, 3);

		/* 7. 전문 type set */
		switch(_recv_message_type)
    	{
    	    case MSG_0800_001 :
    	        strncpy(_send_message.msg_type, "0810", 4);
    	        strncpy(_send_message.opr_type, "001", 3);
    	        break;
    	    case MSG_0800_301 :
    	        strncpy(_send_message.msg_type, "0810", 4);
    	        strncpy(_send_message.opr_type, "301", 3);
    	        break;
    	    case MSG_0800_040 :
    	        strncpy(_send_message.msg_type, "0810", 4);
    	        strncpy(_send_message.opr_type, "040", 3);
    	        break;
    	    case MSG_0200_000 :
    	        strncpy(_send_message.msg_type, "0210", 4);
    	        strncpy(_send_message.opr_type, "000", 3);
    	        break ;
    	    default :
				memset(_message, 0x00, sizeof(_message));
				sprintf(_message, "Invalid Message Type..%d (In Message Set)", _recv_message_type);
				throw _message;
    	}

		/* 8. Data 번호 및 갯수 */
    	strncpy(_send_message.data_no, _recv_message.data_no, 8);
    	strncpy(_send_message.data_cnt, _recv_message.data_cnt, 2);
	}
	else
	{
		/* 5. Error Code Set */
		strncpy(_send_message.err_code, "00", 2);

		/* 6. 기관 ID Set */
    	strncpy(_send_message.gigwan_id, _company_id, 3);

		/* 7. 전문 type set */
		switch(_send_message_type)
    	{
    	    case MSG_0800_001 :
    	        strncpy(_send_message.msg_type, "0800", 4);
    	        strncpy(_send_message.opr_type, "001", 3);
    	        break;
    	    case MSG_0800_301 :
    	        strncpy(_send_message.msg_type, "0800", 4);
    	        strncpy(_send_message.opr_type, "301", 3);
    	        break;
    	    case MSG_0800_040 :
    	        strncpy(_send_message.msg_type, "0800", 4);
    	        strncpy(_send_message.opr_type, "040", 3);
    	        break;
    	    case MSG_0200_000 :
    	        strncpy(_send_message.msg_type, "0200", 4);
    	        strncpy(_send_message.opr_type, "000", 3);
    	        break ;
    	    default :
				memset(_message, 0x00, sizeof(_message));
				sprintf(_message, "Invalid Message Type..%d (In Message Set)", _recv_message_type);
				throw _message;
    	}

		/* 8. Data 번호 및 갯수 */
		char _temp_count[9];
		sprintf(_temp_count, "%.8ld", _last_data_count + 1);
    	strncpy(_send_message.data_no, _temp_count, 8);
    	//strncpy(_send_message.data_cnt, _recv_message.data_cnt, 2);
	}

	return SUCCESS;
}

int C_socket::F_check_socket(int _time)
{
	/* Buffer status 확인 */
	/* _time이 -1일 경우 Event 발생 시점까지 대기, 30000일 경우 30초 대기 */
	_event_cnt = epoll_wait(_epfd, _ep_events, EPOLL_SIZE, _time); 
	if(_event_cnt < 0) /* EPOLL Error */
	{
		memset(_message, 0x00, sizeof(_message));
		sprintf(_message, "Socket EPOLL Error : %s", strerror(errno));
		throw _message;
	}
	else if(_event_cnt == 0) /* Time Out */
	{
		return TIMEOUT;
	}
	else if(_event_cnt > 0) /* Event 발생 */
	{
		for(int i = 0; i < _event_cnt; i++)
		{
			if(_ep_events[i].data.fd == _server_socket)
			{
				_client_address_size = sizeof(_client_address);
				_client_socket = accept(_server_socket, (struct sockaddr*)&_client_address, &_client_address_size);
				if(_client_socket == -1)
				{
					memset(_message, 0x00, sizeof(_message));
					sprintf(_message, "Server Socket Accept Error : %s", strerror(errno));
					throw _message;
				}
				else
				{
					F_set_non_blocking_mode(_client_socket);
					_event.events = EPOLLIN | EPOLLET;
					_event.data.fd = _client_socket;
					epoll_ctl(_epfd, EPOLL_CTL_ADD, _client_socket, &_event);
					_link_status = CONNECT;

					return SUCCESS;
				}
			}
			else if(_ep_events[i].data.fd == _client_socket)
			{
				return SUCCESS;
			}
		}
	}
}

void C_socket::F_set_non_blocking_mode(int _socket)
{
	int flag = fcntl(_socket, F_GETFL, 0);
	fcntl(_socket, F_SETFL, flag | O_NONBLOCK);
}

char* C_socket::F_put_log_recv_message()
{
	memset(_message, 0x00, sizeof(_message));

	sprintf(_message, "RECV %.4s %.1s %.3s %.4s %.3s %.2s %.12s %.2s %.8s %.2s.%ld", _recv_message.message_length, _recv_message.tr_code, _recv_message.gigwan_id, _recv_message.msg_type, _recv_message.opr_type, _recv_message.err_code, _recv_message.time, _recv_message.retry_cnt, _recv_message.data_no, _recv_message.data_cnt, strlen(_recv_message.message_length));

	return _message;
}			

char* C_socket::F_put_log_send_message()
{
	memset(_message, 0x00, sizeof(_message));
	
	sprintf(_message, "SEND %.4s %.1s %.3s %.4s %.3s %.2s %.12s %.2s %.8s %.2s.%ld", _send_message.message_length, _send_message.tr_code, _send_message.gigwan_id, _send_message.msg_type, _send_message.opr_type, _send_message.err_code, _send_message.time, _send_message.retry_cnt, _send_message.data_no, _send_message.data_cnt, strlen(_send_message.message_length));
	
	return _message;
}

int C_socket::F_link_status()
{
	return _link_status;
}

void C_socket::F_set_connect_status()
{
	_connect_status = CONNECT;
}

void C_socket::F_set_error_code(int r_error_code)
{
	_error_code = r_error_code;
}

int C_socket::F_get_error_code()
{
	if(_error_code == NO_ERROR)
		return FAIL;
	else
		return _error_code;
}

int C_socket::F_get_message_type()
{
	if(strncasecmp(_communicate_type, &_recv_gubun, 1) == 0)
	{
		if(_recv_message_type == MSG_0800_001)
			return MSG_0810_001;
		return _recv_message_type;
	}
	else
		return _send_message_type;
}
