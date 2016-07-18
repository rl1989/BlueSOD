#pragma once
#include <string>
#include <vector>

/*Message codes*/
#define REJECTED_LOGIN_MSG       "303"
#define SUCCESSFUL_LOGIN_MSG     "301"
#define UNSUCCESSFUL_LOGIN_MSG   "302"
#define LOGIN_MSG                "300"
#define LOGOUT_MSG               "304"
#define INVALID_MSG              "999"
#define INVALID_MSG_RESPONSE     "990"
#define MSG_BTWN_USERS           "100"
#define REQ_CONVO_LISTS          "201"
#define REQ_CONVO_LISTS_RESPONSE "211"
#define REQ_CONVO_MSGS           "202"
#define REQ_FILE                 "203"
#define REQ_USER_INFO            "204"
#define USER_DOESNT_EXIST        "901"

/*Miscellaneous message entities*/
#define DELIMITER ";"
#define END_OF_MSG "{e}"

/*Message fields*/
#define PASSWORD_FIELD "p:"
#define FROM_FIELD     "f:"
#define TO_FIELD       "t:"
#define MESSAGE_FIELD  "m:"
#define USERNAME_FIELD "u:"
#define DATE_FIELD     "d:"
#define FOR_FIELD      "f:"
#define WITH_FIELD     "w:"
#define FILENAME_FIELD "f:"
#define TARGET_FIELD   "t:"
#define STATUS_FIELD   "s:"

enum class message_t
{
	MESSAGE_TO_USER, REQUEST_CONVO_LISTS, REQUEST_CONVO_MSGS, LOGOUT, INVALID, REQUEST_FILE, REQUEST_USER_INFO, LOGIN,
	CONVERSATION_MSGS, CONVERSATION_LIST, FILE, USER_INFO
};

class MessageBase
{
private:
	message_t m_code{ message_t::INVALID };
	bool m_valid{ false };
public:
	static message_t GetMessageType(const std::string& msg);

	inline bool IsValid();
	inline message_t Code();
	inline std::string Date();

	virtual std::string MakeMessage() = 0;
	virtual std::string MakeQueries() = 0;

	static std::string InvalidMessageResponse();
protected:
	MessageBase(message_t code = message_t::INVALID, bool isValid = false);

	std::string m_date{};
};

/*
100;f:fromUser;t:toUser;m:message;d:date;{e}
*/
class MessageToUser : public MessageBase
{
private:
	std::string m_from{};
	int m_fromId{};
	std::string m_to{};
	int m_toId{};
	std::string m_message{};
	std::string m_status{};

	MessageToUser(const std::string& from, const std::string& to, const std::string& message, const std::string& date, const std::string& status);
	MessageToUser();
public:
	inline std::string From();
	inline int FromId();
	inline std::string To();
	inline int ToId();
	inline std::string Message();
	inline std::string Status();

	static const int code = 100;

	static MessageToUser MakeMessageToUser(const std::string& from, const std::string& to, const std::string& message, const std::string& date, const std::string& status);
	static MessageToUser* ParseMessageToUser(const std::string& msg);
	std::string InvalidMessageResponse();
	std::string UserDNEResponse();
	std::string MakeResponse();
	std::string MakeMessage();
	std::string MakeQueries();
};

class InvalidMessage : public MessageBase
{
private:
	static const int code = 999;
public:
	InvalidMessage();
	
	std::string MakeQueries();
	std::string MakeResponse();
	std::string MakeMessage();
};

class UserInfoRequest : public MessageBase
{
private:
	std::string m_requester{};
	std::string m_target{};

	UserInfoRequest(const std::string& _for, const std::string& target);
	UserInfoRequest();
public:
	inline std::string Requester();
	inline std::string Target();

	static UserInfoRequest* ParseMessage(const std::string& msg);
	std::string MakeResponse();
	std::string MakeMessage();
	std::string MakeQueries();
};

/*
	203;u:username;f:filename;{e}
*/
class FileRequestMessage : public MessageBase
{
private:
	std::string m_requester{};
	std::string m_filename{};

	FileRequestMessage(const std::string& _for, const std::string& filename);
	FileRequestMessage();
public:
	inline std::string Requester();
	inline std::string Filename();

	static const int code = 203;

	static FileRequestMessage* ParseMessage(const std::string& msg);
	std::string MakeResponse();
	std::string MakeMessage();
	std::string MakeQueries();
};

/*
	304;u:logoutUser;{e}
*/
class LogoutMessage : public MessageBase
{
private:
	std::string m_username{};

	explicit LogoutMessage(const std::string& username);
	LogoutMessage();
public:
	inline std::string Username();

	static const int code = 304;

	static LogoutMessage* ParseMessage(const std::string& msg);
	std::string MakeResponse();
	std::string MakeMessage();
	std::string MakeQueries();
};

class ConversationMessagesMessage : public MessageBase
{
private:
	std::string m_with;
	std::vector<MessageToUser> m_messages;
public:
	ConversationMessagesMessage(const std::string& with, std::vector<MessageToUser>&& messages);
	ConversationMessagesMessage() = default;
	ConversationMessagesMessage(const ConversationMessagesMessage& cmm) = default;
	ConversationMessagesMessage(ConversationMessagesMessage&& cmm) = default;
	~ConversationMessagesMessage() = default;

	std::string MakeResponse();
	std::string MakeMessage();
	std::string MakeQueries();
};

/*
	202;u:username;w:withUser;{e}
*/
class RequestConversationMsgs : public MessageBase
{
private:
	std::string m_requester{};
	std::string m_with{};

	RequestConversationMsgs(const std::string& _for, const std::string& to);
	RequestConversationMsgs();
public:
	inline std::string Requester();
	inline std::string With();

	static const int code = 202;

	static RequestConversationMsgs* ParseRequest(const std::string& msg);
	std::string MakeResponse();
	std::string MakeMessage();
	std::string MakeQueries();
};

class ConversationListMessage : public MessageBase
{
private:
	std::vector<std::string> m_list{};

public:
	explicit ConversationListMessage(const std::vector<std::string>& list);

	inline std::vector<std::string> GetList();

	std::string MakeResponse();
	std::string MakeMessage();
	std::string MakeQueries();
};

/*
	201;u:forUsername;{e}
*/
class RequestConversationListMessage : public MessageBase
{
private:
	std::string m_requester{};
	
	explicit RequestConversationListMessage(const std::string& _for);
	RequestConversationListMessage();
public:
	inline std::string Requester();

	static const int code = 201;

	static RequestConversationListMessage* ParseRequest(const std::string& msg);
	static std::string MakeResponse(std::vector<std::string> listOfUsers);
	std::string MakeResponse();
	std::string MakeMessage();
	std::string MakeQueries();
};

class LoginMessage : public MessageBase
{
private:
	std::string m_username;
	std::string m_password;

	LoginMessage(const std::string& username, const std::string& password);
	LoginMessage();
public:
	inline std::string Username();
	inline std::string Password();

	static const int code = 300;

	inline bool IsValid();
	std::string MakeResponse();
	std::string MakeMessage();
	std::string MakeQueries();

	static LoginMessage* ParseLoginMsg(const std::string& msg);
};