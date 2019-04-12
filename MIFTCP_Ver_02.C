/*---------------------------------- 통합 Interface Program  ------------------------------*/
/*-----------------------------------------------------------------------------------------*/
/*-- #01 PROGRAM 명   : 금융투자협회 I/F PROGRAM                                         --*/
/*-- #02 PROGRAM ID   : MIFTCPC                                                          --*/
/*-- #03 사용  언어   : C                                                                --*/
/*-- #04 최초코딩일   : 2015 년 08 월 02 일                                              --*/
/*-- #05 가 동 일 자  : 2015 년 09 월 21 일                                              --*/
/*-- #06 START-UP MSG : ZMIFPROG/$XXXXXX, NOWAIT/ 기관코드 TR_CODE $DISK02.FAMST.TCONFIG --*/
/*-----------------------------------------------------------------------------------------*/
/*-- #07 프로그램 수정                                                                   --*/
/*-----------------------------------------------------------------------------------------*/

#pragma ssv0   "$system.system" /*pragma : 호출할 함수를 */
#pragma ssv1   "$system.ztcpip" /*         찾는 경로     */

#include <paramh>
#include <socketh>
#include <inh>
#include <netdbh>
#include <mathh>
#include <stdio.h>
#include <time.h>
#include <fcntlh>
#include <stringh>
#include <stdlibh>
#include <memoryh>
#include <errnoh>
#include <talh>
#include <cextdecs(CONTROL, AWAITIOX, PROCESS_STOP_, FILE_CLOSE_, FILE_GETINFO_, FILE_OPEN_, \
                   WRITE, WRITEX, READ, READX, KEYPOSITIONX, DNUMOUT, NUMOUT, DNUMIN, NUMIN, \
                   SETMODE, CANCELREQ, BEGINTRANSACTION, ENDTRANSACTION, ABORTTRANSACTION,   \
                   READUPDATELOCKX, WRITEUPDATEUNLOCKX, POSITION, DELAY)>

extern int EncData(unsigned char *,unsigned int,unsigned char **,unsigned int *);
extern int DecData(unsigned char *, unsigned int InputBytes, unsigned char **, unsigned int *);

/*----------------------------------------------------------------------------*/
/*----------------------------- 1. Operation result --------------------------*/
/*----------------------------------------------------------------------------*/
#define   save_abend            1
#define   SUCCESS               0
#define   FAIL                  -1
#define   AWAITIO_ANY           -1

/*----------------------------------------------------------------------------*/
/*----------------------------- 2. File access type --------------------------*/
/*----------------------------------------------------------------------------*/
#define   read_write            0
#define   read_only             1
#define   write_only            2

/*----------------------------------------------------------------------------*/
/*----------------------------- 3. File read mode ----------------------------*/
/*----------------------------------------------------------------------------*/
#define   exact                 2
#define   generic               1
#define   approximate           0

/*----------------------------------------------------------------------------*/
/*----------------------------- 4. Access sharing option ---------------------*/
/*----------------------------------------------------------------------------*/
#define   shared_access         0
#define   exclusive_access      1

/*----------------------------------------------------------------------------*/
/*----------------------------- 5. DELAY -------------------------------------*/
/*----------------------------------------------------------------------------*/
#define   TCP_IO_DELAY          3000   /* 30초 : 수신 시 네트워크 지연 감안 10초 추가 */
#define   FILE_IO_DELAY         2500   /* 25초 :송신 시 파일 기다리는 시간 */
#define   BIND_IO_DELAY         1000   /* 10초 : wait time of socket recreation */
#define   BATCH_IO_DELAY        0003   /* 0.03초 : 배치업무(K2,F3) Delay */
#define   BATCH_IO_DELAY_L4     0100   /* 1.00초 : 배치업무(K2,F3) Delay */

/*----------------------------------------------------------------------------*/
/*----------------------------- 6. Message 구분 ------------------------------*/
/*----------------------------------------------------------------------------*/
#define   SND_RCV_CHK           500    /* 500: 송신시 recv 있음 */
#define   UNDEFINED             -1
#define   MESSAGE_LENGTH        4      /* 전문길이정보 */
#define   BLOCK_COUNT           1      /* 전문블록 갯수 */
#define   MSG_0800_001          8001
#define   MSG_0810_001          8101
#define   MSG_0800_301          8031
#define   MSG_0810_301          8131
#define   MSG_0800_040          8040
#define   MSG_0810_040          8140
#define   MSG_0200_000          2000
#define   MSG_0210_000          2100
#define   MSG_START             11
#define   MSG_ERR               99

/*----------------------------------------------------------------------------*/
/*----------------------------- 7. Message error 구분 ------------------------*/
/*----------------------------------------------------------------------------*/
#define   HEADER_LENGTH         45
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
#define   ERR_CODE_INVALID      95     /* 정의는 되어 있지만 사용하지 않음 */
#define   TIME_INVALID          96     /* 정의는 되어 있지만 사용하지 않음 */
#define   RETRY_CNT_INVALID     97     /* 정의는 되어 있지만 사용하지 않음 */
#define   DATA_NO_INVALID       98
#define   DATA_CNT_INVALID      99

/*----------------------------------------------------------------------------*/
/*----------------------------- 8. JANG Status 구분 --------------------------*/
/*----------------------------------------------------------------------------*/
#define   JANG_BEFORE           0      /* 장전 */
#define   JANG_LAST             1      /* 직전거래일 */
#define   JANG_TODAY            2      /* 당일 */
#define   JANG_CLOSE            9      /* 장마감 */
#define   JANG_BATCH            8      /* 신용정보 전체송신 Set */
#define   JANG_LINK_MAGAM       7      /* 송신 시 마감TR 전송 후 Set */

/*----------------------------------------------------------------------------*/
/*----------------------------- 9. ETC ---------------------------------------*/
/*----------------------------------------------------------------------------*/
#define   IO_TIME_OUT           40
#define   LOG_TYPE_NORMAL       1
#define   LOG_TYPE_RECV         2
#define   LOG_TYPE_SEND         3
#define   MSG_STRT              4
#define   MSG_NORMAL_END        5
#define   MSG_ABNORMAL_END      6
#define   MSG_LINK              7
#define   MSG_RE_LINK           8
#define   MSG_RETRY             9
#define   MSG_RECV_ERR          10

/*----------------------------------------------------------------------------*/
/*----------------------------- 1. Socket 관련 변수 설정 ---------------------*/
/*----------------------------------------------------------------------------*/
int    socket_num;
int    new_socket_num;
int    address_len, port_number;
short  sock_err;
char   ip_address[24], gc_inet[8], gc_ip[20];
struct sockaddr_in  loc_addr;
struct sockaddr_in  from_addr;
struct in_addr in;

/*----------------------------------------------------------------------------*/
/*----------------------------- 2. Socket recv, send length 관련 변수 설정 ---*/
/*----------------------------------------------------------------------------*/
int    gi_read_message_length;         /* 소켓에서 읽은 전문 length : Header(45Bytes) + DATA부(n Bytes) */
int    gi_send_message_length;         /* 소켓에 송신한 전문 length : Header(45Bytes) + DATA부(n Bytes) */
short  gs_message_length;              /* 송신전문 길이 */
short  gs_retry_cnt;                   /* 재전송 숫자 */
short  gs_rcv_err_code;                /* rcv data error code */
short  gs_snd_err_code;                /* snd data error code */
struct send_nw_str *gst_snw;           /* 송신Bytes를 return받는 변수 */

/*----------------------------------------------------------------------------*/
/*----------------------------- 3. Header 관련 변수 설정 ---------------------*/
/*----------------------------------------------------------------------------*/
char   TR_CODE                  [10];

/*----------------------------------------------------------------------------*/
/*----------------------------- 4. Date and Time, JANG 변수 선언부 -----------*/
/*----------------------------------------------------------------------------*/
char   gc_today_date            [9];
char   gc_edit_date             [11];
char   gc_today_time            [7];
char   gc_edit_time             [9];
char   gc_rcv_time              [7];
char   gc_jang_start_time       [7];
char   gc_jang_end_time         [7];
char   gc_lastmarket_start_time [7];
char   gc_lastmarket_end_time   [7];
char   gc_jang_status           [2];
char   gc_lastmarket_jang_status[2];
short  gs_jang_status           = 0;   /* 0:접수 전, 1:직전 접수 중(신용 접수), 2:당일 접수 중(신용/직전 접수마감), 9:종료(전체 송신) */

/*----------------------------------------------------------------------------*/
/*----------------------------- 5. Data count & status 변수 선언부 -----------*/
/*----------------------------------------------------------------------------*/
long   gl_last_data_count       = 0L;
long   gl_next_data_count       = 0L;
short  gs_proc_status           = 0;
short  gs_link_status           = 0;   /* 0:미LINK, 1:LINK, 2:Link종료송신 3:Link종료수신(최종 종료) */
short  gs_start_status          = 0;   /* 0: 초기 또는 재기동, 1: 기동 후 */
short  gs_conn_status           = 0;   /* 0: socket connect 전, 1: socket connect 후 */
char   gc_last_data_count       [9];
char   gc_next_data_count       [9];
char   gc_proc_status           [2];
char   gc_link_status           [3];

long   gl_rcv_data_no           = 0L;  /* 수신한 header부의 data no */
long   gl_rcv_seq_no            = 0L;  /* 수신한 data부의 data seq */
short  gs_rcv_data_cnt          = 0;   /* 수신한 header부의 block no */
short  gs_rcv_message_type      = 0;   /* 수신한 Message Type */
short  gs_snd_message_type      = 0;   /* 송신할 Message Type */
unsigned short gs_bytesread     = 0;

/*----------------------------------------------------------------------------*/
/*----------------------------- 6. Awaito ------------------------------------*/
/*----------------------------------------------------------------------------*/
short gs_io_error_no            =   0; /* Awaitio Error number */
long  gl_tagback                =   0; /* Awaitio에서 돌려받는 tag */
long  gl_socket_tag             = 990; /* for socket nowatio I/O ID */
long  gl_new_socket_tag         = 991; /* for new socket nowatio I/O ID */
long  gl_rcv_tag                = 992; /* for recv_nw ID */
long  gl_snd_tag                = 993; /* for send_nw ID */
long  gl_file_tag               = 994; /* for control27 ID */

/*----------------------------------------------------------------------------*/
/*----------------------------- 7. File Open Number 변수 선언부 --------------*/
/*----------------------------------------------------------------------------*/
short gs_open_data_file_num     = -1;
short gs_open_data_file_num_27  = -1;
short gs_open_log_file_num      = -1;
short gs_open_cnt_file_num      = -1;
short gs_open_jang_file_num     = -1;
short gs_open_msg_file_num      = -1;
short gs_tconfig_file_num       = -1;
char  gc_tconfig_file_name      [27];
char  gc_tconfig_key            [6];

/*----------------------------------------------------------------------------*/
/*----------------------------- 8. Memory buffer 관련 변수 설정 --------------*/
/*----------------------------------------------------------------------------*/
char   gc_cnt_key               [6];   /* count File Key */
char   gc_display_message       [150]; /* Message & Log buffer */
char   gc_rcv_buff              [4000];/* 수신 buffer */
char   gc_data_file_buff        [4000];/* Read한 data를 담는 buffer */

struct RCV_MESSAGE_DEF{         /* 수신 메세지 definition */
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
} gst_rcv_buff;                        /* 수신 버퍼 총 4000 byte */

struct SEND_MESSAGE_DEF{        /* 송신 메세지 definition */
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
  char snd_data              [3951];
} gst_snd_buff;                        /* 송신 버퍼 총 4000 byte */

struct SEND_MESSAGE_996_DEF{    /* PB 송신 메세지 definition */
  char snd_996_header           [61];
  char snd_996_comp_id          [3];
  char snd_996_data             [3936];
} gst_snd_996_buff;                    /* 송신 버퍼 총 4000 byte */

/*----------------------------------------------------------------------------*/
/*----------------------------- 1. TCONFIG File 관련 변수 설정 ---------------*/
/*----------------------------------------------------------------------------*/
struct IFCONFIG_FILE_DEF{       /* 공통:"TCONFIG" File Definition */
  char gc_line_id               [5];
  char gc_delimiter1            [1];
  char gc_comm_type             [1];
  char gc_delimiter2            [1];
  char gc_proc_name             [10];
  char gc_delimiter3            [1];
  char gc_obj_name              [29];
  char gc_delimiter4            [1];
  char gc_port_num              [8];
  char gc_delimiter5            [1];
  char gc_ip_num                [42];
  char gc_delimiter6            [1];
  char gc_tcp_proc_name         [10];
  char gc_delimiter7            [1];
  char gc_data_file_name        [29];
  char gc_delimiter8            [1];
  char gc_log_file_name         [29];
  char gc_delimiter9            [1];
  char gc_cnt_file_name         [29];
  char gc_delimiter10           [1];
  char gc_jang_file_name        [29];
  char gc_delimiter11           [1];
  char gc_cnt_gubun             [2];
  char gc_delimiter12           [1];
  char gc_message_length        [4];
  char gc_delimiter13           [1];
  char gc_msg_file_name         [25];
  char filler                   [1];
} gst_tconfig_rec;

/*----------------------------- TCONFIG LENGTH 변수 선언부 -------------------*/
char  gc_tconfig_comp_id[4], gc_tconfig_tr_code[3], gc_tconfig_comm_type  [2];
short gs_tconfig_proc_name_len      =0;  char gc_tconfig_proc_name        [8];
short gs_tconfig_obj_name_len       =0;  char gc_tconfig_obj_name        [27];
short gs_tconfig_port_num_len       =0;  char gc_tconfig_port_num         [6];  unsigned short gs_tconfig_port_num=0;
short gs_tconfig_ip_num_len         =0;  char gc_tconfig_ip_num          [40];
short gs_tconfig_tcp_proc_name_len  =0;  char gc_tconfig_tcp_proc_name    [8];
short gs_tconfig_data_file_name_len =0;  char gc_tconfig_data_file_name  [27];
short gs_tconfig_log_file_name_len  =0;  char gc_tconfig_log_file_name   [27];
short gs_tconfig_cnt_file_name_len  =0;  char gc_tconfig_cnt_file_name   [27];
short gs_tconfig_jang_file_name_len =0;  char gc_tconfig_jang_file_name  [27];
short gs_tconfig_msg_file_name_len  =0;  char gc_tconfig_msg_file_name   [27];
short gs_tconfig_message_length     =0;  char gc_tconfig_message_length   [5];
char  gc_tconfig_cnt_gubun         [3];

/*----------------------------------------------------------------------------*/
/*----------------------------- 2. MSG file 관련 변수 설정 -------------------*/
/*----------------------------------------------------------------------------*/
struct MSG_FILE_DEF {           /* B-TRIS:"BMSGF99", 상속인 금융거래조회:"KMSGR99" File Definition */
  char process_id               [8];
  char sigak                    [8];
  char gubun                    [2];
  char error_code               [2];
  char message                  [60];
  char message_len              [2];
  char debug_ok                 [1];
  char bell_ok                  [1];
  char on_batch                 [1];
  char group_no                 [2];
  char disp_cnt                 [2];
  char filler                   [62];  /* file rec : 150 bytes + 2 bytes(0x00) */
} gst_msg_rec;

/*----------------------------------------------------------------------------*/
/*----------------------------- 3. 수신 File 관련 변수 설정 ------------------*/
/*----------------------------------------------------------------------------*/
struct BGERERV_FILE_DEF{        /* (1) B-TRIS, 채권거래내역(L1):"BGERERV" File Definition */
  char bgererv_if_whisa_code    [3];
  char bgererv_if_seq_no        [8];
  char bgererv_if_error_code    [3];
  char bgererv_if_rcv_sigak     [6];
  char bgererv_tr_code          [2];
  char bgererv_sub_tr_code      [2];
  char tr_code                  [2];
  char sub_tr_code              [2];
  char gerercv                  [112];
  char gerercv_bogo_il          [8];
  char gerercv1                 [376];
  char filler                   [26];
  char filler1                  [1];   /* file rec : 550 bytes + 1 bytes(0x00) */
} gst_bgererv_rec;

struct BSOKDLRCV_FILE_DEF{      /* (2) B-TRIS, 채권판매정보(L7):"BSOKRCV", 전문딜러(T2):"BDLRRCV" File Definition */
  char bsodlrcv_if_whisa_code   [3];
  char bsodlrcv_if_seq_no       [8];
  char bsodlrcv_if_error_code   [3];
  char bsodlrcv_if_rcv_sigak    [6];
  char bsodlrcv_tr_code         [2];
  char bsodlrcv_sub_tr_code     [2];
  char tr_code                  [2];
  char sub_tr_code              [2];
  char soakdlrcv                [488];
  char filler                   [34];
  char filler1                  [1];   /* file rec : 550 bytes + 1 bytes(0x00) */
} gst_bsodlrcv_rec;

struct BCDRPGRCV_FILE_DEF{      /* (3) B-TRIS, CD수신(M1):"BCDGRCV", REPO수신(M3)"BRPGRCV" File Definition */
  char bcdrpgrcv_if_whisa_code  [3];
  char bcdrpgrcv_if_seq_no      [8];
  char bcdrpgrcv_if_error_code  [3];
  char bcdrpgrcv_if_rcv_sigak   [6];
  char bcdrpgrcv_tr_code        [2];
  char bcdrpgrcv_sub_tr_code    [2];
  char tr_code                  [2];
  char sub_tr_code              [2];
  char cdrpgercv                [488];
  char filler                   [34];
  char filler1                  [1];   /* file rec : 550 bytes + 1 bytes(0x00) */
} gst_bcdrpgrcv_rec;

struct KMBRRCV_FILE_DEF {       /* (4) 상속인 금융거래조회, 요청 수신(K1):"KMBRRCV" File Definition */
  char kmbrrcv_tr_code          [2];
  char kmbrrcv_sub_code         [2];
  char kmbrrcv_system_type      [1];
  char kmbrrcv_mbr_no           [3];
  char kmbrrcv_mbr_seq_no       [6];
  char kmbrrcv_rjt_code         [3];
  char kmbrrcv_rcv_hhmmss       [6];
  char mbr_no                   [3];
  char fs_seq_no                [11];
  char fs_yymmdd                [8];
  char app_yymmdd               [8];
  char jeobsu_gubun             [2];
  char name_tgt                 [30];
  char code_tgt                 [16];
  char accnt_cnt                [2];
  char jango_gubun              [1];
  char debt_amt                 [18];
  char debt_not_decided         [1];
  char filler                   [377];
  char filler1                  [1];   /* file rec : 500 bytes + 1 bytes(0x00) */
} gst_ftsrcv_rec;

struct RR_FILE_DEF {            /* (5) 신용정보 수신 File Definition */
  char gc_data_pk               [8];   /* pk = DATA SEQ와 동일 */
  char gc_h_tr_code             [9];   /* TR-CODE */
  char gc_h_member_code         [3];   /* 기관ID */
  char gc_h_initial_type        [4];   /* 전문TYPE */
  char gc_h_work_type           [3];   /* 운용TYPE */
  char gc_h_error_code          [2];   /* 오류CODE */
  char gc_h_send_datetime       [12];  /* TIME */
  char gc_h_resend_count        [2];   /* 재전송횟수 */
  char gc_h_data_number         [8];   /* DATA 번호 */
  char gc_h_data_count          [2];   /* DATA 개수 */
  char gc_space                 [1];   /* Data구분자 */
  char gc_data                  [120];
  char filler                   [26];
  char filler1                  [1];   /* file rec : 200 bytes + 1 bytes(0x00) */
} gst_farcv_rec;

/*----------------------------------------------------------------------------*/
/*----------------------------- 4. 송신 File 관련 변수 설정 ------------------*/
/*----------------------------------------------------------------------------*/
struct FTS_KSND_DEF {           /* (1) 상속인 금융거래조회, 요청 송신(K2):"KSNDnnn" File Definition */
  char ksnd000_seq_no           [6];
  char ksnd000_tr_code          [2];
  char ksnd000_sub_code         [2];
  char ksnd000_mbr_no           [3];
  char ksnd000_fs_seq_no        [11];
  char ksnd000_fs_yymmdd        [8];
  char ksnd000_app_yymmdd       [8];
  char ksnd000_jeobsu_gubun     [2];
  char req_type                 [1];
  char fs_seq_no                [11];
  char fs_yymmdd                [8];
  char app_yymmdd               [8];
  char jeobsu_gubun             [2];
  char name_req                 [30];
  char code_req                 [16];  /* 16  -> 13 */
  char zip_req                  [16];  /* 16  -> 6 */
  char address_req              [208]; /* 208 -> 200 */
  char phone_req                [32];  /* 32  -> 24 */
  char mobile_req               [32];  /* 32  -> 24 */
  char email_req                [64];  /* 64  -> 50 */
  char name_inherit             [30]; 
  char code_inherit             [16];  /* 16  -> 13 */
  char zip_inherit              [16];  /* 16  -> 6 */
  char address_inherit          [208]; /* 208 -> 200 */
  char phone_inherit            [32];  /* 32  -> 24 */
  char mobile_inherit           [32];  /* 32  -> 24 */
  char email_inherit            [64];  /* 64  -> 50 */
  char name_tgt                 [30];
  char code_tgt                 [16];  /* 16  -> 13 */
  char zip_tgt                  [16];  /* 16  -> 6 */
  char address_tgt              [208]; /* 208 -> 200 */
  char yymmdd_tgt               [8];
  char relation_tgt             [2];
  char gubun_tgt                [2];
  char kofia_inform_il          [8];
  char jeobsu_gigwan            [4];
  char list_jijum               [100];
  char list_number              [20];
  char trans_gigwan             [20];
  char change_cancel            [500];
/* char   filler                [2]; */
  char filler1                  [1]; /* file rec : 1,802 bytes + 1 bytes(0x00) */
} gst_fts_ksnd_rec;

struct FTS_KRJT_DEF {           /* (2) 상속인 금융거래조회, 거부 송신(K3):"KRJETnnn" File Definition */
  char krjt000_seq_no           [6];
  char krjt000_tr_code          [2];
  char krjt000_sub_tr_code      [2];
  char krjt000_rjt_code         [3];
  char krjt000_system_type      [1];
  char krjt000_mbr_no           [3];
  char krjt000_mbr_seq_no       [6];
  char mbr_no                   [3];
  char fs_seq_no                [11];
  char fs_yymmdd                [8];
  char app_yymmdd               [8];
  char jeobsu_gubun             [2];
  char name_tgt                 [30];
  char code_tgt                 [16];
  char accnt_cnt                [2];
  char jango_gubun              [1];
  char debt_amt                 [18];
  char debt_not_decided         [1];
  char filler                   [377];
  char filler1                  [1];   /* file rec : 500 bytes + 1 bytes(0x00) */
} gst_fts_krjt_rec;

