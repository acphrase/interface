#include"main.h"

/*----------------------------------------------------------------------------*/
/*----------------------------- 1. Message 구분 ------------------------------*/
/*----------------------------------------------------------------------------*/
#define SND_RCV_CHK           500    /* 500: 송신시 recv 있음 */
#define UNDEFINED             -1
#define MESSAGE_LENGTH        4      /* 전문길이정보 */
#define BLOCK_COUNT           1      /* 전문블록 갯수 */
#define MSG_0800_001          8001
#define MSG_0810_001          8101
#define MSG_0800_301          8031
#define MSG_0810_301          8131
#define MSG_0800_040          8040
#define MSG_0810_040          8140
#define MSG_0200_000          2000
#define MSG_0210_000          2100
#define MSG_START             11
#define MSG_ERR               99
#define HEADER_LENGTH		  4
#define BUF_SIZE			  4000
#define EPOLL_SIZE			  5
#define RECV_GUBUN			  'r'
#define SEND_GUBUN			  's'
#define HEADER_TR_CODE		  'N'

/*----------------------------------------------------------------------------*/
/*----------------------------- 2. Message error 구분 ------------------------*/
/*----------------------------------------------------------------------------*/
#define NO_ERROR              0
#define SEQ_ERROR             1
#define CNT_ERROR             2
#define MARKET_BEF_ERROR      3
#define MARKET_AFT_ERROR      4
#define FORMAT_ERROR          14
#define PARSING_ERROR         18
#define TR_CODE_INVALID       91
#define GIGWAN_ID_INVALID     92
#define MSG_TYPE_INVALID      93
#define OPR_TYPE_INVALID      94
#define DATA_NO_INVALID       98
#define DATA_CNT_INVALID      99


struct RECV_MESSAGE_DEF
{ /* 수신 메세지 definition */
  char message_length           [4];
  char tr_code                  [9];   /* TR-code : "N       " */
  char gigwan_id                [3];   /* 기관 id : 999 */
  char msg_type                 [4];   /* 전문 type : 0800,0810,0200,0210 */
  char opr_type                 [3];   /* 운용 type : 000,001,002,040,301 */
  char err_code                 [2];   /* 오류 code : 00:정상,기타:ERROR */
  char time                     [12];  /* 날짜 및 시간 : yymmddhhmmss */
  char retry_cnt                [2];   /* 재송횟수 */
  char data_no                  [8];   /* (Header부)DATA 번호 */
  char data_cnt                 [2];   /* DATA 갯수 */
  char data_seq                 [8];   /* (Data부) DATA seq */
  char data_tr_code             [2];   /* (Data부) TR code */
  char data_sub_tr_code         [2];   /* (Data부) SUB TR code */
  char rcv_data                 [3939];
}; /* 수신 버퍼 총 4000 byte */

struct SEND_MESSAGE_DEF
{ /* 송신 메세지 definition */
  char message_length           [4];
  char tr_code                  [9];   /* TR-code : "N       " */
  char gigwan_id                [3];   /* 기관 id : 999 */
  char msg_type                 [4];   /* 전문 type : 0800,0810,0200,0210 */
  char opr_type                 [3];   /* 운용 type : 000,001,002,040,301 */
  char err_code                 [2];   /* 오류 code : 00:정상,기타:ERROR */
  char time                     [12];  /* 날짜 및 시간 : yymmddhhmmss */
  char retry_cnt                [2];   /* 재송횟수 */
  char data_no                  [8];   /* DATA SEQ */
  char data_cnt                 [2];   /* DATA 갯수 */
  char snd_data              [3951];
}; /* 송신 버퍼 총 4000 byte */

struct SEND_MESSAGE_PB_DEF
{ /* PB 송신 메세지 definition */
  char snd_pb_header           [61];
  char snd_pb_comp_id          [3];
  char snd_pb_data             [3936];
}; /* 송신 버퍼 총 4000 byte */

class C_socket
{
	private :
		/* 1. Status Message Variable */
		char _message[150];

		/* 2. Socket Variable */
		struct sockaddr_in _server_address;
		struct sockaddr_in _client_address;
		int _server_socket;
		int _client_socket;
		socklen_t _client_address_size;

