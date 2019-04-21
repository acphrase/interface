#include"main.h"

class C_data
{
	private :
		FILE *_data = NULL;
		int _data_fd;
		char _write_message[150];
		mutex lock_func;

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
			/* 1. Mutual Exclosion Lock */
			lock_func.lock();

			/* 2. Write Data To File */
			if(fprintf(_data, "%s\n", r_data) == EOF);
			{
				memset(_write_message, 0x00, sizeof(_write_message));
				sprintf(_write_message, "Data File Write Error..");
				throw _write_message;
			}

			//fwrite(r_data, size_t size, size_t n, _data);
			//fputc('\n', _data);

			/* 3. Mutual Exclosion Unlock */
			lock_func.unlock();
		}

		int F_get_data_fd()
		{
			return _data_fd;
		}
};
