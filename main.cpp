#include"main.h"
#include"C_time.h"
#include"C_config.h"
#include"C_log.h"
#include"C_msg.h"
#include"C_data.h"
#include"C_cnt.h"
#include"C_jang.h"
#include"C_socket.h"

class C_main_handle
{
	private :
		struct sockaddr_in s_add;
		char **_key;
		char **_config_path;
		char _message[150];
		C_time _date_time;
		C_config _config;
		C_log _log;
		C_msg _msg;
		C_data _data;
		C_cnt _cnt;
		C_jang _jang;
		int _data_fd;
		int _jang_status;
		C_socket _socket;

	public :
		C_main_handle( int *argc, char *argv[]) : _key(&argv[1]), _config_path(&argv[2])
		{
			/* Argument Read, Check and Display */
			if(*argc != 3)
			{
				cout << "Parameter 수가 맞지 않는군요." << endl;
				cout << "Usage : " << *argv << " <WhisaCode+TR> <Config File>" << endl;
				exit(1);
			}
		}

		void F_get_date_time()
		{
			_date_time.F_update_date_time();
			//cout << _date_time.F_get_time() << endl;
			//cout << _date_time.F_get_date() << endl;
			//cout << _date_time.F_get_time_edit() << endl;
			//cout << _date_time.F_get_date_edit() << endl;
		}

		void F_get_config()
		{
			try
			{
				/* 1. read config */
				_config.F_read_config(*_key, *_config_path);
				//cout << _config.F_get_company_id() << endl; 
				//cout << _config.F_get_tr_code() << endl; 
				//cout << _config.F_get_communication_type() << endl;
				//cout << _config.F_get_process_name() << endl;
				//cout << _config.F_get_object_name() << endl; 
				//cout << _config.F_get_port_number() << endl;
				//cout << _config.F_get_ip_number() << endl;
				//cout << _config.F_get_tcp_process_name() << endl;
				//cout << _config.F_get_data_file_name() << endl; 
				//cout << _config.F_get_log_file_name() << endl; 
				//cout << _config.F_get_cnt_file_name() << endl;
				//cout << _config.F_get_jang_file_name() << endl;
				//cout << _config.F_get_cnt_gubun() << endl;
				//cout << _config.F_get_message_length() << endl;
				//cout << _config.F_get_msg_file_name() << endl;
				//cout << _config.F_get_header_tr_code() << endl;

				/* 2. check socket infrom to socket */
                F_set_check_socket_information();
			}
			catch(const char* _message)
			{
				cout << _message << endl;
				exit(1);
			}
		}

		void F_set_check_socket_information()
		{
			int _data_length = 0;
			_data_length = atoi(_config.F_get_message_length());
			char* _ip_number = _config.F_get_ip_number();
			char* _port_number = _config.F_get_port_number();
			char* _company_id = _config.F_get_company_id();
			char* _tr_code = _config.F_get_tr_code();
			char* _communicate_type = _config.F_get_communication_type();

			if(_socket.F_set_config_information(_data_length, _ip_number, _port_number, _company_id, _tr_code, _communicate_type) != SUCCESS)
			{
				throw "Check Socket Setting Error..";
			}
		}

		void F_open_file()
		{
			try
			{
				/* 1. LOG File Open */
				_log.F_open_log_file(_config.F_get_process_name(), _config.F_get_log_file_name());
				
				/* 2. MSG File Open */
				_log.F_write_log(_msg.F_open_msg_file(_config.F_get_process_name(), _config.F_get_msg_file_name()));

				/* 3. Program Start Message */
				memset(_message, 0x00, sizeof(_message));
				sprintf(_message, "Interface PROGRAM STARTING                          ");
				_log.F_write_log(_message);
				_msg.F_write_msg(_message);

				/* 4. Data File Open */
				_log.F_write_log(_data.F_open_data_file(_config.F_get_data_file_name()));
				_data_fd = _data.F_get_data_fd();

				/* 5. CNT File Open */
				_log.F_write_log(_cnt.F_open_cnt_file(_config.F_get_cnt_file_name(), _config.F_get_company_id(), _config.F_get_cnt_gubun()));

				/* 6. Jang File Open */
				_log.F_write_log(_jang.F_open_jang_file(_config.F_get_jang_file_name()));
			}
			catch(const char* _message)
			{
				_log.F_write_log(_message);
				_msg.F_write_msg(_message);
				exit(1);
			}
		}

