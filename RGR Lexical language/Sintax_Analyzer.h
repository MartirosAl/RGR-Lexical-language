#pragma once
#include <iostream>
using namespace std; 
#include <list>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <stack>
#include "Lexical_Analyzer.h"

// ����� Sintax ��������� �������������� ���������� �� ������ ������������ �����������
class Sintax : protected TableToken
{
protected:

	//����: ���������� -> ������ ������ (������ ������� � ��������� ����)
	vector<pair<string, vector<string>>> vec_rules = {
	{"<S>", {"<Ads>", "<Program>"}},
	{"<Ads>", {"declare", "<ads>", ";", "<Ads>"}},
	{"<Ads>", {"eps"}},
	{"<ads>", {"V", "as", "<TYPE>", ",", "<ads>"}},
	{"<ads>", {"V", "as", "<TYPE>", ";"}},
	{"<Program>", {"<Operation>", "<Program>"}},
	{"<Program>", {"eps"}},
	{"<Operation>", {";"}},
	{"<Operation>", {"<Assignment>"}},
	{"<Operation>", {"<while>"}},
	{"<Operation>", {"<for>"}},
	{"<Operation>", {"<if>"}},
	{"<Operation>", {"<input>"}},
	{"<Operation>", {"<print>"}},
	{"<Operation>", {"<label>"}},
	{"<Operation>", {"<transition>"}},
	{"<Operation>", {"<select>"}},
	{"<Operation>", {"<exception>"}},
	{"<Operation>", {"<comment>"}},
	{"<Assignment>", {"V", "=", "<E>", ";"}},
	{"<while>", {"while", "<E>", "rel", "<E>", "do", "<Program>", "od", ";"}},
	{"<for>", {"for", "V", "from", "<E>", "to", "<E>", "<byE>", "do", "<Program>", "od", ";"}},
	{"<byE>", {"by", "<E>"}},
	{"<byE>", {"eps"}},
	{"<if>", {"if", "(", "<Test>", ")", "<Program>", "else", "<Program>", "fi", ";"}},
	{"<if>", {"if", "(", "<Test>", ")", "<Program>", "fi", ";"}},
	{"<input>", {"input", ";"}},
	{"<print>", {"print", "<E>", ";"}},
	{"<label>", {"L"}},
	{"<transition>", {"goto", "L", ";"}},
	{"<select>", {"select", "<E>", "in", "<case>", "ni", ";"}},
	{"<case>", {"case", "C", ":", "<Program>", "<case>"}},
	{"<case>", {"case", "C", ":", "<Program>"}},
	{"<case>", {"otherwise", ":", "<Program>"}},
	{"<exception>", {"raise", ";"}},
	{"<comment>", {"rem"}},
	{"<E>", {"<E>", "+", "<T>"}},
	{"<E>", {"<E>", "-", "<T>"}},
	{"<E>", {"<T>"}},
	{"<E>", {"(", "<E>", ")"}},
	{"<T>", {"<T>", "*", "<F>"}},
	{"<T>", {"<T>", "/", "<F>"}},
	{"<T>", {"<T>", "%", "<F>"}},
	{"<T>", {"<F>"}},
	{"<F>", {"V"}},
	{"<F>", {"C"}},
	{"<F>", {"get", "(", "<E>", ",", "<E>", ")"}},
	{"<Test>", {"<E>", "rel", "<E>"}},
	{"<TYPE>", {"int"}},
	{"<TYPE>", {"bignumber"}}

	};

	// ������ ������������ ����������
	vector<string> nonterminals = { "<S>", "<Ads>", "<ads>", "<Program>", "<Operation>", "<Assignment>", "<while>", "<for>", "<byE>", "<if>", "<input>", "<print>", "<label>", "<transition>", "<select>", "<case>", "<exception>", "<comment>", "<E>", "<T>", "<F>", "<Test>", "<TYPE>" };

	// ������ ���������� ����������
	vector<string> terminals = { "declare", ";", "V", "as", ",", "=", "while", "rel", "do", "od", "for", "from", "to", "by", "eps", "if", "(", ")", "else", "fi", "input", "print", "L", "goto", "select", "in", "ni", "case", "C", ":", "otherwise", "raise", "rem", "+", "-", "*", "/", "%", "get", "int", "bignumber" };

	// ������ �������� ���� ����������
	const vector<string> Keywords
	{
		"eps", "V", "C", "rel", "rem", "L"
	};

