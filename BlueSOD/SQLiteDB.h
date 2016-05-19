#pragma once
#include <string>
#include <sqlite3.h>

using std::string;

class SQLiteDB
{
private:
	string db;
	sqlite3 object;
	sqlite3_stmt statement;
public:
	SQLiteDB();
	~SQLiteDB();
};