/*----------------------------------------------------------------------------*/
/*----------------------------- 5. Count(SEQ) File 관련 변수 설정 ------------*/
/*----------------------------------------------------------------------------*/
struct BOND_CNT_REC_DEF {       /* (1) B-TRIS채권:"BSQIFRV, BSQIFSD", 상속인 금융거래조회:"KSQMBSD, KSQMBRV" File Definition */
  char gc_member_code           [3];   /* 기관코드 */
  char gc_upmu_gubun            [2];   /* 구분코드 */
  char gc_proc_status           [1];   /* 0:미기동,1:정상기동,2:비정상STOP,3:정상STOP */
  char bsqif_b_link_setup       [1];   /* 미사용 */
  char gc_link_time             [6];   /* LINK 시각 */
  char bsqif_b_link_time        [6];   /* 미사용 */
  char bsqif_b_transfer         [1];   /* 미사용 */
  char bsqif_error_gubun        [1];   /* 미사용 */
  char gc_error_num             [2];   /* Interface  error code */
  char bsqif_error_time         [6];   /* 미사용 */
  char gc_data_count            [8];   /* 일련번호 */
  char gc_time                  [6];   /* Data 송수신 시각 */
  char gc_stop_gubun            [1];   /* 9: 정상, 5: 기타비정상, 1: 정상 기동중 */
  char gc_stop_time             [6];   /* HHMMSS */
  char bsqif_b_stop_gubun       [1];   /* 미사용 */
  char bsqif_b_stop_time        [6];   /* 미사용 */
  char gc_link_status           [2];   /* 00:미LINK, 01:LINK, 02:Link종료송신 03:Link종료수신(최종 종료) */
  char bsqif_gita_link_time     [6];   /* 미사용 */
  char gc_restart_time          [6];   /* 기동 또는 재기동 시각 */
  char bsqif_use_bit            [1];   /* 0: 사용, 1: 미사용 */
  char bsqifv_rel_key           [8];   /* 미사용 */
  char gc_complete_yn           [1];   /* 1:완료 */
  char gc_date                  [8];   /* Data 송수신 일자 */
  char gc_filler                [11];
} gst_bond_cnt_rec;                    /* length = 100 */

struct FA_CNT_REC_DEF {         /* (2) 신용 Cnt File Definition */
  char gc_member_code           [3];   /* pk 기관코드 */
  char gc_record_count          [8];   /* Block count */
  char gc_data_count            [8];   /* Data count */
  char gc_complete_yn           [1];   /* 0:미완료, 1:완료 */
  char gc_date                  [8];   /* Data 송수신 일자 */
  char gc_time                  [6];   /* Data 송수신 시각 */
  char gc_proc_status           [1];   /* 0:미기동,1:정상기동,2:비정상STOP,3:정상STOP */
  char gc_b_status              [1];   /* (미사용) */
  char gc_line_status           [1];   /* (미사용)1:메인  / 2:백업 */
  char gc_t_record_count        [8];   /* (미사용) */
  char gc_t_data_count          [8];   /* (미사용) */
  char gc_t_complete_yn         [1];   /* (미사용) */
  char gc_t_date                [8];   /* (미사용) */
  char gc_restart_time          [6];   /* 기동 또는 재기동 시각 */
  char gc_tm_status             [1];   /* (미사용) */
  char gc_error_num             [2];   /* Interface error code */
  char gc_link_status           [2];   /* 00:미LINK, 01:LINK, 02:Link종료송신 03:Link종료수신(최종 종료) */
  char gc_link_time             [6];   /* LINK 시각 */
  char gc_filler                [1];
} gst_fa_cnt_rec;                      /* length = 80 */

/*----------------------------------------------------------------------------*/
/*----------------------------- 6. JANG File 관련 변수 설정 ------------------*/
/*----------------------------------------------------------------------------*/
struct BOND_JANG_FILE_DEF {     /* (1) B-TRIS:"BMMJANG" File Definition */
  char gc_upmu_gubun            [1];   /* Key : 'B */
  char gc_jang_gubun            [1];   /* 미사용 */
  char gc_if_start_sigak        [6];   /* IF기동 시간 */
  char gc_lastmarket_start_time [6];   /* 직전거래일 시작시간 */
  char gc_lastmarket_end_time   [6];   /* 직전거래일 종료시간 */
  char gc_lastmarket_jang_status[1];   /* (직전)0:접수전,1:접수중,9:종료 */
  char gc_start_time            [6];   /* 당일 시작시간 */
  char gc_end_time              [6];   /* 당일 종료시간 */
  char gc_jang_status           [1];   /* (당일)0:접수전,1:접수중,9:종료 */
  char gc_gere_end_yn           [1];   /* 미사용 */
  char gc_gere_end_sigak        [6];   /* 미사용 */
  char gc_suik_end_yn           [1];   /* 미사용 */
  char gc_suik_end_sigak        [6];   /* 미사용 */
  char gc_balf_end_yn           [1];   /* 미사용 */
  char gc_balf_end_sigak        [6];   /* 미사용 */
  char gc_last_magam_yn         [1];   /* 미사용 */
  char gc_last_magam_sigak      [6];   /* 미사용 */
  char gc_batif_start_sigak     [6];   /* 미사용 */
  char filler                   [32];
} gst_bond_jang_rec;                   /* length = 100 */

struct FTS_JANG_FILE_DEF {      /* (2) 상속인 금융거래:"KOPRMGT" File Definition */
  char gc_use_type              [1];   /* Key : 'K' */
  char gc_lastmarket_jang_status[1];   /* 미사용 */
  char gc_jang_status           [1];   /* 0:접수전,1:접수중,2:종료 */
  char gc_if_start_time         [6];   /* IF기동 시간 */
  char gc_start_time            [6];   /* 당일 시작시간 */
  char gc_end_time              [6];   /* 당일 종료시간 */
  char gc_process1_end_yn       [1];   /* 미사용 */
  char gc_process1_end_time     [6];   /* 미사용 */
  char gc_process2_end_yn       [1];   /* 미사용 */
  char gc_process2_end_time     [6];   /* 미사용 */
  char gc_accept_end_yn         [1];   /* 미사용 */
  char gc_accept_end_time       [6];   /* 미사용 */
  char gc_divide_end_yn         [1];   /* 미사용 */
  char gc_divide_end_time       [6];   /* 미사용 */
  char gc_reject_end_yn         [1];   /* 미사용 */
  char gc_reject_end_time       [6];   /* 미사용 */
  char filler                   [44];
} gst_fts_jang_rec;                    /* length = 100 */

struct FA_JANG_FILE_DEF {       /* (3) 신용 Jang File Definition */
  char gc_marketopen_date       [8];   /* Key : 'YYYYMMDD' */
  char gc_start_time            [6];   /* 당일 시작시간 */
  char gc_end_time              [6];   /* 당일 종료시간 */
  char gc_lastmarket_jang_status[1];   /* 0:접수, 1: 접수종료 */
  char gc_merge_start           [1];   /* 미사용 */
  char gc_jang_status           [1];   /* 0:전송전, 1: 전송시작 */
  char gc_if_date               [8];   /* 미사용 */
  char gc_if_time               [6];   /* 미사용 */
  char gc_merge_date            [8];   /* 미사용 */
  char gc_merge_time            [6];   /* 미사용 */
  char gc_sa_date               [8];   /* 미사용 */
  char gc_sa_time               [6];   /* 미사용 */
  char gc_lastmarket_date       [8];
  char gc_filler                [7];
} gst_fa_jang_rec;                     /* length = 80 */

/************************/
/* 함수 프로토타입 선언 */
/************************/
short t0001_create_socket(void);					 /* Socket Create, Bind, Listen, Accept */
short t0002_control27_io_read(void);       			 /* 송신 시, Data File Status Check */
short t0003_io_check(short,long);          			 /* Socket Status Check */
short t0004_data_read(void);               			 /* 송신 Data File Read */
void  t0005_seq_change(void);              			 /* 송신 시, 수신 측 응답 Count(SEQ)가 적은 경우 Count(SEQ) 자동 맞춤 */
short t1110_read_from_socket(int);         			 /* Socket에서 전문길이 및 Message Read */
short t1120_rcv_message_check(void);       			 /* 수신 Message Header 및 Data 이상유무 확인 */
void  t1130_message_set(short,int,short,long,short); /* 송신 Message Setting */
short t1140_send_to_socket(void);          			 /* 송신 Message 송신 */
void  t1150_write_rcv_rec(void);           			 /* 수신 Data를 수신 File에 Write */
void  t1151_update_cnt_status(short);      			 /* 해당 Count(SEQ) File에 현재 Status Write */
/* void  t1152_update_cnt_err(void);          			해당 Count(SEQ) File에 Error_Num Write  */
/* void  t1153_update_cnt_strt(void);         			해당 Count(SEQ) File에 Process Start Status Write */
void  t1154_write_msg_rec(short);          			 /* "gc_display_message" Buffer의 내용을 Msg File에 Write */
void  t1160_write_log(short);              			 /* "gc_display_message" Buffer의 내용을 Log File에 Write */
/* void  t1170_close_fa_rec(void); */
void  t9010_get_date(void);                			 /* 시스템 날짜 가져오기 */
void  t9020_get_time(void);                			 /* 시스템 시간 가져오기 */
void  t9030_get_config(void);              			 /* TCONFIG에서 정보 가져오기 */
void  t9040_open_file(void);               			 /* TCONFIG에서 가져 온 정보로 관련파일들 Open */
void  t9050_get_count_status(void);        			 /* 송수신 Count(SEQ)을 읽어와서 Variable Setting */
void  t9060_get_jang_status(void);         			 /* JANG File을 읽어와서 Variable Setting */
void  t9070_stop_process(short);           			 /* Process 정상/비정상 종료 송수신 Count(SEQ) File에 Setting */

/*---------------------------------*/
int main(int argc, char* argv[])
/*---------------------------------*/
{
	short ls_result = 0, ls_tmf_status = -1;

	/* Argument Read, Check and Display */
	if(argc != 3)
	{
		printf("Parameter 수가 맞지 않는군요. \n");
		printf("Usage : %s <WhisaCode+TR> <TCONFIG File path> \n", argv[0]);
		PROCESS_STOP_(,, save_abend);
	}

    strcpy(gc_tconfig_key, argv[1]);           /* ex) 001L1 */
    strcpy(gc_tconfig_file_name, argv[2]);     /* ex) $BDAT01.TCPDAT.TCONFIG */

	/* 사전 처리 */
	t9010_get_date();                          /* 시스템 날짜 가져오기 */
    t9020_get_time();                          /* 시스템 시간 가져오기 */
    t9030_get_config();                        /* TCONFIG에서 정보 가져오기 */
    t9040_open_file();                         /* TCONFIG에서 가져 온 정보로 관련파일들 Open */
    t9050_get_count_status();                  /* 송수신 Count(SEQ)을 읽어와서 Variable Setting */
    t9060_get_jang_status();                   /* JANG File을 읽어와서 Variable Setting */
    if(gs_proc_status == 0)
	{
		t1151_update_cnt_status(MSG_STRT);     /* 해당 Count(SEQ) File에 Process Start Status Write */
	}

    /* 정상 종료 Status Check */
    /* 3 : 정상 STOP, 9 : 정상 */
    if(gs_proc_status == 3 || gs_proc_status == 9)
	{
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "Process Normal Stop..%d", gs_proc_status);
        t1160_write_log(LOG_TYPE_NORMAL);
        t9070_stop_process(SUCCESS);
    }

    /* 0810/040 미수신 처리 */
    /* gs_link_status : 0:미LINK, 1:LINK, 2:Link종료송신 3:Link종료수신(최종 종료) */
    /* gs_jang_status : 0:접수전,1:접수중,2:종료 */
    /* JANG_LINK_MAGAM : 7 (송신 시 마감TR 전송 후 Set) */
    if(gs_link_status == 2)
	{
		gs_jang_status = JANG_LINK_MAGAM;
	}

	while(1)
	{
		/* 0. Socket Create, Bind, Listen, Accept */
		if(gs_conn_status == 0)
		{
			while(t0001_create_socket() == FAIL);
		}
	
		/* 1. Interface 수신 */
		/* timeout(응답시간 30초 초과) 3번까지 확인 */
	    /* 3번 이상 일 경우 로그 기록 후, 소켓 재생성 */
	    /* t1110_read_from_socket에서 IO_TIME_OUT(40)을 반환하면 응답시간 30초 초과 된 것 */
	    /* t1110_read_from_socket : 데이터 송수신 Socket에서 전문길이 및 메시지 읽어옴 */
		while(1)
		{
			if(gs_retry_cnt < 3)
			{ /* 재송횟수 3번 이하 */
				if(ls_result = t1110_read_from_socket(MESSAGE_LENGTH))
				{ /* TimeOut 또는 그 이외의 Error 발생 하였을 경우 */
					/* t1110_read_from_socket의 결과가 40이 반환되어 time out 일 경우 */
	                /* 송신(S)은 gst_snd_buff에 현재 시스템 날짜, 시간, 재송횟수 메시지 구성 후 send */
	                /* 수신(R)은 메시지와 로그에 기록을 남긴 후 다시 소켓 read 반복 */
					if(ls_result == IO_TIME_OUT) /* "ls_result"가 "40"일 경우 : TimeOut */
					{
						gs_retry_cnt += 1; /* 재송횟수 +1 Count */
	
						if(strncmp(gc_tconfig_comm_type, "S", 1) == 0) /* 송신 Interface인 경우 */
						{ /* TimeOut일 경우 송신은 Retry.. */
							t9020_get_time();
							strncpy(gst_snd_buff.time, &gc_today_date[2], 6);
							strncpy(&gst_snd_buff.time[6], gc_today_time, 6);
							NUMOUT(gst_snd_buff.retry_cnt, gs_retry_cnt, 10, 2);
	
							gs_snd_err_code = t1140_send_to_socket(); /* Message Send */
	
							if(gs_snd_err_code != NO_ERROR)
							{
								memset(gc_display_message, 0x00, sizeof(gc_display_message));
								sprintf(gc_display_message, "SEND RETRY error..%d", gs_snd_err_code);
								t1160_write_log(LOG_TYPE_NORMAL);
								t9070_stop_process(FAIL);
							}
						}
						else /* 수신 Interface인 경우 */
						{ /* TimeOut일 경우 수신은 다시 Read Wait.. */
							memset(gc_display_message, 0x00, sizeof(gc_display_message));
							sprintf(gc_display_message, "RECV Time Out...Keep..%d", ls_result);
							t1160_write_log(LOG_TYPE_NORMAL);
							t9070_stop_process(FAIL);
						}
					}
					else /* TimeOut 이외의 Error */
					{
						memset(gc_display_message, 0x00, sizeof(gc_display_message));
						sprintf(gc_display_message, "RECV Socket AWAITIO Error..%d", ls_result);
						t1160_write_log(LOG_TYPE_NORMAL);
						gs_conn_status = 0; /* 수신 실패 시, Socket Connetc Status 초기화 */
						FILE_CLOSE_((short)new_socket_num); /* Socket Close */
						while(t0001_create_socket() != SUCCESS); /* Socket Recreate */
					}
				}
				else
				{ /* 정상 수신 되었을 경우 */
					gs_retry_cnt = 0;
	
					/* 수신 Data Error Code Check */
					if(strncmp(gst_rcv_buff.err_code, "00", 2) == 0)
						break; /* 정상 수신되었으므로 Loop문 나감 */
					else if(strncmp(gst_rcv_buff.err_code, "98", 2) == 0)
						break; /* Header와 Data부의 일련번호가 일치하지 않는 Error -> Message Check에서 처리 */
					else if(strncmp(gst_rcv_buff.err_code, "90", 2) == 0)
					{ /* AnyLink가 회선장애 등으로 회신하는 경우임 */
						if(strncmp(gc_tconfig_comp_id, "5", 1) == 0)
						{ /* 은행 송신은 마지막 전문 재송신 */
							if(strncmp(gc_tconfig_comm_type, "S", 1) == 0)
							{
								gs_snd_err_code = t1140_send_to_socket();
								if(gs_snd_err_code != NO_ERROR)
								{
									memset(gc_display_message, 0x00, sizeof(gc_display_message));
									sprintf(gc_display_message, "SEND 90 Error Retry Fail..%d", gs_snd_err_code);
									t1160_write_log(LOG_TYPE_NORMAL);
									t9070_stop_process(FAIL);
								}
							}
						}
						else
						{
							gs_start_status = 0; /* 대외기관 연결 Error -> 초기 기동 상태로 변경(은행 제외) */
						}
					}
					else
					{ /* 기타 Error */
					
						memset(gc_display_message, 0x00, sizeof(gc_display_message));
						sprintf(gc_display_message, "RECV Error Code..%2.2hs", gst_rcv_buff.err_code);
						t1160_write_log(LOG_TYPE_NORMAL);
						t9070_stop_process(FAIL);
					}
				}
			}
			else
			{ /* 재송횟수 3번 초과 */
				memset(gc_display_message, 0x00, sizeof(gc_display_message));
				sprintf(gc_display_message, "RECV RETRY %d Times Error..", gs_retry_cnt);
				t1160_write_log(LOG_TYPE_NORMAL);
	
				gs_conn_status = 0; /* 수신 3회 실패 시, Socket Connetc Status 초기화 */
				FILE_CLOSE_((short)new_socket_num); /* Socket Close */
				while(t0001_create_socket() != SUCCESS); /* Socket Recreate (Any Link 장애 대비) */
				gs_retry_cnt = 0;
			}
		}
	
		/* 2. Message Check, Message Set, Data Set */
	    do
	    {
	        /* (1) Message Check */
	        gs_rcv_err_code = t1120_rcv_message_check();
	        if(gs_rcv_err_code != NO_ERROR)
	        {
	            t1151_update_cnt_status(MSG_ERR);
	            memset(gc_display_message, 0x00, sizeof(gc_display_message));
	            sprintf(gc_display_message, "Interface Error Code..%d", gs_rcv_err_code);
	            t1160_write_log(LOG_TYPE_NORMAL);
	            t1154_write_msg_rec(MSG_RECV_ERR);
	        }
	
	        /* (2) 송신 Interface일 경우 */
	        if(strncmp(gc_tconfig_comm_type, "S", 1) == 0)
	        {
				if(gs_rcv_err_code == NO_ERROR)
				{ /* 정상 */
					if(gs_rcv_message_type == MSG_0810_040)
	            	{ /* 종료 수신일 경우 */
	            	    ls_tmf_status = BEGINTRANSACTION();
	            	    if(ls_tmf_status != 0)
	            	    {
	            	        memset(gc_display_message, 0x00, sizeof(gc_display_message));
	            	        sprintf(gc_display_message, "SEND-END BEGINTRANSACTION Error..%d", ls_tmf_status);
	            	        t1160_write_log(LOG_TYPE_NORMAL);
	            	        t9070_stop_process(FAIL);
	            	    }
	
	            	    t1151_update_cnt_status(gs_rcv_message_type);
	
	            	    ls_tmf_status = ENDTRANSACTION();
	            	    if(ls_tmf_status != 0)
	            	    {
	            	        memset(gc_display_message, 0x00, sizeof(gc_display_message));
	            	        sprintf(gc_display_message, "SEND-END ENDTRANSACTION Error..%d", ls_tmf_status);
	            	        t1160_write_log(LOG_TYPE_NORMAL);
	            	        t9070_stop_process(FAIL);
	            	    }
	            	    t9070_stop_process(SUCCESS);
	            	}
				}
				else
	        	{ /* 송신 응답 Error */
	        	    /* 송신 시 수신측 요청에 따라 적은 SEQ 변경, 수신은 변경 없음 */
	        	    if(gs_rcv_err_code == DATA_NO_INVALID)
	        	    {
	        	        if(gl_rcv_data_no < gl_last_data_count)
	        	        { /* 응답 SEQ가 작은 경우 */
	        	            t0005_seq_change();
	        	        }
	        	        else
	        	        { /* 응답 SEQ가 큰 경우 */
	        	            if(gs_rcv_message_type != MSG_0800_001)
	        	            {
	        	                memset(gc_display_message, 0x00, sizeof(gc_display_message));
	        	                sprintf(gc_display_message, "RCV Data No(%ld)..Last No(%ld)", gl_rcv_data_no, gl_last_data_count);
	        	                t1160_write_log(LOG_TYPE_NORMAL);
	        	                t9070_stop_process(FAIL);
	        	            }
	        	        }
	        	    }
	        	    else
	        	    {
	        	        memset(gc_display_message, 0x00, sizeof(gc_display_message));
	        	        sprintf(gc_display_message, "SEND RESPONSE Error..%d", gs_rcv_err_code);
	        	        t1160_write_log(LOG_TYPE_NORMAL);
	        	        t9070_stop_process(FAIL);
	        	    }
	        	}
			}
	
	        /* (3) 송신 Message 생성 */
	        switch(gs_rcv_message_type)
	        {
	            case MSG_0800_001 :
	                t1130_message_set(MSG_0810_001, gs_rcv_err_code, gs_retry_cnt, gl_last_data_count, 0);
	                /* 송신 Link 응답 */
	                if(strncmp(gc_tconfig_comm_type, "S", 1) == 0)
	                {
	                    gs_snd_err_code = t1140_send_to_socket();
	                    if(gs_snd_err_code != NO_ERROR)
	                    {
	                        memset(gc_display_message, 0x00, sizeof(gc_display_message));
	                        sprintf(gc_display_message, "SEND 0810_001 Error..%d", gs_snd_err_code);
	                        t1160_write_log(LOG_TYPE_NORMAL);
	                        t9070_stop_process(FAIL);
	                    }
	
	                    if((gs_rcv_err_code == NO_ERROR) && (gs_snd_err_code == NO_ERROR))
	                    { /* rcv와 snd 둘다 Error이 없을 때, Update */
	                        ls_tmf_status = BEGINTRANSACTION();
	                        if(ls_tmf_status != 0)
	                        {
	                            memset(gc_display_message, 0x00, sizeof(gc_display_message));
	                            sprintf(gc_display_message, "[S:0810/001] CNT UPDATTE BEGINTRANSACTION Error..%d", ls_tmf_status);
	                            t1160_write_log(LOG_TYPE_NORMAL);
	                            t9070_stop_process(FAIL);
	                        }
	
	                        t1151_update_cnt_status(gs_snd_message_type);
	
	                        ls_tmf_status = ENDTRANSACTION();
	                        if(ls_tmf_status != 0)
	                        {
	                                                            
	                            memset(gc_display_message, 0x00, sizeof(gc_display_message));
	                            sprintf(gc_display_message, "[S:0810/001] CNT UPDATTE ENDTRANSACTION Error..%d", ls_tmf_status);
	                            t1160_write_log(LOG_TYPE_NORMAL);
	                            t9070_stop_process(FAIL);
	                        }
	                    }
	                }
	                break;
	            case MSG_0800_301 :
	                t1130_message_set(MSG_0810_301, gs_rcv_err_code, gs_retry_cnt, gl_last_data_count, 0);
	                break;
	            case MSG_0810_301 :
	                break; /* 3-2. (송신 Data message 생성)에서 message set */
	            case MSG_0800_040 :
	                t1130_message_set(MSG_0810_040, gs_rcv_err_code, gs_retry_cnt, gl_last_data_count, 0);
	                break;
	            case MSG_0810_040 :
	                break; /* 2-1. 송신프로그램 종료에서 처리 */
	            case MSG_0200_000 :
	                if(gs_rcv_err_code == NO_ERROR)
	                {
	                    t1130_message_set(MSG_0210_000, gs_rcv_err_code, gs_retry_cnt, gl_next_data_count, BLOCK_COUNT);
	                }
	                else
	                {
	                    t1130_message_set(MSG_0210_000, gs_rcv_err_code, gs_retry_cnt, gl_last_data_count, BLOCK_COUNT);
	                }
	                break;
	            case MSG_0210_000 :
	                break; /* 3-2. (송신 Data message 생성)에서 message set */
	            default :
	                break ;
	        }
	
	        /* (4) 송신 Data Message 생성 */
	        if(strncmp(gc_tconfig_comm_type,"S",1) == 0)
	        {
	            ls_result = t0002_control27_io_read(); /* control27 및 recv 후 송신 Message 생성 */
	            if(ls_result == 0)
	            {  /* Data Read */
	                t1130_message_set(MSG_0200_000, 0, 0, gl_next_data_count, BLOCK_COUNT);
	                if((strncmp(gc_tconfig_tr_code, "K2", 2) == 0)) /* Any Link 부하 방지를 위하여 배치 업무(K2) Delay */
	                    DELAY(BATCH_IO_DELAY);
	                if((strncmp(gc_tconfig_tr_code, "L4", 2) == 0)) /* Any Link 부하 방지를 위하여 배치 업무(L4) Delay */
	                    DELAY(BATCH_IO_DELAY_L4);
	            }
	            else if(ls_result == SND_RCV_CHK)
	            { /* 0800/001 수신 */
	                memset(gc_display_message, 0x00, sizeof(gc_display_message));
	                sprintf(gc_display_message, "RECV in SEND..%d", ls_result);
	                t1160_write_log(LOG_TYPE_NORMAL);
	            }
	            else
	            { /* Data TimeOut */
	                if(gs_jang_status == JANG_LINK_MAGAM)
	                    t1130_message_set(MSG_0800_040, 0, 0, gl_last_data_count, 0); /* Interface 마감 생성 */
	                else
	                    t1130_message_set(MSG_0800_301, 0, 0, gl_last_data_count, 0); /* TimeOut 0800/301 생성 */
	            }
	        }
	    } while (ls_result == SND_RCV_CHK); /* 0800/001 수신 시 Loop */
	
	                
	    /* 3. Interface 송신 */
	    gs_snd_err_code = t1140_send_to_socket(); /* 송신 */
	    if(gs_snd_err_code != NO_ERROR)
	    {
	        memset(gc_display_message, 0x00, sizeof(gc_display_message));
	        sprintf(gc_display_message, "SEND error..%d", gs_snd_err_code);
	        t1160_write_log(LOG_TYPE_NORMAL);
	        gs_conn_status = 0; /* 송신실패 시 socket recreation */
	        FILE_CLOSE_((short)new_socket_num);
	    }
	    else
	    { /* 정상 송신 */
	        gs_retry_cnt = 0;
	
	        /* 4. Rcv File & Count & Status Update */
	        /* 수신도 응답송신한 후 File 처리. 파일처리 오류 시 Flow Control에서 SEQ 정합성 Check */
	        if((gs_rcv_err_code == SUCCESS) && (gs_snd_err_code == SUCCESS))
	        { /* rcv와 snd 모두 Error 없을 때 Update */
	            ls_tmf_status = BEGINTRANSACTION();
	            if(ls_tmf_status != 0)
	            {
	                memset(gc_display_message, 0x00, sizeof(gc_display_message));
	                sprintf(gc_display_message, "CNT UPDATTE BEGINTRANSACTION Error..%d", ls_tmf_status);
	                t1160_write_log(LOG_TYPE_NORMAL);
	                t9070_stop_process(FAIL);
	            }
	
	            if(gs_rcv_message_type == MSG_0200_000)
	            {
	                t1150_write_rcv_rec();
	            }
	
	            t1151_update_cnt_status(gs_snd_message_type);
	
	            ls_tmf_status = ENDTRANSACTION();
	            if(ls_tmf_status != 0)
	            {
	                memset(gc_display_message, 0x00, sizeof(gc_display_message));
	                sprintf(gc_display_message, "CNT UPDATTE ENDTRANSACTION Error..%d", ls_tmf_status);
	                t1160_write_log(LOG_TYPE_NORMAL);
	                t9070_stop_process(FAIL);
	            }
	        }
	
	        /* 5. 수신 프로그램 종료 */
	        if((strncmp(gc_tconfig_comm_type, "R", 1) == 0) && (gs_snd_message_type == MSG_0810_040) && (gs_rcv_err_code == NO_ERROR))
	        {
	           t9070_stop_process(SUCCESS);
	        }
	    }
	}
}

