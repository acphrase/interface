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
			cout << ip << endl;
			cout << *ip << endl;
			cout << port << endl;
			cout << *port << endl;
			/* 1. Socket에 할당 할 정보 Setting */
			_server_address.sin_family = AF_INET;	/* 주소 체계 : IPv4 */
			cout << _server_address.sin_family << endl;

			_server_address.sin_addr.s_addr = inet_addr(ip);	/* server ip (Network bytes 주소로 변환) */
			cout << inet_addr(ip) << endl;
			cout << _server_address.sin_addr.s_addr << endl;

			_server_address.sin_port = htons((unsigned short)atoi(port));	/* server port number */
			cout << htons((unsigned short)atoi(port)) << endl;
			cout << _server_address.sin_port << endl;

			/* 2. Create Socket */
			_server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
			if(_server_socke)
				throw "Socket Create Error..";

			/* 3. Add Option Socket */

			/* 4. Bind Socket */
			if(bind(_server_socket, (struct sockaddr*)&_server_address, sizeof(_server_address)));
			{
				cout << strerror(errno) << endl;
				throw "Socket Bind Error..";
			}

			/* 5. Listen Socket */
			if(listen(_server_socket, 2))
				throw "Socket Listen Error..";
			else
				cout << "Socket(NO = " << _server_socket << ") Listening.." << endl;

			/* 6. Accept Socket */
			_client_address_size = sizeof(_client_address);
			_client_socket = accept(_server_socket, (struct sockaddr*)&_client_address, &_client_address_size);
			if(_client_socket)
				throw "Socket Accept Error..";
		}
};
