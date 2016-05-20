#pragma once
#include "sqlite3.h"
#include <string>
#include <mutex>

#define DEFAULT_USER_DB_LOCATION "user.db"

using std::string;

class SQLiteDb
{
private:
	string m_dbLoc;
	sqlite3* m_sqlObject;
	sqlite3_stmt* m_sqlStatement;
	bool m_hasRows;
public:
	SQLiteDb()
		: SQLiteDb(DEFAULT_USER_DB_LOCATION)
	{}
	SQLiteDb(const string& dbLocation)
		: m_dbLoc{dbLocation},
		m_sqlObject{ nullptr },
		m_sqlStatement{ nullptr },
		m_hasRows{ false }
	{
		OpenDb();
	}
	~SQLiteDb()
	{
		CloseDb();
	}

	bool IsOpen() { return m_sqlObject != nullptr; }
	bool ExecuteStatement(string statement);
	inline bool HasRows() { return m_hasRows; }
	const char* GetLastErrMsg() { return sqlite3_errmsg(m_sqlObject); }
	int GetLastErrCode() { return sqlite3_errcode(m_sqlObject); }

private:
	bool OpenDb();
	void CloseDb();

	/*
		TO DO:
		  1) Determine how to properly protect against SQL injection.
	*/
	string CleanStatement(string statement);
};