/* Socket Create, Bind, Listen, Accept */
/*------------------------------------------------------*/
short t0001_create_socket()
/*------------------------------------------------------*/
{
	int error = 0, flen = 0, opt_val = 1;

	/* 1. TCP Process Name Setting */
	socket_set_inet_name(gc_tconfig_tcp_proc_name); /* ex) $ZB26D */

	/* 2. Socket 정보 Setting */
	loc_addr.sin_family = AF_INET; /* 주소 체계 : IPv4 */
	loc_addr.sin_port = gs_tconfig_port_num; /* server port number */
	loc_addr.sin_addr.s_addr = inet_addr(gc_tconfig_ip_num); /* server ip (Network bytes 주소로 변환) */

	/* 3. Create Socket */
	/* socket_num : descriptor(handle). */
    /* AF_INET : 주소체계인데, 메뉴얼 설명으론 프로토콜 체계로 사용. TCP/IP 일 경우. */
    /* SOCK_STREM : 프로토콜 타입으로 TCP. */
    /* 0 : IP number넣는 곳이나 SOCK_STREAM은 무시. 0 세팅. */
    /* 2 : File Open Procedure Parameter. 2는 파일의 맨 끝에 Write. */
    /* 0 : Tandem에서는 지원하지 않는 소켓으로 sync는 항상 0 세팅. */
    socket_num = socket_nw(AF_INET, SOCK_STREAM, 0, 2, 0);
    if (socket_num < 0)
	{
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "Socket create error..");
        t1160_write_log(LOG_TYPE_NORMAL);
        t9070_stop_process(FAIL);
    }
	
	/* 4. Add Option Socket(Reuse) */
	/* 소켓 관련 옵션 추가. */
    /* socket_num : 소켓 번호 */
    /* SOL_SOCKET : 관리 수준 지정. 여기서는 소켓 수준. */
    /* SO_REUSEADDR : 옵션 이름. 여기서는 Bind시, port를 재사용하겠다는 뜻. */
    /* (char*)&opt_val : 소켓 옵션의 사용 여부. 1 = true. */
    /* sizeof(opt_val) : 소켓 옵션 값의 길이. */
    /* gl_socket_tag : setsockopt_nw시, nowait 명령으로 할 것인지에 대해 사용하는 매개변수. */
    opt_val = 1;
    error = setsockopt_nw(socket_num, SOL_SOCKET, SO_REUSEADDR, (char*)&opt_val, sizeof(opt_val), gl_socket_tag);
    if (error < 0)
	{
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "Socket SETSOCKOPT(%d)", error);
        t1160_write_log(LOG_TYPE_NORMAL);
        t9070_stop_process(FAIL);
    }
  
    if(error = t0003_io_check((short)socket_num, -1))
	{
        memset(gc_display_message,0x00,sizeof(gc_display_message));
        sprintf(gc_display_message,"Socket SETSOCKOPT Error(%d)", error);
        t1160_write_log(LOG_TYPE_NORMAL);
        FILE_CLOSE_((short)socket_num);
        DELAY(BIND_IO_DELAY);
        return FAIL;
    }

	/* 5. Bind Socket */
    /* 생성 한 소켓에 주소체계, port번호 연결. */
    /* gl_socket_tag : bind_nw시, nowait 명령으로 할 것인지에 대해 사용하는 매개변수. */
	error = bind_nw(socket_num, (struct sockaddr*)&loc_addr, sizeof(loc_addr), gl_socket_tag);
    if (error < 0)
	{
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "Socket Bind Error(%d)", error);
        t1160_write_log(LOG_TYPE_NORMAL);
        t9070_stop_process(FAIL);
    }
  
    if(error = t0003_io_check((short)socket_num, -1))
	{
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "Socket Bind AwaitIO Error(%d)", error);
        t1160_write_log(LOG_TYPE_NORMAL);
        FILE_CLOSE_((short)socket_num);
        DELAY(BIND_IO_DELAY);
        return FAIL;
    }
    else
	{
        /* 6. Listen From Socket */
  	    /* 생성한 소켓에 접속 대기 큐 설정. */
  	    /* 설정한 1은 1개의 클라이언트만 접속 대기 허용. 나머지는 접속 거부 안내 발송. */
        error = listen(socket_num,1);
        if(error < 0)
	    {
            memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "Socket Listen Error(%d)", error);
            t1160_write_log(LOG_TYPE_NORMAL);
            t9070_stop_process(FAIL);
        }
        else
	    {
            memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "Socket(NO = %d) Listening..", socket_num);
            t1160_write_log(LOG_TYPE_NORMAL);
        }
  
        /* 7. Accept from Client */
    	/* socket_nw 로 만들어진 "nowait socket"으로의 접속 허용. */
    	/* 접속 허용이 되면 "accept_nw2"로 허용하는 새로운 socket으로 데이터 송수신 */
    	/* from_addr : 클라이언트의 IP 및 Port 번호 정보. */
    	/* gl_socket_tag : accept_nw시, nowait 명령으로 할 것인지에 대해 사용하는 매개변수. */
        flen = sizeof(from_addr);
        error = accept_nw(socket_num, (struct sockaddr*)&from_addr, &flen, gl_socket_tag);
        if(error < 0)
		{
			memset(gc_display_message, 0x00, sizeof(gc_display_message));
			sprintf(gc_display_message, "Socket Listen Error(%d)", error);
            t1160_write_log(LOG_TYPE_NORMAL);
            t9070_stop_process(FAIL);
        }
        else
		{
            error = t0003_io_check((short)socket_num, -1); /*무한 대기 */
            if(error != 0)
			{
               memset(gc_display_message, 0x00, sizeof(gc_display_message));
               sprintf(gc_display_message, "Socket accept AwaitIO Error(%d)", error);
               t1160_write_log(LOG_TYPE_NORMAL);
               t9070_stop_process(FAIL);
            }
		}

        error = getpeername(socket_num, (struct sockaddr*)&from_addr, &flen);
        memcpy(&in, &from_addr.sin_addr.s_addr, 4);

		/* 8. New Socket Create */
		/* 윈도우나 리눅스에서의 "accept()" 함수는 반환값이 새로 생성되는 socket임. */
  	    /* 새로 반환되는 socket으로 클라이언트와 데이터 송수신이 이루어짐. */
  	    /* tandem에서는 "nowait socket"을 사용하기 위해 "accept_nw()" 함수를 사용하였고, */
  	    /* "accept_nw()"함수는 "accept_nw2()" 함수로 새로운 socket과 데이터 송수신 연결 허용을 함. */
  	    /* 그러기 위한 새로운 socket 생성 과정임. */
        new_socket_num = socket_nw(AF_INET, SOCK_STREAM, 0, 2, 0);
        if(new_socket_num < 0)
	    {
	        memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "NEW Socket create Error(%d)", new_socket_num);
            t1160_write_log(LOG_TYPE_NORMAL);
            t9070_stop_process(FAIL);
        }

		/* 9. Accept for Data Streaming */
  	    /* "accept_nw"에서 허용한 클라이언트의 IP와 Port 번호를 새로운 socket에 연결시켜 데이터 송수신. */
  	    /* gl_new_socket_tag : accept_nw2시, nowait 명령으로 할 것인지에 대해 사용하는 매개변수. */
        error = accept_nw2(new_socket_num, (struct sockaddr *)&from_addr, gl_new_socket_tag);
        if(error < 0)
		{
			memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "NEW Socket accept Error(%d)", error);
            t1160_write_log(LOG_TYPE_NORMAL);
            t9070_stop_process(FAIL);
        }

        if(error = t0003_io_check((short)new_socket_num, -1))
		{
			memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "New Socket accept AwaitIO Error(%d)", error);
            t1160_write_log(LOG_TYPE_NORMAL);
            t9070_stop_process(FAIL);
        }
  
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        /* sprintf(gc_display_message, "Connected with %s %d port", inet_ntoa(in), from_addr.sin_port); */
        sprintf(gc_display_message, "TCP/IP Connected ( IP:%s port:%d )", inet_ntoa(in), from_addr.sin_port);
        t1160_write_log(LOG_TYPE_NORMAL);
        t1154_write_msg_rec(MSG_STRT);
  
        gs_conn_status = 1;
  
        FILE_CLOSE_((short)socket_num);
    }

    return SUCCESS;
}

/* 송신 시, Data File Read */
/*------------------------------------------------------*/
short t0002_control27_io_read()
/*------------------------------------------------------*/
{
	int li_result, li_come_length;
	short ls_result;
	_cc_status CC = 0; /* condition variable */

	/* 1. "L3(장운영 송신), 장 중 또는 Batch 시간, Data File CONTROL(27) Setting */
	if((strncmp(gc_tconfig_tr_code, "L3", 2) == 0) || (gs_jang_status != JANG_BEFORE) || (gs_jang_status == JANG_BATCH))
	{ /* "L3(장운영 송신)"은 장시간과 상관없이 실행  */
		CC = CONTROL(gs_open_data_file_num_27, 27,, gl_file_tag); /* Data File에 정보가 들어올때까지 대기 */
		if(_status_ne(CC))
		{
			FILE_GETINFO_(gs_open_data_file_num_27, &ls_result);
			memset(gc_display_message, 0x00, sizeof(gc_display_message));
			sprintf(gc_display_message, "Control 27 Error(%d)", ls_result);
			t1160_write_log(LOG_TYPE_NORMAL);
			t9070_stop_process(FAIL);
		}

		/* Data File Read */
		ls_result = t0004_data_read();
		if(ls_result == 0) /* SUCCESS = 0 */
		{ /* DATA 있을 경우 */
			CC = CANCELREQ(gs_open_data_file_num_27, gl_file_tag); /* Control27 해제하면서 Control27 tag Set */
			if(_status_ne(CC))
			{
				memset(gc_display_message, 0x00, sizeof(gc_display_message));
				sprintf(gc_display_message, "CANCELREQ(Control 27) Error(%d)", gs_open_data_file_num_27);
				t1160_write_log(LOG_TYPE_NORMAL);
				t9070_stop_process(FAIL);
			}

			return SUCCESS;
		}
	}

	/* 2. Receive Data Read from Socket */
	memset(gc_rcv_buff, 0x00, sizeof(gc_rcv_buff));
	li_result = recv_nw(new_socket_num, gc_rcv_buff, HEADER_LENGTH + 4, 0, gl_rcv_tag);
	if(li_result < 0)
	{
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
		sprintf(gc_display_message, "Control27 Sock Read Error : %d..", li_result);
		t1160_write_log(LOG_TYPE_NORMAL);
		t9070_stop_process(FAIL);
	}

	/* 3. Socket & Data File AWAITIOX */
	ls_result = t0003_io_check(AWAITIO_ANY, FILE_IO_DELAY); /* AWAITIOX 호출(Data File, Socket 모두에게) */
	if(gl_tagback != 0) /* AWAITIOX 실행 후 받은 tag가 0이 아닌 경우(완료 된 I/O가 있는 경우) */
	{
		if(gl_tagback == gl_file_tag) /* "gl_file_tag"는 CONTROL(27)이 실행 된 I/O의 반환 값 */
		{ /* Data File인 경우 */ 
			ls_result = t0004_data_read();
			if(ls_result != SUCCESS)
			{
				memset(gc_display_message, 0x00, sizeof(gc_display_message));
				sprintf(gc_display_message, "DATA Control27 Read Error : %d..", ls_result);
				t1160_write_log(LOG_TYPE_NORMAL);
				t9070_stop_process(FAIL);
			}

			CC = CANCELREQ((short)new_socket_num, gl_rcv_tag); /* Socket의 AWAITIOX 해제하면서 recv_nw tag Set */
			if(_status_ne(CC))
			{
				memset(gc_display_message, 0x00, sizeof(gc_display_message));
				sprintf(gc_display_message, "RECV_NW CANCELREQ Error : %d..", gs_open_data_file_num_27);
				t1160_write_log(LOG_TYPE_NORMAL);
				t9070_stop_process(FAIL);
			}
		}
		else
		{ /* Socket인 경우 */
			memset((char*)&gst_rcv_buff, 0x00, sizeof(gst_rcv_buff));
			memcpy((char*)&gst_rcv_buff, gc_rcv_buff, HEADER_LENGTH + 4);

			if((strncmp(gc_tconfig_tr_code, "L3", 2) == 0) || (gs_jang_status != JANG_BEFORE) || (gs_jang_status == JANG_BATCH))
			{
				CC = CANCELREQ(gs_open_data_file_num_27, gl_file_tag); /* Data File의 Control27 해제하면서 Control27 tag Set */
				if(_status_ne(CC))
				{
					memset(gc_display_message, 0x00, sizeof(gc_display_message));
					sprintf(gc_display_message, "RECV_NW Data File CANCELREQ Error : %ld..", gl_tagback);
					t1160_write_log(LOG_TYPE_NORMAL);
					t9070_stop_process(FAIL);
				}
			}

			li_come_length = gs_bytesread; /* Socket에서 읽어온 길이 */
			if(li_come_length == HEADER_LENGTH + 4) /* 전문길이 45Bytes + 전문길이가 담긴 변수 값 4Bytes */
			{
				gi_read_message_length = HEADER_LENGTH + 4;
				t1160_write_log(LOG_TYPE_RECV);
				return SND_RCV_CHK;
			}
			else /* 약속 된 전문 길이가 다른 경우 */
			{
				memset(gc_display_message, 0x00, sizeof(gc_display_message));
				sprintf(gc_display_message, "RECV Length Error In Send(%d)", li_come_length);
				t1160_write_log(LOG_TYPE_NORMAL);
				t9070_stop_process(FAIL);
			}
		}
	}
	else
	{ /* "gl_tagback"이 "0"인 경우 : 대기중인 I/O가 하나도 없는 경우 */
		CC = CANCELREQ((short)new_socket_num, gl_rcv_tag); /* Socket의 AWAITIOX 해제하면서 recv_nw tag Set */
		if(_status_ne(CC))
		{
			memset(gc_display_message, 0x00, sizeof(gc_display_message));
			sprintf(gc_display_message, "RECV_NW CANCELREQ Error(%d)", gs_open_data_file_num_27);
			t1160_write_log(LOG_TYPE_NORMAL);
			t9070_stop_process(FAIL);
		}

		if((strncmp(gc_tconfig_tr_code, "L3", 2) == 0) || (gs_jang_status != JANG_BEFORE) || (gs_jang_status == JANG_BATCH))
		{
			CC = CANCELREQ(gs_open_data_file_num_27, gl_file_tag); /* Data File의 Control27 해제하면서 Control27 tag Set */

			if(_status_ne(CC))
			{
				memset(gc_display_message, 0x00, sizeof(gc_display_message));
				sprintf(gc_display_message, "RECV_NW 2 Data File CANCELREQ Error(%d)", gl_tagback);
				t1160_write_log(LOG_TYPE_NORMAL);
				t9070_stop_process(FAIL);
			}
		}

		return IO_TIME_OUT;
	}

	return SUCCESS;
}

/* Socket Status Check */
/*------------------------------------------------------*/
short t0003_io_check(short ls_completeio, long toval)
/*------------------------------------------------------*/
/* ls_completeio = 확인 할 Socket Number */
/* toval = 기다리는 시간 */
/* -1 : ls_completeio에서 지정 한 파일의 이 전 프로세스가 끝나기를 계속 기다림 */
/* 3000 : 30초 기다림 */
/* 0 : 상태만 즉시 리턴 */
{
	short file_info = 0;
	_cc_status CC = 0; /* condition variable */

	gs_io_error_no = 0;
	gl_tagback = 0;
	gs_bytesread = 0;

	/* (AWAITIOX) 읽혔을 때 : IO Finish */
	/*            Time Out  : IO 유지(Sync Depth 2가 최대) */
	
	CC = AWAITIOX(&ls_completeio, (long*)&gst_snw, &gs_bytesread, &gl_tagback, toval);
	if(_status_ne(CC))
	{ /* 읽혔을 때 0, timeout 등 error -1 */
		FILE_GETINFO_(ls_completeio, &file_info);
		gs_io_error_no = file_info;
		return gs_io_error_no; /* Time Out은 40 반환 */
	}

	return SUCCESS;
}

/* 송신 Data File Read */
/*------------------------------------------------------*/
short t0004_data_read()
/*------------------------------------------------------*/
{
	short file_info = -1;
	_cc_status CC = 0;

	/* 1. Key Position */
	if(strncmp(gc_tconfig_tr_code, "K", 1) == 0)
	{ /* 상속인 금융거래조회 Key는 6자리 */
		CC = KEYPOSITIONX(gs_open_data_file_num, &gc_next_data_count[2], NULL, 6, exact);
	}
	else
	{ /* B-TRIS Key는 8자리 */
		CC = KEYPOSITIONX(gs_open_data_file_num, gc_next_data_count, NULL, 8, exact);
	}

	if(_status_ne(CC))
	{
		FILE_GETINFO_(gs_open_data_file_num, &file_info);
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
		sprintf(gc_display_message, "OPEN DATA Key Positioning Error. Error Code = %d", file_info);
		t1160_write_log(LOG_TYPE_NORMAL);
		t9070_stop_process(FAIL);
	}

	/* 2. Data File Read */
	memset(gc_data_file_buff, 0x00, sizeof(gc_data_file_buff));
	CC = READX(gs_open_data_file_num, gc_data_file_buff, sizeof(gc_data_file_buff));
	if(_status_ne(CC))
	{
		FILE_GETINFO_(gs_open_data_file_num, &file_info);
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
		sprintf(gc_display_message, "DATA File Reached End-Of-File(%d) = %s", file_info, gc_tconfig_data_file_name);
		t1160_write_log(LOG_TYPE_NORMAL);
		return file_info;
	}
	else
	{
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
		sprintf(gc_display_message, "Data Read..%43.43hs", gc_data_file_buff);
		t1160_write_log(LOG_TYPE_NORMAL);
	}

	/* 3. 상속인 금융거래조회일 경우 Buffer Setting */ 
	if(strncmp(gc_tconfig_tr_code, "K2", 2) == 0)
	{
		memset((char*)&gst_fts_ksnd_rec, 0x00, sizeof(gst_fts_ksnd_rec));
		memcpy((char*)&gst_fts_ksnd_rec, gc_data_file_buff, 1802);
	}
	else if(strncmp(gc_tconfig_tr_code, "K3", 2) == 0)
	{
		memset((char*)&gst_fts_krjt_rec, 0x00, sizeof(gst_fts_krjt_rec));
		memcpy((char*)&gst_fts_krjt_rec, gc_data_file_buff, 500);
	}

	return SUCCESS;
}

