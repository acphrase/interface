#ifndef __COMMON_VARIABLE_LIBRARY__
#define __COMMON_VARIABLE_LIBRARY__

/*----------------------------------------------------------------------------*/
/*----------------------------- 1. JANG Status 구분 --------------------------*/
/*----------------------------------------------------------------------------*/
#define   JANG_BEFORE           0      /* 장 시작 전 */
#define   JANG_LAST             1      /* 전일장 */
#define   JANG_TODAY            2      /* 당일장 */
#define   JANG_CLOSE            9      /* 장마감 */
#define   JANG_LINK_MAGAM       7      /* 송신 시 마감TR 전송 후 Set */

/*----------------------------------------------------------------------------*/
/*----------------------------- 2. Status 구분 -------------------------------*/
/*----------------------------------------------------------------------------*/
#define SUCCESS 0
#define FAIL -1
#define CONNECT 1
#define DISCONNECT 0
#define TIMEOUT 2

/*----------------------------------------------------------------------------*/
/*----------------------------- 3. Message 구분 ------------------------------*/
/*----------------------------------------------------------------------------*/
#define RECV_GUBUN			'R'
#define SEND_GUBUN			'S'
#define NORMAL				0
#define START				11
#define END					22
#define ERROR				99
#define MSG_0800_001		8001
#define MSG_0810_001		8101
#define MSG_0800_301		8031
#define MSG_0810_301		8131
#define MSG_0800_040		8040
#define MSG_0810_040		8140
#define MSG_0200_000		2000
#define MSG_0210_000		2100

/*----------------------------------------------------------------------------*/
/*----------------------------- 4. Message error 구분 ------------------------*/
/*----------------------------------------------------------------------------*/
#define   NO_ERROR              0
#define   SEQ_ERROR             1
#define   CNT_ERROR             2
#define   MARKET_BEF_ERROR      3
#define   MARKET_AFT_ERROR      4
#define   FORMAT_ERROR          14
#define   PARSING_ERROR         18
#define   TR_CODE_INVALID       91
#define   GIGWAN_ID_INVALID     92
#define   MSG_TYPE_INVALID      93     /* AnyLink에서 처리 */
#define   OPR_TYPE_INVALID      94     /* AnyLink에서 처리 */
#define   DATA_NO_INVALID       98
#define   DATA_CNT_INVALID      99

/*----------------------------------------------------------------------------*/
/*------------------------- 1. CONFIG ----------------------------------------*/
/*----------------------------------------------------------------------------*/
#define CONFIG_RECORD_LENGTH 267

/*----------------------------------------------------------------------------*/
/*----------------------------- 1. Message 구분 ------------------------------*/
/*----------------------------------------------------------------------------*/
#define SND_RCV_CHK			500		/* 500: 송신시 recv 있음 */
#define UNDEFINED			-1
#define HEADER_LENGTH		4		/* 전문길이정보 */
#define BLOCK_COUNT			1		/* 전문블록 갯수 */

/*----------------------------------------------------------------------------*/
/*----------------------------- 2. TIMEOUT 구분 ------------------------------*/
/*----------------------------------------------------------------------------*/
#define WAIT_TIMEOUT		-1
#define RECV_TIMEOUT		30000
#define SEND_TIMEOUT		30000
#define CHECK_TIMEOUT		1
//#define RECV_TIMEOUT		3000
//#define SEND_TIMEOUT		3000

/*----------------------------------------------------------------------------*/
/*----------------------------- 3. ETC ---------------------------------------*/
/*----------------------------------------------------------------------------*/
#define HEADER_TR_CODE		'N'
#define BUF_SIZE			4000
#define EPOLL_SIZE			5

/*----------------------------------------------------------------------------*/
/*------------------------- 1. COUNT(SEQUENCE) -------------------------------*/
/*----------------------------------------------------------------------------*/
#define CNT_RECORD_LENGTH 100
#define INCOMPLETE "0"
#define COMPLETE "1"
#define ABNORMAL_STATUS "2"
#define NORMAL_STATUS "3"
#define ABNORMAL_STOP "5"
#define NOW_TIME 6
#define TODAY_DATE 8
#define NORMAL_STOP "9"

/*----------------------------------------------------------------------------*/
/*------------------------- 1. DATA ------------------------------------------*/
/*----------------------------------------------------------------------------*/
#define CHECK_DATA 1

