#include"main.h"

class C_data
{
	private :
		FILE *_data;
		int _data_fd;
		char _write_message[150];

	public :
		C_data()
		{
			memset(_write_message, 0x00, sizeof(_write_message));
			_data_fd = 0;
		}

		~C_data()
		{
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
				/* File Descriptor 저장 */
				_data_fd = fileno(_data);

				/* Start Data File Open Message */
				memset(_write_message, 0x00, sizeof(_write_message));
				sprintf(_write_message, "Data File OPEN(%s)..", _data_file);
				return _write_message;
			}
		}

		int F_get_data_fd()
		{
			return _data_fd;
		}
};
