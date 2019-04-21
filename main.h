#ifndef __MAIN_HEADER__
#define __MAIN_HEADER__

#include<iostream>
#include<ctime>
#include<cstring>
#include<fstream>
#include<cstdlib>
#include<cerrno>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/epoll.h>
#include<sys/time.h>

using namespace std;

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
#define MSG_0800_001		8001
#define MSG_0810_001		8101
#define MSG_0800_301		8031
#define MSG_0810_301		8131
#define MSG_0800_040		8040
#define MSG_0810_040		8140
#define MSG_0200_000		2000
#define MSG_0210_000		2100
#define MSG_START			11
#define MSG_ERR				99

/*----------------------------------------------------------------------------*/
/*----------------------------- 7. Message error 구분 ------------------------*/
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

#endif
