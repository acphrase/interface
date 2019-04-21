#include"main.h"
#include"C_time.h"

#define CNT_RECORD_LENGTH 100
#define INCOMPLETE "0"
#define COMPLETE "1"
#define ABNORMAL_STATUS "2"
#define NORMAL_STATUS "3"
#define ABNORMAL_STOP "5"
#define NOW_TIME 6
#define TODAY_DATE 8
#define NORMAL_STOP "9"

struct CNT_DEF{
	char company_id		[3];   /* 회원사 코드 */
	char cnt_gubun		[2];   /* 업무 구분 코드 */
	char process_status	[1];   /* Main Process 상태 (0:미기동, 1:정상기동, 2:비정상STOP, 3:정상STOP) */
	char b_process_setup[1];   /* -- 미사용 (Backup Process 상태) */
	char process_time	[6];   /* Main Process 기동 시각 */
	char b_process_time	[6];   /* -- 미사용 (Backup Process 기동 시각)*/
	char b_transfer		[1];   /* -- 미사용 (Main, Backup 전환 여부 0:Main, 1:Backup) */
	char error_gubun	[1];   /* -- 미사용 (Error 구분) */
	char error_code		[2];   /* Interface Error Code */
	char error_time		[6];   /* -- 미사용 (Error 발생 시각) */
	char data_count		[8];   /* 마지막 데이터 수신 및 송신 개수 (일련번호) */
	char time			[6];   /* 마지막 데이터 수신 및 송신 시각 */
	char stop_gubun		[1];   /* Main Interface 종료 구분 (9:정상, 5:기타비정상, 1:정상 기동중) */
	char stop_time		[6];   /* Main Interface 종료 시각 */
	char b_stop_gubun	[1];   /* -- 미사용 (Backup Interface 종료 구분) */
	char b_stop_time	[6];   /* -- 미사용 (Backup Interface 종료 시각) */
	char link_status	[2];   /* Connect 여부 (00:미Link, 01:Link, 02:Link종료송신 03:Link종료수신(최종 종료)) */
	char link_time		[6];   /* -- 미사용 (Connect 시각) */
	char restart_time	[6];   /* 최초 Link 시각 또는 재 Link 시각 */
	char use_bit		[1];   /* Interface 사용 여부 (0:사용, 1:미사용) */
	char rel_key		[8];   /* -- 미사용 */
	char complete_yn	[1];   /* Interface 완료 여부 (0:미완료, 1:완료) */
	char date			[8];   /* Data 송수신 일자 */
	char filler			[11];  /* -- 미사용 (여분) */
	//char end			[1];   /* NULL */
};

class C_cnt
{
	private :
		fstream _cnt;
		CNT_DEF _cnt_record;
		C_time _date_time;
		char _write_message		[150];
		char _cnt_key			[6];
		char _data_count		[9];	/* 마지막 데이터 수신 및 송신 개수 (일련번호) */
		long _data_count_num;
		char _process_status	[2];	/* Main Process 상태 (0:미기동, 1:정상기동, 2:비정상STOP, 3:정상STOP) */
		int _process_status_num;
		char _link_status		[3];	/* Connect 여부 (00:미Link, 01:Link, 02:Link종료송신 03:Link종료수신(최종 종료)) */
		int _link_status_num;
		long _line_temp;				/* 위치 지정자 임시 저장 */
		char* _company_id;
		char* _cnt_gubun;

	public :
		C_cnt()
		{
			memset(&_cnt_record, 0x00, sizeof(_cnt_record));
			memset(_write_message, 0x00, sizeof(_write_message));
			memset(_cnt_key, 0x00, sizeof(_cnt_key));
			//memset(company_id, 0x00, sizeof(company_id));
			//memset(cnt_gubun, 0x00, sizeof(cnt_gubun));
			memset(_process_status, 0x00, sizeof(_process_status));
			//memset(b_process_setup, 0x00, sizeof(b_process_setup));
			//memset(process_time, 0x00, sizeof(process_time));
			//memset(b_process_time, 0x00, sizeof(b_process_time));
			//memset(b_transfer, 0x00, sizeof(b_transfer));
			//memset(error_gubun, 0x00, sizeof(error_gubun));
			//memset(error_num, 0x00, sizeof(error_num));
			//memset(error_time, 0x00, sizeof(error_time));
			memset(_data_count, 0x00, sizeof(_data_count));
			//memset(time, 0x00, sizeof(time));
			//memset(stop_gubun, 0x00, sizeof(stop_gubun));
			//memset(stop_time, 0x00, sizeof(stop_time));
			//memset(b_stop_gubun, 0x00, sizeof(b_stop_gubun));
			//memset(b_stop_time, 0x00, sizeof(b_stop_time));
			memset(_link_status, 0x00, sizeof(_link_status));
			//memset(link_time, 0x00, sizeof(link_time));
			//memset(restart_time, 0x00, sizeof(restart_time));
			//memset(use_bit, 0x00, sizeof(use_bit));
			//memset(rel_key, 0x00, sizeof(rel_key));
			//memset(complete_yn, 0x00, sizeof(complete_yn));
			//memset(date, 0x00, sizeof(date));
			_process_status_num = -1;
			_data_count_num = 0;
			_link_status_num = -1;
			_line_temp = 0;
		}