/* 송신 시, 수신 측 응답 Count(SEQ)가 적은 경우 Count(SEQ) 자동 맞춤 */
/*------------------------------------------------------*/
void t0005_seq_change()
/*------------------------------------------------------*/
{
	short file_info = 0, ls_tmf_status = -1;
	_cc_status CC = 0; /* condition variable */

	gl_last_data_count = gl_rcv_data_no;
	gl_next_data_count = gl_last_data_count + 1L;

	memset(gc_last_data_count, 0x00, sizeof(gc_last_data_count));
	memset(gc_next_data_count, 0x00, sizeof(gc_next_data_count));
	DNUMOUT(gc_last_data_count, gl_last_data_count, 10, 8);
	DNUMOUT(gc_next_data_count, gl_next_data_count, 10, 8);

	t9020_get_time();

	ls_tmf_status = BEGINTRANSACTION();
	if(ls_tmf_status != 0)
	{
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
		sprintf(gc_display_message, "CNT UPDATE BEGINTRANSACTION Error..%d", ls_tmf_status);
		t1160_write_log(LOG_TYPE_NORMAL);
		t9070_stop_process(FAIL);
	}

	/* 1. Key Position */
	KEYPOSITIONX(gs_open_cnt_file_num, gc_cnt_key, NULL, 5, exact);

	/* 2. Read Count(SEQ) File */
	CC = READUPDATELOCKX(gs_open_cnt_file_num, (char*)&gst_bond_cnt_rec, sizeof(gst_bond_cnt_rec));
	if(_status_ne(CC))
	{
		FILE_GETINFO_(gs_open_cnt_file_num, &file_info);
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
		sprintf(gc_display_message, "SEQ Change Count File READUPDATELOCKX Error = %d", file_info);
		t1160_write_log(LOG_TYPE_NORMAL);
		ABORTTRANSACTION();
		t9070_stop_process(FAIL);
	}

	/* 3. Set Buffer */
	strncpy(gst_bond_cnt_rec.gc_data_count, gc_last_data_count, 8);
	strncpy(gst_bond_cnt_rec.gc_date, gc_today_date, 8);
	strncpy(gst_bond_cnt_rec.gc_time, gc_today_time, 6);

	/* 4. Write Count(SEQ) File */
	CC = WRITEUPDATEUNLOCKX(gs_open_cnt_file_num, (char*)&gst_bond_cnt_rec, sizeof(gst_bond_cnt_rec));
	if(_status_ne(CC))
	{
		FILE_GETINFO_(gs_open_cnt_file_num, &file_info);
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
		sprintf(gc_display_message, "SEQ Change Count File WRITEUPDATELOCKX Error = %d", file_info);
		t1160_write_log(LOG_TYPE_NORMAL);
		ABORTTRANSACTION();
		t9070_stop_process(FAIL);
	}

	ls_tmf_status = ENDTRANSACTION();
	if(ls_tmf_status != 0)
	{
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
		sprintf(gc_display_message, "CNT UPDATE ENDTRANSACTION Error..%d", ls_tmf_status);
		t1160_write_log(LOG_TYPE_NORMAL);
		t9070_stop_process(FAIL);
	}

	gs_rcv_err_code = SUCCESS;
}

/* Socket에서 전문길이 및 Message Read */
/*------------------------------------------------------*/
short t1110_read_from_socket(int li_must_read_length)
/*------------------------------------------------------*/
{
	char  lc_read_message_length [5];       /* 수신전문에 포함되어 있는 전문길이 */
	short ls_read_message_length = 0;       /* 전문길이 */
  	int   li_total_read_count    = 0;       /* 소켓에서 읽은 총 바이트 수 */
  	int   li_remained_length     = 0;       /* 소켓에서 읽어야 하는 남은 바이트 수 */
  	int   li_come_length         = 0;       /* 소켓에서 읽은 바이트 수 */
  	int   li_result              = -1;
  	short ls_numin_status        = 0;
  	short ls_result              = -1;

	/* 1. Message length read */
	memset((char*)&gst_rcv_buff, 0x00, sizeof(gst_rcv_buff));
	memset(gc_rcv_buff, 0x00, sizeof(gc_rcv_buff));

	/* 전문길이(4bytes)를 다 수신 할 때까지 Loop */
  	/* 데이터를 수신 하는 소켓에서 전문 길이를 읽어옴. ex) 0045 */
	do
	{ /* 전문길이(4bytes) 읽기 시작 */
		li_remained_length = li_must_read_length - li_total_read_count;
		
		/* (1) Receive Socket */
		/* 데이터를 수신하는 소켓에서 "gc_rcv_buff"로 읽어 옴 */
		li_result = recv_nw(new_socket_num, &gc_rcv_buff[li_total_read_count], li_remained_length, 0, gl_rcv_tag);
		if(li_result < 0)
		{
			memset(gc_display_message, 0x00, sizeof(gc_display_message));
			sprintf(gc_display_message, "New Socket Message Length Read Error : %d..", li_result);
         	t1160_write_log(LOG_TYPE_NORMAL);
         	return FAIL;
		}

		/* (2) AwaitIO Check */
		/* 초기에는 소켓 상태를 확인하여 소켓에 데이터가 들어올때까지 기다림 */
		/* 30초 초과 시, return 값 40 반환 및 함수 종료 */
		/* 접수 중일때는 소켓 상태를 30초만 기다리며 응답상태를 확인 */
		/* gs_bytesread 변수에 읽어 온 전문길이(bytes) 저장 */
		if(gs_start_status == 0)
		{ /* 초기 Link */
			memset(gc_display_message, 0x00, sizeof(gc_display_message));
			sprintf(gc_display_message, "Link 대기 중 ...");
         	t1160_write_log(LOG_TYPE_NORMAL);
         	if(gs_link_status != 0)
				t1154_write_msg_rec(MSG_RE_LINK);

         	ls_result = t0003_io_check((short)new_socket_num, -1); /* 무한대기 */
		}
		else
			ls_result = t0003_io_check((short)new_socket_num, TCP_IO_DELAY); /* 30초 대기 */

		if(ls_result != SUCCESS)
			return ls_result;

		/* (3) Read Count Add */
		/* gs_bytesread는 AWAITIOX()에서 recv_nw로 소켓에서 받은 데이터를 카운트한 변수 */
		/* 소켓 상태를 확인하여 받아 온 전문 길이 값 gs_bytesread 변수가 0이면 데이터 없음 */
		li_come_length = gs_bytesread;
		if(li_come_length == 0)
		{
			memset(gc_display_message, 0x00, sizeof(gc_display_message));
			sprintf(gc_display_message, "Socket First Read %d bytes..", li_come_length);
         	t1160_write_log(LOG_TYPE_NORMAL);
         	return FAIL;
		}
		else
			li_total_read_count += li_come_length;
	}while(li_total_read_count < li_must_read_length);

	/* 2. 위에서 읽은 전문길이(4bytes)를 "lc_read_message_length"에 저장 */
	memset(lc_read_message_length, 0x00, sizeof(lc_read_message_length));
	strncpy(lc_read_message_length, gc_rcv_buff, MESSAGE_LENGTH);
	NUMIN(lc_read_message_length, &ls_read_message_length, 10, &ls_numin_status); /* char -> short 변환 */
	if(ls_numin_status != 0)
	{  /* 전문길이값을 int로 전환 */
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
		sprintf(gc_display_message, "rcv length Numin error : %s", lc_read_message_length);
     	t1160_write_log(LOG_TYPE_NORMAL);
     	t9070_stop_process(FAIL);
	}

	/* ls_read_message_length < 0 이면, 전문길이 정보가 없거나 수신 메시지 오류 */
	if(ls_read_message_length < 0)
	{
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
		sprintf(gc_display_message, "rcv Message length error : %d..", ls_read_message_length);
     	t1160_write_log(LOG_TYPE_NORMAL);
     	t9070_stop_process(FAIL);
	}
	else /* 정상인 경우 "gst_rcv_buff.message_length"에 전문길이 저장 */
		memcpy(gst_rcv_buff.message_length, lc_read_message_length, MESSAGE_LENGTH);

	/* 3. Message(Header + Data) Read */
	/* 변수 초기화 */
	li_remained_length = 0;
	li_total_read_count = 0;
	li_come_length = 0;

	memset(gc_rcv_buff, 0x00, sizeof(gc_rcv_buff));
	
	/* "ls_read_message_length"만큼 다 수신 할 때까지 Loop */
	do
	{
		li_remained_length = ls_read_message_length - li_total_read_count;

		/* (1) Receive Socket */
		/* 데이터를 수신하는 소켓에서 "gc_rcv_buff"로 읽어 옴 */
		li_result = recv_nw(new_socket_num, &gc_rcv_buff[li_total_read_count], li_remained_length, 0, gl_rcv_tag);
		if(li_result < 0)

			{
				memset(gc_display_message, 0x00, sizeof(gc_display_message));
				sprintf(gc_display_message, "New Socket Message(Header + Data) Read Error : %d..", li_come_length);
         		t1160_write_log(LOG_TYPE_NORMAL);
         		return FAIL;
			}

		/* (2) AwaitIO Check */
		/* 30초 초과 시, "ls_result"로 "40"을 반환, TIMEOUT */
		/* gs_bytesread 변수에 읽어 온 전문길이(bytes) 저장 */
		ls_result = t0003_io_check((short)new_socket_num, TCP_IO_DELAY); /* IO Check */
		if(ls_result != SUCCESS)
			return ls_result;

		/* (3) Read Count Add */
		/* gs_bytesread는 AWAITIOX()에서 recv_nw로 소켓에서 받은 데이터를 카운트한 변수 */
		/* 소켓 상태를 확인하여 받아 온 전문 길이 값 gs_bytesread 변수가 0이면 데이터 없음 */
		li_come_length = gs_bytesread;
		if(li_come_length == 0)
		{
			memset(gc_display_message, 0x00, sizeof(gc_display_message));
			sprintf(gc_display_message, "Socket Second Read %d bytes..", li_come_length);
         	t1160_write_log(LOG_TYPE_NORMAL);
         	return FAIL;
		}
		else
			li_total_read_count += li_come_length;
	}while(li_total_read_count < ls_read_message_length);

	/* 4. RCV Struct set */
	/* "gst_rcv_buff"에 수신 메시지 저장 */
	memcpy(gst_rcv_buff.tr_code, gc_rcv_buff, ls_read_message_length);
	gi_read_message_length = li_total_read_count + li_must_read_length;
	
	t1160_write_log(LOG_TYPE_RECV);

	return SUCCESS;
}

/* 수신 Message Header 및 Data 이상유무 확인 */
/*------------------------------------------------------------------*/
short t1120_rcv_message_check()
/*------------------------------------------------------------------*/
{
	short ls_numin_status = 0;
	char lc_data_no_string[9];
	char lc_data_cnt_string[3];
	char lc_data_seq_string[9];

	/* Current TIME & JANG STATUS READ & message type set */
	t9020_get_time();
	memset(gc_rcv_time, 0x00, sizeof(gc_rcv_time));
	strncpy(gc_rcv_time, gc_today_time, 6);

	if(gs_jang_status != JANG_LINK_MAGAM)
		t9060_get_jang_status();

	gs_rcv_message_type = UNDEFINED;
	if((strncmp(gst_rcv_buff.msg_type, "0800", 4) == 0) && (strncmp(gst_rcv_buff.opr_type, "001", 3) == 0))
		gs_rcv_message_type = MSG_0800_001;
	else if((strncmp(gst_rcv_buff.msg_type, "0800", 4) == 0) && (strncmp(gst_rcv_buff.opr_type, "301", 3) == 0))
		gs_rcv_message_type = MSG_0800_301;
	else if((strncmp(gst_rcv_buff.msg_type, "0810", 4) == 0) && (strncmp(gst_rcv_buff.opr_type, "301", 3) == 0))
		gs_rcv_message_type = MSG_0810_301;
	else if((strncmp(gst_rcv_buff.msg_type, "0800", 4) == 0) && (strncmp(gst_rcv_buff.opr_type, "040", 3) == 0))
		gs_rcv_message_type = MSG_0800_040;
	else if((strncmp(gst_rcv_buff.msg_type, "0810", 4) == 0) && (strncmp(gst_rcv_buff.opr_type, "040", 3) == 0))
		gs_rcv_message_type = MSG_0810_040;
	else if((strncmp(gst_rcv_buff.msg_type, "0200", 4) == 0) && (strncmp(gst_rcv_buff.opr_type, "000", 3) == 0))
		gs_rcv_message_type = MSG_0200_000;
	else if((strncmp(gst_rcv_buff.msg_type, "0210", 4) == 0) && (strncmp(gst_rcv_buff.opr_type, "000", 3) == 0))
		gs_rcv_message_type = MSG_0210_000;
	else
	{
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
		sprintf(gc_display_message, "Rcv Message type error : %4.4hs..%3.3hs", gst_rcv_buff.msg_type, gst_rcv_buff.opr_type);
		t1160_write_log(LOG_TYPE_NORMAL);
		t9070_stop_process(FAIL);
	}

	/* 0. Parsing & 포맷 Error CHECK */
	if(gs_rcv_message_type == MSG_0200_000)
	{
		if(strncmp(gc_tconfig_tr_code, gst_rcv_buff.data_tr_code, 2) != 0)
			return PARSING_ERROR;

		if(strlen((char*)&gst_rcv_buff) != (gs_tconfig_message_length + HEADER_LENGTH + 4))
			return FORMAT_ERROR;
	}

	/* 1. JANG File CHECK(0200/000, 0800/301 수신만) */
	if(gs_rcv_message_type == MSG_0200_000)
	{
		if(gs_jang_status == JANG_BEFORE)
			return MARKET_BEF_ERROR;
		if(gs_jang_status == JANG_CLOSE)
			return MARKET_AFT_ERROR;
	}

	if(gs_rcv_message_type == MSG_0800_301)
	{
		if(gs_jang_status == JANG_CLOSE)
			return MARKET_AFT_ERROR;
	}

	/* 2. HEADER TR-CODE CHECK */
	if(strncmp(gst_rcv_buff.tr_code, TR_CODE, 1) != 0)
	{
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
		sprintf(gc_display_message, "[Header] TR_CODE error..%3.3hs", gst_rcv_buff.tr_code);
		t1160_write_log(LOG_TYPE_NORMAL);
		return TR_CODE_INVALID;
	}

	/* 3. 기관 ID CHECK */
	if(strncmp(gst_rcv_buff.gigwan_id, gc_tconfig_comp_id, 3) != 0)
	{
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
		sprintf(gc_display_message, "[Header] Gigwan ID error..%3.3hs", gst_rcv_buff.gigwan_id);
		t1160_write_log(LOG_TYPE_NORMAL);
		return GIGWAN_ID_INVALID;
	}

	/* 4. (Anylink check) 전문 & 운용 type CHECK */
	/* (1) 허용 MSG type CHECK */
	if(strncmp(gc_tconfig_comm_type, "R", 1) == 0)
	{ /* 수신 프로세스인 경우 허용 MSG type Filtering */
		if((gs_rcv_message_type == MSG_0810_001) || \
		   (gs_rcv_message_type == MSG_0810_301) || \
	 	   (gs_rcv_message_type == MSG_0810_040) || \
	 	   (gs_rcv_message_type == MSG_0210_000))
		{
			memset(gc_display_message, 0x00, sizeof(gc_display_message));
			sprintf(gc_display_message, "Invalid Recv[R] MSG TYPE : %4.4hs.%3.3hs", gst_rcv_buff.msg_type, gst_rcv_buff.opr_type);
			t1160_write_log(LOG_TYPE_NORMAL);
			return MSG_TYPE_INVALID;
		}
	}
	else
	{ /* 송신 프로세스인 경우 허용 MSG type Filtering */
		if((gs_rcv_message_type == MSG_0810_001) || \
		   (gs_rcv_message_type == MSG_0800_301) || \
		   (gs_rcv_message_type == MSG_0800_040) || \
		   (gs_rcv_message_type == MSG_0200_000))
		{
			memset(gc_display_message, 0x00, sizeof(gc_display_message));
			sprintf(gc_display_message, "Invalid Recv[S] MSG TYPE : %4.4hs.%3.3hs", gst_rcv_buff.msg_type, gst_rcv_buff.opr_type);
			t1160_write_log(LOG_TYPE_NORMAL);
			return MSG_TYPE_INVALID;
		}

		if(gs_rcv_message_type == MSG_0810_040)
		{ /* 0800/040을 보내지 않았는데 0810/040이 들어오는 경우 */
			if(gs_snd_message_type != MSG_0800_040)
			{
			memset(gc_display_message, 0x00, sizeof(gc_display_message));
			sprintf(gc_display_message, "Invalid Recv[S] OPR TYPE : %4.4hs.%3.3hs", gst_rcv_buff.msg_type, gst_rcv_buff.opr_type);
			t1160_write_log(LOG_TYPE_NORMAL);
			return OPR_TYPE_INVALID;
			}
		}
	}

	/* (2) (개시 전) 전문/운용 type CHECK */
	if(gs_link_status == 0)
	{
		if((gs_rcv_message_type == MSG_0800_301) || \
		   (gs_rcv_message_type == MSG_0810_301) || \
		   (gs_rcv_message_type == MSG_0800_040) || \
		   (gs_rcv_message_type == MSG_0810_040) || \
		   (gs_rcv_message_type == MSG_0200_000) || \
		   (gs_rcv_message_type == MSG_0210_000))
		{
			memset(gc_display_message, 0x00, sizeof(gc_display_message));
			sprintf(gc_display_message, "RCV data before 0800/001 : %4.4hs %3.3hs", gst_rcv_buff.msg_type, gst_rcv_buff.opr_type);
			t1160_write_log(LOG_TYPE_NORMAL);
			return MSG_TYPE_INVALID;
		}
	}

	/* 5. (Header부) DATA No check */
	if((gs_rcv_message_type != MSG_0800_301) && (gs_rcv_message_type != MSG_0810_301))
	{
		memset(lc_data_no_string, 0x00, sizeof(lc_data_no_string));
		strncpy(lc_data_no_string, (char*)&gst_rcv_buff.data_no, 8);
		DNUMIN(lc_data_no_string, &gl_rcv_data_no, 10, &ls_numin_status);
		if(ls_numin_status != 0)
		{
			memset(gc_display_message, 0x00, sizeof(gc_display_message));
			sprintf(gc_display_message, "RCV [header] data no Numin error : %s", lc_data_no_string);
			t1160_write_log(LOG_TYPE_NORMAL);
			t9070_stop_process(FAIL);
		}

		if(strncmp(gc_tconfig_comm_type, "S", 1) == 0)
		{ /* 송신 프로세스인 경우 */
			if(gl_rcv_data_no != gl_last_data_count)
			{ /* 마지막 count와 같아야 함 */
				memset(gc_display_message, 0x00, sizeof(gc_display_message));
				sprintf(gc_display_message, "RCV [S header] data no : %ld..%ld", gl_rcv_data_no, gl_last_data_count);
				t1160_write_log(LOG_TYPE_NORMAL);
				return DATA_NO_INVALID;
			}
		}
		else
		{ /* 수신 프로세스인 경우 */
			if((gs_rcv_message_type == MSG_0800_001) || (gs_rcv_message_type == MSG_0800_040))
			{ /* 개시와 종료인 경우, 마지막 count와 같아야 함 */
				if(gl_rcv_data_no != gl_last_data_count)
				{
					memset(gc_display_message, 0x00, sizeof(gc_display_message));
					sprintf(gc_display_message, "RCV [R header] data no : %ld..%ld", gl_rcv_data_no, gl_last_data_count);
					t1160_write_log(LOG_TYPE_NORMAL);
					return DATA_NO_INVALID;
				}
			}
			else
			{ /* 일반 수신인 경우, 마지막 전송 "count + 1" 과 같아야 함 */
				if(gl_rcv_data_no != gl_next_data_count)
				{
					memset(gc_display_message, 0x00, sizeof(gc_display_message));
					sprintf(gc_display_message, "RCV [R header] data no : %ld..%ld", gl_rcv_data_no, gl_next_data_count);
					t1160_write_log(LOG_TYPE_NORMAL);
					return DATA_NO_INVALID;
				}
			}
		}
	}
	
	/* 6. (Header부) DATA Cnt check */
	memset(lc_data_cnt_string, 0x00, sizeof(lc_data_cnt_string));
	strncpy(lc_data_cnt_string, (char*)&gst_rcv_buff.data_cnt, 2);
	NUMIN(lc_data_cnt_string, &gs_rcv_data_cnt, 10, &ls_numin_status);
	if(ls_numin_status != 0)
	{
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
		sprintf(gc_display_message, "RCV [header] data Cnt Numin error : %s", lc_data_cnt_string);
		t1160_write_log(LOG_TYPE_NORMAL);
		t9070_stop_process(FAIL);
	}

	/*gs_rcv_data_cnt = block count */ 
	/*데이터가 없을 경우 block count는 0, 있을 경우 1 */
	if((gs_rcv_message_type != MSG_0200_000) && (gs_rcv_message_type != MSG_0210_000))
	{ /* 0200 이외에는 block count 는 '0' 이어야 함 */
		if(gs_rcv_data_cnt != 0)
		{
			memset(gc_display_message, 0x00, sizeof(gc_display_message));
			sprintf(gc_display_message, "RCV data block cnt : %ld..", gs_rcv_data_cnt);
			t1160_write_log(LOG_TYPE_NORMAL);
			return DATA_CNT_INVALID;
		}
	}
	else
	{ /* 0200의 block count 는 '1' 이어야 함 */
		if(gs_rcv_data_cnt != BLOCK_COUNT)
		{
			memset(gc_display_message, 0x00, sizeof(gc_display_message));
			sprintf(gc_display_message, "RCV data block cnt : %ld..", gs_rcv_data_cnt);
			t1160_write_log(LOG_TYPE_NORMAL);
			return DATA_CNT_INVALID;
		}
	}

	/* 7. INNER SEQUENCE CHECK */
	if(gs_rcv_message_type == MSG_0200_000)
	{ /* 0200/000인 경우만 check */
		memset(lc_data_seq_string, 0x00, sizeof(lc_data_seq_string));
		strncpy(lc_data_seq_string, (char*)&gst_rcv_buff.data_seq, 8);
		DNUMIN(lc_data_seq_string, &gl_rcv_seq_no, 10, &ls_numin_status);
		if(ls_numin_status != 0)
		{
			memset(gc_display_message, 0x00, sizeof(gc_display_message));
			sprintf(gc_display_message, "RCV [Data] data seq Numin error : %s", lc_data_cnt_string);
			t1160_write_log(LOG_TYPE_NORMAL);
			t9070_stop_process(FAIL);
		}

		if(gl_rcv_data_no != gl_rcv_seq_no)
		{
			memset(gc_display_message, 0x00, sizeof(gc_display_message));
			sprintf(gc_display_message, "RCV [Data] data seq error : %ld, %ld", gl_rcv_data_no, gl_rcv_seq_no);
			t1160_write_log(LOG_TYPE_NORMAL);
			return SEQ_ERROR;
		}
	}

	return NO_ERROR;
}

