#ifndef __JANG_HEADER__
#define __JANG_HEADER__

#include"main.h"

class C_jang
{
	private :
		fstream _jang;
		char _write_message[150];
		BMMJANG_DEF _jang_bnd;
		KOPRMGT_DEF _jang_fts;
		char _btris;
		char _upmu_gubun;						/* Key : 'B' or 'K' */
		char _start_time				[7];	/* 당일장 시작 시각 (08:00:00) */
		char _end_time					[7];	/* 장 마감 시각 (19:30:00) */
		char _jang_status				[2];	/* 장 상태 (0:접수전, 1:접수중, 9:종료) */
		int _jang_status_num;
		char _lastmarket_start_time		[7];	/* 전일장(전일 미비사항 추가 공시) 시작 시각 (07:00:00) */
		char _lastmarket_end_time		[7];	/* 전일장(전일 미비사항 추가 공시) 마감 시각 (08:00:00) */
		char _lastmarket_status			[2];	/* 전일장(전일 미비사항 추가 공시) 상태 (0:접수전, 1:접수중, 9:종료) */
		int _lastmarket_status_num;
		
	public :
		C_jang();
		~C_jang();
		char* F_open_jang_file(char* _jang_file);
		void F_read_jang();
		int F_get_jang_status();
};

#endif
