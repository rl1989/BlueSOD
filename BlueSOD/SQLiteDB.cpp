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

int SQLiteDb::ExecuteStatement(const string& statement)
{
	sqlite3_finalize(m_sqlStatement);

	string cleanedStmt = CleanStatement(statement);
	int res = sqlite3_prepare_v2(m_sqlObject, cleanedStmt.c_str(), cleanedStmt.size(), &m_sqlStatement, nullptr);

	if (res != SQLITE_OK)
		return res;

	switch (res = StepNextRow())
	{
		case SQLITE_ROW:
			SetColumnCount(sqlite3_column_count(m_sqlStatement));
			break;
		default:
			SetColumnCount(0);
			return res;
	}

	CreateNewBuffers();
	FillRowData();

	return res;
}

int SQLiteDb::GetColumnInt(int col)
{
	return *m_intColBuffer[col];
}

string& SQLiteDb::GetColumnTxt(int col)
{
	return *m_txtColBuffer[col];
}

double SQLiteDb::GetColumnDouble(int col)
{
	return *m_dblColBuffer[col];
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
	return std::move(statement);
}

void SQLiteDb::SetColumnCount(int c)
{
	m_numberOfColumns = c;
}

template<typename B>
void SQLiteDb::DeleteRowData(B** buffer)
{
	if (buffer == nullptr)
		return;
	for (int i = 0; i < ColumnCount(); i++)
	{
		if (buffer[i] != nullptr)
		{
			delete buffer[i];
			buffer[i] = nullptr;
		}
	}
}

template<typename B>
void SQLiteDb::DeleteBuffer(B** buffer)
{
	if (buffer != nullptr)
		delete buffer;
	buffer = nullptr;
}

void SQLiteDb::ClearRowData()
{
	DeleteRowData(m_intColBuffer);
	DeleteRowData(m_dblColBuffer);
	DeleteRowData(m_txtColBuffer);
}

void SQLiteDb::ClearBuffers()
{
	ClearRowData();
	DeleteBuffer(m_intColBuffer);
	DeleteBuffer(m_dblColBuffer);
	DeleteBuffer(m_txtColBuffer);
}

void SQLiteDb::CreateNewBuffers()
{
	ClearBuffers();

	int columns = ColumnCount();
	m_intColBuffer = new int*[columns] {nullptr};
	m_dblColBuffer = new double*[columns] {nullptr};
	m_txtColBuffer = new string*[columns] {nullptr};
}

void SQLiteDb::FillRowData()
{
	ClearRowData();

	for (int i = 0; i < ColumnCount(); i++)
	{
		switch (sqlite3_column_type(m_sqlStatement, i))
		{
			case SQLITE_INTEGER:
				m_intColBuffer[i] = new int;
				*m_intColBuffer[i] = sqlite3_column_int(m_sqlStatement, i);
				break;
			case SQLITE_FLOAT:
				m_dblColBuffer[i] = new double;
				*m_dblColBuffer[i] = sqlite3_column_double(m_sqlStatement, i);
				break;
			case SQLITE_TEXT:
				m_txtColBuffer[i] = new string{(const char*)sqlite3_column_text(m_sqlStatement, i)};
				break;
			default:
				break;
		}
	}
}