/* 송신 Message Setting */
/*----------------------------------------------------------------------------------------------------------*/
void t1130_message_set(short msg_type, int err_code, short retry_count, long data_no, short data_cnt)
/*----------------------------------------------------------------------------------------------------------*/
{
    char lc_data_no_string[9];
    char lc_data_cnt_string[3];
    char gc_dec_tgt[210], gc_dec_code[210];
    int li_error = 0;
    unsigned char *encount2 = NULL;
    unsigned int dncoutlen = 0;

    gs_snd_message_type = msg_type;

    memset((char*)&gst_snd_buff, 0x00, sizeof(gst_snd_buff));

    /* TR-Code Set */
    strncpy(gst_snd_buff.tr_code, TR_CODE, 9);

    /* 기관 ID Set */
    strncpy(gst_snd_buff.gigwan_id, gc_tconfig_comp_id, 3);

    /* 전문 type set */
    switch(msg_type)
    {
        case MSG_0810_001 :
            strncpy(gst_snd_buff.msg_type, "0810", 4);
            strncpy(gst_snd_buff.opr_type, "001", 3);
            break;
        case MSG_0800_301 :
            strncpy(gst_snd_buff.msg_type, "0800", 4);
            strncpy(gst_snd_buff.opr_type, "301", 3);
            break;
        case MSG_0810_301 :
            strncpy(gst_snd_buff.msg_type, "0810", 4);
            strncpy(gst_snd_buff.opr_type, "301", 3);
            break;
        case MSG_0800_040 :
            strncpy(gst_snd_buff.msg_type, "0800", 4);
            strncpy(gst_snd_buff.opr_type, "040",3);
            break;
        case MSG_0810_040 :
            strncpy(gst_snd_buff.msg_type, "0810", 4);
            strncpy(gst_snd_buff.opr_type, "040", 3);
            break;
        case MSG_0200_000 :
            strncpy(gst_snd_buff.msg_type, "0200", 4);
            strncpy(gst_snd_buff.opr_type, "000", 3);
            break;
        case MSG_0210_000 :
            strncpy(gst_snd_buff.msg_type, "0210", 4);
            strncpy(gst_snd_buff.opr_type, "000", 3);
            break ;
        default :
            memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "Invalid Message Type..%d (In Message Set)", msg_type);
            t1160_write_log(LOG_TYPE_NORMAL);
            t9070_stop_process(FAIL);
    }

    /* Error Code Set */
    switch (err_code)
    {
        case NO_ERROR :
            strncpy(gst_snd_buff.err_code, "00", 2);
            break;
        case SEQ_ERROR : /* DATA SEQ Error */
            strncpy(gst_snd_buff.err_code, "01", 2);
            break;
        case CNT_ERROR : /* DATA 건수 Error */
            strncpy(gst_snd_buff.err_code, "02", 2);
            break;
        case MARKET_BEF_ERROR : /* 장개시 전 Error */
            strncpy(gst_snd_buff.err_code, "03", 2);
            break; 
        case MARKET_AFT_ERROR : /* 장마감 Error */
            strncpy(gst_snd_buff.err_code, "04", 2);
            break; 
        case FORMAT_ERROR : /* 전문 길이 Error */
            strncpy(gst_snd_buff.err_code, "14", 2);
            break; 
        case PARSING_ERROR : /* 전문TR Error */
            strncpy(gst_snd_buff.err_code, "18", 2);
            break; 
        case TR_CODE_INVALID :
            strncpy(gst_snd_buff.err_code, "91", 2);
            break;
        case GIGWAN_ID_INVALID :
            strncpy(gst_snd_buff.err_code, "92", 2);
            break;
        case MSG_TYPE_INVALID :
            strncpy(gst_snd_buff.err_code, "93", 2);
            break;
        case DATA_NO_INVALID : /* DATA Number Error */
            strncpy(gst_snd_buff.err_code, "98", 2);
            break; 
        case DATA_CNT_INVALID : /* DATA 개수 Error */
            strncpy(gst_snd_buff.err_code, "99", 2);
            break; 
        default :
            memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "Invalid Error Code..%d(In Message Set)", err_code);
            t1160_write_log(LOG_TYPE_NORMAL);
            t9070_stop_process(FAIL);
    }

    /* Date & Time Set */
    t9020_get_time();
    strncpy(gst_snd_buff.time, &gc_today_date[2], 6);
    strncpy(&gst_snd_buff.time[6], gc_today_time, 6);

    /* Retry Set */
    NUMOUT(gst_snd_buff.retry_cnt, retry_count, 10, 2);

    /* Data 번호 및 갯수 */
    memset(lc_data_no_string, 0x00, sizeof(lc_data_no_string));
    memset(lc_data_cnt_string, 0x00, sizeof(lc_data_cnt_string));
    DNUMOUT(lc_data_no_string, data_no, 10, 8);
    NUMOUT(lc_data_cnt_string, data_cnt, 10, 2);
    strncpy(gst_snd_buff.data_no, lc_data_no_string, 8);
    strncpy(gst_snd_buff.data_cnt, lc_data_cnt_string, 2);

    /* Data부 Set */
    if(msg_type == MSG_0200_000)
    {
        if((strncmp(gc_tconfig_tr_code, "L2", 2) == 0) ||   /* "L2" 거래 송신 */
           (strncmp(gc_tconfig_tr_code, "L8", 2) == 0) ||   /* "L8" 소액 송신 */
           (strncmp(gc_tconfig_tr_code, "M2", 2) == 0) ||   /* "M2" CD 송신 */
           (strncmp(gc_tconfig_tr_code, "M4", 2) == 0) ||   /* "M4" RP 송신 */
           (strncmp(gc_tconfig_tr_code, "T3", 2) == 0))     /* "T3" 전문딜러 송신 */
        {
            strncpy(gst_snd_buff.snd_data, gc_data_file_buff, 8);
            strncpy(&gst_snd_buff.snd_data[8], &gc_data_file_buff[38], gs_tconfig_message_length - 8);
        }
        else if(strncmp(gc_tconfig_tr_code, "L3", 2) == 0)  /* "L3" 장운영 송신 */
            strncpy(gst_snd_buff.snd_data, gc_data_file_buff, gs_tconfig_message_length);
        else if(strncmp(gc_tconfig_tr_code, "L4", 2) == 0)  /* "L4" 발행정보 송신 */
        {
            strncpy(gst_snd_buff.snd_data, gc_data_file_buff,8);
            strncpy(&gst_snd_buff.snd_data[8], &gc_data_file_buff[12], gs_tconfig_message_length - 8);
        }
         else if(strncmp(gc_tconfig_tr_code, "T1", 2) == 0)
         {
             strncpy(gst_snd_buff.snd_data, gc_data_file_buff, 8);
             strncpy(&gst_snd_buff.snd_data[8], &gc_data_file_buff[34], gs_tconfig_message_length - 8);
         }
         else if(strncmp(gc_tconfig_tr_code, "K3", 2) == 0) /* "K3" 거부 송신 */
         { /* 상속인 금융거래조회의 Key는 6자리 및 암호화 해제 */
             if(strncmp(gst_fts_krjt_rec.krjt000_sub_tr_code, "93", 2) != 0)
             {
                 strncpy(gst_snd_buff.snd_data     , "00"                                , 2);
                 strncpy(&gst_snd_buff.snd_data[2] , gst_fts_krjt_rec.krjt000_seq_no     , 6);    /* 일련번호 */
                 strncpy(&gst_snd_buff.snd_data[8] , gst_fts_krjt_rec.krjt000_tr_code    , 2);    /* TR코드 */
                 strncpy(&gst_snd_buff.snd_data[10], gst_fts_krjt_rec.krjt000_sub_tr_code, 2);    /* SUB TR코드 */
                 strncpy(&gst_snd_buff.snd_data[12], gst_fts_krjt_rec.krjt000_rjt_code   , 3);    /* 거부코드 */
                 strncpy(&gst_snd_buff.snd_data[15], gst_fts_krjt_rec.mbr_no             , 3);    /* 증권사코드 */
                 strncpy(&gst_snd_buff.snd_data[18], gst_fts_krjt_rec.fs_seq_no          , 11);   /* 접수번호 */
                 strncpy(&gst_snd_buff.snd_data[29], gst_fts_krjt_rec.fs_yymmdd          , 8);    /* 접수일자 */
                 strncpy(&gst_snd_buff.snd_data[37], gst_fts_krjt_rec.app_yymmdd         , 8);    /* 승인일자 */
                 strncpy(&gst_snd_buff.snd_data[45], gst_fts_krjt_rec.jeobsu_gubun       , 2);    /* 접수구분 */
                 strncpy(&gst_snd_buff.snd_data[47], gst_fts_krjt_rec.name_tgt           , 30);   /* 성명 */
    
                 memset(gc_dec_tgt, 0x00, sizeof(gc_dec_tgt));
                 memset(gc_dec_code, 0x00, sizeof(gc_dec_code));
                 memcpy(gc_dec_tgt, gst_fts_krjt_rec.code_tgt, 16);
                 li_error = DecData((unsigned char *)&gc_dec_tgt, 16, &encount2, &dncoutlen);
                 if(li_error != NO_ERROR)
                 {
                     memset(gc_display_message, 0x00, sizeof(gc_display_message));
                     sprintf(gc_display_message, "K3 Code_tgt Error..%d(In Message Set)", li_error);
                     t1160_write_log(LOG_TYPE_NORMAL);
                     t9070_stop_process(FAIL);
                 }
                 else
                     strncpy(gc_dec_code, (const char *)encount2, 13);
    
                 strncpy(&gst_snd_buff.snd_data[77] , gc_dec_code                        , 13);   /* 생년월일 */
                 strncpy(&gst_snd_buff.snd_data[90] , gst_fts_krjt_rec.accnt_cnt         , 2);    /* 계좌존재확인 */
                 strncpy(&gst_snd_buff.snd_data[92] , gst_fts_krjt_rec.jango_gubun       , 1);    /* 잔고존재여부 */
                 strncpy(&gst_snd_buff.snd_data[93] , gst_fts_krjt_rec.debt_amt          , 18);   /* 채무금액 */
                 strncpy(&gst_snd_buff.snd_data[111], gst_fts_krjt_rec.debt_not_decided  , 1);    /* 금액미확정채무 */
                 strncpy(&gst_snd_buff.snd_data[112], gst_fts_krjt_rec.filler            , 370);  /* filler */
                 strncpy(&gst_snd_buff.snd_data[482], "                     "            , 18);   /* space */
             }
             else
             { /* sub TR 93 마감 */
                 memset((char *)&gst_snd_buff.snd_data,' ',gs_tconfig_message_length);
                 strncpy(gst_snd_buff.snd_data     , "00"                                , 2);
                 strncpy(&gst_snd_buff.snd_data[2] , gst_fts_krjt_rec.krjt000_seq_no     , 6);    /* 일련번호 */
                 strncpy(&gst_snd_buff.snd_data[8] , gst_fts_krjt_rec.krjt000_tr_code    , 2);    /* TR코드 */
                 strncpy(&gst_snd_buff.snd_data[10], gst_fts_krjt_rec.krjt000_sub_tr_code, 2);    /* SUB TR코드 */
             }
         }
         else if(strncmp(gc_tconfig_tr_code, "K2", 2) ==0) /* "K2" 요청 송신 */
         { /* 상속인 금융거래조회의 Key는 6자리 및 암호화 해제 */
             if(strncmp(gst_fts_ksnd_rec.ksnd000_sub_code, "92", 2) != 0)
             {
                 strncpy(gst_snd_buff.snd_data     , "00"                             , 2);
                 strncpy(&gst_snd_buff.snd_data[2] , gst_fts_ksnd_rec.ksnd000_seq_no  , 6);    /* 일련번호 */
                 strncpy(&gst_snd_buff.snd_data[8] , gst_fts_ksnd_rec.ksnd000_tr_code , 2);    /* TR코드 */
                 strncpy(&gst_snd_buff.snd_data[10], gst_fts_ksnd_rec.ksnd000_sub_code, 2);    /* SUB TR코드 */
                 strncpy(&gst_snd_buff.snd_data[12], gst_fts_ksnd_rec.req_type        , 1);    /* 처리구분 */
                 strncpy(&gst_snd_buff.snd_data[13], gst_fts_ksnd_rec.fs_seq_no       , 11);   /* 접수번호 */
                 strncpy(&gst_snd_buff.snd_data[24], gst_fts_ksnd_rec.fs_yymmdd       , 8);    /* 접수일자 */
                 strncpy(&gst_snd_buff.snd_data[32], gst_fts_ksnd_rec.app_yymmdd      , 8);    /* 승인일자 */
                 strncpy(&gst_snd_buff.snd_data[40], gst_fts_ksnd_rec.jeobsu_gubun    , 2);    /* 접수구분 */
                 strncpy(&gst_snd_buff.snd_data[42], gst_fts_ksnd_rec.name_req        , 30);   /* 성명 */

                 memset(gc_dec_tgt, 0x00, sizeof(gc_dec_tgt));
                 memcpy(gc_dec_tgt, gst_fts_ksnd_rec.code_req, 16);
                 li_error = DecData((unsigned char *)&gc_dec_tgt, 16, &encount2, &dncoutlen);
                 if(li_error != NO_ERROR)
                 {
                     memset(gc_display_message, 0x00, sizeof(gc_display_message));
                     sprintf(gc_display_message, "K2 code_req Error..%d(In Message Set)", li_error);
                     t1160_write_log(LOG_TYPE_NORMAL);
                     t9070_stop_process(FAIL);
                 }
                 else
                     strncpy(&gst_snd_buff.snd_data[72], (const char *)encount2, dncoutlen);      /* 생년월일(13자리) */

                 memset(gc_dec_tgt, 0x00, sizeof(gc_dec_tgt));
                 memcpy(gc_dec_tgt, gst_fts_ksnd_rec.zip_req, 16);
                 li_error = DecData((unsigned char *)&gc_dec_tgt, 16, &encount2, &dncoutlen);
                 if(li_error != NO_ERROR)
                 {
                     memset(gc_display_message, 0x00, sizeof(gc_display_message));
                     sprintf(gc_display_message, "K2 zip_req Error..%d(In Message Set)", li_error);
                     t1160_write_log(LOG_TYPE_NORMAL);
                     t9070_stop_process(FAIL);
                 }
                 else
                     strncpy(&gst_snd_buff.snd_data[85], (const char *)encount2, dncoutlen);      /* 우편번호(6자리) */

                 memset(gc_dec_tgt, 0x00, sizeof(gc_dec_tgt));
                 memcpy(gc_dec_tgt, gst_fts_ksnd_rec.address_req, 208);
                 li_error = DecData((unsigned char *)&gc_dec_tgt, 208, &encount2, &dncoutlen);
                 if(li_error != NO_ERROR)
                 {
                     memset(gc_display_message, 0x00, sizeof(gc_display_message));
                     sprintf(gc_display_message, "K2 address_req Error..%d(In Message Set)", li_error);
                     t1160_write_log(LOG_TYPE_NORMAL);
                     t9070_stop_process(FAIL);
                 }
                 else
                     strncpy(&gst_snd_buff.snd_data[91], (const char *)encount2, dncoutlen);      /* 주소(200자리) */

                 memset(gc_dec_tgt, 0x00, sizeof(gc_dec_tgt));
                 memcpy(gc_dec_tgt, gst_fts_ksnd_rec.phone_req, 32);
                 li_error = DecData((unsigned char *)&gc_dec_tgt, 32, &encount2, &dncoutlen);
                 if(li_error != NO_ERROR)
                 {
                     memset(gc_display_message, 0x00, sizeof(gc_display_message));
                     sprintf(gc_display_message, "K2 phone_req Error..%d(In Message Set)", li_error);
                     t1160_write_log(LOG_TYPE_NORMAL);
                     t9070_stop_process(FAIL);
                 }
                 else
                     strncpy(&gst_snd_buff.snd_data[291], (const char *)encount2, dncoutlen);     /* 전화번호(24자리) */

                 memset(gc_dec_tgt, 0x00, sizeof(gc_dec_tgt));
                 memcpy(gc_dec_tgt, gst_fts_ksnd_rec.mobile_req, 32);
                 li_error = DecData((unsigned char *)&gc_dec_tgt, 32, &encount2, &dncoutlen);
                 if(li_error != NO_ERROR)
                 {
                     memset(gc_display_message, 0x00, sizeof(gc_display_message));
                     sprintf(gc_display_message, "K2 mobile_req Error..%d(In Message Set)", li_error);
                     t1160_write_log(LOG_TYPE_NORMAL);
                     t9070_stop_process(FAIL);
                 }
                 else
                     strncpy(&gst_snd_buff.snd_data[315], (const char *)encount2, dncoutlen);     /* 휴대폰(24자리) */

                 memset(gc_dec_tgt, 0x00, sizeof(gc_dec_tgt));
                 memcpy(gc_dec_tgt, gst_fts_ksnd_rec.email_req, 64);
                 li_error = DecData((unsigned char *)&gc_dec_tgt, 64, &encount2, &dncoutlen);
                 if(li_error != NO_ERROR)
                 {
                     memset(gc_display_message, 0x00, sizeof(gc_display_message));
                     sprintf(gc_display_message, "K2 email_req Error..%d(In Message Set)", li_error);
                     t1160_write_log(LOG_TYPE_NORMAL);
                     t9070_stop_process(FAIL);
                 }
                 else
                     strncpy(&gst_snd_buff.snd_data[339], (const char *)encount2, dncoutlen);     /* 이메일(50자리) */

                 /* 상속인 Set */
                 strncpy(&gst_snd_buff.snd_data[389], gst_fts_ksnd_rec.name_inherit, 30);         /* 성명 */

                 memset(gc_dec_tgt, 0x00, sizeof(gc_dec_tgt));
                 memcpy(gc_dec_tgt, gst_fts_ksnd_rec.code_inherit, 16);
                 li_error = DecData((unsigned char *)&gc_dec_tgt, 16, &encount2, &dncoutlen);
                 if(li_error != NO_ERROR)
                 {
                     memset(gc_display_message, 0x00, sizeof(gc_display_message));
                     sprintf(gc_display_message, "K2 code_inherit Dec Error..%d(In Message Set)", li_error);
                     t1160_write_log(LOG_TYPE_NORMAL);
                     t9070_stop_process(FAIL);
                 }
                 else
                     strncpy(&gst_snd_buff.snd_data[419], (const char *)encount2, dncoutlen);     /* 생년월일(13자리) */

                 memset(gc_dec_tgt, 0x00, sizeof(gc_dec_tgt));
                 memcpy(gc_dec_tgt, gst_fts_ksnd_rec.zip_inherit, 16);
                 li_error = DecData((unsigned char *)&gc_dec_tgt, 16, &encount2, &dncoutlen);
                 if(li_error != NO_ERROR)
                 {
                     memset(gc_display_message, 0x00, sizeof(gc_display_message));
                     sprintf(gc_display_message, "K2 zip_inherit Error..%d(In Message Set)", li_error);
                     t1160_write_log(LOG_TYPE_NORMAL);
                     t9070_stop_process(FAIL);
                 }
                 else
                     strncpy(&gst_snd_buff.snd_data[432], (const char *)encount2, dncoutlen);     /* 우편번호(6자리) */

                 memset(gc_dec_tgt, 0x00, sizeof(gc_dec_tgt));
                 memcpy(gc_dec_tgt, gst_fts_ksnd_rec.address_inherit, 208);
                 li_error = DecData((unsigned char *)&gc_dec_tgt, 208, &encount2, &dncoutlen);
                 if(li_error != NO_ERROR)
                 {
                     memset(gc_display_message, 0x00, sizeof(gc_display_message));
                     sprintf(gc_display_message, "K2 address_inherit Error..%d(In Message Set)", li_error);
                     t1160_write_log(LOG_TYPE_NORMAL);
                     t9070_stop_process(FAIL);
                 }
                 else
                     strncpy(&gst_snd_buff.snd_data[438], (const char *)encount2, dncoutlen);     /* 주소(200자리) */

                 memset(gc_dec_tgt, 0x00, sizeof(gc_dec_tgt));
                 memcpy(gc_dec_tgt, gst_fts_ksnd_rec.phone_inherit, 32);
                 li_error = DecData((unsigned char *)&gc_dec_tgt, 32, &encount2, &dncoutlen);
                 if(li_error != NO_ERROR)
                 {
                     memset(gc_display_message, 0x00, sizeof(gc_display_message));
                     sprintf(gc_display_message, "K2 phone_inherit Error..%d(In Message Set)", li_error);
                     t1160_write_log(LOG_TYPE_NORMAL);
                     t9070_stop_process(FAIL);
                 }
                 else
                     strncpy(&gst_snd_buff.snd_data[638], (const char *)encount2, dncoutlen);     /* 전화번호(24자리) */

                 memset(gc_dec_tgt, 0x00, sizeof(gc_dec_tgt));
                 memcpy(gc_dec_tgt, gst_fts_ksnd_rec.mobile_inherit, 32);
                 li_error = DecData((unsigned char *)&gc_dec_tgt, 32, &encount2, &dncoutlen);
                 if(li_error != NO_ERROR)
                 {
                     memset(gc_display_message, 0x00, sizeof(gc_display_message));
                     sprintf(gc_display_message, "K2 mobile_inherit Error..%d(In Message Set)", li_error);
                     t1160_write_log(LOG_TYPE_NORMAL);
                     t9070_stop_process(FAIL);
                 }
                 else
                     strncpy(&gst_snd_buff.snd_data[662], (const char *)encount2, dncoutlen);     /* 휴대폰(24자리) */

                 memset(gc_dec_tgt, 0x00, sizeof(gc_dec_tgt));
                 memcpy(gc_dec_tgt, gst_fts_ksnd_rec.email_inherit, 64);
                 li_error = DecData((unsigned char *)&gc_dec_tgt, 64, &encount2, &dncoutlen);
                 if(li_error != NO_ERROR)
                 {
                     memset(gc_display_message, 0x00, sizeof(gc_display_message));
                     sprintf(gc_display_message, "K3 email_inherit Error..%d(In Message Set)", li_error);
                     t1160_write_log(LOG_TYPE_NORMAL);
                     t9070_stop_process(FAIL);
                 }
                 else
                     strncpy(&gst_snd_buff.snd_data[686],(const char *)encount2,dncoutlen);       /* 이메일(50자리) */

                 /* 피상속인 Set */
                 strncpy(&gst_snd_buff.snd_data[736], gst_fts_ksnd_rec.name_tgt, 30);             /* 성명 */

                 memset(gc_dec_tgt, 0x00, sizeof(gc_dec_tgt));
                 memcpy(gc_dec_tgt, gst_fts_ksnd_rec.code_tgt, 16);
                 li_error = DecData((unsigned char *)&gc_dec_tgt, 16, &encount2, &dncoutlen);
                 if(li_error != NO_ERROR)
                 {
                     memset(gc_display_message, 0x00, sizeof(gc_display_message));
                     sprintf(gc_display_message, "K2 code_tgt Dec Error..%d(In Message Set)", li_error);
                     t1160_write_log(LOG_TYPE_NORMAL);
                     t9070_stop_process(FAIL);
                 }
                 else
                     strncpy(&gst_snd_buff.snd_data[766], (const char *)encount2, dncoutlen);     /* 주민번호(13자리) */

                 memset(gc_dec_tgt, 0x00, sizeof(gc_dec_tgt));
                 memcpy(gc_dec_tgt, gst_fts_ksnd_rec.zip_tgt, 16);
                 li_error = DecData((unsigned char *)&gc_dec_tgt, 16, &encount2, &dncoutlen);
                 if(li_error != NO_ERROR)
                 {
                     memset(gc_display_message, 0x00, sizeof(gc_display_message));
                     sprintf(gc_display_message, "K2 zip_tgt Error..%d(In Message Set)", li_error);
                     t1160_write_log(LOG_TYPE_NORMAL);
                     t9070_stop_process(FAIL);
                 }
                 else
                     strncpy(&gst_snd_buff.snd_data[779], (const char *)encount2, dncoutlen);     /* 우편번호(6자리) */

                 memset(gc_dec_tgt, 0x00, sizeof(gc_dec_tgt));
                 memcpy(gc_dec_tgt, gst_fts_ksnd_rec.address_tgt, 208);
                 li_error = DecData((unsigned char *)&gc_dec_tgt, 208, &encount2, &dncoutlen);
                 if(li_error != NO_ERROR)
                 {
                     memset(gc_display_message, 0x00, sizeof(gc_display_message));
                     sprintf(gc_display_message, "K2 address_tgt Error..%d(In Message Set)", li_error);
                     t1160_write_log(LOG_TYPE_NORMAL);
                     t9070_stop_process(FAIL);
                 }
                 else
                     strncpy(&gst_snd_buff.snd_data[785], (const char*)encount2, dncoutlen);      /* 주소(200자리) */

                 strncpy(&gst_snd_buff.snd_data[985] , gst_fts_ksnd_rec.yymmdd_tgt     , 8);      /*사유발생일 */
                 strncpy(&gst_snd_buff.snd_data[993] , gst_fts_ksnd_rec.relation_tgt   , 2);      /*관계 */
                 strncpy(&gst_snd_buff.snd_data[995] , gst_fts_ksnd_rec.gubun_tgt      , 2);      /*사유 */
                 strncpy(&gst_snd_buff.snd_data[997] , gst_fts_ksnd_rec.kofia_inform_il, 8);      /*협회통보일 */
                 strncpy(&gst_snd_buff.snd_data[1005], gst_fts_ksnd_rec.jeobsu_gigwan  , 4);      /*접수기관 */
                 strncpy(&gst_snd_buff.snd_data[1009], gst_fts_ksnd_rec.list_jijum     , 100);    /*등록지점 */
                 strncpy(&gst_snd_buff.snd_data[1109], gst_fts_ksnd_rec.list_number    , 20);     /*연락처 */
                 strncpy(&gst_snd_buff.snd_data[1129], gst_fts_ksnd_rec.trans_gigwan   , 20);     /*이첩기관 */
                 strncpy(&gst_snd_buff.snd_data[1149], gst_fts_ksnd_rec.change_cancel  , 500);    /*변경사유 */
                 strncpy(&gst_snd_buff.snd_data[1649], "                              ", 30);     /*추후 사용용 */
                 strncpy(&gst_snd_buff.snd_data[1679], "                              ", 30);     /*추후 사용용 */
                 strncpy(&gst_snd_buff.snd_data[1709], "                              ", 30);     /*추후 사용용 */
                 strncpy(&gst_snd_buff.snd_data[1739], "                              ", 30);     /*추후 사용용 */
                 strncpy(&gst_snd_buff.snd_data[1769], "                              ", 30);     /*추후 사용용 */
                 strncpy(&gst_snd_buff.snd_data[1799], "                              ", 1);

                 if(strncmp(gc_tconfig_comp_id, "996", 3) == 0)
                 { /* PB는 sub_tr 뒤에 회사코드 추가 */
                     gs_message_length = (short)strlen((char*)&gst_snd_buff.tr_code);             /* 전문길이 생성 */

                     memset((char*)&gst_snd_996_buff, 0x00, sizeof(gst_snd_996_buff));
                     memcpy((char*)&gst_snd_996_buff.snd_996_header , (char*)&gst_snd_buff, 61);
                     memcpy((char*)&gst_snd_996_buff.snd_996_comp_id, (char*)&gst_fts_ksnd_rec.ksnd000_mbr_no, 3);
                     memcpy((char*)&gst_snd_996_buff.snd_996_data   , (char*)&gst_snd_buff.snd_data[12], 1785);

                     memset((char*)&gst_snd_buff, 0x00, sizeof(gst_snd_buff));
                     memcpy((char*)&gst_snd_buff, (char *)&gst_snd_996_buff, 1849);
                 }

             }
             else
             { /* sub TR 92 마감 */
                 memset((char*)&gst_snd_buff.snd_data, ' ', gs_tconfig_message_length);
                 strncpy(gst_snd_buff.snd_data     , "00", 2);
                 strncpy(&gst_snd_buff.snd_data[2] , gst_fts_ksnd_rec.ksnd000_seq_no  , 6);       /*일련번호 */
                 strncpy(&gst_snd_buff.snd_data[8] , gst_fts_ksnd_rec.ksnd000_tr_code , 2);       /*TR코드 */
                 strncpy(&gst_snd_buff.snd_data[10], gst_fts_ksnd_rec.ksnd000_sub_code, 2);       /*SUB TR코드 */
             }
         }
         else
             strncpy(gst_snd_buff.snd_data, gc_data_file_buff, gs_tconfig_message_length);
    }

    /* 전문길이 Set */
    gs_message_length = (short)strlen((char*)&gst_snd_buff.tr_code);     /*전문길이 생성 */
    NUMOUT(gst_snd_buff.message_length, gs_message_length, 10, 4);
}

