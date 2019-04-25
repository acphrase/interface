#ifndef __OPEN_CONFIG_HEADER__
#define __OPEN_CONFIG_HEADER__

#include"main.h"

class C_config
{
	private :
		char _tconfig_temp		[CONFIG_RECORD_LENGTH];
		char company_id			[4];		/* 회원사 코드 */
		char tr_code			[3];
		char communicate_type	[2];
		char process_name		[11];
		char object_name   		[30];
		char port_number   		[9];
		char ip_number     		[43];
		char tcp_process_name 	[11];
		char data_file_name		[30];
		char log_file_name 		[30];
		char cnt_file_name 		[30];
		char jang_file_name		[30];
		char cnt_gubun     		[3];		/* 업무 구분 코드 */
		char message_length		[5];
		char msg_file_name 		[26];
		char header_tr_code 	[10];
		char _error_message     [150];

  	public :
		C_config();
		void F_read_config(char *rckey, char *rcconfig);
		char* F_get_company_id();
		char* F_get_tr_code();
		char* F_get_communication_type();
		char* F_get_process_name();
		char* F_get_object_name();
		char* F_get_port_number();
		char* F_get_ip_number();
		char* F_get_tcp_process_name();
		char* F_get_data_file_name();
		char* F_get_log_file_name();
		char* F_get_cnt_file_name();
		char* F_get_jang_file_name();
		char* F_get_cnt_gubun();
		char* F_get_message_length();
		char* F_get_msg_file_name();
		char* F_get_header_tr_code();
};

#endif
