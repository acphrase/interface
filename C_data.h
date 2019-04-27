#ifndef __DATA_HEADER__
#define __DATA_HEADER__

#include"common.h"

class C_data
{
	private :
		/* 1. Status Message Variable */
		char _write_message[150];

		/* 2. File Descriptor Variable */
		FILE *_data = NULL;
		int _data_fd = 0;

		/* 3. Buffer Variable */
		BGERERV_FILE_DEF _bgererv_file;
		BSOKDLRCV_FILE_DEF _bsokdlrcv_file;
		BCDRPGRCV_FILE_DEF _bcdrpgrcv_file;
		KMBRRCV_FILE_DEF _kmbrrcv_file;
		KSND_DEF _ksnd_file;
		KRJT_DEF _krjt_file;

		/* 4. EPOLL Variable */
		struct epoll_event *_ep_events;
		struct epoll_event _event;
		int _epfd;
		int _event_cnt;

		/* 5. Mutex Variable */
		mutex lock_func;

		/* 6. ETC */
		char* _data_file;	/* Data File Name */
		char* _rs_gubun;	/* 송수신 구분 */

	public :
		C_data();
		~C_data();

		/* Data File Open */
		char* F_open_data_file(char* r_data_file, char* r_rs_gubun);

		/* Check Data File Status */
		int F_check_data(int _time);

		/* Write Data To File */
		char* F_write_data(const char* r_data);

		/* Read Data File */
		char* F_read_data();

		/* Modify FD Non Blocking Mode */
		void F_set_non_blocking_mode(int _fd);
};

#endif
