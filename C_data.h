#include"main.h"

class C_data
{
	private :
		FILE *_data;
		int _data_fd;
		char _write_message[150];
		char _write_buffer[150];

	public :
		C_data()
		{
			memset(_write_message, 0x00, sizeof(_write_message));
			_data_fd = 0;
		}

		~C_data()
		{
			fclose(_data);
		}

		char* F_open_data_file(char* _data_file)
		{
			_data = fopen(_data_file, "at");
			if(_data == NULL)
			{
				memset(_write_message, 0x00, sizeof(_write_message));
				sprintf(_write_message, "Data File Open Error(%s)..", _data_file);
				cout << _write_message << endl;
				throw _write_message;
			}
			else
			{
				/* 1. File Descriptor 저장 */
				_data_fd = fileno(_data);

				/* 2. Start Data File Open Message */
				memset(_write_message, 0x00, sizeof(_write_message));
				sprintf(_write_message, "Data File OPEN(%s)..", _data_file);
				return _write_message;
			}
		}

		void F_write_data(const char* r_data)
		{
			/* 1. Write Buffer Init */
			memset(_write_data, 0x00, sizeof(_write_data));
		
			/* 2. Write Message Setting */
			sprintf(_write_data, r_data);
		
			/* 3. Write Data To File */
			_data.seekp(0, ios::end);		/* Data File의 맨 마지막에 위치 */
			_data << _write_data << endl;	/* Data Buffer에 Setting 한 Data Write */
			if(_data.bad())
			{
				throw _write_data;
			}
		}

		int F_get_data_fd()
		{
			return _data_fd;
		}
};
