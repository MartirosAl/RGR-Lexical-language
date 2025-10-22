#pragma once
#include <iostream>
using namespace std; 
#include <list>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <stack>
#include "Lexical_Analyzer.h"

// Класс Sintax реализует синтаксический анализатор на основе лексического анализатора
class Sintax : protected TableToken
{
protected:

	//Пара: нетерминал -> список правил (каждое правило — отдельная пара)
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

	// Список нетерминалов грамматики
	vector<string> nonterminals = { "<S>", "<Ads>", "<ads>", "<Program>", "<Operation>", "<Assignment>", "<while>", "<for>", "<byE>", "<if>", "<input>", "<print>", "<label>", "<transition>", "<select>", "<case>", "<exception>", "<comment>", "<E>", "<T>", "<F>", "<Test>", "<TYPE>" };

	// Список терминалов грамматики
	vector<string> terminals = { "declare", ";", "V", "as", ",", "=", "while", "rel", "do", "od", "for", "from", "to", "by", "eps", "if", "(", ")", "else", "fi", "input", "print", "L", "goto", "select", "in", "ni", "case", "C", ":", "otherwise", "raise", "rem", "+", "-", "*", "/", "%", "get", "int", "bignumber" };

	// Список ключевых слов грамматики
	const vector<string> Keywords
	{
		"eps", "V", "C", "rel", "rem", "L"
	};

	// Структура для хранения элемента канонической таблицы LR-анализатора
	struct canonical_table
	{
		string nonterminal;    // Нетерминал
		int dot;               // Позиция точки в правиле
		vector<string> rule;     // Правило (список символов)
		vector<string> following;// Множество следующих символов (lookahead)

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

	// Структура для хранения информации о переходах (goto)
	struct for_goto
	{
		int number_table; // Номер таблицы откуда этот символ
		string symbol;    // Символ для переноса

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

	// Таблицы goto
	vector<auxiliary_table> сanonical_table_system;

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

			//Создание пустышек
			f.assign(t, "-"); 
			g.assign(t + nont - 1, -1); 
		}
	};

	struct tabular_analyzer
	{
		// | №        | f               | g                       |
		// |------------------------------------------------------|
		// |          | терминалы и eps | терминалы и нетерминалы |

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
	// Конструктор, принимает имя файла с грамматикой
	Sintax();

	// Деструктор
	~Sintax();

	// Выводит все правила грамматики
	void Print_Rules();

	void Write_Rules(ofstream& file);

	//Подразумевается, что у каждой таблицы будет один номер
	void Print_Сanonical_Table_System(const auxiliary_table& can_t);

	//Подразумевается, что у каждой таблицы будет один номер
	void Write_Сanonical_Table_System(const auxiliary_table& can_t, ofstream& file);

	// Выводит список всех нетерминалов
	void Print_Nonterminals();

	void Write_Nonterminals(ofstream& file);

	// Выводит список всех терминалов
	void Print_Terminals();

	void Write_Terminals(ofstream& file);

	void Print_GO_TO_args(list<for_goto> go_to_args);

	void Write_GO_TO_args(list<for_goto> go_to_args, ofstream& file);

	void Print_Firsts(vector<vector<string>> f);

	void Write_Firsts(vector<vector<string>> f, ofstream& file);

	void Write_Tabular_analyzer(Sintax::tabular_analyzer& TabAn, ofstream& file);

	void Print_Stack(stack<string> st);

	//Провераяем входящее на правильность грамматики по восходящему табличному анализатору
	bool Processing_incoming_code(const string file_name);

protected:

	// Сообщает об ошибке и завершает выполнение
	void Error(string error_text);

	// Создаёт вспомогательные таблицы для синтаксического анализа
	vector<auxiliary_table> Create_Tables();

	// Вычисляет множества FIRST для всех нетерминалов
	vector<vector<string>> All_FIRSTs();

	// Вычисляет множество FIRST для нетерминала (или терминала)
	vector<string> FIRST_One(string nonterminal, set<string> visited);

	// Удаление повторяющихся слов
	vector<string> Delete_Repetitions(vector<string> from);

	// Заменяет все eps правила на другие
	vector<string> Replacing_Eps(vector<string> to, vector<string> from);

	// Проверяет, является ли строка нетерминалом
	bool IsNonterminal(string s);

	// Проверяет, является ли строка терминалом
	bool IsTerminal(string s);

	// Форматирование таблицы под общий вид
	vector <Sintax::canonical_table> Formating_Table(vector<Sintax::canonical_table>& can_t);

	// Создает стартовую таблицу для синтаксического анализа
	vector<Sintax::canonical_table> Start_Table(string start_nonterminal);

	vector<Sintax::for_goto> Find_All_Goto(const vector<Sintax::canonical_table>& can_t, int number_table);

	vector<Sintax::canonical_table>GOTO(const for_goto& args, const vector<Sintax::canonical_table>& can_t);

	//Восходящий табличный анализатор
	Sintax::tabular_analyzer Tabular_analyzer(Sintax::tabular_analyzer& TabAn);

	//Распознаем подаваемое слово как вид терминала
	string Word_processing(string word);

	// Выдает номер термилала из таблицы терминалов
	int Terminal_number(string terminal);
	
	// Выдает номер нетермилала из таблицы нетерминалов
	int Nonterminal_number(string nonterminal);

	int find_by_key_begin(const vector<pair<string, vector<string>>>& vec, const string& key);
	int find_by_key_end(const vector<pair<string, vector<string>>>& vec, const string& key);

	bool IsCellFull(int pos1, int pos2, string content);

	int FindRuleInRow(Sintax::canonical_table rule);
	


};