		~C_cnt()
		{
			_cnt.close();
		}

		char* F_open_cnt_file(char* _cnt_file, char* r_company_id, char* r_cnt_gubun)
		{
			/* 1. Count File Open */
			_cnt.open(_cnt_file, ios::in | ios::out);
			cout << _cnt_file << endl;
			if(!_cnt.is_open())
			{
				memset(_write_message, 0x00, sizeof(_write_message));
				sprintf(_write_message, "CNT File Open Error(%s)..", _cnt_file);
				throw _write_message;
			}
			else
			{
			
				memset(_cnt_key, 0x00, sizeof(_cnt_key)); 
				strncpy(_cnt_key, r_company_id, 3); 
				strncpy(&_cnt_key[3], r_cnt_gubun, 2); 

				/* Start CNT File Open Message */
				memset(_write_message, 0x00, sizeof(_write_message));
				sprintf(_write_message, "CNT File OPEN(%s)..", _cnt_file);
				return _write_message;
			}
		}

		void F_read_cnt()
		{
			/* 1. Get Record To Buffer */ 
			_cnt.seekg(_line_temp, ios::beg);
			_cnt.read(_cnt_record.company_id, CNT_RECORD_LENGTH + 1);
			cout << _cnt_record.company_id << endl;

			/* 2. Key Positioning & Get Record To Buffer */ 
			if(strncmp(_cnt_record.company_id, _cnt_key, 5) != 0)
			{
				_cnt.seekg(0, ios::beg); /* CNT File의 맨 처음에 위치 */ 
				while(!_cnt.eof()) 
				{ 
					_line_temp = _cnt.tellg();
					_cnt.read(_cnt_record.company_id, CNT_RECORD_LENGTH + 1);
					if(_cnt.bad())
					{
						memset(_write_message, 0x00, sizeof(_write_message));
						sprintf(_write_message, "CNT File Read Error..%s", _cnt_key);
						throw _write_message;
					}

					if(strncmp(_cnt_record.company_id, _cnt_key, 5) == 0)
						break;
				}

				if(strncmp(_cnt_record.company_id, _cnt_key, 5) != 0)
				{
					memset(_write_message, 0x00, sizeof(_write_message));
					sprintf(_write_message, "CNT File Key Positioning Error..%s", _cnt_key);
					throw _write_message;
				}
			}
		}

		void F_write_cnt()
		{
			/* 1. Positioning */
			_cnt.seekp(_line_temp, ios::beg); /* CNT File의 해당 회원사 Record 위치 */

			/* 2. Buffer Write */
			cout << _cnt_record.company_id << endl;
			//_cnt.write(_cnt_record.company_id, CNT_RECORD_LENGTH);
			_cnt << _cnt_record.company_id;
			if(_cnt.bad())
			{
				memset(_write_message, 0x00, sizeof(_write_message));
				sprintf(_write_message, "CNT File Write Error..%s", _cnt_key);
				throw _write_message;
			}
		}

		long F_get_last_data_count()
		{
			/* Data Count(SEQ) 변수 Setting ("최종 수신 또는 송신 번호" -> "_data_count") */
			/* 마지막 데이터 수신 및 송신 개수 (일련번호) */
			strncpy(_data_count, _cnt_record.data_count, 8);
			_data_count_num = atol(_data_count);
			return _data_count_num;
		}

		char* F_get_last_data()
		{
			memset(_write_message, 0x00, sizeof(_write_message));
			sprintf(_write_message, "Last Count : %ld", F_get_last_data_count());
			return _write_message;
		}

		char* F_get_process()
		{
			memset(_write_message, 0x00, sizeof(_write_message));
			//sprintf(_write_message, "Proc Status : %d", F_get_process_status());
			sprintf(_write_message, "Proc Status : %.1s", _cnt_record.process_status);
			return _write_message;
		}

