#ifndef __CONTROL_HEADER__
#define __CONTROL_HEADER__

#include"common.h"
#include"C_config.h"
#include"C_log.h"
#include"C_msg.h"
#include"C_data.h"
#include"C_cnt.h"
#include"C_jang.h"
#include"C_socket.h"

class C_control
{
	private :
		/* 1. Status Message Variable */
		char _message[150];

		/* 2. Class Variable */
		C_config _config;
		C_log _log;
		C_msg _msg;
		C_data _data;
		C_cnt _cnt;
		C_jang _jang;
		C_socket _socket;

		/* 3. ETC */
		char **_key;
		char **_config_path;
		int _jang_status;
		int _error_code;

	public :
		C_control(char *argv[]);

		void F_set_check_socket_information();

		void F_open_file();

		void F_get_cnt();

		void F_get_jang();

        void F_start();

		int F_create_socket();

		void F_read_message();

		void F_check_message();

		void F_send_message();

		void F_set_send_message_type();

		void F_update_cnt(int msg_type);

		char* F_get_communicate_type();

		int F_get_link_status();
		
		void F_stop_process(int option);
};

#endif
