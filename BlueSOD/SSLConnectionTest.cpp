#ifdef __SSL_CONNECTION_TEST__
#define __SSL_CONNECTION_TEST__

#include <WinSock2.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <string>
#include <iostream>
#include "SSL_concurrency.h"

#define SSL_PEM_LOCATIONS "G:\\Ricky\\Documents\\Programming\\Tools\\SSLKeys\\Private\\"
#define SSL_PASSWORD_FILE_LOCATION "G:\\Ricky\\Documents\\Programming\\Tools\\SSLKeys\\Private\\"
#define CERTIFICATE_FILE "ricky-anthony.asuscomm.com.cert.pem"
#define PRIVATE_KEY_FILE "ricky-anthony.asuscomm.com.key.pem"
#define PASSWORD_FILE "password.txt"

using std::string;
using std::cout;
using std::endl;

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")

int PasswordCallBack(char* buffer, int sizeOfBuffer, int rwflag, void* data);

int main()
{
	WSAData d;
	WSAStartup(MAKEWORD(2, 2), &d);
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	u_long mode = 1;
	ioctlsocket(s, FIONBIO, &mode);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = ADDR_ANY;
	addr.sin_port = htons(2048);
	/*bind(s, (const *)&addr, sizeof(addr));
	listen(s, SOMAXCONN);
	SOCKET client = accept(s, nullptr, nullptr);*/

	thread_setup();
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	OpenSSL_add_ssl_algorithms();

	//Configure the SSL Context
	SSL_CTX* ctx = SSL_CTX_new(SSLv23_server_method());
	SSL_CTX_set_ecdh_auto(ctx, 1);
	//Path to the certificate.
	string cPath = string(SSL_PEM_LOCATIONS);
	cPath += CERTIFICATE_FILE;
	//Path to the private key.
	string pkPath = string(SSL_PEM_LOCATIONS);
	pkPath += PRIVATE_KEY_FILE;
	if (SSL_CTX_use_certificate_chain_file(ctx, cPath.c_str()) != 1)
	{
		cout << "Error loading certificate from " << cPath << endl;
		cout << "Error code: " << ERR_error_string(ERR_get_error(), nullptr) << endl;
		system("PAUSE");
		return 1;
	}
	//Tell OpenSSL where to find our password.
	SSL_CTX_set_default_passwd_cb(ctx, &PasswordCallBack);
	//Tell OpenSSL where to find our private key.
	if (SSL_CTX_use_PrivateKey_file(ctx, pkPath.c_str(), SSL_FILETYPE_PEM) != 1)
	{
		cout << "Error loading private key from " << pkPath << endl;
		cout << "Error code: " << ERR_error_string(ERR_get_error(), nullptr) << endl;
		system("PAUSE");
		return 1;
	}

	SSL* ssl = SSL_new(ctx);
	BIO* b = BIO_new_socket(s, BIO_NOCLOSE);
	SSL_set_bio(ssl, b, b);
	int res = SSL_accept(ssl);
	cout << "SSL_accept result: " << res << endl;
	if (res >= 0)
	{
		cout << "Error accepting SSL connection." << endl;
		cout << "Error code: " << ERR_error_string(ERR_get_error(), nullptr) << endl;
		system("PAUSE");
		return 1;
	}

	BIO_free(b);
	closesocket(s);
	SSL_free(ssl);
	SSL_CTX_free(ctx);
	EVP_cleanup();

	system("PAUSE");
	return 0;
}

int PasswordCallBack(char* buffer, int sizeOfBuffer, int rwflag, void* data)
{
	FILE* file;
	//Load the name of the password file.
	string fName = string(SSL_PASSWORD_FILE_LOCATION);
	fName += PASSWORD_FILE;

	file = fopen(fName.c_str(), "r");
	//Get the password.
	fgets(buffer, sizeOfBuffer - 2, file);
	buffer[sizeOfBuffer - 1] = '\0';

	fclose(file);

	return static_cast<int>(strlen(buffer));
}

#endif //__SSL_CONNECTION_TEST__