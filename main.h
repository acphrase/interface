#ifndef __MAIN_HEADER__
#define __MAIN_HEADER__

#include<iostream>
#include<ctime>
#include<cstring>
#include<fstream>
#include<cstdlib>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<unistd.h>

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

#endif