/* 송신 Message 송신 */
/*-----------------------------------------------*/
short t1140_send_to_socket()
/*-----------------------------------------------*/
{
	short ls_result = 0;
	int gone_length = 0;

	/* 1. Message Send */
	/* "gst_snd_buff"에 Setting 한 Message 송신 */
	if((send_nw(new_socket_num, (char*)&gst_snd_buff, (int)strlen((char*)&gst_snd_buff), 0, gl_snd_tag)) < 0)
	{
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
		sprintf(gc_display_message, "Socket Write Error(%d)..", gl_snd_tag);
		t1160_write_log(LOG_TYPE_NORMAL);
		return FAIL;
	}
	else
	{
		ls_result = t0003_io_check((short)new_socket_num, -1); /*무한 대기 */
		if(ls_result != SUCCESS)
			return ls_result;
	}

	gone_length = gst_snw->nb_sent;

	if(gone_length == 0)
	{
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
		sprintf(gc_display_message, "Snd Message : %d bytes", gone_length);
		t1160_write_log(LOG_TYPE_NORMAL);
		return FAIL;
	}
	else
	{
		gi_send_message_length = gone_length;
		t1160_write_log(LOG_TYPE_SEND);
	}

	/* 2. Link Status Check */ 
	/* 송신이 개시응답이고, 수신 에러코드가 0일 경우 Link 되었음을 표시 */
	if((gs_snd_message_type == MSG_0810_001) && (gs_rcv_err_code == NO_ERROR))
		gs_start_status = 1; /* Link Start! */

	/* 3. 마감 Message Check */
	/* 송신 Message의 Data부의 마감 Message가 Setting 되었을 경우, 장상태를 마감으로 변경 */
	if((strncmp(&gst_snd_buff.snd_data[10], "91", 2)==0) ||
	   (strncmp(&gst_snd_buff.snd_data[10], "92", 2)==0) ||
       (strncmp(&gst_snd_buff.snd_data[10], "93", 2)==0) ||
       (strncmp(&gst_snd_buff.snd_data[10], "94", 2)==0) ||
       (strncmp(&gst_snd_buff.snd_data[10], "99", 2)==0))
	{
		gs_jang_status = JANG_LINK_MAGAM;
	}

	return SUCCESS;
}

/* 수신 Data를 수신 File에 Write */
/*--------------------------------------------------------*/
void t1150_write_rcv_rec()
/*--------------------------------------------------------*/
{
    short file_info = 0;
    unsigned short ls_write_count = 0;
    long ll_record_spec = -1L; /* EOF에 position */
    unsigned char *encount1 = NULL;
    unsigned int encoutlen;
    char code_tgt[17], lc_enc_code[17];
    int li_error = -1;
    _cc_status CC; /* status variable */

    if(strncmp(gc_tconfig_tr_code, "L1", 2) == 0)
    { /* L1 채권 거래 내역 */
        memset((char*)&gst_bgererv_rec, 0x00, sizeof(gst_bgererv_rec));

        strncpy(gst_bgererv_rec.bgererv_if_whisa_code, gc_tconfig_comp_id, 3);
        strncpy(gst_bgererv_rec.bgererv_if_seq_no, gc_next_data_count, 8);

        if((strncmp(gst_rcv_buff.data_sub_tr_code, "11", 2) == 0) && (gs_jang_status <= JANG_LAST))
        {
            strncpy(gst_bgererv_rec.bgererv_if_error_code, "502", 3); /* L111 장 전 오류 */
        }
        else if((strncmp(gst_rcv_buff.data_sub_tr_code, "12", 2) == 0) && (gs_jang_status >= JANG_TODAY))
        {
            strncpy(gst_bgererv_rec.bgererv_if_error_code, "503", 3); /* L112 장종료 후 오류 */
        }
        else
            strncpy(gst_bgererv_rec.bgererv_if_error_code, "000", 3); /* 정상 */

        strncpy(gst_bgererv_rec.bgererv_if_rcv_sigak, gc_rcv_time, 6);
        strncpy(gst_bgererv_rec.bgererv_tr_code, gst_rcv_buff.data_tr_code, 2);
        strncpy(gst_bgererv_rec.bgererv_sub_tr_code, gst_rcv_buff.data_sub_tr_code, 2);
        strncpy(gst_bgererv_rec.tr_code, gst_rcv_buff.data_tr_code, 2);
        strncpy(gst_bgererv_rec.sub_tr_code, gst_rcv_buff.data_sub_tr_code, 2);
        strncpy(gst_bgererv_rec.gerercv, gst_rcv_buff.rcv_data, 112);
        strncpy(gst_bgererv_rec.gerercv_bogo_il, "00000000", 8);
        strncpy(gst_bgererv_rec.gerercv1, &gst_rcv_buff.rcv_data[112], 9);
        strncpy(&gst_bgererv_rec.gerercv1[9], "00000000000000000000000000", 21);
        strncpy(&gst_bgererv_rec.gerercv1[30], &gst_rcv_buff.rcv_data[142], 346);
        strncpy(gst_bgererv_rec.filler, "00000000000000000000000000", 26);

        CC = POSITION(gs_open_data_file_num, ll_record_spec);
        if(_status_ne(CC))
        {
            FILE_GETINFO_(gs_open_data_file_num, &file_info);
            memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "BGERERV file write Position error code = %d", file_info);
            t1160_write_log(LOG_TYPE_NORMAL);
            ABORTTRANSACTION();
            t9070_stop_process(FAIL);
        }

        CC = WRITEX(gs_open_data_file_num, (char*)&gst_bgererv_rec, (short)strlen((char*)&gst_bgererv_rec), &ls_write_count);
        if(_status_ne(CC))
        {
            FILE_GETINFO_(gs_open_data_file_num, &file_info);
            memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "BGERERV file write error code = %d", file_info);
            t1160_write_log(LOG_TYPE_NORMAL);
            ABORTTRANSACTION();
            t9070_stop_process(FAIL);
        }
        else
        {
            memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "BGERERV file write..%d", ls_write_count);
            t1160_write_log(LOG_TYPE_NORMAL);
        }
    }

    else if((strncmp(gc_tconfig_tr_code, "L7", 2) == 0) || (strncmp(gc_tconfig_tr_code, "T2", 2) == 0))
    { /* L7 채권판매정보, T2 전문딜러호가 */
        memset((char*)&gst_bsodlrcv_rec, 0x00, sizeof(gst_bsodlrcv_rec));

        strncpy(gst_bsodlrcv_rec.bsodlrcv_if_whisa_code, gc_tconfig_comp_id, 3);
        strncpy(gst_bsodlrcv_rec.bsodlrcv_if_seq_no, gc_next_data_count, 8);
        strncpy(gst_bsodlrcv_rec.bsodlrcv_if_error_code, "000", 3); /* 정상 */
        strncpy(gst_bsodlrcv_rec.bsodlrcv_if_rcv_sigak, gc_rcv_time, 6);
        strncpy(gst_bsodlrcv_rec.bsodlrcv_tr_code, gst_rcv_buff.data_tr_code, 2);
        strncpy(gst_bsodlrcv_rec.bsodlrcv_sub_tr_code, gst_rcv_buff.data_sub_tr_code, 2);
        strncpy(gst_bsodlrcv_rec.tr_code, gst_rcv_buff.data_tr_code, 2);
        strncpy(gst_bsodlrcv_rec.sub_tr_code, gst_rcv_buff.data_sub_tr_code, 2);
        strncpy(gst_bsodlrcv_rec.soakdlrcv, gst_rcv_buff.rcv_data, 488);
        strncpy(gst_bsodlrcv_rec.filler, "0000000000000000000000000000000000", 34);

        CC = POSITION(gs_open_data_file_num, ll_record_spec);
        if(_status_ne(CC))
        {
            FILE_GETINFO_(gs_open_data_file_num, &file_info);
            memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "SODL file write Position error code = %d", file_info);
            t1160_write_log(LOG_TYPE_NORMAL);
            ABORTTRANSACTION();
            t9070_stop_process(FAIL);
        }

        CC = WRITEX(gs_open_data_file_num, (char*)&gst_bsodlrcv_rec, (short)strlen((char*)&gst_bsodlrcv_rec), &ls_write_count);
        if(_status_ne(CC))
        {
            FILE_GETINFO_(gs_open_data_file_num, &file_info);
            memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "SODL file write error code = %d", file_info);
            t1160_write_log(LOG_TYPE_NORMAL);
            ABORTTRANSACTION();
            t9070_stop_process(FAIL);
        }
        else
        {
            memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "SODL file write..%d", ls_write_count);
            t1160_write_log(LOG_TYPE_NORMAL);
        }
    }

    else if((strncmp(gc_tconfig_tr_code, "M1", 2) == 0) || (strncmp(gc_tconfig_tr_code, "M3", 2) == 0))
    { /* M1 CD거래내역, M3 RP거래내역 */
        memset((char*)&gst_bcdrpgrcv_rec, 0x00, sizeof(gst_bcdrpgrcv_rec));

        strncpy(gst_bcdrpgrcv_rec.bcdrpgrcv_if_whisa_code, gc_tconfig_comp_id, 3);
        strncpy(gst_bcdrpgrcv_rec.bcdrpgrcv_if_seq_no, gc_next_data_count, 8);

        if((strncmp(gst_rcv_buff.data_sub_tr_code, "11", 2) == 0) && (gs_jang_status == JANG_BEFORE))
        {
            strncpy(gst_bcdrpgrcv_rec.bcdrpgrcv_if_error_code, "502", 3); /* M1, M3 11 장 전 오류 */
        }
        else if((strncmp(gst_rcv_buff.data_sub_tr_code, "12", 2) == 0) && (gs_jang_status >= JANG_TODAY))
        {
            strncpy(gst_bcdrpgrcv_rec.bcdrpgrcv_if_error_code, "503", 3); /* M1, M3 12 장종료 후 오류 */
        }
        else
            strncpy(gst_bcdrpgrcv_rec.bcdrpgrcv_if_error_code, "000", 3); /* 정상 */

        strncpy(gst_bcdrpgrcv_rec.bcdrpgrcv_if_rcv_sigak, gc_rcv_time, 6);
        strncpy(gst_bcdrpgrcv_rec.bcdrpgrcv_tr_code, gst_rcv_buff.data_tr_code, 2);
        strncpy(gst_bcdrpgrcv_rec.bcdrpgrcv_sub_tr_code, gst_rcv_buff.data_sub_tr_code, 2);
        strncpy(gst_bcdrpgrcv_rec.tr_code, gst_rcv_buff.data_tr_code, 2);
        strncpy(gst_bcdrpgrcv_rec.sub_tr_code, gst_rcv_buff.data_sub_tr_code, 2);
        strncpy(gst_bcdrpgrcv_rec.cdrpgercv, gst_rcv_buff.rcv_data, 488);
        strncpy(gst_bcdrpgrcv_rec.filler, "0000000000000000000000000000000000", 34);

         CC = POSITION(gs_open_data_file_num, ll_record_spec);
        if(_status_ne(CC))
        {
            FILE_GETINFO_(gs_open_data_file_num, &file_info);
            memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "CDRP file write Position error code = %d", file_info);
            t1160_write_log(LOG_TYPE_NORMAL);
            ABORTTRANSACTION();
            t9070_stop_process(FAIL);
        }

        CC = WRITEX(gs_open_data_file_num, (char*)&gst_bcdrpgrcv_rec, (short)strlen((char*)&gst_bcdrpgrcv_rec), &ls_write_count);
        if(_status_ne(CC))
        {
            FILE_GETINFO_(gs_open_data_file_num, &file_info);
            memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "CDRP file write error code = %d", file_info);
            t1160_write_log(LOG_TYPE_NORMAL);
            ABORTTRANSACTION();
            t9070_stop_process(FAIL);
        }
        else
        {
            memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "CDRP file write..%d", ls_write_count);
            t1160_write_log(LOG_TYPE_NORMAL);
        }
    }

    else if(strncmp(gc_tconfig_tr_code, "K1", 2) == 0)
    { /* K1 요청 수신, 상속인 금융거래조회  */
        memset((char*)&gst_ftsrcv_rec, 0x00, sizeof(gst_ftsrcv_rec));

        strncpy(gst_ftsrcv_rec.kmbrrcv_tr_code, gst_rcv_buff.data_tr_code, 2);
        strncpy(gst_ftsrcv_rec.kmbrrcv_sub_code, gst_rcv_buff.data_sub_tr_code, 2);

        if(strncmp(gc_tconfig_comp_id,"996",3) == 0)
            strncpy(gst_ftsrcv_rec.kmbrrcv_system_type, "3", 1);
        else
            strncpy(gst_ftsrcv_rec.kmbrrcv_system_type, "1", 1);

        strncpy(gst_ftsrcv_rec.kmbrrcv_mbr_no, gc_tconfig_comp_id, 3);
        strncpy(gst_ftsrcv_rec.kmbrrcv_mbr_seq_no, &gst_rcv_buff.data_seq[2], 6);
        strncpy(gst_ftsrcv_rec.kmbrrcv_rjt_code, "000", 3);
        strncpy(gst_ftsrcv_rec.kmbrrcv_rcv_hhmmss, gc_rcv_time, 6);
        strncpy(gst_ftsrcv_rec.mbr_no, &gst_rcv_buff.rcv_data[0], 3);
        strncpy(gst_ftsrcv_rec.fs_seq_no, &gst_rcv_buff.rcv_data[3], 11);
        strncpy(gst_ftsrcv_rec.fs_yymmdd, &gst_rcv_buff.rcv_data[14], 8);
        strncpy(gst_ftsrcv_rec.app_yymmdd, &gst_rcv_buff.rcv_data[22], 8);
        strncpy(gst_ftsrcv_rec.jeobsu_gubun, &gst_rcv_buff.rcv_data[30], 2);
        memcpy (gst_ftsrcv_rec.name_tgt, &gst_rcv_buff.rcv_data[32], 30);

        memset(code_tgt, 0x00, sizeof(code_tgt));
        memset(lc_enc_code, 0x00, sizeof(lc_enc_code));
        memcpy(code_tgt, &gst_rcv_buff.rcv_data[62], 13);

        li_error = EncData((unsigned char*)code_tgt, 13, &encount1, &encoutlen);
        if(li_error != NO_ERROR)
        {
               memset(gc_display_message, 0x00, sizeof(gc_display_message));
               sprintf(gc_display_message, "K1 code_tgt error..%d(in message_set)", li_error);
               t1160_write_log(LOG_TYPE_NORMAL);
               t9070_stop_process(FAIL);
        }
        else
            memcpy(lc_enc_code,(const char *)encount1, 16);

        memcpy (gst_ftsrcv_rec.code_tgt, lc_enc_code, 16);
        strncpy(gst_ftsrcv_rec.accnt_cnt, &gst_rcv_buff.rcv_data[75], 2);
        strncpy(gst_ftsrcv_rec.jango_gubun, &gst_rcv_buff.rcv_data[77], 1);
        strncpy(gst_ftsrcv_rec.debt_amt, &gst_rcv_buff.rcv_data[78], 18);
        strncpy(gst_ftsrcv_rec.debt_not_decided, &gst_rcv_buff.rcv_data[96], 1);
        strncpy(gst_ftsrcv_rec.filler, &gst_rcv_buff.rcv_data[97], 377);

         CC = POSITION(gs_open_data_file_num, ll_record_spec);
        if(_status_ne(CC))
        {
            FILE_GETINFO_(gs_open_data_file_num, &file_info);
            memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "K1 file write Position error code = %d", file_info);
            t1160_write_log(LOG_TYPE_NORMAL);
            ABORTTRANSACTION();
            t9070_stop_process(FAIL);
        }

        CC = WRITEX(gs_open_data_file_num, (char*)&gst_ftsrcv_rec, 500, &ls_write_count);
        if(_status_ne(CC))
        {
            FILE_GETINFO_(gs_open_data_file_num, &file_info);
            memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "K1 file write error code = %d", file_info);
            t1160_write_log(LOG_TYPE_NORMAL);
            ABORTTRANSACTION();
            t9070_stop_process(FAIL);
        }
        else
        {
            memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "K1 file write..%d", ls_write_count);
            t1160_write_log(LOG_TYPE_NORMAL);
        }
    }
}