		/* 3. Socket Status Variable */
        int _link_status;				/* Socket 생성 후 Link 유무 변수 */
        int _connect_status;            /* Socket Link 후 개시 유무 변수 */

		/* 4. Buffer Variable */
        char _recv_buffer[BUF_SIZE];
        char _send_buffer[BUF_SIZE];
        RECV_MESSAGE_DEF _recv_message;
        SEND_MESSAGE_DEF _send_message;
        SEND_MESSAGE_PB_DEF _send_message_pb;
		int _recv_length;
		int _recv_message_type;

		/* 5. EPOLL Variable */
		struct epoll_event *_ep_events;
		struct epoll_event _event;
		int _epfd;
		int _event_cnt;

		/* 6. Market Status Variable */
		int _jang_status;

		/* 7. Config Status Variable */
		int _data_length;
		long _last_data_count;
		char* _company_id;
		char* _tr_code;
		char* _communicate_type;
		
	public :
		C_socket()
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
            memset(&_send_message_pb, 0x00, sizeof(_send_message_pb));
			_recv_length = 0;
			_recv_message_type = UNDEFINED;

			_ep_events = new struct epoll_event;
			_epfd = epoll_create(EPOLL_SIZE);
			_event_cnt = 0;
		}

		~C_socket()
		{
			delete _ep_events;
			close(_server_socket);
			close(_client_socket);
		}

		int F_set_config_information(int r_data_length, char* r_company_id, char* r_tr_code, char* r_communicate_type)
		{
			_data_length = r_data_length;
			_company_id = r_company_id;
			_tr_code = r_tr_code;
			_communicate_type = r_communicate_type;

			return SUCCESS;
		}

		char* F_create_socket(char *ip, char *port)
		{
			int state = 0;
			int _option = 0;
			int _check_option = 0;
			socklen_t _option_length;
			socklen_t _check_length;

			/* 1. Socket에 할당 할 정보 Setting */
			_server_address.sin_family = AF_INET;	/* 주소 체계 : IPv4 */
			_server_address.sin_addr.s_addr = inet_addr(ip);	/* server ip (Network bytes 주소로 변환) */
			_server_address.sin_port = htons((unsigned short)atoi(port));	/* server port number */

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

		char* F_accept_socket()
		{
			/* 1. Accept Socket */
			/* server socket에 대기중인 client의 정보를 _client_address에 저장 */
			/* 접속 허용하여 데이터 송수신 시작 */
			F_set_non_blocking_mode(_server_socket);
			_event.events = EPOLLIN;
			_event.data.fd = _server_socket;
			epoll_ctl(_epfd, EPOLL_CTL_ADD, _server_socket, &_event);

			_event_cnt = epoll_wait(_epfd, _ep_events, EPOLL_SIZE, -1); /* 연결 될 때 까지 무한 대기 */ 
			if(_event_cnt < 0)
			{
				memset(_message, 0x00, sizeof(_message));
				sprintf(_message, "Socket EPOLL Error : %s", strerror(errno));
				throw _message;
			}
			else if(_event_cnt > 0)
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
							memset(_message, 0x00, sizeof(_message));
							sprintf(_message, "TCP/IP Connected ( IP:%s port:%d )", inet_ntoa(_client_address.sin_addr), ntohs(_client_address.sin_port));

							F_set_non_blocking_mode(_client_socket);
							_event.events = EPOLLIN | EPOLLET;
							_event.data.fd = _client_socket;
							epoll_ctl(_epfd, EPOLL_CTL_ADD, _client_socket, &_event);
							_link_status = CONNECT;

							return _message;
						}
					}
				}
			}
		}

		
		
		int F_recv_message(int _time, int r_jang_status, long r_last_data_count)
		{
			_jang_status = r_jang_status;
			_last_data_count = r_last_data_count;
			int _header_length = 0;
			int _message_length = 0;
			int _check_result = 0;

			/* 수신 가능한 상태인지 _client_socket 확인 */
			/* 수신 할 데이터가 존재 할 경우 "SUCCESS" 반환 */
			/* 전달한 시간을 초과 할 경우 "TIMEOUT" 반환 */
			_check_result = F_check_descriptor(_time);
			if(_check_result == SUCCESS)
			{	
				_header_length = F_read_socket(HEADER_LENGTH);
				if(_header_length > 0)
				{
					strncpy(_recv_message.message_length, _recv_buffer, HEADER_LENGTH);

					_message_length = F_read_socket(atoi(_recv_buffer));
					if(_message_length > 0)
					{
						strncpy(_recv_message.tr_code, _recv_buffer, sizeof(_recv_message) - HEADER_LENGTH);
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

				//F_check_message();
				//F_put_log_message();

				return SUCCESS;
			}
			else if(_check_result == TIMEOUT)
			{
				return TIMEOUT;
			}
			else
			{
				throw "Socket Status Error..";
			}
		}


        int F_read_socket(int _message_length)
        {
			/* 1. Buffer 초기화 */
			memset(&_recv_buffer, 0x00, sizeof(_recv_buffer));
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

			/* 3. Message Length Return */
			return _recv_length;
        }

		int F_check_message()
		{
            char _recv_gubun = RECV_GUBUN;
            char _send_gubun = SEND_GUBUN;
            char _header_tr_code = HEADER_TR_CODE;
			_recv_message_type = UNDEFINED;

			/* 1. Message 유형 구분 */
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

			/* 2. Parsing & 포맷 Error CHECK */
			if(_recv_message_type == MSG_0200_000)
			{
				if(strncmp(_tr_code, _recv_message.data_tr_code, 2) != 0)
					throw "[Header] Parsing Error..";

				char _length[5];
				sprintf(_length, _recv_message.message_length, HEADER_LENGTH);
				int _header_message_length = atoi(_length);

				if(strlen(_recv_buffer) != (HEADER_LENGTH + _header_message_length + _data_length))
					throw "[Header] Format Error..";
			}

			/* 3. JANG File CHECK(0200/000, 0800/301 수신만) */
			if(_recv_message_type == MSG_0200_000)
			{
				if(_jang_status == JANG_BEFORE)
					throw "[Header] Market is Not Started..";
				if(_jang_status == JANG_CLOSE)
					throw "[Header] Market is Aleady End..";
			}

			if(_recv_message_type == MSG_0800_301)
			{
				if(_jang_status == JANG_CLOSE)
					throw "[Header] Market is Aleady End..";
			}

			/* 4. HEADER TR-CODE CHECK */
			if(strncmp(_recv_message.tr_code, &_header_tr_code, 1) != 0)
			{
				memset(_message, 0x00, sizeof(_message));
				sprintf(_message, "[Header] TR_CODE error..%.3s", _recv_message.tr_code);
				throw _message;
			}

			/* 5. 기관 ID CHECK */
			if(strncmp(_recv_message.gigwan_id, _company_id, 3) != 0)
			{

				memset(_message, 0x00, sizeof(_message));
				sprintf(_message, "[Header] Gigwan ID error..%.3s", _recv_message.gigwan_id);
				throw _message;
			}

			/* 6. 전문 & 운용 type CHECK */
			/* (1) 허용 MSG type CHECK */
			if(strncmp(_communicate_type, &_recv_gubun, 1) == 0)
			{ /* 수신 프로세스인 경우 허용 MSG type Filtering */
				if((_recv_message_type == MSG_0810_001) || 
				   (_recv_message_type == MSG_0810_301) ||
			 	   (_recv_message_type == MSG_0810_040) ||
			 	   (_recv_message_type == MSG_0210_000))
				{
					memset(_message, 0x00, sizeof(_message));
					sprintf(_message, "Invalid Recv[R] MSG TYPE : %.4s.%.3s", _recv_message.msg_type, _recv_message.opr_type);
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
					throw _message;
				}

				/* 0800/040(마감요청)을 보내지 않았는데 0810/040(마감요청응답)이 들어오는 경우 */
				//if(_recv_message_type == MSG_0810_040)
				//{
				//	if(_send_message_type != MSG_0800_040)
				//	{
				//		memset(_message, 0x00, sizeof(_message));
				//		sprintf(_message, "Invalid Recv[S] OPR TYPE : %.4s.%.3s", _recv_message.msg_type, _recv_message.opr_type);
				//		throw _message;
				//	}
				//}
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
					throw _message;
				}
			}

			/* 7. (Header부) DATA Sequence Number Check */
			char _data_no[9];
			long _recv_data_no;

			memset(_data_no, 0x00, sizeof(_data_no));
			strncpy(_data_no, _recv_message.data_no, 8);
			_recv_data_no = atol(_data_no);

			if((_recv_message_type != MSG_0800_301) && (_recv_message_type != MSG_0810_301))
			{
				if(strncmp(_communicate_type, &_send_gubun, 1) == 0)
				{ /* 송신 프로세스인 경우 */
					if(_recv_data_no != _last_data_count)
					{ /* 마지막 count와 같아야 함 */
						memset(_message, 0x00, sizeof(_message));
						sprintf(_message, "RCV [S header] data no : %ld..%ld", _recv_data_no, _last_data_count);
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
							throw _message;
						}
					}
					else
					{ /* 일반 수신인 경우, 마지막 전송 "count + 1" 과 같아야 함 */
						if(_recv_data_no != (_last_data_count + 1))
						{
							memset(_message, 0x00, sizeof(_message));
							sprintf(_message, "RCV [R header] data no : %ld..%ld", _recv_data_no, _last_data_count + 1);
							throw _message;
						}
					}
				}
			}
			
			/* 8. (Header부) DATA Count check */
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
					throw _message;
				}
			}
			else
			{ /* 0200의 Block Count 는 '1' 이어야 함 */
				if(_recv_data_cnt != BLOCK_COUNT)
				{
					memset(_message, 0x00, sizeof(_message));
					sprintf(_message, "RCV Data Block Cnt Error : %d..", _recv_data_cnt);
					throw _message;
				}
			}

			/* 9. INNER SEQUENCE CHECK */
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
					throw _message;
				}
			}


			/* 10. 개시 응답일 경우 Connect 상태 변수 변경 */
			if(_recv_message_type == MSG_0810_001)
			{
				_connect_status = CONNECT;
			}

			return SUCCESS;
		}

		int F_check_descriptor(int _time)
		{
			/* Buffer status 확인 */
			/* _time이 -1일 경우 Event 발생 시점까지 대기, 30000일 경우 30초 대기 */
			_event_cnt = epoll_wait(_epfd, _ep_events, EPOLL_SIZE, _time); 
			if(_event_cnt < 0) /* EPOLL Error */
			{
				memset(_message, 0x00, sizeof(_message));
				sprintf(_message, "Client Socket EPOLL Error : %s", strerror(errno));
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
					if(_ep_events[i].data.fd == _client_socket)
					{
						return SUCCESS;
					}
				}
			}
		}

		void F_set_non_blocking_mode(int _socket)
		{
			int flag = fcntl(_socket, F_GETFL, 0);
			fcntl(_socket, F_SETFL, flag | O_NONBLOCK);
		}

		char* F_put_log_message()
		{
            char _recv_gubun = RECV_GUBUN;
			memset(_message, 0x00, sizeof(_message));

			if(strncmp(_communicate_type, &_recv_gubun, 1) == 0)
			{ /* 수신 상황일 경우 */
				sprintf(_message, "RECV %.4s %.1s %.3s %.4s %.3s %.2s %.12s %.2s %.8s %.2s.%ld", _recv_message.message_length, _recv_message.tr_code, _recv_message.gigwan_id, _recv_message.msg_type, _recv_message.opr_type, _recv_message.err_code, _recv_message.time, _recv_message.retry_cnt, _recv_message.data_no, _recv_message.data_cnt, strlen(_recv_buffer));
			}
			else
			{ /* 송신 상황일 경우 */
				sprintf(_message, "RECV %.4s %.1s %.3s %.4s %.3s %.2s %.12s %.2s %.8s %.2s.%ld", _send_message.message_length, _send_message.tr_code, _send_message.gigwan_id, _send_message.msg_type, _send_message.opr_type, _send_message.err_code, _send_message.time, _send_message.retry_cnt, _send_message.data_no, _send_message.data_cnt, strlen(_send_buffer));
			}

			return _message;
		}			
		
		void F_send_socket()
		{
		}

		int F_link_status()
		{
			return _link_status;
		}

		int F_connect_status()
		{
			return _connect_status;
		}
};
