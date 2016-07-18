#include "Messages.h"

using std::string;
using std::vector;

LoginMessage::LoginMessage(const std::string& username, const std::string& password)
	: MessageBase(message_t::LOGIN, true), m_username{username}, m_password{password}
{}

std::string LoginMessage::Username()
{
	return m_username;
}

std::string LoginMessage::Password()
{
	return m_password;
}

inline bool LoginMessage::IsValid()
{
	return ((MessageBase*) this)->IsValid();
}

LoginMessage* LoginMessage::ParseLoginMsg(const string& msg)
{
	LoginMessage* ret = new LoginMessage{};
	if (msg.substr(0, 3) != LOGIN_MSG)
		return ret;

	/*GetUsername field*/
	string field = USERNAME_FIELD;
	int username_field_pos = msg.find(field);
	int delimiter = msg.find(DELIMITER, 4);
	if (username_field_pos == string::npos || delimiter == string::npos)
		return ret;
	string username = msg.substr(username_field_pos + field.size(), delimiter - 1);

	/*Password field*/
	field = PASSWORD_FIELD;
	int password_field_pos = msg.find(field);
	delimiter = msg.find(DELIMITER, delimiter - 1);
	if (password_field_pos == string::npos || delimiter == string::npos)
		return ret;
	string password = msg.substr(password_field_pos + field.size(), delimiter - 1);

	if (msg.find(END_OF_MSG) == string::npos)
		return ret;

	ret->m_username = username;
	ret->m_password = password;

	return ret;
}

message_t MessageBase::GetMessageType(const string& msg)
{
	message_t ret = message_t::INVALID;
	string type = msg.substr(0, 3);

	if (type == MSG_BTWN_USERS)
		ret = message_t::MESSAGE_TO_USER;
	else if (type == REQ_CONVO_LISTS)
		ret = message_t::REQUEST_CONVO_LISTS;
	else if (type == REQ_CONVO_MSGS)
		ret = message_t::REQUEST_CONVO_MSGS;
	else if (type == REQ_FILE)
		ret = message_t::REQUEST_FILE;
	else if (type == REQ_USER_INFO)
		ret = message_t::REQUEST_USER_INFO;

	return ret;
}

inline bool MessageBase::IsValid()
{
	return m_valid;
}

message_t MessageBase::Code()
{
	return m_code;
}

inline std::string MessageBase::Date()
{
	return m_date;
}

MessageBase::MessageBase(message_t code, bool isValid)
	: m_code{code}, m_valid{isValid}
{}

MessageToUser::MessageToUser(const string& from, const string& to, const string& message, const string& date, const string& status)
	: MessageBase(message_t::MESSAGE_TO_USER), m_from{from}, m_to{to}, m_message{message}, m_status{status}
{
	m_date = date;
}

inline std::string MessageToUser::From()
{
	return m_from;
}

inline int MessageToUser::FromId()
{
	return m_fromId;
}

inline std::string MessageToUser::To()
{
	return m_to;
}

inline int MessageToUser::ToId()
{
	return m_toId;
}

inline std::string MessageToUser::Message()
{
	return m_message;
}

inline std::string MessageToUser::Status()
{
	return m_status;
}

