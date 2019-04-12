#include"main.h"

class C_socket
{
	private :
		struct sockaddr_in _server_address;
		struct sockaddr_in _client_address;
		int _server_socket;
		int _client_socket;
		socklen_t _client_address_size;

	public :
		C_socket()
		{
			memset(&_server_address, 0x00, sizeof(_server_address));
			memset(&_client_address, 0x00, sizeof(_client_address));
			_server_socket = 0;
			_client_socket = 0;
			_client_address_size = 0;
		}

		void F_create_socket(char *ip, char *port)
		{
			int state;
			int _option = 0;
			int _check_option = 0;
			socklen_t _option_length;
			socklen_t _check_length;

			//cout << ip << endl;
			//cout << port << endl;

			/* 1. Socket에 할당 할 정보 Setting */
			_server_address.sin_family = AF_INET;	/* 주소 체계 : IPv4 */
			_server_address.sin_addr.s_addr = inet_addr(ip);	/* server ip (Network bytes 주소로 변환) */
			_server_address.sin_port = htons((unsigned short)atoi(port));	/* server port number */

			/* 2. Create Socket */
			/* _server_socket : descriptor(handle). */
			/* PF_INET : Protocol Family. TCP/IP 일 경우. */
			/* SOCK_STREAM : Protocol Type. TCP. */
			/* IPPROTO_TCP : 앞의 SOCK_STREAM과 중복. 0을 넣어 앞의 값과 동일함을 알려도 됨 */
			_server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
			if(_server_socket == -1)
			{
				cout << "Error : " << strerror(errno) << endl;
				throw "Socket Create Error..";
			}

			/* 3. Add Option Socket */
			_option_length = sizeof(_option);
			_option = 1;
			setsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR, (void*)&_option, _option_length);

			_check_length = sizeof(_check_option);
			state = getsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR, (void*)&_check_option, &_check_length);
			if(state)
			{
				cout << state << endl;
				cout << "Error : " << strerror(errno) << endl;
				throw "Get Socket Option Error..";
			}
			if(_check_option != 1)
			{
				cout << "Error : " << strerror(errno) << endl;
				throw "Set Socket Option Error..";
			}

			/* 4. Bind Socket */
			if(bind(_server_socket, (struct sockaddr*)&_server_address, sizeof(_server_address)) == -1)
			{
				cout << "Error : " << strerror(errno) << endl;
				throw "Socket Bind Error..";
			}

			/* 5. Listen Socket */
			if(listen(_server_socket, 2) == -1)
			{
				cout << "Error : " << strerror(errno) << endl;
				throw "Socket Listen Error..";
			}
			else
			{
				cout << "Socket(NO = " << _server_socket << ") Listening.." << endl;
			}

			/* 6. Accept Socket */
			_client_address_size = sizeof(_client_address);
			_client_socket = accept(_server_socket, (struct sockaddr*)&_client_address, &_client_address_size);
			if(_client_socket == -1)
				throw "Socket Accept Error..";
			else
				cout << "connect" << endl;
		}
};
