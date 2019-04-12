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
		C_jang() : _btris(UPMU_GUBUN), _upmu_gubun(UPMU_GUBUN)
		{
			memset(&_jang_bnd, 0x00, sizeof(_jang_bnd));
			memset(&_jang_fts, 0x00, sizeof(_jang_fts));
			memset(_start_time, 0x00, sizeof(_start_time));
			memset(_end_time, 0x00, sizeof(_end_time));
			memset(_jang_status, 0x00, sizeof(_jang_status));
			_jang_status_num = -1;
		}

		char* F_open_jang_file(char* _jang_file)
		{
			_jang.open(_jang_file, ios::in);
			if(!_jang.is_open())
			{
				memset(_write_message, 0x00, sizeof(_write_message));
				sprintf(_write_message, "Jang File Open Error(%s)..", _jang_file);
				cout << _write_message << endl;
				throw _write_message;
			}
			else
			{
				/* Start Jang File Open Message */
				memset(_write_message, 0x00, sizeof(_write_message));
				sprintf(_write_message, "Jang File OPEN(%s)..", _jang_file);
				return _write_message;
			}
		}

		void F_read_jang()
		{
			/* 1. Jang File Positioning */
			_jang.seekg(0, ios::beg);

			/* 2. Read */
			_jang.getline(_jang_bnd.upmu_gubun, JANG_RECORD_LENGTH);
			if(_jang.bad())
			{
				throw "JANG File Write Error..";
			}

			/* 3. Systemp Check */
			if(strncmp(_jang_bnd.upmu_gubun, &_btris, 1) != 0)
			{ /* 상속인 금융거래 일 경우 */
				strncpy(_jang_fts.upmu_gubun, _jang_bnd.upmu_gubun, JANG_RECORD_LENGTH);
				_upmu_gubun = 'K';
			}

			/* 4. Variable Parsing */
			if(_upmu_gubun == UPMU_GUBUN)
			{ /* B-TRIS 일 경우 (From BMMJANG) */
				strncpy(_start_time, _jang_bnd.start_time, 6);
				strncpy(_end_time, _jang_bnd.end_time, 6);
				strncpy(_jang_status, _jang_bnd.jang_status, 1);
				_jang_status_num = atoi(_jang_status);
				if(_jang_status_num < 0)
				{
					cout << "JANG Status atoi() Error.." << endl;
					throw "JANG Status atoi() Error..";
				}
				strncpy(_lastmarket_start_time, _jang_bnd.lastmarket_start_time, 6);
				strncpy(_lastmarket_end_time, _jang_bnd.lastmarket_end_time, 6);
				strncpy(_lastmarket_status, _jang_bnd.lastmarket_status, 1);
				_lastmarket_status_num = atoi(_lastmarket_status);
				if(_lastmarket_status_num < 0)
				{
					cout << "Lastmarket Status atoi() Error.." << endl;
					throw "Lastmarket Status atoi() Error..";
				}
			}
			else
			{ /* 상속인 금융거래 일 경우 (From KOPRMGT) */
				strncpy(_start_time, _jang_fts.start_time, 6);
				strncpy(_end_time, _jang_fts.end_time, 6);
				strncpy(_jang_status, _jang_fts.jang_status, 1);
				_jang_status_num = atoi(_jang_status);
				if(_jang_status_num < 0)
				{
					cout << "JANG Status atoi() Error.." << endl;
					throw "JANG Status atoi() Error..";
				}
			}
		}

		int F_get_jang_status()
		{ /* B-TRIS 일 경우 (From BMMJANG) */
		  /* (전일장) 0:접수 전, 1:접수 중, 9:종료 */
		  /* (당일장) 0:접수 전, 1:접수 중, 9:종료 */
			/* Status Check & Setting */
			if(_upmu_gubun == UPMU_GUBUN)
			{
				if(_lastmarket_status_num == 0 && _jang_status_num == 0)
					return JANG_BEFORE; /* B-TRIS(장외채권공시) 접수 전 */
				else if(_lastmarket_status_num == 1 && _jang_status_num == 0)
       			    return JANG_LAST;   /* B-TRIS(장외채권공시) 전일장 접수 중 */
       			else if(_lastmarket_status_num == 9 && _jang_status_num == 0)
       			    return JANG_TODAY;  /* B-TRIS(장외채권공시) 전일장 마감, 당일장 미접수 -> 당일장 접수로 간주 */
       			else if(_lastmarket_status_num == 9 && _jang_status_num == 1)
       			    return JANG_TODAY;  /* B-TRIS(장외채권공시) 전일장 마감, 당일장 접수*/
       			else if(_lastmarket_status_num == 9 && _jang_status_num == 9)
       			    return JANG_CLOSE;  /* B-TRIS(장외채권공시) 당일장 마감*/
       			else
       			{
       			    memset(_write_message,0x00,sizeof(_write_message));
       			    sprintf(_write_message,"BOND Jang Status error..%d, %d", _lastmarket_status_num, _jang_status_num);
						throw _write_message;
       			}
			}
			else
			{ /* 상속인 금융거래 일 경우 (From KOPRMGT) */
			  /* 0:접수 전, 1:접수 중, 2:종료 */
				switch(_jang_status_num)
				{
					case 0 :
						return JANG_BEFORE;		/* 상속인 금융거래조회 접수 전 */
					case 1 :
						return JANG_TODAY;		/* 상속인 금융거래조회 접수 중 */
					case 2 :
						return JANG_CLOSE;		/* 상속인 금융거래조회 접수 마감 */
					default :
						memset(_write_message, 0x00, sizeof(_write_message));
						sprintf(_write_message, "FTS JANG Status Error..%d", _jang_status_num);
						throw _write_message;
				}
			}
		}
};
