#pragma once
#include <iostream>
using namespace std; 
#include <list>
#include <unordered_map>
#include <algorithm>
#include "Lexical_Analyzer.h"

// Класс Sintax реализует синтаксический анализатор на основе лексического анализатора
class Sintax : protected TableToken
{
protected:

	// Карта: нетерминал -> список правил (каждое правило — список строк)
	map <string, vector<vector<string>>> map_rules;

	// Список нетерминалов грамматики
	vector<string> nonterminals;

	// Список терминалов грамматики
	vector<string> terminals;

	// Список ключевых слов грамматики
	const vector<string> Keywords
	{
		"[eps]", "[V]", "[C]", "[rel]", "[rem]", "[L]"
	};

	// Структура для хранения элемента канонической таблицы LR-анализатора
	struct canonical_table
	{
		string nonterminal;    // Нетерминал
		int dot;               // Позиция точки в правиле
		vector<string> rule;     // Правило (список символов)
		vector<string> following;// Множество следующих символов (lookahead)
		int number_table;		 // Номер таблицы в которой она находится

		bool operator==(const canonical_table& other) const
		{
			return nonterminal == other.nonterminal && dot == other.dot && rule == other.rule && following == other.following;
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
		vector<canonical_table> table;
		for_goto goto_from;

		bool operator==(const auxiliary_table& other) const
		{
			return table == other.table && goto_from == other.goto_from;
		}
	};

	// Таблицы goto
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



public:
	// Конструктор, принимает имя файла с грамматикой
	Sintax();

	// Деструктор
	~Sintax();

	// Выводит все правила грамматики
	void Print_Rules();

	void Write_Rules(ofstream& file);

	//Подразумевается, что у каждой таблицы будет один номер
	void Print_Small_Table(const auxiliary_table& can_t);

	//Подразумевается, что у каждой таблицы будет один номер
	void Write_Small_Table(const auxiliary_table& can_t, ofstream& file);

	// Выводит список всех нетерминалов
	void Print_Nonterminals();

	void Write_Nonterminals(ofstream& file);

	// Выводит список всех терминалов
	void Print_Terminals();

	void Write_Terminals(ofstream& file);

	void Print_GO_TO_args(list<for_goto> go_to_args);

	void Write_GO_TO_args(list<for_goto> go_to_args, ofstream& file);

	void Print_Firsts(vector<vector<vector<string>>> f);

	void Write_Tabular_analyzer(Sintax::tabular_analyzer& TabAn, ofstream& file);

protected:






	// Читает правила из файла и заполняет map_rules, nonterminals, terminals
	void Rule_to_code(fstream& file);

	// Удаляет дубликаты eps в map_rules
	void Remove_Duplicate_Eps();

	// Сообщает об ошибке в правиле и завершает выполнение
	void Rule_Error(string error_text, fstream& file);

	// Сообщает об ошибке и завершает выполнение
	void Error(string error_text);

	// Находит все нетерминалы в файле
	void Find_Nonterminals(fstream& file);

	// Создаёт вспомогательные таблицы для синтаксического анализа
	vector<auxiliary_table> Create_Tables(string start_nonterminal_ = "<S>");

	// Вычисляет множество FIRST для нетерминала (или терминала)
	vector<string> FIRST_One(string nonterminal, set<string> visited);

	// Вычисляет множество FIRST для следующего элемента после текущего в правиле
	vector<vector<string>> FIRST_One_for_next(const vector<string>::const_iterator it, const vector<string>& r);

	// Декартово произведение двух списков строк
	vector<string> Cartesian_Product(vector<string> to, vector<string> from);

	// Обрезает каждый список в from до длины n
	vector<string> Clipping(int n, vector<string> from);

	// Удаление повторяющихся слов
	vector<string> Delete_Repetitions(vector<string> from);

	// Заменяет все eps правила на другие
	vector<string> Replacing_Eps(vector<string> to, vector<string> from);

	// Проверяет, является ли строка нетерминалом
	bool IsNonterminal(string s);

	// Проверяет, является ли строка терминалом
	bool IsTerminal(string s);

	// Проверяет, является ли строка ключевым словом
	bool IsKeyword(string s);

	// Форматирование таблицы под общий вид
	vector <Sintax::canonical_table> Formating_Table(vector<Sintax::canonical_table>& can_t);

	// Создает стартовую таблицу для синтаксического анализа
	vector<Sintax::canonical_table> Start_Table(string start_nonterminal);

	vector<Sintax::for_goto> Find_All_Goto(const vector<Sintax::canonical_table>& can_t);

	Sintax::for_goto Find_One_Goto(const Sintax::canonical_table& can_t);

	vector<Sintax::canonical_table>GOTO(const for_goto& args, const vector<Sintax::canonical_table>& can_t, int number_table);

	//Восходящий табличный анализатор
	Sintax::tabular_analyzer Tabular_analyzer(Sintax::tabular_analyzer& TabAn);


};