/*----------------------------------------------------------------------------*/
/*------------------------- 1. JANG ------------------------------------------*/
/*----------------------------------------------------------------------------*/
#define JANG_RECORD_LENGTH 101
#define UPMU_GUBUN 'B'


/*----------------------------------------------------------------------------*/
/*------------------------- 1. JANG ------------------------------------------*/
/*----------------------------------------------------------------------------*/
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
/*----------------------------------------------------------------------------*/
/*------------------------- 1. Data 수신 File 관련 변수 ----------------------*/
/*----------------------------------------------------------------------------*/
struct BGERERV_FILE_DEF
{	/* (1) B-TRIS, 채권거래내역(L1):"BGERERV" File Definition */
	char company_id			[3];
	char sequence_number	[8];
  	char error_code			[3];
  	char receive_time		[6];
  	char tr_code			[2];
  	char sub_tr_code		[2];
  	char tr_code2			[2];
  	char sub_tr_code2		[2];
  	char gerercv			[112];
  	char bogo_il			[8];
  	char gerercv1			[376];
  	char filler				[26];
  	char filler1			[1];   /* file rec : 550 bytes + 1 bytes(0x00) */
};

struct BSOKDLRCV_FILE_DEF
{	/* (2) B-TRIS, 채권판매정보(L7):"BSOKRCV", 전문딜러(T2):"BDLRRCV" File Definition */
	char company_id			[3];
	char sequence_number	[8];
  	char error_code			[3];
  	char receive_time		[6];
  	char tr_code			[2];
  	char sub_tr_code		[2];
  	char tr_code2			[2];
  	char sub_tr_code2		[2];
  	char soakdlrcv			[488];
  	char filler				[34];
  	char filler1			[1];   /* file rec : 550 bytes + 1 bytes(0x00) */
};

struct BCDRPGRCV_FILE_DEF
{	/* (3) B-TRIS, CD수신(M1):"BCDGRCV", REPO수신(M3)"BRPGRCV" File Definition */
	char company_id			[3];
	char sequence_number	[8];
  	char error_code			[3];
  	char receive_time		[6];
  	char tr_code			[2];
  	char sub_tr_code		[2];
  	char tr_code2			[2];
  	char sub_tr_code2		[2];
  	char cdrpgercv			[488];
  	char filler				[34];
  	char filler1			[1];   /* file rec : 550 bytes + 1 bytes(0x00) */
};

struct KMBRRCV_FILE_DEF
{	/* (4) 상속인 금융거래조회, 요청 수신(K1):"KMBRRCV" File Definition */
	char tr_code			[2];
	char sub_tr_code		[2];
  	char system_type		[1];
  	char company_id			[3];
  	char sequence_number	[6];
  	char reject_code		[3];
  	char receive_time		[6];
  	char company_id2		[3];
  	char fs_sequence_no		[11];
  	char fs_time			[8];
  	char app_time			[8];
  	char jeobsu_gubun		[2];
  	char name_tgt			[30];
  	char code_tgt			[16];
  	char accnt_cnt			[2];
  	char jango_gubun		[1];
  	char debt_amt			[18];
  	char debt_not_decided	[1];
  	char filler				[377];
  	char filler1			[1];   /* file rec : 500 bytes + 1 bytes(0x00) */
};

