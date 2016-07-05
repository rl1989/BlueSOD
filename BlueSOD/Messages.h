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
#define BTWN_FIELD     "b:"
#define FILENAME_FIELD "f:"
#define TARGET_FIELD   "t:"
#define STATUS_FIELD   "s:"

enum class message_t
{
	MESSAGE, REQUEST_CONVO_LISTS, REQUEST_CONVO_MSGS, LOGOUT, INVALID, REQUEST_FILE, REQUEST_USER_INFO, LOGIN
};

class UserInfoRequest : public MessageBase
{
private:
	std::string m_for;
	std::string m_target;

	UserInfoRequest(const std::string& _for, const std::string& target);
public:
	inline std::string For();
	inline std::string Target();

	static UserInfoRequest ParseMessage(const std::string& msg);
};

class FileRequestMessage : public MessageBase
{
private:
	std::string m_for;
	std::string m_filename;

	FileRequestMessage(const std::string& _for, const std::string& filename);
public:
	inline std::string For();
	inline std::string Filename();

	static FileRequestMessage ParseMessage(const std::string& msg);
};

class LogoutMessage : public MessageBase
{
private:
	std::string m_username;

	explicit LogoutMessage(const std::string& username);
public:
	inline std::string Username();

	static LogoutMessage ParseMessage(const std::string& msg);
};

class RequestConversationMsgs : public MessageBase
{
private:
	std::string m_for;
	std::string m_to;

	RequestConversationMsgs(const std::string& _for, const std::string& to);
public:
	inline std::string For();
	inline std::string To();

	static RequestConversationMsgs ParseRequest(const std::string& msg);
};

class RequestConversationLists : public MessageBase
{
private:
	std::string m_for;
	
	explicit RequestConversationLists(const std::string& _for);
public:
	inline std::string For();

	static RequestConversationLists ParseRequest(const std::string& msg);
	static string MakeResponse(std::vector<std::string> listOfUsers);
};

class MessageToUser : public MessageBase
{
private:
	std::string m_from;
	std::string m_to;
	std::string m_message;
	std::string m_date;
	std::string m_status;

	MessageToUser(const std::string& from, const std::string& to, const std::string& message, const std::string& date, const std::string& status);
public:
	inline std::string From();
	inline std::string To();
	inline std::string Message();
	inline std::string Date();
	inline std::string Status();

	static MessageToUser ParseMessageToUser(const std::string& msg);
	std::string MakeQueries();
	std::string MakeResponse();
	std::string InvalidMessageResponse();
	std::string UserDNEResponse();
	MessageToUser GenerateMessageForToUser();
	std::string MakeMessage();
};

class LoginMessage : public MessageBase
{
private:
	std::string m_username;
	std::string m_password;

	LoginMessage(const std::string& username, const std::string& password);
public:

	inline std::string Username();
	inline std::string Password();

	inline bool IsValid();

	static LoginMessage ParseLoginMsg(const std::string& msg);
};

class MessageBase
{
private:
	message_t m_code{ message_t::INVALID };
	bool m_valid{ false };
public:
	MessageBase(const MessageBase& mb);

	static message_t GetMessageType(const std::string& msg);

	inline bool IsValid();
	inline message_t Code();

	static std::string InvalidMessageResponse();
protected:
	MessageBase(message_t code = message_t::INVALID, bool isValid = false);
};