		void F_get_cnt()
		{
			try
			{
				/* 1. 해당 회원사 Record Read */
				_cnt.F_read_cnt();

				/* 2. Last Data Count Status Check */
				memset(_message, 0x00, sizeof(_message));
				_log.F_write_log(_cnt.F_get_last_data());

				/* 3. Process Status Check */
				memset(_message, 0x00, sizeof(_message));
				_log.F_write_log(_cnt.F_get_process());

				/* 4. Link Status Check */
				memset(_message, 0x00, sizeof(_message));
				_log.F_write_log(_cnt.F_get_link());
			}
			catch(const char* _message)
			{
				_log.F_write_log(_message);
				_msg.F_write_msg(_message);
				exit(1);
			}
		}

		void F_get_jang()
		{
			try
			{
				_jang.F_read_jang();
				_jang_status = _jang.F_get_jang_status();
			}
			catch(const char* _message)
			{
				_log.F_write_log(_message);
				_msg.F_write_msg(_message);
				exit(1);
			}
		}

        void F_start()
        {
            if(!_socket.F_link_status())
            {
                while(!_socket.F_link_status())
					F_create_socket();
            }
        }

		int F_create_socket()
		{
			try
			{
				/* 1. Socket Create, Bind, Listen */
				memset(_message, 0x00, sizeof(_message));
				sprintf(_message, _socket.F_create_socket());
				_log.F_write_log(_message);

				/* 2. Socket Accept */
				memset(_message, 0x00, sizeof(_message));
				sprintf(_message, _socket.F_accept_socket());
				_log.F_write_log(_message);
				_msg.F_write_msg(_message);

				/* 3. Status Message */
				memset(_message, 0x00, sizeof(_message));
				sprintf(_message, "Link 대기 중...");
				_log.F_write_log(_message);
				_msg.F_write_msg(_message);
			}
			catch(const char* _message)
			{
				_log.F_write_log(_message);
				_msg.F_write_msg(_message);
			}
		}

		void F_read_message()
		{
			int _result = FAIL;

			while(1)
			{
				try
				{
					long _last_data_count = 0;
					_last_data_count = _cnt.F_get_last_data_count();
					F_get_jang();

					_result = _socket.F_recv_message(_jang_status, _last_data_count);

					/* 정상 수신 */
					if(_result == SUCCESS) 
						break;

					/* 재송횟수 3회 초과 */
					else
					{
						memset(_message, 0x00, sizeof(_message));
						sprintf(_message, "RECV RETRY %d Times Error..", _result);

						/* Socket recreate */
						F_start();
					}
				}
				catch(const char* _message)
				{
					_log.F_write_log(_message);
					_msg.F_write_msg(_message);
				}
				catch(const int _fail)
				{
					memset(_message, 0x00, sizeof(_message));
					sprintf(_message, "SEND Retry Error..%d", _fail);
					F_stop_process(FAIL);
				}
			}
		}

		void F_check_message()
		{
			try
			{
				_socket.F_check_message();
			}
			catch(const char* _message)
			{
				_log.F_write_log(_message);
				_msg.F_write_msg(_message);
			}
		}

		void F_stop_process(int option)
		{
			try
			{
				if(_cnt.F_put_process_stop(option) == SUCCESS)
				{
					memset(_message, 0x00, sizeof(_message));
					sprintf(_message, "Interface PROGRAM NORMAL STOP");
					_log.F_write_log(_message);
					_msg.F_write_msg(_message);
					exit(0);
				}
				else
				{
					memset(_message, 0x00, sizeof(_message));
					sprintf(_message, "Interface PROGRAM ABNORMAL STOP");
					_log.F_write_log(_message);
					_msg.F_write_msg(_message);
					exit(1);
				}
			}
			catch(const char* _message)
			{
				_log.F_write_log(_message);
				_msg.F_write_msg(_message);
				exit(1);
			}
		}
};

int main(int argc, char *argv[])
{
	
	argc = 3;
	argv[1] = "999r1";
	argv[2] = "tconfig";


	/* Parameter Check */
	C_main_handle _control(&argc, argv);

	/* 사전 처리 */
	_control.F_get_date_time();			/* 현재 시스템 날짜와 시간으로 Update */
	_control.F_get_config();			/* CONFIG에서 정보 가져오기 */ 
	_control.F_open_file();				/* CONFIG에서 가져 온 정보로 관련파일들 Open */
	_control.F_get_cnt();				/* Process상태, Link여부, 마지막 데이터 개수 */
	_control.F_get_jang();				/* JANG File을 읽어와서 Variable Setting */
	//_control.F_stop_process(FAIL);
	//_control.F_stop_process(SUCCESS);
	
    while(1)
    {
        /* 0. Socket Create, Bind, Listen, Accept */
        _control.F_start();

        /* 1. 수신 */
		_control.F_read_message();

        /* 2. Message Check, Message Set, Data Set */
		_control.F_check_message();
        
        /* 3. 송신 */
    }

	return 0;
}
