#pragma once
#include <string>
#include <mutex>
#include "sqlite3.h"

using std::string;

class SQLiteDb
{
private:
	string m_dbLoc;
	sqlite3* m_sqlObject;
	sqlite3_stmt* m_sqlStatement;
	int m_lastErr;
	bool m_hasRows;
public:
	SQLiteDb();
	~SQLiteDb();

	bool ExecuteStatement(string statement);
	inline bool HasRows() { return m_hasRows; }


private:
	bool OpenDb();
	void CloseDb();

	/*
		TO DO:
		  1) Determine how to properly protect against SQL injection.
	*/
	string CleanStatement(string statement);
};