	// ��������� ��� �������� �������� ������������ ������� LR-�����������
	struct canonical_table
	{
		string nonterminal;    // ����������
		int dot;               // ������� ����� � �������
		vector<string> rule;     // ������� (������ ��������)
		vector<string> following;// ��������� ��������� �������� (lookahead)

		bool operator==(const canonical_table& other) const
		{
			if (other.following.size() != following.size())
				return false;
			for (size_t i = 0; i < following.size(); i++)
			{
				if (find(other.following.begin(), other.following.end(), following[i]) == other.following.end())
					return false;
			}
			return nonterminal == other.nonterminal &&
				dot == other.dot &&
				rule == other.rule;
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
		vector<canonical_table> rules;
		for_goto goto_from;
		int number_table;

		bool operator==(const auxiliary_table& other) const
		{
			return rules == other.rules && goto_from == other.goto_from;
		}
	};

	// ������� goto
	vector<auxiliary_table> �anonical_table_system;

	vector<auxiliary_table> not_included_tables;

	vector<vector<string>> Firsts;

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

	tabular_analyzer TabAn;


public:
	// �����������, ��������� ��� ����� � �����������
	Sintax();

	// ����������
	~Sintax();

	// ������� ��� ������� ����������
	void Print_Rules();

	void Write_Rules(ofstream& file);

	//���������������, ��� � ������ ������� ����� ���� �����
	void Print_�anonical_Table_System(const auxiliary_table& can_t);

	//���������������, ��� � ������ ������� ����� ���� �����
	void Write_�anonical_Table_System(const auxiliary_table& can_t, ofstream& file);

	// ������� ������ ���� ������������
	void Print_Nonterminals();

	void Write_Nonterminals(ofstream& file);

	// ������� ������ ���� ����������
	void Print_Terminals();

	void Write_Terminals(ofstream& file);

	void Print_GO_TO_args(list<for_goto> go_to_args);

	void Write_GO_TO_args(list<for_goto> go_to_args, ofstream& file);

	void Print_Firsts(vector<vector<string>> f);

	void Write_Firsts(vector<vector<string>> f, ofstream& file);

	void Write_Tabular_analyzer(Sintax::tabular_analyzer& TabAn, ofstream& file);

	void Print_Stack(stack<string> st);

	//���������� �������� �� ������������ ���������� �� ����������� ���������� �����������
	bool Processing_incoming_code(const string file_name);

protected:

	// �������� �� ������ � ��������� ����������
	void Error(string error_text);

	// ������ ��������������� ������� ��� ��������������� �������
	vector<auxiliary_table> Create_Tables();

	// ��������� ��������� FIRST ��� ���� ������������
	vector<vector<string>> All_FIRSTs();

	// ��������� ��������� FIRST ��� ����������� (��� ���������)
	vector<string> FIRST_One(string nonterminal, set<string> visited);

	// �������� ������������� ����
	vector<string> Delete_Repetitions(vector<string> from);

	// �������� ��� eps ������� �� ������
	vector<string> Replacing_Eps(vector<string> to, vector<string> from);

	// ���������, �������� �� ������ ������������
	bool IsNonterminal(string s);

	// ���������, �������� �� ������ ����������
	bool IsTerminal(string s);

	// �������������� ������� ��� ����� ���
	vector <Sintax::canonical_table> Formating_Table(vector<Sintax::canonical_table>& can_t);

	// ������� ��������� ������� ��� ��������������� �������
	vector<Sintax::canonical_table> Start_Table(string start_nonterminal);

	vector<Sintax::for_goto> Find_All_Goto(const vector<Sintax::canonical_table>& can_t, int number_table);

	vector<Sintax::canonical_table>GOTO(const for_goto& args, const vector<Sintax::canonical_table>& can_t);

	//���������� ��������� ����������
	Sintax::tabular_analyzer Tabular_analyzer(Sintax::tabular_analyzer& TabAn);

	//���������� ���������� ����� ��� ��� ���������
	string Word_processing(string word);

	// ������ ����� ��������� �� ������� ����������
	int Terminal_number(string terminal);
	
	// ������ ����� ����������� �� ������� ������������
	int Nonterminal_number(string nonterminal);

	int find_by_key_begin(const vector<pair<string, vector<string>>>& vec, const string& key);
	int find_by_key_end(const vector<pair<string, vector<string>>>& vec, const string& key);

	bool IsCellFull(int pos1, int pos2, string content);

	int FindRuleInRow(Sintax::canonical_table rule);
	


};