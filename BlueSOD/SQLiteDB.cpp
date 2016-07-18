#pragma once

#define DEBUG

#include "SQLiteDB.h"
#ifdef DEBUG
#include <iostream>


using std::cout;
using std::endl;
#endif
using std::make_unique;
using std::array;
using std::string;

const string MESSAGES_TABLE = "";

int SQLiteDb::ExecuteStatement(const string& statement)
{
	sqlite3_finalize(m_sqlStatement);

	string cleanedStmt = CleanStatement(statement);
	int res = sqlite3_prepare_v2(m_sqlObject, cleanedStmt.c_str(), cleanedStmt.size(), &m_sqlStatement, nullptr);

	/*The statement was bad, so notify. Anything other than SQLITE_OK is bad.*/
	if (res != SQLITE_OK)
		return res;

	switch (res = StepNextRow())
	{
		case SQLITE_ROW:
			SetColumnCount(sqlite3_column_count(m_sqlStatement));
			break;
		default:
			SetColumnCount(0);
			break;
	}

	return res;
}

int SQLiteDb::GetColumnInt(int col)
{ 
	return sqlite3_column_int(m_sqlStatement, col);
}

string SQLiteDb::GetColumnTxt(int col)
{
	string data = (const char*)sqlite3_column_text(m_sqlStatement, col);
	return data;
}

double SQLiteDb::GetColumnDouble(int col)
{
	return sqlite3_column_double(m_sqlStatement, col);
}

time_t SQLiteDb::GetColumnTime(int col)
{
	return static_cast<time_t>(sqlite3_column_int64(m_sqlStatement, col));
}

int SQLiteDb::StepNextRow()
{
	int res = sqlite3_step(m_sqlStatement);
	m_hasRows = res == SQLITE_ROW;
	return res;
}

bool SQLiteDb::Open(const string& dbLoc)
{
	if (IsOpen())
		CloseDb();

	m_dbLoc = dbLoc;

	return OpenDb();
}

int SQLiteDb::ColumnCount()
{
	return m_numberOfColumns;
}

string SQLiteDb::MessagesTableName(const string& username)
{
	return username + MESSAGES_TABLE;
}

bool SQLiteDb::OpenDb()
{
	int res = sqlite3_open_v2(m_dbLoc.c_str(), &m_sqlObject, SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
	
	return res == SQLITE_OK;
}

void SQLiteDb::CloseDb()
{
	sqlite3_finalize(m_sqlStatement);
	sqlite3_close_v2(m_sqlObject);
}

string SQLiteDb::CleanStatement(const string& statement)
{
	return statement;
}

void SQLiteDb::SetColumnCount(int c)
{
	m_numberOfColumns = c;
}