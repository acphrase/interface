#include"C_control.h"

int main(int argc, char *argv[])
{

	argc = 3;
	argv[1] = "999r1";
	argv[2] = "tconfig";

	/* Parameter Check */
	if(argc != 3)
	{
		cout << "Parameter 수가 맞지 않는군요." << endl;
		cout << "Usage : " << argv[0] << " <WhisaCode+TR> <Config File>" << endl;
		exit(1);
	}

	char* _communicate_type;
	char _send_gubun = SEND_GUBUN;
	C_control _control(argv);
			
	_communicate_type = _control.F_get_communicate_type();
	while(1)
    {
        /* 1. Socket Create, Bind, Listen, Accept */
        _control.F_start();

		/* 2. Recv Message */
		_control.F_read_message();
		
        /* 3. Message Check & Message Set */
		_control.F_check_message();

		if(strncasecmp(_communicate_type, &_send_gubun, 1) == 0)
		{
			/* 4. Setting Send Message Type */
			_control.F_set_send_message_type();
		}

		/* 5. Send Message */
		_control.F_send_message();
    }

	return 0;
}
