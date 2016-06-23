#pragma once
#include <iostream>
#include <random>
#include <time.h>
#include <string>
#include <vector>
#include <ctime>
#include <chrono>
#include "ClientInfoList.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

vector<string> names = {"Ricky Lindsay", "Michael Hernandez", "John Hakami", "Anthony Rodriguez",
"Cynthia Lindsay", "Amber Price", "Walter Wilson", "Dorothy Wilson", "Beth Lopez", "Courtney Hernandez",
"Larry Lopez", "Bernard Miller", "Lyrone Sanders", "Michael Henry", "Amanda Henry", "John Henry",
"John Henry Jr", "Donna Henry", "Daniel Henry", "Nancy Weaver", "Jimmy Weaver", "Jimmy Usry", "Hillary Clinton",
"Donald Trump", "Barack Obama", "Bernie Sanders", "Steve Balmer", "Bill Gates", "Steve Jobs", "Bill Clinton",
"Google", "Microsoft", "Apple", "Facebook", "Star Wars", "Samsung", "LG", "Motorola", "HTC", "Dr Pepper", "Coca-Cola",
"Pepsi", "Publix", "Wal-Mart","Target","Wendy's","McDonald's","Burger King","Star Wars", "Princess Leia",
"Luke Skywalker","Darth Vader","Anakin Skywalker","Count Dooku","Queen Amidala","Chancelor Palpatine",
"Emperor Palpatine","Han Solo","Ben Solo","Poe Dameron","Rey","Fin","Kylo Ren","The Republic",
"The Empire", "The Rebellion","The Force","The Darkside","Chewbaca","Chewie","Harry Potter","Ron Weasely",
"Hermione Granger","Lord Voldemort","Dobby","Hagrid","Snape","Dumbledore","Tom Riddle",
"Draco Malfoy","Neville Longbottom","Ginny Weaseley","Kia Optima","Nissan Frontier",
"Nissan Xterra", "Toyota Camry", "Nissan 350Z","The United States of America", "North America",
"Great Britain", "The United Kingdom", "England", "Europe", "France", "Germany", "Belgium",
"Russia", "USSR", "Spain", "Italy", "The Holy Roman Empire", "The Roman Empire", "The Roman Republic",
"The Eastern Roman Empire", "The Western Roman Empire", "The Catholic Church", "The Orthodox Church",
"Jesus", "Peter", "Paul", "Mark", "Luke", "John", "Matthew", "Doubting Thomas", "Judas"};

void ClientInfoListStressTest();
void ClientInfoListTest();
void AlgorithmTest();
void MemoryTest();

int main()
{
	MemoryTest();

	system("PAUSE");
}

void MemoryTest()
{
	using namespace std::chrono;
	srand(time(nullptr));

	auto begin = steady_clock::now();
	int irand = rand() % 10000 + 1;
	int jrand = rand() % 100000 + 1;

	cout << "Inner loop " << jrand << endl;
	cout << "Outter loop " << irand << endl;

	for (int i = 0; i < irand; i++)
	{
		for (int j = 0; j < jrand; j++)
		{
			char* buffer = new char[2024];
			delete[] buffer;
		}
	}
	auto end = steady_clock::now();
	cout << "Took " << duration_cast<seconds>(end - begin).count() << endl;
}
void ClientInfoListTest()
{
	ClientInfoList cil;
	srand(time(nullptr));
	NewConnectionInfo ci{};

	for (int i = 0; i < 10; i++)
	{
		cil.Add(std::move(ci), names[rand() % names.size()]);
	}
	for (int i = 0; i < cil.Size(); i++)
	{
		cout << cil[i].GetUsername() << endl;
	}
}
void AlgorithmTest()
{
	std::list<int> ints;
	srand(time(nullptr));
	
	const int NUM_ELEM = 10;
	const int RANGE = 100;
	for (int i = 0; i < NUM_ELEM; i++)
	{
		int num = rand() % RANGE + 1;
		if (ints.empty())
		{
			ints.push_back(num);
			continue;
		}
		auto it = ints.begin();
		while (it != ints.end())
		{
			if (num < *it)
			{
				ints.insert(it, num);
				break;
			}
			else if (num == *it)
			{
				break;
			}
			it++;
		}
	}
	for (int i : ints)
	{
		cout << i << endl;
	}
}

void ClientInfoListStressTest()
{
	ClientInfoList list;
	NewConnectionInfo ci{};
	srand(time(nullptr));
	int times = 0;

	system("PAUSE");

	while (times < 20)
	{
		std::chrono::steady_clock::time_point before = std::chrono::steady_clock::now();

		int numTimes = rand() % 10000 + 1;
		for (int i = 0; i < numTimes; i++)
		{
			int numNames = rand() % names.size() + 1;
			for (int j = 0; j < numNames; j++)
			{
				list.Add(std::move(ci), names[rand() % names.size()]);
			}
			list.RemoveAll();
			//cout << "Time after attempt #" << i + 1 << ": " << time(nullptr) << endl;
		}

		std::chrono::steady_clock::time_point after = std::chrono::steady_clock::now();

		cout << "After " << numTimes << " attempts: " << std::chrono::duration_cast<std::chrono::seconds>(after - before).count() << endl;
		times++;
	}
}