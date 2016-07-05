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

LoginMessage LoginMessage::ParseLoginMsg(const string& msg)
{
	if (msg.substr(0, 3) != LOGIN_MSG)
		return LoginMessage{};

	/*GetUsername field*/
	string field = USERNAME_FIELD;
	int username_field_pos = msg.find(field);
	int delimiter = msg.find(DELIMITER, 4);
	if (username_field_pos == string::npos || delimiter == string::npos)
		return LoginMessage{};
	string username = msg.substr(username_field_pos + field.size(), delimiter - 1);

	/*Password field*/
	field = PASSWORD_FIELD;
	int password_field_pos = msg.find(field);
	delimiter = msg.find(DELIMITER, delimiter - 1);
	if (password_field_pos == string::npos || delimiter == string::npos)
		return LoginMessage{};
	string password = msg.substr(password_field_pos + field.size(), delimiter - 1);

	if (msg.find(END_OF_MSG) == string::npos)
		return LoginMessage();

	return LoginMessage{username, password};
}

message_t MessageBase::GetMessageType(const string& msg)
{
	message_t ret = message_t::INVALID;
	string type = msg.substr(0, 3);

	if (type == MSG_BTWN_USERS)
		ret = message_t::MESSAGE;
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

MessageBase::MessageBase(message_t code, bool isValid)
	: m_code{code}, m_valid{isValid}
{}

MessageToUser::MessageToUser(const string& from, const string& to, const string& message, const string& date, const string& status)
	: MessageBase(message_t::MESSAGE), m_from{from}, m_to{to}, m_message{message}, m_date{date}, m_status{status}
{}

inline std::string MessageToUser::From()
{
	return m_from;
}

inline std::string MessageToUser::To()
{
	return m_to;
}

inline std::string MessageToUser::Message()
{
	return m_message;
}

inline std::string MessageToUser::Date()
{
	return m_date;
}

inline std::string MessageToUser::Status()
{
	return m_status;
}

MessageToUser MessageToUser::ParseMessageToUser(const std::string& msg)
{
	if (msg.substr(0, 3) != MSG_BTWN_USERS)
		return MessageToUser();

	/*From field*/
	string field = FROM_FIELD;
	int from_field_pos = msg.find(field);
	int delimiter = msg.find(DELIMITER, 4);
	if (from_field_pos == string::npos || delimiter == string::npos)
		return MessageToUser();
	string from = msg.substr(from_field_pos + field.size(), delimiter - 1);

	/*To field*/
	field = TO_FIELD;
	int to_field_pos = msg.find(field);
	delimiter = msg.find(DELIMITER, delimiter + 1);
	if (to_field_pos == string::npos || delimiter == string::npos)
		return MessageToUser();
	string to = msg.substr(to_field_pos + field.size(), delimiter - 1);

	/*Message field*/
	field = MESSAGE_FIELD;
	int msg_field_pos = msg.find(field);
	delimiter = msg.find(DELIMITER, delimiter + 1);
	if (msg_field_pos == string::npos || delimiter == string::npos)
		return MessageToUser();
	string message = msg.substr(msg_field_pos + field.size(), delimiter - 1);

	/*Date field*/
	field = DATE_FIELD;
	int date_field_pos = msg.find(field);
	delimiter = msg.find(DELIMITER, delimiter + 1);
	if (date_field_pos == string::npos || delimiter == string::npos)
		return MessageToUser();
	string date = msg.substr(date_field_pos + field.size(), delimiter - 1);

	/*Status field*/
	field = STATUS_FIELD;
	int status_field_pos = msg.find(field);
	delimiter = msg.find(DELIMITER, delimiter + 1);
	if (status_field_pos == string::npos || delimiter == string::npos)
		return MessageToUser();
	string status = msg.substr(status_field_pos + field.size(), delimiter - 1);

	if (msg.find(END_OF_MSG) == string::npos)
		return MessageToUser();

	return MessageToUser(from, to, message, date, status);
}

RequestConversationLists::RequestConversationLists(const std::string & _for)
	: MessageBase(message_t::REQUEST_CONVO_LISTS, true), m_for{_for}
{}

inline string RequestConversationLists::For()
{
	return m_for;
}

RequestConversationLists RequestConversationLists::ParseRequest(const string& msg)
{
	if (msg.substr(0, 3) != REQ_CONVO_LISTS)
		return RequestConversationLists();

	/*GetUsername field*/
	string field = USERNAME_FIELD;
	int username_field_pos = msg.find(field);
	int delimiter = msg.find(DELIMITER, 4);
	if (username_field_pos == string::npos || delimiter == string::npos)
		return RequestConversationLists();
	string username = msg.substr(username_field_pos + field.size(), delimiter - 1);

	if (msg.find(END_OF_MSG) == string::npos)
		return RequestConversationLists();

	return RequestConversationLists{username};
}

string RequestConversationLists::MakeResponse(vector<string> listOfUsers)
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
	: MessageBase(message_t::REQUEST_CONVO_MSGS, true), m_for{_for}, m_to{to}
{}

inline string RequestConversationMsgs::For()
{
	return m_for;
}

inline string RequestConversationMsgs::To()
{
	return m_to;
}

RequestConversationMsgs RequestConversationMsgs::ParseRequest(const string& msg)
{
	if (msg.substr(0, 3) != REQ_CONVO_MSGS)
		return RequestConversationMsgs();

	/*For User field*/
	string field = USERNAME_FIELD;
	int username_field_pos = msg.find(field);
	int delimiter = msg.find(DELIMITER, 4);
	if (username_field_pos == string::npos || delimiter == string::npos)
		return RequestConversationMsgs();
	string username = msg.substr(username_field_pos + field.size(), delimiter - 1);

	/*Between field*/
	field = BTWN_FIELD;
	int between_field_pos = msg.find(field);
	delimiter = msg.find(DELIMITER, delimiter + 1);
	if (between_field_pos == string::npos || delimiter == string::npos)
		return RequestConversationMsgs();
	string between = msg.substr(between_field_pos + field.size(), delimiter - 1);

	if (msg.find(END_OF_MSG) == string::npos)
		return RequestConversationMsgs();

	return RequestConversationMsgs{username, between};
}

LogoutMessage::LogoutMessage(const string & username)
	: MessageBase(message_t::LOGOUT, true), m_username{username}
{}

inline string LogoutMessage::Username()
{
	return m_username;
}

LogoutMessage LogoutMessage::ParseMessage(const string& msg)
{
	if (msg.substr(0, 3) != LOGOUT_MSG)
		return LogoutMessage();

	/*GetUsername field*/
	string field = USERNAME_FIELD;
	int username_field_pos = msg.find(field);
	int delimiter = msg.find(DELIMITER, 4);
	if (username_field_pos == string::npos || delimiter == string::npos)
		return LogoutMessage();
	string username = msg.substr(username_field_pos + field.size(), delimiter - 1);

	if (msg.find(END_OF_MSG) == string::npos)
		return LogoutMessage();

	return LogoutMessage{ username };
}

FileRequestMessage::FileRequestMessage(const string& _for, const string& filename)
	: MessageBase(message_t::REQUEST_FILE, true), m_for{_for}, m_filename{filename}
{
}

inline string FileRequestMessage::For()
{
	return m_for;
}

inline string FileRequestMessage::Filename()
{
	return m_filename;
}

FileRequestMessage FileRequestMessage::ParseMessage(const string& msg)
{
	if (msg.substr(0, 3) != REQ_FILE)
		return FileRequestMessage();

	/*GetUsername field*/
	string field = USERNAME_FIELD;
	int username_field_pos = msg.find(field);
	int delimiter = msg.find(DELIMITER, 4);
	if (username_field_pos == string::npos || delimiter == string::npos)
		return FileRequestMessage();
	string username = msg.substr(username_field_pos + field.size(), delimiter - 1);

	/*Filename field*/
	field = FILENAME_FIELD;
	int filename_field_pos = msg.find(field);
	delimiter = msg.find(DELIMITER, delimiter + 1);
	if (filename_field_pos == string::npos || delimiter == string::npos)
		return FileRequestMessage();
	string filename = msg.substr(filename_field_pos + field.size(), delimiter - 1);

	if (msg.find(END_OF_MSG) == string::npos)
		return FileRequestMessage();

	return FileRequestMessage{username, filename};
}

UserInfoRequest::UserInfoRequest(const string& _for, const string& target)
	: MessageBase(message_t::REQUEST_USER_INFO, true), m_for{_for}, m_target{target}
{}

inline std::string UserInfoRequest::For()
{
	return m_for;
}

inline std::string UserInfoRequest::Target()
{
	return m_target;
}

UserInfoRequest UserInfoRequest::ParseMessage(const string& msg)
{
	if (msg.substr(0, 3) != REQ_USER_INFO)
		return UserInfoRequest();

	/*For field*/
	string field = FOR_FIELD;
	int for_field_pos = msg.find(field);
	int delimiter = msg.find(DELIMITER, 4);
	if (for_field_pos == string::npos || delimiter == string::npos)
		return UserInfoRequest();
	string _for = msg.substr(for_field_pos + field.size(), delimiter - 1);

	/*Target field*/
	field = TARGET_FIELD;
	int target_field_pos = msg.find(field);
	delimiter = msg.find(DELIMITER, delimiter - 1);
	if (target_field_pos == string::npos || delimiter == string::npos)
		return UserInfoRequest();
	string target = msg.substr(target_field_pos + field.size(), delimiter - 1);

	if (msg.find(END_OF_MSG) == string::npos)
		return UserInfoRequest();

	return UserInfoRequest{ _for, target };
}
