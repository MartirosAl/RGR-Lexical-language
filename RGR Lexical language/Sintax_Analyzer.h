#pragma once
#include <iostream>
using namespace std; 
#include <list>
#include <unordered_map>
#include <algorithm>
#include "Lexical_Analyzer.h"

// ����� Sintax ��������� �������������� ���������� �� ������ ������������ �����������
class Sintax : protected TableToken
{
public:
	// �����������, ��������� ��� ����� � �����������
	Sintax(string file_name);

	// ������� ��� ������� ����������
	void Print_Rules();

	// ������� ������ ���� ������������
	void Print_Nonterminals();

	// ������� ������ ���� ����������
	void Print_Terminals();

protected:
	// �����: ���������� -> ������ ������ (������ ������� � ������ �����)
	map <string, vector<vector<string>>> map_rules;

	// ������ ������������ ����������
	vector<string> nonterminals;

	// ������ ���������� ����������
	vector<string> terminals;

	// ������ ������� �� ����� � ��������� map_rules, nonterminals, terminals
	void Rule_to_code(fstream& file);

	// ������� ��������� eps � map_rules
	void Remove_Duplicate_Eps();

	// �������� �� ������ � ������� � ��������� ����������
	void Rule_Error(string error_text, fstream& file);

	// �������� �� ������ � ��������� ����������
	void Error(string error_text);

	// ������� ��� ����������� � �����
	void Find_Nonterminals(fstream& file);

	// ������ ��������������� ������� ��� ��������������� �������
	void Create_Tables();

	// ��������� ��������� FIRST ��� ����������� (��� ���������)
	vector<vector<string>> FIRST_One(string nonterminal, set<string> visited);

	// ��������� ��������� FIRST ��� ���������� �������� ����� �������� � �������
	vector<vector<string>> FIRST_One_for_next(vector<string>::iterator t, const vector<string>& r);

	void Print_Firsts(vector<vector<vector<string>>> f);

	// ��������� ������������ ���� ������� ������� �����
	vector<vector<string>> Cartesian_Product(vector<vector<string>> to, vector<vector<string>> from);

	// �������� ������ ������ � from �� ����� n
	vector<vector<string>> Clipping(int n, vector<vector<string>> from);

	// ���������, �������� �� ������ ������������
	bool IsNonterminal(string s);

	// ���������, �������� �� ������ ����������
	bool IsTerminal(string s);

	// ���������, �������� �� ������ �������� ������
	bool IsKeyword(string s);

	// ������ �������� ���� ����������
	const vector<string> Keywords
	{
		"[eps]", "[V]", "[C]", "[rel]", "[rem]", "[L]"
	};

	// ��������� ��� �������� �������� ������������ ������� LR-�����������
	struct canonical_table
	{
		string nonterminal;    // ����������
		int dot;               // ������� ����� � �������
		vector<string> rule;     // ������� (������ ��������)
		vector<string> following;// ��������� ��������� �������� (lookahead)
		int number_table;		 // ����� ������� � ������� ��� ���������
	};

	// ������� ������������ ������ ��� ��������������� �������
	vector<vector<canonical_table>> canonical_table_system;

	// ��������� ��� �������� ���������� � ��������� (goto)
	struct for_goto
	{
		int number_table; // ����� ������� ������ ���� ������
		string symbol;    // ������ ��� ��������

		bool operator==(const for_goto& other) const 
		{
			return number_table == other.number_table && symbol == other.symbol;
		}
	};

	// ��������� ��������� ������� ��� ��������������� �������
	vector<canonical_table> Start_Table(string start_nonterminal);

	vector<Sintax::for_goto> Find_All_Goto(const vector<canonical_table>& can_t);

	Sintax::for_goto Find_One_Goto(const canonical_table& can_t);

	void Print_Canonical_Table(const vector<canonical_table>& can_t);

	vector<canonical_table>GOTO(const for_goto& args, const vector<canonical_table>& can_t, int number_table);
};