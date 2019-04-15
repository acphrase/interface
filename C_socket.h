#include"main.h"

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
		struct sockaddr_in _server_address;
		struct sockaddr_in _client_address;
		int _server_socket;
		int _client_socket;
		char _message[150];
		socklen_t _client_address_size;
        char _recv_buffer[4000];
        char _send_buffer[4000];
        RECV_MESSAGE_DEF _recv_message;
        SEND_MESSAGE_DEF _send_message;
        SEND_MESSAGE_PB_DEF _send_message_pb;

	public :
		C_socket()
		{
			memset(&_server_address, 0x00, sizeof(_server_address));
			memset(&_client_address, 0x00, sizeof(_client_address));
			memset(_message, 0x00, sizeof(_message));
			_server_socket = 0;
			_client_socket = 0;
			_client_address_size = 0;
            memset(_recv_buffer, 0x00, sizeof(_recv_buffer));
            memset(_send_buffer, 0x00, sizeof(_send_buffer));
            memset(&_recv_message, 0x00, sizeof(_recv_message));
            memset(&_send_message, 0x00, sizeof(_send_message));
            memset(&_send_message_pb, 0x00, sizeof(_send_message_pb));
		}

		char* F_create_socket(char *ip, char *port)
		{
			int state = 0;
			int _option = 0;
			int _check_option = 0;
			socklen_t _option_length;
			socklen_t _check_length;

			//cout << ip << endl;
			//cout << port << endl;

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
			if(listen(_server_socket, 2) == -1)
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
			_client_address_size = sizeof(_client_address);
			_client_socket = accept(_server_socket, (struct sockaddr*)&_client_address, &_client_address_size);
			if(_client_socket == -1)
			{
				memset(_message, 0x00, sizeof(_message));
				sprintf(_message, "Socket Accept Error : %s", strerror(errno));
				throw _message;
			}
			else
			{
				memset(_message, 0x00, sizeof(_message));
				sprintf(_message, "TCP/IP Connected ( IP:%s port:%d )", inet_ntoa(_client_address.sin_addr), ntohs(_client_address.sin_port));
				return _message;
			}
		}

        void F_read_socket(int _header_length)
        {
            int _recv_length = 0;
            int _remain_length = _header_length - _recv_length;

	        /* 1. Message length read */
            while(_remain_length < _header_length)
            {
                _recv_length = recv(_client_socket, &_recv_buffer[_recv_length], _remain_length, 0);
                if(_recv_length < 0)
                {
                    memset(_message, 0x00, sizeof(_message));
                    sprintf(_message, "New Socket Message Length Read Error : %d", _recv_length);
                    throw _message;
                }
            }
        }
};
