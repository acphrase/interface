#ifndef __OPEN_CONFIG_HEADER__
#define __OPEN_CONFIG_HEADER__

#include"common.h"

class C_config
{
	private :
		/* 1. Buffer Variable */
		char _config_temp		[CONFIG_RECORD_LENGTH];

		/* 2. Config Parsing Variable */
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
		char status_file_name	[30];
		char jang_file_name		[30];
		char status_gubun 		[3];		/* 업무 구분 코드 */
		char message_length		[5];
		char msg_file_name 		[26];
		char header_tr_code 	[10];
		char _error_message     [150];

		/* 3. ETC */
		char _temp_length		[3];

  	public :
		C_config();
	
		/* Read Config File */
		void F_read_config(char *rckey, char *rcconfig);

		/* Return (1) 회원사 번호 */
		char* F_get_company_id();

		/* Return (2) TR Code */
		char* F_get_tr_code();

		/* Return (3) Communication Type (수신 : R, 송신 : S) */
		char* F_get_communication_type();

		/* Return (4) Process Name ($XnnnX) */
		char* F_get_process_name();

		/* Return (5) Object Name (Program Path & Name) */
		char* F_get_object_name();

		/* Return (6) Port Num */
		char* F_get_port_number();
		
		/* Return (7) IP Num */
		char* F_get_ip_number();

		/* Return (8) TCP PROC Name ($ZB26D) */
		char* F_get_tcp_process_name();

		/* Return (9) DATA File Name (수, 송신 파일) */
		char* F_get_data_file_name();

		/* Return (10) LOG File Name */
		char* F_get_log_file_name();

		/* Return (11) CNT File Name (SEQ 파일) */
		char* F_get_status_file_name();

		/* Return (12) JANG File Name */
		char* F_get_jang_file_name();

		/* Return (13) 업무 구분 번호 */
		char* F_get_status_gubun();

		/* Return (14) Message Length */
		char* F_get_message_length();

		/* Return (15) MSG File Name */
		char* F_get_msg_file_name();

		/* Return (16) HEADER TR_CODE */
		char* F_get_header_tr_code();
};

#endif
