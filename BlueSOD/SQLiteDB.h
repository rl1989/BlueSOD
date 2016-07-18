#pragma once
#include "sqlite3.h"
#include <string>
#include <mutex>
#include <memory>
#include <array>
#include <mutex>

#define DEFAULT_USER_DB_LOCATION "user.db"

#define USERNAME_COL "Username"
#define PASSWORD_COL "Password"
#define ID_COL       "Id"
#define USER_INFO_DB "user.db"

//#define UNIQUE_PTR_DEBUG

class SQLiteDb
{
private:
	std::string m_dbLoc;
	sqlite3* m_sqlObject;
	sqlite3_stmt* m_sqlStatement;
	bool m_hasRows;
	int m_numberOfColumns;
	const static std::string MESSAGES_TABLE;
	std::mutex m_mutex;
public:
	SQLiteDb()
		: SQLiteDb(DEFAULT_USER_DB_LOCATION)
	{}
	SQLiteDb(const std::string& dbLocation)
		: m_dbLoc{dbLocation},
		m_sqlObject{ nullptr },
		m_sqlStatement{ nullptr },
		m_hasRows{ false },
		m_numberOfColumns{ 0 }
	{
		OpenDb();
	}
	~SQLiteDb()
	{
		CloseDb();
	}

	inline void Lock() { m_mutex.lock(); }
	inline void Unlock() { m_mutex.unlock(); }
	bool IsOpen() { return m_sqlObject != nullptr; }
	int ExecuteStatement(const std::string& statement);
	inline bool HasRows() { return m_hasRows; }
	int GetColumnInt(int col);
	std::string GetColumnTxt(int col);
	double GetColumnDouble(int col);
	time_t GetColumnTime(int col);
	inline int StepNextRow();
	const char* GetLastErrMsg() { return sqlite3_errmsg(m_sqlObject); }
	int GetLastErrCode() { return sqlite3_errcode(m_sqlObject); }
	bool Open(const std::string& dbLoc);
	int ColumnCount();

	/*
	TO DO:
	1) Determine how to properly protect against SQL injection.
	*/
	static std::string CleanStatement(const std::string& statement);

	static std::string MessagesTableName(const std::string& username);

private:
	bool OpenDb();
	void CloseDb();

	void SetColumnCount(int c);
};