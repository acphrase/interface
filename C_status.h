#ifndef __STATUS_HEADER__
#define __STATUS_HEADER__

#include"common.h"
#include"C_time.h"

class C_status
{
	private :
		/* 1. Status Message Variable */
		char _write_message[150];

		/* 2. File Stream Variable */
		fstream _status;

		/* 3. Buffer Variable */
		STATUS_DEF _status_record;

		/* 4. Time Class */
		C_time _date_time;

		/* 5. Mutex Variable */
		mutex lock_func;

		/* 6. ETC */
		char* _time;
		char _status_key		[6];
		char _data_count		[9];	/* 마지막 데이터 수신 및 송신 개수 (일련번호) */
		long _data_count_num;
		char _process_status	[2];	/* Main Process 상태 (0:미기동, 1:정상기동, 2:비정상STOP, 3:정상STOP) */
		int _process_status_num;
		char _link_status		[3];	/* Connect 여부 (00:미Link, 01:Link, 02:Link종료송신 03:Link종료수신(최종 종료)) */
		int _link_status_num;
		long _line_temp;				/* 위치 지정자 임시 저장 */
		char* _company_id;
		char* _status_gubun;

	public :
		C_status();
		~C_status();

		/* Status File Open */
		char* F_open_status_file(char* _status_file, char* r_company_id, char* r_status_gubun);
		
		/* Read Status File */
		void F_read_status();

		/* Write Status File */
		void F_write_status();

		/* Update Status File */
		int F_update_status(int msg_type);

		/* Update Process Stop To Status File */
		int F_process_stop(int option);

		/* Return Last Data Count Number */
		long F_get_last_data_count();

		/* Return Last Data Count String for Log */
		char* F_get_last_data();

		/* Return Process Status String for Log */
		char* F_get_process();

		/* Return Link Status String for Log */
		char* F_get_link();

		/* Get & Setting TCPIP Error Code */
		void F_setting_tcpip_error_code(int r_error_code);
};

#endif
