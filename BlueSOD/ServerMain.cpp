#pragma once
#include "SQLiteDB.h"
#include <iostream>
#include <string>
#include <utility>

using std::move;
using std::string;
using std::cout;
using std::endl;

/*
	Testing the SQLiteDb object.
*/

int main()
{
	SQLiteDb db{"test.db"};

	int res = db.ExecuteStatement("CREATE TABLE another_test(name TEXT PRIMARY KEY, age INT);");
	cout << res << endl;
	res = db.ExecuteStatement("INSERT INTO another_test(name, age) VALUES('Ricky', 26)");
	cout << res << endl;
	res = db.ExecuteStatement("SELECT name, age FROM another_test WHERE name='Ricky'");
	string name = move(db.GetColumnTxt(0));
	int age = db.GetColumnInt(1);
	cout << "AND HIS NAME IS " << name << " AT THE AGE OF " << age << endl;

	system("PAUSE");
}