/*----------------------------------------------------------------------------*/
/*------------------------- 2. 송신 File 관련 변수 설정 ----------------------*/
/*----------------------------------------------------------------------------*/
struct KSND_DEF
{	/* (1) 상속인 금융거래조회, 요청 송신(K2):"KSNDnnn" File Definition */
	char sequence_number	[6];
	char tr_code			[2];
  	char sub_tr_code		[2];
  	char company_id			[3];
  	char sequence_number2	[11];
  	char fs_time			[8];
  	char app_time			[8];
  	char jeobsu_gubun		[2];
  	char req_type			[1];
  	char fs_sequence_number	[11];
  	char fs_time2			[8];
  	char app_time2			[8];
  	char jeobsu_gubun2		[2];
  	char name_req			[30];
  	char code_req			[16];  /* 16  -> 13 */
  	char zip_req			[16];  /* 16  -> 6 */
  	char address_req		[208]; /* 208 -> 200 */
  	char phone_req			[32];  /* 32  -> 24 */
  	char mobile_req			[32];  /* 32  -> 24 */
  	char email_req			[64];  /* 64  -> 50 */
  	char name_inherit		[30]; 
  	char code_inherit		[16];  /* 16  -> 13 */
  	char zip_inherit		[16];  /* 16  -> 6 */
  	char address_inherit	[208]; /* 208 -> 200 */
  	char phone_inherit		[32];  /* 32  -> 24 */
  	char mobile_inherit		[32];  /* 32  -> 24 */
  	char email_inherit		[64];  /* 64  -> 50 */
  	char name_tgt			[30];
  	char code_tgt			[16];  /* 16  -> 13 */
  	char zip_tgt			[16];  /* 16  -> 6 */
  	char address_tgt		[208]; /* 208 -> 200 */
  	char yymmdd_tgt			[8];
  	char relation_tgt		[2];
  	char gubun_tgt			[2];
  	char kofia_inform_il	[8];
  	char jeobsu_gigwan		[4];
  	char list_jijum			[100];
  	char list_number		[20];
  	char trans_gigwan		[20];
  	char change_cancel		[500];
	/* char   filler                [2]; */
	char filler1			[1]; /* file rec : 1,802 bytes + 1 bytes(0x00) */
};

struct KRJT_DEF
{	/* (2) 상속인 금융거래조회, 거부 송신(K3):"KRJTnnn" File Definition */
	char sequence_number	[6];
	char tr_code			[2];
  	char sub_tr_code		[2];
  	char reject_code		[3];
  	char system_type		[1];
  	char company_id			[3];
  	char sequence_number2	[6];
  	char company_id2		[3];
  	char fs_sequence_number	[11];
  	char fs_time			[8];
  	char app_time			[8];
  	char jeobsu_gubun		[2];
  	char name_tgt			[30];
  	char code_tgt			[16];
  	char accnt_cnt			[2];
  	char jango_gubun		[1];
  	char debt_amt			[18];
  	char debt_not_decided	[1];
  	char filler				[377];
  	char filler1			[1];   /* file rec : 500 bytes + 1 bytes(0x00) */
};

/*----------------------------------------------------------------------------*/
/*------------------------- 1. CNT -------------------------------------------*/
/*----------------------------------------------------------------------------*/
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

struct RECV_MESSAGE_DEF
{ /* 수신 메세지 definition */
  char message_length           [4];
  char tr_code                  [9];   /* TR-code : "N       " */
  char gigwan_id                [3];   /* 기관 id : 999 */
  char msg_type                 [4];   /* 전문 type : 0800,0810,0200,0210 */
  char opr_type                 [3];   /* 운용 type : 000,001,002,040,301 */
  char err_code                 [2];   /* 오류 code : 00:정상,기타:ERROR */
  char time                     [12];  /* 날짜 및 시간 : yymmddhhmmss */
  char retry_cnt                [2];   /* 재송횟수 */
  char data_no                  [8];   /* (Header부)DATA 번호 */
  char data_cnt                 [2];   /* DATA 갯수 */
  char data_seq                 [8];   /* (Data부) DATA seq */
  char data_tr_code             [2];   /* (Data부) TR code */
  char data_sub_tr_code         [2];   /* (Data부) SUB TR code */
  char rcv_data                 [3939];
}; /* 수신 버퍼 총 4000 byte */

struct SEND_MESSAGE_DEF
{ /* 송신 메세지 definition */
  char message_length           [4];
  char tr_code                  [9];   /* TR-code : "N       " */
  char gigwan_id                [3];   /* 기관 id : 999 */
  char msg_type                 [4];   /* 전문 type : 0800,0810,0200,0210 */
  char opr_type                 [3];   /* 운용 type : 000,001,002,040,301 */
  char err_code                 [2];   /* 오류 code : 00:정상,기타:ERROR */
  char time                     [12];  /* 날짜 및 시간 : yymmddhhmmss */
  char retry_cnt                [2];   /* 재송횟수 */
  char data_no                  [8];   /* DATA SEQ */
  char data_cnt                 [2];   /* DATA 갯수 */
  char snd_data					[3951];
}; /* 송신 버퍼 총 4000 byte */

#endif
