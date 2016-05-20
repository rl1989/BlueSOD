#include "SQLiteDB.h"

bool SQLiteDb::ExecuteStatement(string statement)
{
	string cleanedStmt = CleanStatement(statement);
	int res = sqlite3_prepare_v2(m_sqlObject, cleanedStmt.c_str(), cleanedStmt.size() + 1, &m_sqlStatement, nullptr);
	if (res != SQLITE_OK)
	{
		return false;
	}

	res = sqlite3_step(m_sqlStatement);
	switch (res)
	{
		case SQLITE_DONE:
			m_hasRows = false;
			sqlite3_reset(m_sqlStatement);
			return true;
		case SQLITE_ROW:
			m_hasRows = true;
			return true;
		default:
			return false;
	}
}

bool SQLiteDb::OpenDb()
{
	int res = sqlite3_open_v2(m_dbLoc.c_str(), &m_sqlObject, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
	
	return res == SQLITE_OK;
}

void SQLiteDb::CloseDb()
{
	sqlite3_finalize(m_sqlStatement);
	sqlite3_close_v2(m_sqlObject);
}

string SQLiteDb::CleanStatement(string statement)
{
	return statement;
}
