#include"C_config.h"

C_config::C_config()
{
	memset(_tconfig_temp, 0x00, sizeof(_tconfig_temp));
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

void C_config::F_read_config(char *_key, char *_config_file)
{
	char _temp_length[3];

	/* (1) "TCONFIG" File Open */
	ifstream tconfig(_config_file, ios::in);
	if(!tconfig.is_open())
	{
		memset(_error_message, 0x00, sizeof(_error_message));
		sprintf(_error_message, "TCONFIG File Open Error..%s", _key);
		throw _error_message;
	}

	/* (2) Key Positioning & Read */
	/* TOCINFG file에서 실행 시 주어진 변수로 원하는 레코드 위치로 이동 ex) "001L1" */
	while(!tconfig.eof())
	{
		tconfig.getline(_tconfig_temp, CONFIG_RECORD_LENGTH);
		if(tconfig.bad())
		{
			memset(_error_message, 0x00, sizeof(_error_message));
			sprintf(_error_message, "CONFIG File Read Error..%s", _config_file);
			throw _error_message;
		}
		if(strncmp(_tconfig_temp, _key, 5) == 0)
			break;
	}

	if(strncmp(_tconfig_temp, _key, 5) != 0)
	{
		memset(_error_message, 0x00, sizeof(_error_message));
		sprintf(_error_message, "TCONFIG Key Positioning Error..%s", _key);
		throw _error_message;
	}

	/* (3) "TCONFIG" File Close */
	tconfig.close();
	if(tconfig.bad())
	{
		memset(_error_message, 0x00, sizeof(_error_message));
		sprintf(_error_message, "TCONFIG File Close Error..%s", _key);
		throw _error_message;
	}

	/* (4) Parsing */
	/* TCONFIG Variable Setting */
	/* 1. 회원사 번호 */
	strncpy(company_id, _tconfig_temp, 3);
	/* 2. TR Code */
	strncpy(tr_code, &_tconfig_temp[3], 2);
	cout << "TR CODE Type.." << tr_code << endl;
	/* 3. Communication type set (수신 : R, 송신 : S) */
	strncpy(communicate_type, &_tconfig_temp[6], 1);
	cout << "Commuicate Type.." << communicate_type << endl;
	/* 4. Process name set ($XnnnX) */
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(process_name, &_tconfig_temp[11], atoi(strncpy(_temp_length, &_tconfig_temp[8], 2)));
	/* 5. Object Name set (Program Path & Name) */
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(object_name, &_tconfig_temp[22], atoi(strncpy(_temp_length, &_tconfig_temp[19], 2)));
	/* 6. Port Num set */
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(port_number, &_tconfig_temp[52], atoi(strncpy(_temp_length, &_tconfig_temp[49], 2)));
	/* 7. IP Num set */
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(ip_number, &_tconfig_temp[61], atoi(strncpy(_temp_length, &_tconfig_temp[58], 2)));
	/* 8. TCP PROC Name set ($ZB26D) */
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(tcp_process_name, &_tconfig_temp[104], atoi(strncpy(_temp_length, &_tconfig_temp[101], 2)));
	/* 9. DATA File Name set (수, 송신 파일) */
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(data_file_name, &_tconfig_temp[115], atoi(strncpy(_temp_length, &_tconfig_temp[112], 2)));
	/* 10. LOG File Name set */
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(log_file_name, &_tconfig_temp[145], atoi(strncpy(_temp_length, &_tconfig_temp[142], 2)));
	/* 11. CNT File Name set (SEQ 파일) */
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(cnt_file_name, &_tconfig_temp[175], atoi(strncpy(_temp_length, &_tconfig_temp[172], 2)));
	/* 12. JANG File Name set */
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(jang_file_name, &_tconfig_temp[205], atoi(strncpy(_temp_length, &_tconfig_temp[202], 2)));
	/* 13. 업무 구분 번호 set */
	strncpy(cnt_gubun, &_tconfig_temp[232], 2);
	/* 14. Message Length */
	strncpy(message_length, &_tconfig_temp[235], 4);
	/* 15. MSG File Name set */
	memset(_temp_length, 0x00, sizeof(_temp_length));
	strncpy(msg_file_name, &_tconfig_temp[243], atoi(strncpy(_temp_length, &_tconfig_temp[240], 2)));
	/* 16. HEADER TR_CODE Set */
	strncpy(header_tr_code, "N        ", 9);
}

char* C_config::F_get_company_id()
{
	return company_id;
}

char* C_config::F_get_tr_code()
{
	return tr_code;
}

char* C_config::F_get_communication_type()
{
	return communicate_type;
}

char* C_config::F_get_process_name()
{
	return process_name;
}

char* C_config::F_get_object_name()
{
	return object_name;
}

char* C_config::F_get_port_number()
{
	return port_number;
}

char* C_config::F_get_ip_number()
{
	return ip_number;
}

char* C_config::F_get_tcp_process_name()
{
	return tcp_process_name;
}

char* C_config::F_get_data_file_name()
{
	return data_file_name;
}

char* C_config::F_get_log_file_name()
{
	return log_file_name;
}

char* C_config::F_get_cnt_file_name()
{
	return cnt_file_name;
}

char* C_config::F_get_jang_file_name()
{
	return jang_file_name;
}

char* C_config::F_get_cnt_gubun()
{
	return cnt_gubun;
}

char* C_config::F_get_message_length()
{
	return message_length;
}

char* C_config::F_get_msg_file_name()
{
	return msg_file_name;
}

char* C_config::F_get_header_tr_code()
{
	return header_tr_code;
}