MessageToUser* MessageToUser::ParseMessageToUser(const std::string& msg)
{
	MessageToUser* ret = new MessageToUser{};
	if (msg.substr(0, 3) != MSG_BTWN_USERS)
		return ret;

	/*From field*/
	string field = FROM_FIELD;
	int from_field_pos = msg.find(field);
	int delimiter = msg.find(DELIMITER, 4);
	if (from_field_pos == string::npos || delimiter == string::npos)
		return ret;
	string from = msg.substr(from_field_pos + field.size(), delimiter - 1);

	/*To field*/
	field = TO_FIELD;
	int to_field_pos = msg.find(field);
	delimiter = msg.find(DELIMITER, delimiter + 1);
	if (to_field_pos == string::npos || delimiter == string::npos)
		return ret;
	string to = msg.substr(to_field_pos + field.size(), delimiter - 1);

	/*Message field*/
	field = MESSAGE_FIELD;
	int msg_field_pos = msg.find(field);
	delimiter = msg.find(DELIMITER, delimiter + 1);
	if (msg_field_pos == string::npos || delimiter == string::npos)
		return ret;
	string message = msg.substr(msg_field_pos + field.size(), delimiter - 1);

	/*Date field*/
	field = DATE_FIELD;
	int date_field_pos = msg.find(field);
	delimiter = msg.find(DELIMITER, delimiter + 1);
	if (date_field_pos == string::npos || delimiter == string::npos)
		return ret;
	string date = msg.substr(date_field_pos + field.size(), delimiter - 1);

	/*Status field*/
	field = STATUS_FIELD;
	int status_field_pos = msg.find(field);
	delimiter = msg.find(DELIMITER, delimiter + 1);
	if (status_field_pos == string::npos || delimiter == string::npos)
		return ret;
	string status = msg.substr(status_field_pos + field.size(), delimiter - 1);

	if (msg.find(END_OF_MSG) == string::npos)
		return ret;

	ret->m_from = from;
	ret->m_to = to;
	ret->m_message = message;
	ret->m_date = date;
	ret->m_status = status;

	return ret;
}

RequestConversationListMessage::RequestConversationListMessage(const std::string & _for)
	: MessageBase(message_t::REQUEST_CONVO_LISTS, true), m_requester{_for}
{}

inline string RequestConversationListMessage::Requester()
{
	return m_requester;
}

RequestConversationListMessage* RequestConversationListMessage::ParseRequest(const string& msg)
{
	RequestConversationListMessage* ret = new RequestConversationListMessage{};
	if (msg.substr(0, 3) != REQ_CONVO_LISTS)
		return ret;

	/*GetUsername field*/
	string field = USERNAME_FIELD;
	int username_field_pos = msg.find(field);
	int delimiter = msg.find(DELIMITER, 4);
	if (username_field_pos == string::npos || delimiter == string::npos)
		return ret;
	string username = msg.substr(username_field_pos + field.size(), delimiter - 1);

	if (msg.find(END_OF_MSG) == string::npos)
		return ret;

	ret->m_requester = username;

	return ret;
}

string RequestConversationListMessage::MakeResponse(vector<string> listOfUsers)
{
	string msg{REQ_CONVO_LISTS_RESPONSE};
	msg += DELIMITER;
	for (auto username : listOfUsers)
	{
		msg += USERNAME_FIELD;
		msg += username;
		msg += DELIMITER;
	}
	msg += END_OF_MSG;

	return msg;
}

RequestConversationMsgs::RequestConversationMsgs(const string& _for, const string& to)
	: MessageBase(message_t::REQUEST_CONVO_MSGS, true), m_requester{_for}, m_with{to}
{}

inline string RequestConversationMsgs::Requester()
{
	return m_requester;
}

inline string RequestConversationMsgs::With()
{
	return m_with;
}

RequestConversationMsgs* RequestConversationMsgs::ParseRequest(const string& msg)
{
	RequestConversationMsgs* ret = new RequestConversationMsgs{};

	if (msg.substr(0, 3) != REQ_CONVO_MSGS)
		return ret;

	/*Username User field*/
	string field = FOR_FIELD;
	int username_field_pos = msg.find(field);
	int delimiter = msg.find(DELIMITER, 4);
	if (username_field_pos == string::npos || delimiter == string::npos)
		return ret;
	string username = msg.substr(username_field_pos + field.size(), delimiter - 1);

	/*Between field*/
	field = WITH_FIELD;
	int between_field_pos = msg.find(field);
	delimiter = msg.find(DELIMITER, delimiter + 1);
	if (between_field_pos == string::npos || delimiter == string::npos)
		return ret;
	string between = msg.substr(between_field_pos + field.size(), delimiter - 1);

	if (msg.find(END_OF_MSG) == string::npos)
		return ret;

	ret->m_requester = username;
	ret->m_with = between;

	return ret;
}

