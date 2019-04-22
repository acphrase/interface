#ifndef __SOCKET_HEADER__
#define __SOCKET_HEADER__

#include"main.h"
#include"C_time.h"

/*----------------------------------------------------------------------------*/
/*----------------------------- 1. Message 구분 ------------------------------*/
/*----------------------------------------------------------------------------*/
#define SND_RCV_CHK			500		/* 500: 송신시 recv 있음 */
#define UNDEFINED			-1
#define HEADER_LENGTH		4		/* 전문길이정보 */
#define BLOCK_COUNT			1		/* 전문블록 갯수 */

/*----------------------------------------------------------------------------*/
/*----------------------------- 2. TIMEOUT 구분 ------------------------------*/
/*----------------------------------------------------------------------------*/
#define WAIT_TIMEOUT		-1
#define RECV_TIMEOUT		30000
#define SEND_TIMEOUT		30000
#define CHECK_TIMEOUT		1
//#define RECV_TIMEOUT		3000
//#define SEND_TIMEOUT		3000

/*----------------------------------------------------------------------------*/
/*----------------------------- 3. ETC ---------------------------------------*/
/*----------------------------------------------------------------------------*/
#define HEADER_TR_CODE		'N'
#define BUF_SIZE			4000
#define EPOLL_SIZE			5

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
  char snd_data					[3951];
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
		int _recv_length;
		int _recv_message_type;
		int _send_message_type;

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
		char* _ip_number;
		char* _port_number;
		char* _company_id;
		char* _tr_code;
		char* _communicate_type;

		/* 8. Time Class */
		C_time _date_time;
		char* _date;
		char* _time;

		/* 9. ETC */
        char _recv_gubun = RECV_GUBUN;
        char _send_gubun = SEND_GUBUN;
        char _header_tr_code = HEADER_TR_CODE;
		int	_retry_check = 0; /* 재전송 시도 횟수 변수 */
		int _error_code = 0; /* Message Error Code */
		
	public :
		C_socket();
		~C_socket();
		int F_set_config_information(int r_data_length, char* r_ip_number, char* r_port_number, char* r_company_id, char* r_tr_code, char* r_communicate_type);
		char* F_create_socket();
		char* F_accept_socket();
		void F_put_retry_init();
		int F_recv_message();
		int F_send_message();
		void F_event_timeout();
        int F_read_socket(int _message_length);
		int F_write_socket();
		int F_check_message(int r_jang_status, long r_last_data_count);
		int F_set_message();
		int F_check_socket(int _time);
		void F_set_non_blocking_mode(int _socket);
		char* F_put_log_recv_message();
		char* F_put_log_send_message();
		int F_link_status();
		void F_set_connect_status();
		void F_set_error_code(int r_error_code);
		int F_get_error_code();
		int F_get_message_type();
};

#endif