		char* F_get_link()
		{
			memset(_write_message, 0x00, sizeof(_write_message));
			sprintf(_write_message, "Link Status : %.2s", _cnt_record.link_status);
			return _write_message;
		}

		void F_setting_tcpip_error_code(int r_error_code)
		{
			char temp[3];
			sprintf(temp, "%.2d", r_error_code);
			strncpy(_cnt_record.error_code, temp, 2); /* TCPIP Error Code Set */
		}

		void F_update_cnt(int msg_type)
		{
			/* 1. Read Record */
			F_read_cnt();

			/* 2. Buffer Time Setting */
			strncpy(&_cnt_record.date[0], _date_time.F_get_date(), 8);
			strncpy(&_cnt_record.time[0], _date_time.F_get_time(), 6);

			/* 3. Update Variable Setting */
			switch(msg_type)
			{
				case MSG_START :
					strncpy(&_cnt_record.restart_time[0], _date_time.F_get_time(), 6); /* 재시작 시간 Set */
					strncpy(&_cnt_record.process_status[0], "1", 1); /* "1"은 정상 기동 Set */
					break;
				case MSG_ERR :
					break;
				default :
    		        strncpy(&_cnt_record.error_code[0], "00", 2); /* TCPIP Error Code Set */

					switch(msg_type)
					{
						case MSG_0810_001 :
							strncpy(&_cnt_record.process_status[0], "1", 1);		/* 비정상 Stop 후 재기동 시 반영 */
							strncpy(&_cnt_record.link_status[0], "01", 2);			/* 01은 link 됨 */
							strncpy(&_cnt_record.link_time[0], _date_time.F_get_time(), 6);	/* link 시간 기재 */
							strncpy(&_cnt_record.complete_yn[0], "0", 1);			/* 0 : 사용중, 1 : 사용완료 */
							break;

						case MSG_0800_301 :
							break;

						case MSG_0810_301 :
							break;

						case MSG_0800_040 :
							strncpy(&_cnt_record.link_status[0], "02", 2);			/* 02는 link 종료 송신 */
							break;

						case MSG_0810_040 :
							strncpy(&_cnt_record.link_status[0], "03", 2);			/* 03은 link 종료 수신 */
							break;

						case MSG_0200_000 :
						case MSG_0210_000 :
							long _temp_count;
							char _add_count[9];
							memset(_add_count, 0x00, sizeof(_add_count));
							
							_temp_count = F_get_last_data_count();
							_temp_count += 1;

							sprintf(_add_count, "%.8ld", _temp_count);

							strncpy(_cnt_record.data_count, _add_count, 8); /* 일련번호 세팅 */
							break;

						default :
							memset(_write_message, 0x00, sizeof(_write_message));
							sprintf(_write_message, "Invalid Message Type..%d (in message_set)", msg_type);
							throw _write_message;
					}
							break;
			}

			/* 4. Write Record */
			F_write_cnt();
		}

		int F_put_process_stop(int option)
		{
			/* 1. Read Record */
			F_read_cnt();

			/* 2. Buffer Time Setting */
			strncpy(_cnt_record.date, _date_time.F_get_date(), sizeof(_cnt_record.date));
			strncpy(_cnt_record.time, _date_time.F_get_time(), sizeof(_cnt_record.time));
			strncpy(_cnt_record.stop_time, _date_time.F_get_time(), sizeof(_cnt_record.time));

			/* 3. Buffer Status Setting */
			if(option == SUCCESS)
			{ /* 정상 종료 */
				strncpy(_cnt_record.process_status, NORMAL_STATUS, sizeof(_cnt_record.process_status));
				strncpy(_cnt_record.stop_gubun, NORMAL_STOP, sizeof(_cnt_record.stop_gubun));
				strncpy(_cnt_record.complete_yn, COMPLETE, sizeof(_cnt_record.complete_yn));
			}
			else
			{ /* 비정상 종료 */
				strncpy(_cnt_record.process_status, ABNORMAL_STATUS, sizeof(_cnt_record.process_status));
				strncpy(_cnt_record.stop_gubun, ABNORMAL_STOP, sizeof(_cnt_record.stop_gubun));
			}

			/* 4. Write Record */
			F_write_cnt();

			if(option == SUCCESS)
				return SUCCESS;
			else if(option == FAIL)
				return FAIL;
			else
				throw "Process Stop Error";
		}
};