LogoutMessage::LogoutMessage(const string & username)
	: MessageBase(message_t::LOGOUT, true), m_username{username}
{}

inline string LogoutMessage::Username()
{
	return m_username;
}

LogoutMessage* LogoutMessage::ParseMessage(const string& msg)
{
	LogoutMessage* ret = new LogoutMessage{};

	if (msg.substr(0, 3) != LOGOUT_MSG)
		return ret;

	/*GetUsername field*/
	string field = USERNAME_FIELD;
	int username_field_pos = msg.find(field);
	int delimiter = msg.find(DELIMITER, 4);
	if (username_field_pos == string::npos || delimiter == string::npos)
		return ret;
	string username = msg.substr(username_field_pos + field.size(), delimiter - 1);

	if (msg.find(END_OF_MSG) == string::npos)
		return ret;

	ret->m_username = username;

	return ret;
}

FileRequestMessage::FileRequestMessage(const string& _for, const string& filename)
	: MessageBase(message_t::REQUEST_FILE, true), m_requester{_for}, m_filename{filename}
{
}

inline string FileRequestMessage::Requester()
{
	return m_requester;
}

inline string FileRequestMessage::Filename()
{
	return m_filename;
}

FileRequestMessage* FileRequestMessage::ParseMessage(const string& msg)
{
	FileRequestMessage* ret = new FileRequestMessage{};

	if (msg.substr(0, 3) != REQ_FILE)
		return ret;

	/*GetUsername field*/
	string field = USERNAME_FIELD;
	int username_field_pos = msg.find(field);
	int delimiter = msg.find(DELIMITER, 4);
	if (username_field_pos == string::npos || delimiter == string::npos)
		return ret;
	string username = msg.substr(username_field_pos + field.size(), delimiter - 1);

	/*Filename field*/
	field = FILENAME_FIELD;
	int filename_field_pos = msg.find(field);
	delimiter = msg.find(DELIMITER, delimiter + 1);
	if (filename_field_pos == string::npos || delimiter == string::npos)
		return ret;
	string filename = msg.substr(filename_field_pos + field.size(), delimiter - 1);

	if (msg.find(END_OF_MSG) == string::npos)
		return ret;

	ret->m_requester = username;
	ret->m_filename = filename;

	return ret;
}

UserInfoRequest::UserInfoRequest(const string& _for, const string& target)
	: MessageBase(message_t::REQUEST_USER_INFO, true), m_requester{_for}, m_target{target}
{}

inline std::string UserInfoRequest::Requester()
{
	return m_requester;
}

inline std::string UserInfoRequest::Target()
{
	return m_target;
}

UserInfoRequest* UserInfoRequest::ParseMessage(const string& msg)
{
	UserInfoRequest* ret = new UserInfoRequest{};

	if (msg.substr(0, 3) != REQ_USER_INFO)
		return ret;

	/*For field*/
	string field = FOR_FIELD;
	int for_field_pos = msg.find(field);
	int delimiter = msg.find(DELIMITER, 4);
	if (for_field_pos == string::npos || delimiter == string::npos)
		return ret;
	string _for = msg.substr(for_field_pos + field.size(), delimiter - 1);

	/*Target field*/
	field = TARGET_FIELD;
	int target_field_pos = msg.find(field);
	delimiter = msg.find(DELIMITER, delimiter - 1);
	if (target_field_pos == string::npos || delimiter == string::npos)
		return ret;
	string target = msg.substr(target_field_pos + field.size(), delimiter - 1);

	if (msg.find(END_OF_MSG) == string::npos)
		return ret;

	ret->m_requester = _for;
	ret->m_target = target;

	return ret;
}
