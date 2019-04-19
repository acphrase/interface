#ifndef __JANG_HEADER__
#define __JANG_HEADER__

#include"main.h"

#define JANG_RECORD_LENGTH 101
#define UPMU_GUBUN 'B'

struct BMMJANG_DEF{
	char upmu_gubun				[1];	/* Key : 'B' */
	char working_day_gubun		[1];	/* -- 미사용 (1:영업일, 2:휴일) */
	char if_start_time			[6];	/* I/F 기동 시각 (06:30:00) */
	char lastmarket_start_time	[6];	/* 전일장(전일 미비사항 추가 공시) 시작 시각 (07:00:00) */
	char lastmarket_end_time	[6];	/* 전일장(전일 미비사항 추가 공시) 마감 시각 (08:00:00) */
	char lastmarket_status		[1];	/* 전일장(전일 미비사항 추가 공시) 상태 (0:접수전, 1:접수중, 9:종료) */
	char start_time				[6];	/* 당일장 시작 시각 (08:00:00) */
	char end_time				[6];	/* 당일장 마감 시각 (19:30:00) */
	char jang_status			[1];	/* 당일장 상태 (0:접수전, 1:접수중, 9:종료) */
	char end_yn					[1];	/* -- 미사용 (채권장외거래내역 공시 종료 여부 0:접수전, 9:종료) */
	char gere_end_time			[6];	/* -- 미사용 (채권장외거래내역 공시 종료 시각) */
	char suik_end_yn			[1];	/* -- 미사용 (수익률 공시 종료 여부 0:접수전, 9:종료) */
	char suik_end_time			[6];	/* -- 미사용 (수익률 공시 종료 시각) */
	char bal_end_yn				[1];	/* -- 미사용 (발행정보 공시 종료 여부 0:접수전, 9:종료) */
	char bal_end_time			[6];	/* -- 미사용 (발행정보 공시 종료 시각) */
	char magam_yn				[1];	/* -- 미사용 (최종 마감 여부 0:운영중, 9:마감) */
	char magam_time				[6];	/* -- 미사용 (최종 마감 시각) */
	char batch_if_start_time	[6];	/* -- 미사용 (배치 I/F 기동 시각 (17:30:00) */
	char filler					[32];	/* -- 미사용 (여분) */
	char end					[1];	/* NULL */
};

struct KOPRMGT_DEF{
	char upmu_gubun				[1];	/* Key : 'K' */
	char working_day_gubun		[1];	/* -- 미사용 (1. 영업일, 2. 휴일) */
	char jang_status			[1];	/* 장 상태 (0:접수전, 1:접수중, 9:종료) */
	char if_start_time			[6];	/* I/F 기동 시각 (06:30:00) */
	char start_time				[6];	/* 장 시작 시각 (08:00:00) */
	char end_time				[6];	/* 장 마감 시각 (19:30:00) */
	char process1_end_yn		[1];	/* -- 미사용 (가공1 종료 여부) */
	char process1_end_time		[6];	/* -- 미사용 (가공1 종료 시각) */
	char process2_end_yn		[1];	/* -- 미사용 (가공2 종료 여부) */
	char process2_end_time		[6];	/* -- 미사용 (가공2 종료 시각) */
	char accept_end_yn			[1];	/* -- 미사용 (요청 수신(K1) 종료 여부) */
	char accept_end_time		[6];	/* -- 미사용 (요청 수신(K1) 종료 시각) */
	char divide_end_yn			[1];	/* -- 미사용 (요청 송신(K2) 종료 여부) */
	char divide_end_time		[6];	/* -- 미사용 (요청 송신(K2) 종료 시각) */
	char reject_end_yn			[1];	/* -- 미사용 (거부 송신(K3) 종료 여부) */
	char reject_end_time		[6];	/* -- 미사용 (거부 송신(K3) 종료 시각) */
	char filler					[44];	/* -- 미사용 (여분) */
	char end					[1];	/* NULL */
};

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
