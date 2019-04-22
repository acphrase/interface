#ifndef __CNT_HEADER__
#define __CNT_HEADER__

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
		mutex lock_func;

	public :
		C_cnt();
		~C_cnt();
		char* F_open_cnt_file(char* _cnt_file, char* r_company_id, char* r_cnt_gubun);
		void F_read_cnt();
		void F_write_cnt();
		long F_get_last_data_count();
		char* F_get_last_data();
		char* F_get_process();
		char* F_get_link();
		void F_setting_tcpip_error_code(int r_error_code);
		int F_update_cnt(int msg_type);
		int F_put_process_stop(int option);
};

#endif
