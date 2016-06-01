#pragma once
#include "sqlite3.h"
#include <string>
#include <mutex>
#include <memory>
#include <array>

#define DEFAULT_USER_DB_LOCATION "user.db"

#define USERNAME_COL "Username"
#define PASSWORD_COL "Password"
#define USER_INFO_DB "user.db"

//#define UNIQUE_PTR_DEBUG

using std::string;
using std::unique_ptr;

class SQLiteDb
{
private:
	std::string m_dbLoc;
	sqlite3* m_sqlObject;
	sqlite3_stmt* m_sqlStatement;
	bool m_hasRows;

#ifdef UNIQUE_PTR_DEBUG
	std::unique_ptr<int*> m_intData;
	std::unique_ptr<double*> m_dblData;
	std::unique_ptr<unsigned char**> m_txtData;
#else
	int** m_intColBuffer;
	double** m_dblColBuffer;
	std::string** m_txtColBuffer;
	int m_numberOfColumns;
#endif
public:
	SQLiteDb()
		: SQLiteDb(DEFAULT_USER_DB_LOCATION)
	{}
	SQLiteDb(const std::string& dbLocation)
		: m_dbLoc{dbLocation},
		m_sqlObject{ nullptr },
		m_sqlStatement{ nullptr },
		m_hasRows{ false }
#ifndef UNIQUE_PTR_DEBUG
		, m_intColBuffer{nullptr}, m_dblColBuffer{nullptr}, m_txtColBuffer{nullptr}, m_numberOfColumns{ 0 }
#else

#endif
	{
		OpenDb();
	}
	~SQLiteDb()
	{
		CloseDb();
		ClearBuffers();
	}

	bool IsOpen() { return m_sqlObject != nullptr; }
	int ExecuteStatement(const std::string& statement);
	inline bool HasRows() { return m_hasRows; }
	int GetColumnInt(int col);
	std::string& GetColumnTxt(int col);
	double GetColumnDouble(int col);
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

private:
	bool OpenDb();
	void CloseDb();

	void SetColumnCount(int c);
	template<typename B>
	inline void DeleteRowData(B** buffer);
	template<typename B>
	inline void DeleteBuffer(B** buffer);
	void ClearRowData();
	void ClearBuffers();
	void CreateNewBuffers();
	void FillRowData();
};