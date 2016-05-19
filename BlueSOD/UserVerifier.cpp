#include "UserVerifier.h"



UserVerifier::UserVerifier(const string& db)
{

}


UserVerifier::~UserVerifier()
{
}

void UserVerifier::AddRequest(const ClientInfo & info)
{
	queue.push(info);
}

ClientInfo UserVerifier::GetFulfilledRequest()
{
	ClientInfo info = finished.top();
	finished.pop();
	return std::move(info);
}

bool UserVerifier::HasFulFilledRequest()
{
	return !finished.empty();
}