/* 해당 SEQ 파일 Update */
/* send message type만 */
/*-----------------------------------------------*/
void t1151_update_cnt_status(short msg_type)
/*-----------------------------------------------*/
{
	short file_info = 0, ls_tmf_status = 0;
	char lc_rcv_err_code[3];
	_cc_status CC; /* condition variable */

	memset((char*)&gst_bond_cnt_rec, 0x00, sizeof(gst_bond_cnt_rec));

	if((msg_type == MSG_STRT) || (msg_type == MSG_ERR))
	{
		ls_tmf_status = BEGINTRANSACTION();
		if(ls_tmf_status != 0)
		{
			switch(msg_type)
			{
				case MSG_STRT :
					sprintf(gc_display_message, "Cnt_Strt Update BEGINTRANSACTION error..%d", ls_tmf_status);
					break;
				case MSG_ERR :
					sprintf(gc_display_message, "Cnt_Err Update BEGINTRANSACTION error..%d", ls_tmf_status);
					break;
			}		
			
			memset(gc_display_message, 0x00, sizeof(gc_display_message));
			t1160_write_log(LOG_TYPE_NORMAL);
			t9070_stop_process(FAIL);
		}
	}

	/* count file Key Position */
	KEYPOSITIONX(gs_open_cnt_file_num, gc_cnt_key, NULL, 5, exact);

	/* read count file */
	CC = READUPDATELOCKX(gs_open_cnt_file_num, (char*)&gst_bond_cnt_rec, sizeof(gst_bond_cnt_rec));

	if(_status_ne(CC))
	{
		FILE_GETINFO_(gs_open_cnt_file_num, &file_info);
		memset(gc_display_message, 0x00, sizeof(gc_display_message));

		switch(msg_type)
		{
			case MSG_STRT :
				sprintf(gc_display_message, "Cnt_Strt READUPDATELOCKX error code = %d", file_info);
				break;
			case MSG_ERR :
				sprintf(gc_display_message, "Cnt_Err READUPDATELOCKX error code = %d", file_info);
				break;
			default :
				sprintf(gc_display_message, "Update count file READUPDATELOCKX error code = %d", file_info);
		}

		t1160_write_log(LOG_TYPE_NORMAL);
		ABORTTRANSACTION();
		t9070_stop_process(FAIL);
	}

	/* 변경 내용 Set */
	strncpy(gst_bond_cnt_rec.gc_date, gc_today_date, 8); /* 오늘 날짜 Set */
	strncpy(gst_bond_cnt_rec.gc_time, gc_today_time, 6); /* 현재 시간 Set */

	switch(msg_type)
	{
		case MSG_STRT :
			strncpy(gst_bond_cnt_rec.gc_restart_time, gc_today_time, 6); /* 재시작 시간 Set */
			strncpy(gst_bond_cnt_rec.gc_proc_status, "1", 1); /* "1"은 정상 기동 Set */
			break;
		case MSG_ERR :
			memset(lc_rcv_err_code, 0x00, sizeof(lc_rcv_err_code));
			NUMOUT(lc_rcv_err_code, gs_rcv_err_code, 10, 2); /* 숫자를 문자로 전환 */
			strncpy(gst_bond_cnt_rec.gc_error_num, lc_rcv_err_code, 2); /* error num Set */
			break;
		default :
            strncpy(gst_bond_cnt_rec.gc_error_num, "00", 2); /* error num Set */

			switch(msg_type)
			{
				case MSG_0810_001 :
					strncpy(gst_bond_cnt_rec.gc_proc_status, "1", 1);  /* 비정상 Stop 후 재기동 시 반영 */
					strncpy(gst_bond_cnt_rec.gc_link_status, "01", 2); /* 01은 link 됨 */
					strncpy(gst_bond_cnt_rec.gc_link_time, gc_today_time, 6); /* link 시간 기재 */
					strncpy(gst_bond_cnt_rec.gc_complete_yn, "0", 1); /* 0 : 사용중, 1 : 사용완료 */

					gs_link_status = 1;

					memset(gc_display_message, 0x00, sizeof(gc_display_message));
					sprintf(gc_display_message, "Link Complete..");
					t1160_write_log(LOG_TYPE_NORMAL);
					t1154_write_msg_rec(MSG_LINK);
					break;

				case MSG_0800_301 :
					break;

				case MSG_0810_301 :
					break;

				case MSG_0800_040 :
					strncpy(gst_bond_cnt_rec.gc_link_status, "02", 2); /* 02는 link 종료 송신 */
					
					gs_link_status = 2;
					break;

				case MSG_0810_040 :
					strncpy(gst_bond_cnt_rec.gc_link_status, "03", 2); /* 03은 link 종료 수신 */

					gs_link_status = 3;
					break;

				case MSG_0200_000 :
				case MSG_0210_000 :
					gl_last_data_count = gl_last_data_count + 1L;
					gl_next_data_count = gl_last_data_count + 1L;

					memset(gc_last_data_count, 0x00, sizeof(gc_last_data_count));
					memset(gc_next_data_count, 0x00, sizeof(gc_next_data_count));

					DNUMOUT(gc_last_data_count, gl_last_data_count, 10, 8); /* 숫자를 문자로 변환 */
					DNUMOUT(gc_next_data_count, gl_next_data_count, 10, 8); /* 숫자를 문자로 변환 */

					strncpy(gst_bond_cnt_rec.gc_data_count, gc_last_data_count, 8); /* 일련번호 세팅 */
					break;

				default :
					memset(gc_display_message, 0x00, sizeof(gc_display_message));
					sprintf(gc_display_message, "Invalid message type..%d (in message_set)", msg_type);
					t1160_write_log(LOG_TYPE_NORMAL);
					t9070_stop_process(FAIL);
			}
					break;
	}

	/* writeupdate count file */
	CC = WRITEUPDATEUNLOCKX(gs_open_cnt_file_num, (char*)&gst_bond_cnt_rec, sizeof(gst_bond_cnt_rec));

	if(_status_ne(CC))
	{
		FILE_GETINFO_(gs_open_cnt_file_num, &file_info);
		memset(gc_display_message, 0x00, sizeof(gc_display_message));

		switch(msg_type)
		{
			case MSG_STRT :
				sprintf(gc_display_message, "Cnt_Strt WRITEUPDATEUNLOCKX error code = %d", file_info);
				break;
			case MSG_ERR :
				sprintf(gc_display_message, "Cnt_Err WRITEUPDATEUNLOCKX error code = %d", file_info);
				break;
			default :
				sprintf(gc_display_message, "Update count file writeupdate error code = %d", file_info);
				break;
		}

		t1160_write_log(LOG_TYPE_NORMAL);
		ABORTTRANSACTION();
		t9070_stop_process(FAIL);
	}

	if((msg_type == MSG_STRT) || (msg_type == MSG_ERR))
	{
		ls_tmf_status = ENDTRANSACTION();
		if(ls_tmf_status != 0)
		{
			memset(gc_display_message, 0x00, sizeof(gc_display_message));

			switch(msg_type)
			{
				case MSG_STRT :
					sprintf(gc_display_message, "Cnt_Strt ENDTRANSACTION error..%d", ls_tmf_status);
					break;
				case MSG_ERR :
					sprintf(gc_display_message, "Cnt_Err ENDTRANSACTION err..%d", ls_tmf_status);
					break;
			}
			
			t1160_write_log(LOG_TYPE_NORMAL);
			t9070_stop_process(FAIL);
		}
	}
}

/* "gc_display_message" Buffer의 내용을 Msg File에 Write */
/*--------------------------------------------------------*/
void t1154_write_msg_rec(short msg_type)
/*--------------------------------------------------------*/
{
    short file_info = 0, ls_tmf_status = 0;
    unsigned short ls_write_count = 0;
    long ll_record_spec = -1L; /* EOF position */
    char lc_gubun[3], lc_bell[2];
    _cc_status CC; /* condition variable */

    memset(lc_gubun, 0x00, sizeof(lc_gubun));
    memset(lc_bell, 0x00, sizeof(lc_bell));

    switch(msg_type)
    {
        case MSG_STRT :
            strncpy(lc_gubun, "01", 2);
            strncpy(lc_bell, "0", 1);
            break;
        case MSG_NORMAL_END :
            strncpy(lc_gubun, "01", 2);
            strncpy(lc_bell, "0", 1);
            break;
        case MSG_ABNORMAL_END :
            strncpy(lc_gubun, "02", 2);
            strncpy(lc_bell, "1", 1);
            break;
        case MSG_LINK :
            strncpy(lc_gubun, "01", 2);
            strncpy(lc_bell, "0", 1);
            break;
        case MSG_RE_LINK :
            strncpy(lc_gubun, "02", 2);
            strncpy(lc_bell, "1", 1);
            break;
        case MSG_RETRY :
            strncpy(lc_gubun, "02", 2);
            strncpy(lc_bell, "1", 1);
            break;
        case MSG_RECV_ERR :
            strncpy(lc_gubun, "02", 2);
            strncpy(lc_bell, "1", 1);
            break;
        default :
            memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "MSG Invalid message type..%d (in message_set)", msg_type);
            t1160_write_log(LOG_TYPE_NORMAL);
            t9070_stop_process(FAIL);
    }

    memset((char*)&gst_msg_rec, ' ', sizeof(gst_msg_rec));

    strncpy(gst_msg_rec.process_id, gc_tconfig_proc_name, 6);
    strncpy(gst_msg_rec.sigak, gc_today_time, 6);
    strncpy(gst_msg_rec.gubun, lc_gubun, 2);
    strncpy(gst_msg_rec.error_code, "01", 2);
    strncpy(gst_msg_rec.message, gc_display_message, 60);
    strncpy(gst_msg_rec.message_len, "60", 2);
    strncpy(gst_msg_rec.debug_ok, "0", 1);
    strncpy(gst_msg_rec.bell_ok, lc_bell, 1);
    strncpy(gst_msg_rec.on_batch, "1", 1);
    strncpy(gst_msg_rec.group_no, "00", 2);
    strncpy(gst_msg_rec.disp_cnt, "01", 2);
    gst_msg_rec.filler[61] = NULL;

    if(msg_type != MSG_LINK)
    {
        ls_tmf_status = BEGINTRANSACTION();
        if(ls_tmf_status != 0)
        {
            memset(gc_display_message,0x00,sizeof(gc_display_message));
            sprintf(gc_display_message,"MSG file BEGTRAN error..%d",ls_tmf_status);
            t1160_write_log(LOG_TYPE_NORMAL);
        }
    }

    CC = POSITION(gs_open_msg_file_num, ll_record_spec);
    if(_status_ne(CC))
    {
        FILE_GETINFO_(gs_open_msg_file_num, &file_info);
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message,"MSG file write Posiotion error code = %d", file_info);
        t1160_write_log(LOG_TYPE_NORMAL);
        ABORTTRANSACTION();
    }

    if(strncmp(gc_tconfig_tr_code, "K", 1) == 0)
        CC = WRITEX(gs_open_msg_file_num, (char *)&gst_msg_rec, 150, &ls_write_count);
    else
        CC = WRITEX(gs_open_msg_file_num, (char *)&gst_msg_rec, 100, &ls_write_count);

    if(_status_ne(CC))
    {
        FILE_GETINFO_(gs_open_msg_file_num, &file_info);
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message,"MSG file write error code = %d", file_info);
        t1160_write_log(LOG_TYPE_NORMAL);
        ABORTTRANSACTION();
    }

    if(msg_type != MSG_LINK)
    {
        ls_tmf_status = ENDTRANSACTION();
        if(ls_tmf_status != 0)
        {
            memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "MSG file ENDTRAN error..%d", ls_tmf_status);
            t1160_write_log(LOG_TYPE_NORMAL);
        }
    }
}

/* "gc_display_message" Buffer의 내용을 Log File에 Write */
/*--------------------------------------------------------*/
void t1160_write_log(short ls_log_gubun)
/*--------------------------------------------------------*/
{
    char lc_display_message[150];
    int li_result = -1;

    t9020_get_time();

    memset(lc_display_message, 0x00, sizeof(lc_display_message));

    switch(ls_log_gubun)
    {
        case LOG_TYPE_NORMAL :
            sprintf(lc_display_message, "[%6.6hs-%8.8hs] %s", gc_tconfig_proc_name, gc_edit_time, gc_display_message);
            lc_display_message[78] = NULL;
            break;

        case LOG_TYPE_RECV :
          sprintf(lc_display_message,"[%6.6hs-%8.8hs] RECV %4.4hs %1.1hs %3.3hs %4.4hs %3.3hs %2.2hs %12.12hs %2.2hs %8.8hs %2.2hs.%d"
                      ,gc_tconfig_proc_name
                      ,gc_edit_time
                      ,gst_rcv_buff.message_length
                      ,gst_rcv_buff.tr_code
                      ,gst_rcv_buff.gigwan_id
                      ,gst_rcv_buff.msg_type
                      ,gst_rcv_buff.opr_type
                      ,gst_rcv_buff.err_code
                      ,gst_rcv_buff.time
                      ,gst_rcv_buff.retry_cnt
                      ,gst_rcv_buff.data_no
                      ,gst_rcv_buff.data_cnt
                      ,gi_read_message_length
                      );
          lc_display_message[78] = NULL ;
          break;

        case LOG_TYPE_SEND :
          sprintf(lc_display_message,"[%6.6hs-%8.8hs] SEND %4.4hs %1.1hs %3.3hs %4.4hs %3.3hs %2.2hs %12.12hs %2.2hs %8.8hs %2.2hs.%d"
                      ,gc_tconfig_proc_name
                      ,gc_edit_time
                      ,gst_snd_buff.message_length
                      ,gst_snd_buff.tr_code
                      ,gst_snd_buff.gigwan_id
                      ,gst_snd_buff.msg_type
                      ,gst_snd_buff.opr_type
                      ,gst_snd_buff.err_code
                      ,gst_snd_buff.time
                      ,gst_snd_buff.retry_cnt
                      ,gst_snd_buff.data_no
                      ,gst_snd_buff.data_cnt
                      ,gi_send_message_length
                      );
          lc_display_message[78] = NULL ;
          break;

        default : 
          sprintf(lc_display_message,"[%6.6hs-%8.8hs] Invalid Log Type..%d ",gc_tconfig_proc_name,gc_edit_time,ls_log_gubun);
          lc_display_message[78] = NULL ;
    }

    /* display to Monitor, log file */
    /* printf("%s\n",lc_display_message); */
    li_result = WRITEX(gs_open_log_file_num, (char *)&lc_display_message, (short)strlen(lc_display_message));
    if(li_result != 0)
    {
        memset(lc_display_message, 0x00, sizeof(lc_display_message));
        sprintf(lc_display_message,"[%6.6hs-%8.8hs] LOG FILE ERROR \n",gc_tconfig_proc_name,gc_edit_time);
        lc_display_message[78] = NULL ;
        printf("%s",lc_display_message);
        PROCESS_STOP_(,,save_abend);
    }
}


/* 시스템 날짜 가져오기 */
/*-------------------------------------------------------------------------*/
void t9010_get_date()
/*-------------------------------------------------------------------------*/
{
	time_t now;
	struct tm *tm_now;

	now = time(NULL);
	tm_now = localtime(&now);
  
	memset(gc_today_date, 0x00, sizeof(gc_today_date));
	memset(gc_edit_date, 0x00, sizeof(gc_edit_date));

	sprintf(gc_today_date, "%04d%02d%02d", 1900 + tm_now->tm_year, 1 + tm_now->tm_mon, tm_now->tm_mday);
	sprintf(gc_edit_date,"%04d/%02d/%02d", 1900 + tm_now->tm_year, 1 + tm_now->tm_mon, tm_now->tm_mday);
}

/* 시스템 시간 가져오기 */
/*-------------------------------------------------------------------------*/
void t9020_get_time()
/*-------------------------------------------------------------------------*/
{
	time_t now;
	struct tm *tm_now;

	now = time(NULL);
	tm_now = localtime(&now);

	memset(gc_today_time,0x00,sizeof(gc_today_time));
	memset(gc_edit_time ,0x00,sizeof(gc_edit_time));

	sprintf(gc_today_time, "%02d%02d%02d", tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
	sprintf(gc_edit_time, "%02d:%02d:%02d", tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
}

/* TCONFIG에서 정보 가져오기 */
/*-------------------------------------------------------------------------*/
void t9030_get_config()
/*-------------------------------------------------------------------------*/
{
	short file_info = 0;
	int file_name_length;
	char lc_len[3];
	short ls_numin_status = 0;

	_cc_status CC; /* condition variable */

	/* "TCONFIG" file open */
	file_name_length = (int)strlen(gc_tconfig_file_name);
	CC = FILE_OPEN_(gc_tconfig_file_name, (short)file_name_length, &gs_tconfig_file_num, read_only);
	if(_status_ne(CC))
	{
		FILE_GETINFO_(gs_tconfig_file_num, &file_info);
		printf("TCONFIG file open error. error code = %d \n", file_info);
		PROCESS_STOP_(,, save_abend);
	}

	/* Key position */
    /* TOCINFG file에서 실행 시 주어진 변수로 원하는 레코드 위치로 이동 ex) "001L1" */
	CC = KEYPOSITIONX(gs_tconfig_file_num, gc_tconfig_key, NULL, 5, exact);
	if(_status_ne(CC))
	{
		FILE_GETINFO_(gs_tconfig_file_num, &file_info);
		printf("TCONFIG Key positioning error. error code = %d \n", file_info);
		FILE_CLOSE_(gs_tconfig_file_num);
		PROCESS_STOP_(,, save_abend);
	}

	/* Read */
    /* TCONFIG file에서 TCONFIG 변수 구조체로 레코드 읽음. */
	memset((char*)&gst_tconfig_rec, 0x00, sizeof(gst_tconfig_rec));
	CC = READX(gs_tconfig_file_num, (char*)&gst_tconfig_rec, sizeof(gst_tconfig_rec));
	if(_status_ne(CC))
	{
		FILE_GETINFO_(gs_tconfig_file_num, &file_info);
		printf("TCONFIG file read error. error code = %d \n", file_info);
		FILE_CLOSE_(gs_tconfig_file_num);
		PROCESS_STOP_(,, save_abend);
	}

     /* TCONFIG 변수 구조체에서 전역변수로 복사 : 필요한 부분만 파싱 위함. */
     /* 1. 회원사 번호 */
     memset (gc_tconfig_comp_id, 0x00, sizeof(gc_tconfig_comp_id));
     strncpy(gc_tconfig_comp_id, &gst_tconfig_rec.gc_line_id[0], 3);

     /* 2. TR Code */
     memset (gc_tconfig_tr_code, 0x00, sizeof(gc_tconfig_tr_code));
     strncpy(gc_tconfig_tr_code, &gst_tconfig_rec.gc_line_id[3], 2);
     printf("TR CODE Type..%s \n", gc_tconfig_tr_code);

     /* 3. Communication type set (수신 : R, 송신 : S) */
     memset (gc_tconfig_comm_type, 0x00, sizeof(gc_tconfig_comm_type));
     strncpy(gc_tconfig_comm_type, gst_tconfig_rec.gc_comm_type, 1);
     printf("Communication Type.. %s \n",gc_tconfig_comm_type);

     /* 4-1. Process name length set */
     memset(lc_len, 0x00, sizeof(lc_len));
     strncpy(lc_len, gst_tconfig_rec.gc_proc_name, 2);
     NUMIN(lc_len, &gs_tconfig_proc_name_len, 10, &ls_numin_status);
     if(ls_numin_status != 0)
     {
          printf("Tconfig Process name length Numin error : %s \n",lc_len);
          PROCESS_STOP_(,,save_abend);
     }

     /* 4-2. Process name set ($XnnnX) */
     memset(gc_tconfig_proc_name, 0x00, sizeof(gc_tconfig_proc_name));
     strncpy(gc_tconfig_proc_name, &gst_tconfig_rec.gc_proc_name[3], gs_tconfig_proc_name_len);

     /* 5-1. Object Name length set */
     memset(lc_len, 0x00, sizeof(lc_len));
     strncpy(lc_len, gst_tconfig_rec.gc_obj_name, 2);
     NUMIN(lc_len, &gs_tconfig_obj_name_len, 10, &ls_numin_status);
     if(ls_numin_status != 0)
     {
          printf("Tconfig Object Name length Numin error : %s \n",lc_len);
          PROCESS_STOP_(,,save_abend);
     }

     /* 5-2. Object Name set (Program Path & Name) */
     memset(gc_tconfig_obj_name, 0x00, sizeof(gc_tconfig_obj_name));
     strncpy(gc_tconfig_obj_name, &gst_tconfig_rec.gc_obj_name[3], gs_tconfig_obj_name_len);

     /* 6-1. Port Num length set */
     memset(lc_len, 0x00, sizeof(lc_len));
     strncpy(lc_len, gst_tconfig_rec.gc_port_num, 2);
     NUMIN(lc_len, &gs_tconfig_port_num_len, 10, &ls_numin_status);
     if(ls_numin_status != 0)
     {
          printf("Tconfig Port Num length Numin error : %s \n",lc_len);
          PROCESS_STOP_(,,save_abend);
     }

     /* 6-2. Port Num set */
     strncpy(gc_tconfig_port_num, &gst_tconfig_rec.gc_port_num[3], gs_tconfig_port_num_len);
     gs_tconfig_port_num = (unsigned short) atoi(gc_tconfig_port_num);

     /* 7-1. IP Num length set */
     memset(lc_len, 0x00, sizeof(lc_len));
     strncpy(lc_len, gst_tconfig_rec.gc_ip_num, 2);
     NUMIN(lc_len, &gs_tconfig_ip_num_len, 10, &ls_numin_status);
     if(ls_numin_status != 0)
     {
          printf("Tconfig IP Num length Numin error : %s \n",lc_len);
          PROCESS_STOP_(,,save_abend);
     }

     /* 7-2. IP Num set */
     memset(gc_tconfig_ip_num, 0x00, sizeof(gc_tconfig_ip_num));
     strncpy(gc_tconfig_ip_num, &gst_tconfig_rec.gc_ip_num[3], gs_tconfig_ip_num_len);

     /* 8-1. TCP PROC Name length set */
     memset(lc_len, 0x00, sizeof(lc_len));
     strncpy(lc_len, gst_tconfig_rec.gc_tcp_proc_name, 2);
     NUMIN(lc_len, &gs_tconfig_tcp_proc_name_len, 10, &ls_numin_status);
     if(ls_numin_status != 0)
     {
          printf("Tconfig TCP PROC NAME length Numin error : %s \n",lc_len);
          PROCESS_STOP_(,,save_abend);
     }

     /* 8-2. TCP PROC Name set ($ZB26D) */
     memset(gc_tconfig_tcp_proc_name, 0x00, sizeof(gc_tconfig_tcp_proc_name));
     strncpy(gc_tconfig_tcp_proc_name, &gst_tconfig_rec.gc_tcp_proc_name[3], gs_tconfig_tcp_proc_name_len);

     /* 9-1. DATA File Name length set */
     memset(lc_len, 0x00, sizeof(lc_len));
     strncpy(lc_len, gst_tconfig_rec.gc_data_file_name, 2);
     NUMIN(lc_len, &gs_tconfig_data_file_name_len, 10, &ls_numin_status);
     if(ls_numin_status != 0)
     {
          printf("Tconfig DATA FILE NAME length Numin error : %s \n",lc_len);
          PROCESS_STOP_(,,save_abend);
     }

     /* 9-2. DATA File Name set (수, 송신 파일) */
     memset(gc_tconfig_data_file_name, 0x00, sizeof(gc_tconfig_data_file_name));
     strncpy(gc_tconfig_data_file_name, &gst_tconfig_rec.gc_data_file_name[3], gs_tconfig_data_file_name_len);

     /* 10-1. LOG File Name length set */
     memset(lc_len, 0x00, sizeof(lc_len));
     strncpy(lc_len, gst_tconfig_rec.gc_log_file_name, 2);
     NUMIN(lc_len, &gs_tconfig_log_file_name_len, 10, &ls_numin_status);
     if(ls_numin_status != 0)
     {
          printf("Tconfig LOG File Name length Numin error : %s \n",lc_len);
          PROCESS_STOP_(,,save_abend);
     }

     /* 10-2. LOG File Name set */
     memset(gc_tconfig_log_file_name, 0x00, sizeof(gc_tconfig_log_file_name));
     strncpy(gc_tconfig_log_file_name, &gst_tconfig_rec.gc_log_file_name[3], gs_tconfig_log_file_name_len);

     /* 11-1. CNT File Name length set */
     memset(lc_len, 0x00, sizeof(lc_len));
     strncpy(lc_len, gst_tconfig_rec.gc_cnt_file_name, 2);
     NUMIN(lc_len, &gs_tconfig_cnt_file_name_len, 10, &ls_numin_status);
     if(ls_numin_status != 0)
     {
          printf("Tconfig CNT File Name length Numin error : %s \n",lc_len);
          PROCESS_STOP_(,,save_abend);
     }

     /* 11-2. CNT File Name set (SEQ 파일) */
     memset(gc_tconfig_cnt_file_name, 0x00, sizeof(gc_tconfig_cnt_file_name));
     strncpy(gc_tconfig_cnt_file_name, &gst_tconfig_rec.gc_cnt_file_name[3], gs_tconfig_cnt_file_name_len);

     /* 12-1. JANG File Name length set */
     memset(lc_len, 0x00, sizeof(lc_len));
     strncpy(lc_len, gst_tconfig_rec.gc_jang_file_name, 2);
     NUMIN(lc_len, &gs_tconfig_jang_file_name_len, 10, &ls_numin_status);
     if(ls_numin_status != 0)
     {
          printf("Tconfig JANG File Name length Numin error : %s \n",lc_len);
          PROCESS_STOP_(,,save_abend);
     }

     /* 12-2. JANG File Name set */
     memset(gc_tconfig_jang_file_name, 0x00, sizeof(gc_tconfig_jang_file_name));
     strncpy(gc_tconfig_jang_file_name, &gst_tconfig_rec.gc_jang_file_name[3], gs_tconfig_jang_file_name_len);

     /* 13. 업무 구분 번호 set */
     memset(gc_tconfig_cnt_gubun, 0x00, sizeof(gc_tconfig_cnt_gubun));
     strncpy(gc_tconfig_cnt_gubun, gst_tconfig_rec.gc_cnt_gubun, 2);

     /* 14. Message Length */
     memset (gc_tconfig_message_length, 0x00, sizeof(gc_tconfig_message_length));
     strncpy(gc_tconfig_message_length, gst_tconfig_rec.gc_message_length, 4);

     NUMIN(gc_tconfig_message_length, &gs_tconfig_message_length, 10, &ls_numin_status);
     if(ls_numin_status != 0)
     {
          printf("Tconfig Message Length Numin error : %s \n",lc_len);
          PROCESS_STOP_(,,save_abend);
     }

     /* 15-1. MSG File Name length set */
     memset(lc_len, 0x00, sizeof(lc_len));
     strncpy(lc_len, gst_tconfig_rec.gc_msg_file_name, 2);
     NUMIN(lc_len, &gs_tconfig_msg_file_name_len, 10, &ls_numin_status);
     if(ls_numin_status != 0)
     {
          printf("Tconfig MSG File Name length Numin error : %s \n",lc_len);
          PROCESS_STOP_(,,save_abend);
     }

     /* 15-2. MSG File Name set */
     memset (gc_tconfig_msg_file_name, 0x00, sizeof(gc_tconfig_msg_file_name));
     strncpy(gc_tconfig_msg_file_name, &gst_tconfig_rec.gc_msg_file_name[3], gs_tconfig_msg_file_name_len);


     /* 16. HEADER TR_CODE Set */
     memset(TR_CODE, 0x00, sizeof(TR_CODE));
     strncpy(TR_CODE, "N        ", 9);

     FILE_CLOSE_(gs_tconfig_file_num);
}

/* TCONFIG에서 가져 온 정보로 관련파일들 Open */
/*-------------------------------------------------------------------------*/
void t9040_open_file()
/*-------------------------------------------------------------------------*/
{
    int li_result = 0;
    short ls_last_err = 0;

    /* 0. LOG File Open */
    li_result = FILE_OPEN_(gc_tconfig_log_file_name, (short)strlen(gc_tconfig_log_file_name), &gs_open_log_file_num, read_write, shared_access);
    if(li_result != 0)
    {
        FILE_GETINFO_(gs_open_log_file_num, &ls_last_err);
        printf("Log File OPEN Error(%d) \n", ls_last_err);
        t9070_stop_process(FAIL);
    }
    else
    {
		/* Start LOG */
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "====================================================");
        t1160_write_log(LOG_TYPE_NORMAL);
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "Interface PROGRAM                                   ");
        t1160_write_log(LOG_TYPE_NORMAL);
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "====================================================");
        t1160_write_log(LOG_TYPE_NORMAL);

        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "Log File OPEN(%s)..%d", gc_tconfig_log_file_name, gs_open_log_file_num);
        t1160_write_log(LOG_TYPE_NORMAL);
    }

    /* 1. MSG File Open */
    li_result = 0;
    ls_last_err = 0;
    li_result = FILE_OPEN_(gc_tconfig_msg_file_name, (short)strlen(gc_tconfig_msg_file_name), &gs_open_msg_file_num, read_write, shared_access);
    if(li_result != 0)
    {
        FILE_GETINFO_(gs_open_msg_file_num, &ls_last_err);
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "MSG File OPEN Error(%d)", ls_last_err);
        t1160_write_log(LOG_TYPE_NORMAL);
        t9070_stop_process(FAIL);
    }
    else
    {
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "MSG File OPEN(%s)..%d", gc_tconfig_msg_file_name, gs_open_msg_file_num);
        t1160_write_log(LOG_TYPE_NORMAL);
    }

	/* Start MSG */
    memset(gc_display_message, 0x00, sizeof(gc_display_message));
    sprintf(gc_display_message, "Interface PROGRAM STARTING                          ");
    t1160_write_log(LOG_TYPE_NORMAL);
    t1154_write_msg_rec(MSG_STRT);

    /* 2. DATA File(rcv, snd, jangsd..) Open */
    li_result = 0;
    ls_last_err = 0;
    li_result = FILE_OPEN_(gc_tconfig_data_file_name, (short)strlen(gc_tconfig_data_file_name), &gs_open_data_file_num, read_write, shared_access);
    if(li_result != 0)
    {
        FILE_GETINFO_(gs_open_data_file_num, &ls_last_err);
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "DATA File OPEN Error(%d)..%s", ls_last_err, gc_tconfig_data_file_name);
        t1160_write_log(LOG_TYPE_NORMAL);
        t9070_stop_process(FAIL);
    }
    else
    {
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "Data File OPEN(%s)..%d", gc_tconfig_data_file_name, gs_open_data_file_num);
        t1160_write_log(LOG_TYPE_NORMAL);
    }

    /* 2.1 DATA File(rcv, snd, jangsd,..) Open Control27용 */
    li_result = 0;
    ls_last_err = 0;
    li_result = FILE_OPEN_(gc_tconfig_data_file_name, (short)strlen(gc_tconfig_data_file_name), &gs_open_data_file_num_27, read_only, shared_access, 1);
    if(li_result != 0)
    {
        FILE_GETINFO_(gs_open_data_file_num, &ls_last_err);
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "DATA File OPEN(control27) Error(%d)..%s", ls_last_err, gc_tconfig_data_file_name);
        t1160_write_log(LOG_TYPE_NORMAL);
        t9070_stop_process(FAIL);
    }
    else
    {
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "Data File OPEN(control27)(%s)..%d", gc_tconfig_data_file_name, gs_open_data_file_num_27);
        t1160_write_log(LOG_TYPE_NORMAL);
    }

    /* 3. CNT File Open */
    li_result = 0;
    ls_last_err = 0;
    li_result = FILE_OPEN_(gc_tconfig_cnt_file_name, (short)strlen(gc_tconfig_cnt_file_name), &gs_open_cnt_file_num, read_write, shared_access);
    if(li_result != 0)
    {
        FILE_GETINFO_(gs_open_cnt_file_num, &ls_last_err);
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "CNT File OPEN Error(%d)..%s", ls_last_err, gc_tconfig_cnt_file_name);
        t1160_write_log(LOG_TYPE_NORMAL);
        t9070_stop_process(FAIL);
    }
    else
    {
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "CNT File OPEN(%s)..%d", gc_tconfig_cnt_file_name, gs_open_cnt_file_num);
        t1160_write_log(LOG_TYPE_NORMAL);
    }

    /* 4. Jang File Open */
    li_result = 0;
    ls_last_err = 0;
    li_result = FILE_OPEN_(gc_tconfig_jang_file_name, (short)strlen(gc_tconfig_jang_file_name), &gs_open_jang_file_num, read_only, shared_access);
    if(li_result != 0)
    {
        FILE_GETINFO_(gs_open_jang_file_num, &ls_last_err);
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "JANG File OPEN Error(%d)..%s", ls_last_err, gc_tconfig_jang_file_name);
        t1160_write_log(LOG_TYPE_NORMAL);
        t9070_stop_process(FAIL);
    }
    else
    {
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "JANG File OPEN(%s)..%d", gc_tconfig_jang_file_name, gs_open_jang_file_num);
        t1160_write_log(LOG_TYPE_NORMAL);
    }
}

