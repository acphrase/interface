#ifndef __CNT_HEADER__
#define __CNT_HEADER__

#include"main.h"
#include"C_time.h"

class C_cnt
{
	private :
		/* 1. Status Message Variable */
		char _write_message[150];

		/* 2. File Stream Variable */
		fstream _cnt;

		/* 3. Buffer Variable */
		CNT_DEF _cnt_record;

		/* 4. Time Class */
		C_time _date_time;

		/* 5. Mutex Variable */
		mutex lock_func;

		/* 6. ETC */
		char* _time;
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
		C_cnt();
		~C_cnt();

		/* Count File Open */
		char* F_open_cnt_file(char* _cnt_file, char* r_company_id, char* r_cnt_gubun);
		
		/* Read Count File */
		void F_read_cnt();

		/* Write Count File */
		void F_write_cnt();

		/* Update Count File */
		int F_update_cnt(int msg_type);

		/* Update Process Stop Status To Count File */
		int F_put_process_stop(int option);

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
