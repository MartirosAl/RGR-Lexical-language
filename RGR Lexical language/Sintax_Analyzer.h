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
protected:

	// �����: ���������� -> ������ ������ (������ ������� � ������ �����)
	map <string, vector<vector<string>>> map_rules;

	// ������ ������������ ����������
	vector<string> nonterminals;

	// ������ ���������� ����������
	vector<string> terminals;

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

		bool operator==(const canonical_table& other) const
		{
			return nonterminal == other.nonterminal && dot == other.dot && rule == other.rule && following == other.following;
		}
	};

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

	struct auxiliary_table
	{
		vector<canonical_table> table;
		for_goto goto_from;

		bool operator==(const auxiliary_table& other) const
		{
			return table == other.table && goto_from == other.goto_from;
		}
	};

	// ������� goto
	vector<auxiliary_table> tables;

	struct row_tabular_analyzer
	{
		int number_row;
		vector<string> f;
		vector<int> g;

		row_tabular_analyzer(int number)
		{
			number_row = number;
		}

		row_tabular_analyzer(int number, int t, int nont)
		{
			number_row = number;

			//�������� ��������
			f.assign(t, "-"); 
			g.assign(t + nont - 1, -1); 
		}
	};

	struct tabular_analyzer
	{
		// | �        | f               | g                       |
		// |------------------------------------------------------|
		// |          | ��������� � eps | ��������� � ����������� |

		vector<row_tabular_analyzer> rows;

		tabular_analyzer()
		{
			;
		}

		tabular_analyzer(int table, int t, int nont)
		{
			for (int i = 0; i < table; i++)
			{
				rows.push_back(row_tabular_analyzer(i, t, nont));
			}
		}
	};



public:
	// �����������, ��������� ��� ����� � �����������
	Sintax();

	// ����������
	~Sintax();

	// ������� ��� ������� ����������
	void Print_Rules();

	void Write_Rules(ofstream& file);

	//���������������, ��� � ������ ������� ����� ���� �����
	void Print_Small_Table(const auxiliary_table& can_t);

	//���������������, ��� � ������ ������� ����� ���� �����
	void Write_Small_Table(const auxiliary_table& can_t, ofstream& file);

	// ������� ������ ���� ������������
	void Print_Nonterminals();

	void Write_Nonterminals(ofstream& file);

	// ������� ������ ���� ����������
	void Print_Terminals();

	void Write_Terminals(ofstream& file);

	void Print_GO_TO_args(list<for_goto> go_to_args);

	void Write_GO_TO_args(list<for_goto> go_to_args, ofstream& file);

	void Print_Firsts(vector<vector<vector<string>>> f);

	void Write_Tabular_analyzer(Sintax::tabular_analyzer& TabAn, ofstream& file);

protected:






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
	vector<auxiliary_table> Create_Tables(string start_nonterminal_ = "<S>");

	// ��������� ��������� FIRST ��� ����������� (��� ���������)
	vector<string> FIRST_One(string nonterminal, set<string> visited);

	// ��������� ��������� FIRST ��� ���������� �������� ����� �������� � �������
	vector<vector<string>> FIRST_One_for_next(const vector<string>::const_iterator it, const vector<string>& r);

	// ��������� ������������ ���� ������� �����
	vector<string> Cartesian_Product(vector<string> to, vector<string> from);

	// �������� ������ ������ � from �� ����� n
	vector<string> Clipping(int n, vector<string> from);

	// �������� ������������� ����
	vector<string> Delete_Repetitions(vector<string> from);

	// �������� ��� eps ������� �� ������
	vector<string> Replacing_Eps(vector<string> to, vector<string> from);

	// ���������, �������� �� ������ ������������
	bool IsNonterminal(string s);

	// ���������, �������� �� ������ ����������
	bool IsTerminal(string s);

	// ���������, �������� �� ������ �������� ������
	bool IsKeyword(string s);

	// �������������� ������� ��� ����� ���
	vector <Sintax::canonical_table> Formating_Table(vector<Sintax::canonical_table>& can_t);

	// ������� ��������� ������� ��� ��������������� �������
	vector<Sintax::canonical_table> Start_Table(string start_nonterminal);

	vector<Sintax::for_goto> Find_All_Goto(const vector<Sintax::canonical_table>& can_t);

	Sintax::for_goto Find_One_Goto(const Sintax::canonical_table& can_t);

	vector<Sintax::canonical_table>GOTO(const for_goto& args, const vector<Sintax::canonical_table>& can_t, int number_table);

	//���������� ��������� ����������
	Sintax::tabular_analyzer Tabular_analyzer(Sintax::tabular_analyzer& TabAn);


};