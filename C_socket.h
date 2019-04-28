#ifndef __SOCKET_HEADER__
#define __SOCKET_HEADER__

#include"common.h"
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

		/* Setting Config from Config File */
		int F_set_config_information(int r_data_length, char* r_ip_number, char* r_port_number, char* r_company_id, char* r_tr_code, char* r_communicate_type);

		/* Socket Create, Bind, Listen */
		char* F_create_socket();

		/* Socket Accept */
		char* F_accept_socket();

		/* Init Retry Count */
		void F_setting_retry_init();

		/* Receive Message */
		int F_recv_message();

		/* Send Message */
		int F_send_message();

		/* TimeOut Event */
		void F_event_timeout();

		/* Read Socket To Buffer */
        int F_read_socket(int _message_length);

		/* Write Socket From Buffer */
		int F_write_socket();

		/* Check Receive Message Header */
		int F_check_message(int r_jang_status, long r_last_data_count);

		/* Setting Send Message */
		int F_set_message();

		/* Check Socket Status */
		int F_check_socket(int _time);

		/* Modify Socket Non Blocking Mode */
		void F_set_non_blocking_mode(int _socket);

		/* Setting Log Message for Receive Message */
		char* F_setting_log_recv_message();

		/* Setting Log Message for Send Message */
		char* F_setting_log_send_message();

		/* Setting Link Status */
		void F_set_link_status(int _status);

		/* Return Link Status */
		int F_get_link_status();
		
		/* Setting Connect Status */
		void F_set_connect_status(int _status);

		/* Return Connect Status */
		int F_get_connect_status();
		
		/* Setting Error Code */
		void F_set_error_code(int r_error_code);

		/* Return Error Code */
		int F_get_error_code();

		/* Return Message Type */
		int F_get_message_type();

		/* Setting _send_message_type */
		void F_set_send_message_type(int _type);
};

#endif
