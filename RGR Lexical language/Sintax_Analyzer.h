#pragma once
#include <list>
#include <unordered_map>
#include "Lexical_Analyzer.h"

class Sintax : protected TableToken
{
public:
	Sintax(string file_name);

	void Print_Rules();

	void Print_Nonterminals();

	void Print_Terminals();

protected:
	map <string, list<list<string>>> map_rules;


	list<string> nonterminals;
	list<string> terminals;

	void Rule_to_code(fstream& file);

	void Rule_Error(string error_text, fstream& file);

	void Error(string error_text);

	void Find_Nonterminals(fstream& file);

	void Create_Tables();

	list<list<string>> FIRST(int k, string nonterminal);

	list<list<string>> Cartesian_Product(list<list<string>> to, list<list<string>> from);

	list<list<string>> Clipping(int n, list<list<string>> from);

	bool IsNonterminal(string s);

	bool IsTerminal(string s);

	bool IsKeyword(string s);

	const vector<string> Keywords
	{
		"[eps]", "[V]", "[C]", "[rel]", "[rem]", "[L]"
	};

	struct canonical_table
	{
		string nonterminal;
		int dot;
		list<string> rule;
		list<string> following;
	};

	list<list<canonical_table>> canonical_table_system;

	struct for_goto
	{
		string nonterminal;
		string next_dot;
	};

	list<canonical_table> Start_Table(string start_nonterminal, list<list<list<string>>> Firsts);


};