#ifndef __SOCKET_HEADER__
#define __SOCKET_HEADER__

#include"main.h"
#include"C_time.h"

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
