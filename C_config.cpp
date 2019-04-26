#include"C_config.h"

C_config::C_config()
{
	memset(_config_temp, 0x00, sizeof(_config_temp));
	memset(company_id, 0x00, sizeof(company_id));
	memset(tr_code, 0x00, sizeof(tr_code));
	memset(communicate_type, 0x00, sizeof(communicate_type));
	memset(process_name, 0x00, sizeof(process_name));
	memset(object_name, 0x00, sizeof(object_name));
	memset(port_number, 0x00, sizeof(port_number));
	memset(ip_number, 0x00, sizeof(ip_number));
	memset(tcp_process_name, 0x00, sizeof(tcp_process_name));
	memset(data_file_name, 0x00, sizeof(data_file_name));
	memset(log_file_name, 0x00, sizeof(log_file_name));
	memset(cnt_file_name, 0x00, sizeof(cnt_file_name));
	memset(jang_file_name, 0x00, sizeof(jang_file_name));
	memset(cnt_gubun, 0x00, sizeof(cnt_gubun));
	memset(message_length, 0x00, sizeof(message_length));
	memset(msg_file_name, 0x00, sizeof(msg_file_name));
	memset(header_tr_code, 0x00, sizeof(header_tr_code));
}

/* Read Config File */
void C_config::F_read_config(char *_key, char *_config_file)
{
	/* 1. "TCONFIG" File Open */
	ifstream config(_config_file, ios::in);
	if(!config.is_open())
	{
		memset(_error_message, 0x00, sizeof(_error_message));
		sprintf(_error_message, "TCONFIG File Open Error..%s", _key);
		throw _error_message;
	}

	/* 2. Key Positioning & Read */
	/* TOCINFG file에서 실행 시 주어진 변수로 원하는 레코드 위치로 이동 ex) "001L1" */
	while(!config.eof())
	{
		config.getline(_config_temp, CONFIG_RECORD_LENGTH);
		if(config.bad())
		{
			memset(_error_message, 0x00, sizeof(_error_message));
			sprintf(_error_message, "CONFIG File Read Error..%s", _config_file);
			throw _error_message;
		}
		if(strncasecmp(_config_temp, _key, 5) == 0)
			break;
	}

	if(strncasecmp(_config_temp, _key, 5) != 0)
	{
		memset(_error_message, 0x00, sizeof(_error_message));
		sprintf(_error_message, "TCONFIG Key Positioning Error..%s", _key);
		throw _error_message;
	}

	/* 3. "TCONFIG" File Close */
	config.close();
	if(config.bad())
	{
		memset(_error_message, 0x00, sizeof(_error_message));
		sprintf(_error_message, "TCONFIG File Close Error..%s", _key);
		throw _error_message;
	}

	/* 4. Check Config */
	cout << "TR CODE Type.." << F_get_tr_code() << endl;
	cout << "Commuicate Type.." << F_get_communication_type() << endl;
}

/* Return (1) 회원사 번호 */
char* C_config::F_get_company_id()
{
	strncpy(company_id, _config_temp, 3);
	return company_id;
}

/* Return (2) TR Code */
char* C_config::F_get_tr_code()
{
	strncpy(tr_code, &_config_temp[3], 2);
	return tr_code;
}

/* Return (3) Communication Type (수신 : R, 송신 : S) */
char* C_config::F_get_communication_type()
{
	strncpy(communicate_type, &_config_temp[6], 1);
	return communicate_type;
}

/* Return (4) Process Name ($XnnnX) */
char* C_config::F_get_process_name()
{
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(process_name, &_config_temp[11], atoi(strncpy(_temp_length, &_config_temp[8], 2)));
	return process_name;
}

/* Return (5) Object Name (Program Path & Name) */
char* C_config::F_get_object_name()
{
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(object_name, &_config_temp[22], atoi(strncpy(_temp_length, &_config_temp[19], 2)));
	return object_name;
}

/* Return (6) Port Num */
char* C_config::F_get_port_number()
{
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(port_number, &_config_temp[52], atoi(strncpy(_temp_length, &_config_temp[49], 2)));
	return port_number;
}

/* Return (7) IP Num */
char* C_config::F_get_ip_number()
{
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(ip_number, &_config_temp[61], atoi(strncpy(_temp_length, &_config_temp[58], 2)));
	return ip_number;
}

/* Return (8) TCP PROC Name ($ZB26D) */
char* C_config::F_get_tcp_process_name()
{
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(tcp_process_name, &_config_temp[104], atoi(strncpy(_temp_length, &_config_temp[101], 2)));
	return tcp_process_name;
}

/* Return (9) DATA File Name (수, 송신 파일) */
char* C_config::F_get_data_file_name()
{
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(data_file_name, &_config_temp[115], atoi(strncpy(_temp_length, &_config_temp[112], 2)));
	return data_file_name;
}

/* Return (10) LOG File Name */
char* C_config::F_get_log_file_name()
{
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(log_file_name, &_config_temp[145], atoi(strncpy(_temp_length, &_config_temp[142], 2)));
	return log_file_name;
}

/* Return (11) CNT File Name (SEQ 파일) */
char* C_config::F_get_cnt_file_name()
{
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(cnt_file_name, &_config_temp[175], atoi(strncpy(_temp_length, &_config_temp[172], 2)));
	return cnt_file_name;
}

/* Return (12) JANG File Name */
char* C_config::F_get_jang_file_name()
{
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(jang_file_name, &_config_temp[205], atoi(strncpy(_temp_length, &_config_temp[202], 2)));
	return jang_file_name;
}

/* Return (13) 업무 구분 번호 */
char* C_config::F_get_cnt_gubun()
{
	strncpy(cnt_gubun, &_config_temp[232], 2);
	return cnt_gubun;
}

/* Return (14) Message Length */
char* C_config::F_get_message_length()
{
	strncpy(message_length, &_config_temp[235], 4);
	return message_length;
}

/* Return (15) MSG File Name */
char* C_config::F_get_msg_file_name()
{
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(msg_file_name, &_config_temp[243], atoi(strncpy(_temp_length, &_config_temp[240], 2)));
	return msg_file_name;
}

/* Return (16) HEADER TR_CODE */
char* C_config::F_get_header_tr_code()
{
	strncpy(header_tr_code, "N        ", 9);
	return header_tr_code;
}
