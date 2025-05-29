#pragma once
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <variant>
#include <vector>
#include <list>
using namespace std;

class Sintax
{
	Sintax();

	map <string, list<list<string>>> map_rules;

	void Rule_to_code(fstream file);
};