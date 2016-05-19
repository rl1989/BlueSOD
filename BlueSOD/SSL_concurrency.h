//#define WIN32
/* crypto/threads/mttest.c */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
* All rights reserved.
*
* This package is an SSL implementation written
* by Eric Young (eay@cryptsoft.com).
* The implementation was written so as to conform with Netscapes SSL.
*
* This library is free for commercial and non-commercial use as long as
* the following conditions are aheared to.  The following conditions
* apply to all code found in this distribution, be it the RC4, RSA,
* lhash, DES, etc., code; not just the SSL code.  The SSL documentation
* included with this distribution is covered by the same copyright terms
* except that the holder is Tim Hudson (tjh@cryptsoft.com).
*
* Copyright remains Eric Young's, and as such any Copyright notices in
* the code are not to be removed.
* If this package is used in a product, Eric Young should be given attribution
* as the author of the parts of the library used.
* This can be in the form of a textual message at program startup or
* in documentation (online or textual) provided with the package.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. All advertising materials mentioning features or use of this software
*    must display the following acknowledgement:
*    "This product includes cryptographic software written by
*     Eric Young (eay@cryptsoft.com)"
*    The word 'cryptographic' can be left out if the rouines from the library
*    being used are not cryptographic related :-).
* 4. If you include any Windows specific code (or a derivative thereof) from
*    the apps directory (application code) you must include an acknowledgement:
*    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
*
* THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*
* The licence and distribution terms for any publically available version or
* derivative of this code cannot be changed.  i.e. this code cannot simply be
* copied and put under another distribution licence
* [including the GNU Public Licence.]
*/

//#ifdef WIN32
//#include <windows.h>
//#endif
#include <thread>
#include <vector>
#include <shared_mutex>

#include <openssl/crypto.h>

#define MAX_THREAD_NUMBER	100

using std::thread;
using std::vector;
using std::shared_mutex;

void thread_setup(void);
void thread_cleanup(void);

void win32_locking_callback(int mode, int type, char *file, int line);

//static HANDLE *lock_cs;
//static vector<shared_mutex> openSSLMutexes;
shared_mutex* openSSLMutexes;

void thread_setup(void)
{
	int i;

	//lock_cs = static_cast<HANDLE*>(OPENSSL_malloc(CRYPTO_num_locks() * sizeof(HANDLE)));
	//openSSLMutexes = vector<shared_mutex>(CRYPTO_num_locks());
	openSSLMutexes = new shared_mutex[CRYPTO_num_locks()]{};

	/*for (i = 0; i<CRYPTO_num_locks(); i++)
	{
		lock_cs[i] = CreateMutex(NULL, FALSE, NULL);
	}*/

	CRYPTO_set_locking_callback((void(*)(int, int, const char *, int))win32_locking_callback);
	/* id callback defined */
	CRYPTO_set_id_callback((unsigned long (*)())std::this_thread::get_id);
	
}

void thread_cleanup(void)
{
	//int i;

	CRYPTO_set_locking_callback(NULL);
	delete[] openSSLMutexes;
	/*for (i = 0; i<CRYPTO_num_locks(); i++)
		CloseHandle(lock_cs[i]);*/
	//OPENSSL_free(lock_cs);
}

void win32_locking_callback(int mode, int type, char *file, int line)
{
	if (mode & CRYPTO_LOCK)
	{
		//WaitForSingleObject(lock_cs[type], INFINITE);
		openSSLMutexes[type].lock_shared();
	}
	else
	{
		//ReleaseMutex(lock_cs[type]);
		openSSLMutexes[type].unlock_shared();
	}
}