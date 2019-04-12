#include"main.h"
#include"C_time.h"

class C_log
{
	private :
		fstream _log;
		int _log_fd;
		C_time _date_time;
		char _write_message[150];
		char* _time;
		char* _process_name;

	public :
		void F_open_log_file(char* _process, char* _log_file)
		{
			_process_name = _process;
			char _start_message[150];

			_log.open(_log_file, ios::in | ios::app);
			if(!_log.is_open())
			{
				memset(_start_message, 0x00, sizeof(_start_message));
				sprintf(_start_message, "Log File Open Error(%s)..", _log_file);
				cout << _start_message << endl;
				F_write_log(_start_message);
				exit(1);
			}
			else
			{
				/* Start Log Message */
				memset(_start_message, 0x00, sizeof(_start_message));
				sprintf(_start_message, "====================================================");
				F_write_log(_start_message);

				memset(_start_message, 0x00, sizeof(_start_message));
				sprintf(_start_message, "Interface PROGRAM                                   ");
				F_write_log(_start_message);

				memset(_start_message, 0x00, sizeof(_start_message));
				sprintf(_start_message, "====================================================");
				F_write_log(_start_message);

				memset(_start_message, 0x00, sizeof(_start_message));
				sprintf(_start_message, "Log File OPEN(%s)..", _log_file);
				F_write_log(_start_message);
			}
		}

		void F_write_log(const char* _message)
		{

			/* 1. Write Buffer Init */
			memset(_write_message, 0x00, sizeof(_write_message));

			/* 2. Time Update */
			_date_time.F_update_date_time();
			_time = _date_time.F_get_time_edit();

			/* 3. Write Message Setting */
			sprintf(_write_message, "[%6.6s-%8.8s] %s", _process_name, _time, _message);

		
			/* 4. Write Log File */
			_log.seekp(0, ios::end);			/* Log File의 맨 마지막에 위치 */
			_log << _write_message << endl;	/* Log File에 Setting 한 Message Write */
			if(_log.bad())
			{
				sprintf(_write_message, "[%6.6s-%8.8s] Log File Write Error..", _process_name, _time);
				cout << _write_message << endl;
				exit(1);
			}

		}

		void F_write_log_send(char* _message)
		{
			/* 3. Write Message Setting */
			/*
			switch(type)
			{
				case LOG_TYPE_NORMAL :
					sprintf(_write_message, "[%6.6hs-%8.8hs] %s", _process_name, _time, _message);
					break;

				case LOG_TYPE_RECV :
					sprintf(_write_message, "[%6.6hs-%8.8hs] RECV %4.4hs %1.1hs %3.3hs %4.4hs %3.3hs %2.2hs %12.12hs %2.2hs %8.8hs %2.2hs.%d", _process_name, _time, st_rcv_buff.message_length, st_rcv_buff.tr_code, st_rcv_buff.gigwan_id, st_rcv_buff.msg_type, st_rcv_buff.opr_type, st_rcv_buff.err_code, st_rcv_buff.time, st_rcv_buff.retry_cnt, st_rcv_buff.data_no, st_rcv_buff.data_cnt, read_message_length);
					break;

				case LOG_TYPE_SEND :
					sprintf(_write_message, "[%6.6hs-%8.8hs] RECV %4.4hs %1.1hs %3.3hs %4.4hs %3.3hs %2.2hs %12.12hs %2.2hs %8.8hs %2.2hs.%d", _process_name, _time, st_snd_buff.message_length, st_snd_buff.tr_code, st_snd_buff.gigwan_id, st_snd_buff.msg_type, st_snd_buff.opr_type, st_snd_buff.err_code, st_snd_buff.time, st_snd_buff.retry_cnt, st_snd_buff.data_no, st_snd_buff.data_cnt, send_message_length);
					break;

				default :
					sprintf(_write_message, "[%6.6hs-%8.8hs] Invalid Log Type..%d", _process_name, _time, type);
			}
			*/
		}
};