/* 송수신 Count(SEQ)을 읽어와서 Variable Setting */
/*-------------------------------------------------------------------------*/
void t9050_get_count_status()
/*-------------------------------------------------------------------------*/
{
    int file_info = 0;
    short ls_numin_status = 0;
    _cc_status CC; /* condition variable */

    /* Count(SEQ) File Key set */
    /* Key setting : 회사코드(3)+cnt구분(2) */
    memset(gc_cnt_key, 0x00, sizeof(gc_cnt_key));
    strncpy(gc_cnt_key, gc_tconfig_comp_id, 3);
    strncpy(&gc_cnt_key[3], gc_tconfig_cnt_gubun, 2);

    /* Count(SEQ) File Key Positioning (BSQIFSD, BSQIFRV, KSQMBSD, KSQMBRV..) */
    KEYPOSITIONX(gs_open_cnt_file_num, gc_cnt_key, NULL, 5, exact);

    /* Read Count(SEQ) File */
    /* "gst_bond_cnt_rec"에 해당 Record 읽음 */
    memset((char*)&gst_bond_cnt_rec, 0x00, sizeof(gst_bond_cnt_rec));
    CC = READX(gs_open_cnt_file_num, (char*)&gst_bond_cnt_rec, sizeof(gst_bond_cnt_rec));
    if(_status_ne(CC))
    {
        FILE_GETINFO_(gs_open_cnt_file_num, (short*)&file_info);
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "Count(SEQ) File read Error.. error code = %s", gc_tconfig_cnt_file_name);
        t1160_write_log(LOG_TYPE_NORMAL);
        t9070_stop_process(FAIL);
    }

    /* "gst_bond_cnt_rec"에 Setting한 변수에서 필요한 데이터 Setting */
    /* 1. Data Count(SEQ) 변수 Setting ("최종 수신 또는 송신 번호" -> "gl_last_data_count") */
    memset(gc_last_data_count, 0x00, sizeof(gc_last_data_count));
    strncpy(gc_last_data_count, gst_bond_cnt_rec.gc_data_count, 8);

    DNUMIN(gc_last_data_count, &gl_last_data_count, 10, &ls_numin_status); /* char -> long 변환 */
    if(ls_numin_status != 0)
    {
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "Data Count Numin Error : %s", gc_last_data_count);
        t1160_write_log(LOG_TYPE_NORMAL);
        t9070_stop_process(FAIL);
    }
    else
    {
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "Last Count : %ld", gl_last_data_count);
        t1160_write_log(LOG_TYPE_NORMAL);
    }

    gl_next_data_count = gl_last_data_count + 1L; /* 다음 Count 수 Setting */
    DNUMOUT(gc_next_data_count, gl_next_data_count, 10, 8); /* long -> char 변환 */

    /* 2. Proc status 변수 Setting ("Process 기동 여부" -> "gs_proc_status") */
    /* 0:미기동, 1:정상기동, 2:비정상STOP, 3.정상STOP */
    memset(gc_proc_status, 0x00, sizeof(gc_proc_status));
    strncpy(gc_proc_status, gst_bond_cnt_rec.gc_proc_status, 1);

    NUMIN(gc_proc_status, &gs_proc_status, 10, &ls_numin_status); /* char -> short 변환 */
    if(ls_numin_status != 0)
    {
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "Proc Status Numin error : %s", gc_proc_status);
        t1160_write_log(LOG_TYPE_NORMAL);
        t9070_stop_process(FAIL);
    }
    else
    {
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "Proc Status : %d", gs_proc_status);
        t1160_write_log(LOG_TYPE_NORMAL);
    }
    
    /* 3. Link status 변수 Setting ("Process Link 여부" -> "gs_link_status") */
    /* 9:정상, 5:기타비정상, 1:정상 기동중 "gc_link_status"로 저장. */
    memset(gc_link_status, 0x00, sizeof(gc_link_status));
    strncpy(gc_link_status, gst_bond_cnt_rec.gc_link_status, 2);

    NUMIN(gc_link_status, &gs_link_status, 10, &ls_numin_status); /* char -> short 변환 */
    if(ls_numin_status != 0)
    {
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "Link Status Numin error : %s", gc_link_status);
        t1160_write_log(LOG_TYPE_NORMAL);
        t9070_stop_process(FAIL);
    }
    else
    {
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "Link Status : %d", gs_link_status);
        t1160_write_log(LOG_TYPE_NORMAL);
    }
}

/* JANG File을 읽어와서 Variable Setting */
/*-------------------------------------------------------------------------*/
void t9060_get_jang_status()
/*-------------------------------------------------------------------------*/
{
    char lc_key[9];
    int file_info = 0;
    short ls_lastmarket_jang_status = 0;
    short ls_jang_status = 0;
    short ls_numin_status = 0;
    _cc_status CC; /* condition variable */

    /* Count(SEQ) File Key set */
    /* Key setting : 회사코드(3)+cnt구분(2) */
    memset(lc_key, 0x00, sizeof(lc_key));
    if(strncmp(gc_tconfig_tr_code, "K", 1) == 0)
        strncpy(lc_key, "K", 1);
    else
        strncpy(lc_key, "B", 1);

    /* JANG File(BMMJANG, KOPRMGT) Positioning */
    KEYPOSITIONX(gs_open_jang_file_num, lc_key, NULL, 1, exact);

    /*Read Jang File */
    /* "gst_bond_jang_rec" 또는 "gst_fts_jang_rec"에 해당 Record 읽음 */
    memset((char*)&gst_bond_jang_rec, 0x00, sizeof(gst_bond_jang_rec));
    memset((char*)&gst_fts_jang_rec, 0x00, sizeof(gst_fts_jang_rec)); 

    if(strncmp(gc_tconfig_tr_code, "K", 1) == 0)
        CC = READX(gs_open_jang_file_num, (char*)&gst_fts_jang_rec, sizeof(gst_fts_jang_rec));
    else
        CC = READX(gs_open_jang_file_num, (char*)&gst_bond_jang_rec, sizeof(gst_bond_jang_rec));
    if(_status_ne(CC))
    {
        FILE_GETINFO_(gs_open_jang_file_num, (short*)&file_info);
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "JANG File Read Error code = %d..%s", file_info,gc_tconfig_jang_file_name);
        t1160_write_log(LOG_TYPE_NORMAL);
        t9070_stop_process(FAIL);
    }

    memset(gc_jang_start_time,       0x00, sizeof(gc_jang_start_time));
    memset(gc_jang_end_time,         0x00, sizeof(gc_jang_end_time));
    memset(gc_lastmarket_start_time, 0x00, sizeof(gc_lastmarket_start_time));
    memset(gc_lastmarket_end_time,   0x00, sizeof(gc_lastmarket_end_time));
    memset(gc_jang_status,           0x00, sizeof(gc_jang_status));
    memset(gc_lastmarket_jang_status,0x00, sizeof(gc_lastmarket_jang_status));

    if(strncmp(gc_tconfig_tr_code, "K", 1) == 0)
    {
        strncpy(gc_jang_start_time       , gst_fts_jang_rec.gc_start_time             ,6);
        strncpy(gc_jang_end_time         , gst_fts_jang_rec.gc_end_time               ,6);
        strncpy(gc_lastmarket_start_time , "999999"                                   ,6);
        strncpy(gc_lastmarket_end_time   , "999999"                                   ,6);
        strncpy(gc_lastmarket_jang_status, gst_fts_jang_rec.gc_lastmarket_jang_status ,1);
        strncpy(gc_jang_status           , gst_fts_jang_rec.gc_jang_status            ,1);
    }
    else{
        strncpy(gc_jang_start_time       , gst_bond_jang_rec.gc_start_time            ,6);
        strncpy(gc_jang_end_time         , gst_bond_jang_rec.gc_end_time              ,6);
        strncpy(gc_lastmarket_start_time , gst_bond_jang_rec.gc_lastmarket_start_time ,6);
        strncpy(gc_lastmarket_end_time   , gst_bond_jang_rec.gc_lastmarket_end_time   ,6);
        strncpy(gc_lastmarket_jang_status, gst_bond_jang_rec.gc_lastmarket_jang_status,1);
        strncpy(gc_jang_status           , gst_bond_jang_rec.gc_jang_status           ,1);
    }

    NUMIN(gc_lastmarket_jang_status, &ls_lastmarket_jang_status, 10, &ls_numin_status);
    if(ls_numin_status != 0)
    {
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "Lastmarket JANG Status Numin Error : %s", gc_lastmarket_jang_status);
        t1160_write_log(LOG_TYPE_NORMAL);
        t9070_stop_process(FAIL);
    }

    NUMIN(gc_jang_status, &ls_jang_status, 10, &ls_numin_status);
    if(ls_numin_status != 0)
    {
        memset(gc_display_message, 0x00, sizeof(gc_display_message));
        sprintf(gc_display_message, "JANG Status Numin Error : %s", gc_jang_status);
        t1160_write_log(LOG_TYPE_NORMAL);
        t9070_stop_process(FAIL);
    }

    else if(strncmp(gc_tconfig_tr_code,"K",1) ==0)
    { /* 상속인 금융거래조회 JANG Set (From KOPRMGT File) */
      /* 0:접수전,1:접수중,2:종료 */
        if(ls_jang_status == 0)
            gs_jang_status = JANG_BEFORE; /* 금융거래 접수 전 */
        else if(ls_jang_status == 1)
            gs_jang_status = JANG_TODAY;  /* 금융거래 접수 중 */
        else if(ls_jang_status == 2)
            gs_jang_status = JANG_CLOSE;  /* 금융거래 접수 마감 */
        else
        {
            memset(gc_display_message, 0x00, sizeof(gc_display_message));
            sprintf(gc_display_message, "FTS JANG Status Error..%d", ls_jang_status);
            t1160_write_log(LOG_TYPE_NORMAL);
            t9070_stop_process(FAIL);
        }
    }
    else
    { /* B-TRIS JANG Set (From BMMJANG File) */
      /*(당일)0:접수전,1:접수중,9:종료 */
       if(ls_lastmarket_jang_status == 0 && ls_jang_status == 0)
           gs_jang_status = JANG_BEFORE; /* 채권공시 접수 전 */
       else if(ls_lastmarket_jang_status == 1 && ls_jang_status == 0)
           gs_jang_status = JANG_LAST;   /* 채권공시 직전 접수 중 */
       else if(ls_lastmarket_jang_status == 9 && ls_jang_status == 0)
           gs_jang_status = JANG_TODAY;  /* 채권공시 직전 마감, 당일 미접수 -> 당일접수로 간주 */
       else if(ls_lastmarket_jang_status == 9 && ls_jang_status == 1)
           gs_jang_status = JANG_TODAY;  /* 채권공시 직전 마감, 당일 접수*/
       else if(ls_lastmarket_jang_status == 9 && ls_jang_status == 9)
           gs_jang_status = JANG_CLOSE;  /* 채권공시 당일 마감*/
       else
       {
           memset(gc_display_message,0x00,sizeof(gc_display_message));
           sprintf(gc_display_message,"BOND Jang Status error..%d, %d", ls_lastmarket_jang_status, ls_jang_status);
           t1160_write_log(LOG_TYPE_NORMAL);
           t9070_stop_process(FAIL);
       }
   }
}

/* Process 정상/비정상 종료 송수신 Count(SEQ) File에 Setting */
/*-------------------------------------------------------------------------*/
void t9070_stop_process(short ls_stop_opt)
/*-------------------------------------------------------------------------*/
{
	short file_info = 0;
	_cc_status CC; /* condition variable */

	t9020_get_time();
	strncpy(gst_snd_buff.time, &gc_today_date[2], 6);
	strncpy(&gst_snd_buff.time[6], gc_today_time, 6);

	BEGINTRANSACTION();

	/* Count(SEQ) File Key Position */
	KEYPOSITIONX(gs_open_cnt_file_num, gc_cnt_key, NULL, 5, exact);

	/* Count(SEQ) File Read */
	CC = READUPDATELOCKX(gs_open_cnt_file_num, (char*)&gst_bond_cnt_rec, sizeof(gst_bond_cnt_rec));
	if(_status_ne(CC))
	{
		FILE_GETINFO_(gs_open_cnt_file_num, &file_info);
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
		sprintf(gc_display_message, "Process Stop Count File Readupdate Error = %d", file_info);
		t1160_write_log(LOG_TYPE_NORMAL);
		ABORTTRANSACTION();
	}

	/* 종료 구분 Set */
	if(ls_stop_opt == SUCCESS)
	{ /* 정상 종료 */
		strncpy(gst_bond_cnt_rec.gc_date, gc_today_date, 8);
		strncpy(gst_bond_cnt_rec.gc_time, gc_today_time, 6);
		strncpy(gst_bond_cnt_rec.gc_stop_time, gc_today_time, 6);
		strncpy(gst_bond_cnt_rec.gc_proc_status, "3", 1);
		strncpy(gst_bond_cnt_rec.gc_stop_gubun, "9", 1);
		strncpy(gst_bond_cnt_rec.gc_complete_yn, "1", 1);
	}
	else
	{ /* 비정상 종료 */
		strncpy(gst_bond_cnt_rec.gc_date, gc_today_date, 8);
		strncpy(gst_bond_cnt_rec.gc_time, gc_today_time, 6);
		strncpy(gst_bond_cnt_rec.gc_stop_time, gc_today_time, 6);
		strncpy(gst_bond_cnt_rec.gc_proc_status, "2", 1);
		strncpy(gst_bond_cnt_rec.gc_stop_gubun, "5", 1);
	}

	/* Count(SEQ) File Write */
	CC = WRITEUPDATEUNLOCKX(gs_open_cnt_file_num, (char*)&gst_bond_cnt_rec, sizeof(gst_bond_cnt_rec));
	if(_status_ne(CC))
	{
		FILE_GETINFO_(gs_open_cnt_file_num, &file_info);
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
       	sprintf(gc_display_message, "Process Stop count file writeupdate error = %d", file_info);
       	t1160_write_log(LOG_TYPE_NORMAL);
       	ABORTTRANSACTION();
	}

	ENDTRANSACTION();

	if(ls_stop_opt == SUCCESS)
	{
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
       	sprintf(gc_display_message, "Interface PROGRAM NORMAL STOP");
       	t1160_write_log(LOG_TYPE_NORMAL);
		t1154_write_msg_rec(MSG_NORMAL_END);
	}
	else
	{
		memset(gc_display_message, 0x00, sizeof(gc_display_message));
       	sprintf(gc_display_message, "Interface ABPROGRAM NORMAL STOP");
       	t1160_write_log(LOG_TYPE_NORMAL);
		t1154_write_msg_rec(MSG_ABNORMAL_END);
	}

	FILE_CLOSE_(gs_open_data_file_num);
	FILE_CLOSE_(gs_open_data_file_num_27);
	FILE_CLOSE_(gs_open_log_file_num);
	FILE_CLOSE_(gs_open_cnt_file_num);
	FILE_CLOSE_(gs_open_jang_file_num);
	FILE_CLOSE_(gs_open_msg_file_num);
	FILE_CLOSE_((short)new_socket_num);

	PROCESS_STOP_();
}
