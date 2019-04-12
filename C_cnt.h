#include"main.h"
#include"C_time.h"

#define CNT_RECORD_LENGTH 101
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
	char end			[1];   /* NULL */
};

class C_cnt
{
	private :
		fstream _cnt;
		CNT_DEF _cnt_record;
		C_time _date_time;
		char _write_message		[150];
		char _cnt_key			[6];
		char _data_count		[9];   /* 마지막 데이터 수신 및 송신 개수 (일련번호) */
		long _data_count_num;
		char _process_status	[2];   /* Main Process 상태 (0:미기동, 1:정상기동, 2:비정상STOP, 3:정상STOP) */
		int _process_status_num;
		char _link_status		[3];   /* Connect 여부 (00:미Link, 01:Link, 02:Link종료송신 03:Link종료수신(최종 종료)) */
		int _link_status_num;
		long _next_data_count_num;	   /* 다음 데이터 Count */
		long _line_temp;			   /* 위치 지정자 임시 저장 */

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
			_next_data_count_num = 0;
			_link_status_num = -1;
			_line_temp = 0;
		}

		char* F_open_cnt_file(char* _cnt_file, char* _company_id, char* _cnt_gubun)
		{
			_cnt.open(_cnt_file, ios::in | ios::out);
			if(!_cnt.is_open())
			{
				memset(_write_message, 0x00, sizeof(_write_message));
				sprintf(_write_message, "CNT File Open Error(%s)..", _cnt_file);
				throw _write_message;
			}
			else
			{
				/* Start CNT File Open Message */
				memset(_write_message, 0x00, sizeof(_write_message));
				sprintf(_write_message, "CNT File OPEN(%s)..", _cnt_file);
				return _write_message;
			}
		}

		void F_read_cnt(char* _company_id, char* _cnt_gubun)
		{
			/* 1. Key Buffer Init */ 
			memset(_cnt_key, 0x00, sizeof(_cnt_key)); 

			/* 2. Key Setting */ 
			strncpy(_cnt_key, _company_id, 3); 
			strncpy(&_cnt_key[3], _cnt_gubun, 2); 

			/* 3. Key Positioning & Get Record To Buffer */ 
			_cnt.seekg(0, ios::beg);			/* CNT File의 맨 처음에 위치 */ 
			while(!_cnt.eof()) 
			{ 
				_line_temp = _cnt.tellg();
				_cnt.getline(_cnt_record.company_id, CNT_RECORD_LENGTH); 
				//_cnt.read(_cnt_record.company_id, CNT_RECORD_LENGTH);
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

		long F_get_last_data_count()
		{
			/* Data Count(SEQ) 변수 Setting ("최종 수신 또는 송신 번호" -> "_data_count") */
			/* 마지막 데이터 수신 및 송신 개수 (일련번호) */
			strncpy(_data_count, _cnt_record.data_count, 8);
			_data_count_num = atol(_data_count);
			_next_data_count_num = _data_count_num + 1;		/* 다음 Count 수 Setting */
			return _data_count_num;
		}

		long F_get_next_data_count()
		{
			return _next_data_count_num;
		}

		int F_get_process_status()
		{
			/* Main Process 상태 (0:미기동, 1:정상기동, 2:비정상STOP, 3:정상STOP) */
			strncpy(_process_status, _cnt_record.process_status, 1);
			_process_status_num = atoi(_process_status);
			if(_process_status_num < 0)
			{
				cout << "Process Status atoi() Error.." << endl;
				throw "Process Status atoi() Error..";
			}
			return _process_status_num;
		}

		int F_get_link_status()
		{
			/* Connect 여부 (00:미Link, 01:Link, 02:Link종료송신 03:Link종료수신(최종 종료)) */
			strncpy(_link_status, _cnt_record.link_status, 2);
			_link_status_num = atoi(_link_status);
			return _link_status_num;
		}

		int F_put_process_stop(int option)
		{
			/* 1. Buffer Time Setting */
			strncpy(_cnt_record.date, _date_time.F_get_date(), sizeof(_cnt_record.date));
			strncpy(_cnt_record.time, _date_time.F_get_time(), sizeof(_cnt_record.time));
			strncpy(_cnt_record.stop_time, _date_time.F_get_time(), sizeof(_cnt_record.time));

			/* 2. Buffer Status Setting */
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
			
			/* 3. Buffer Write */
			_cnt.seekp(_line_temp, ios::beg);							/* CNT File의 해당 회원사 Record 위치 */
			_cnt.write(_cnt_record.company_id, CNT_RECORD_LENGTH - 1);
			if(_cnt.bad())
			{
				memset(_write_message, 0x00, sizeof(_write_message));
				sprintf(_write_message, "CNT File Write Error..%s", _cnt_key);
				throw _write_message;
			}

			if(option == SUCCESS)
				return SUCCESS;
			else
				throw FAIL;
		}
};
