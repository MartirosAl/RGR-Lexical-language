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
	/*00*/{"<S>", {"<Ads>", "<Program>"}},
	/*01*/{"<Ads>", {"declare", "<ads>", ";", "<Ads>"}},
	/*02*/{"<Ads>", {"eps"}},
	/*03*/{"<ads>", {"V", "as", "<TYPE>", ",", "<ads>"}},
	/*04*/{"<ads>", {"V", "as", "<TYPE>"}},
	/*05*/{"<Program>", {"<Operation>", "<Program>"}}, 
	/*06*/{"<Program>", {"eps"}},
	/*07*/{"<Operation>", {";"}},
	/*08*/{"<Operation>", {"<Assignment>"}},
	/*09*/{"<Operation>", {"<while>"}},
	/*10*/{"<Operation>", {"<for>"}},
	/*11*/{"<Operation>", {"<if>"}},
	/*12*/{"<Operation>", {"<input>"}},
	/*13*/{"<Operation>", {"<print>"}},
	/*14*/{"<Operation>", {"<label>"}},
	/*15*/{"<Operation>", {"<transition>"}},
	/*16*/{"<Operation>", {"<select>"}},
	/*17*/{"<Operation>", {"<exception>"}},
	/*18*/{"<Operation>", {"<comment>"}},
	/*19*/{"<Assignment>", {"V", "=", "<E>", ";"}},
	/*20*/{"<while>", {"while", "<E>", "rel", "<E>", "do", "<Program>", "od", ";"}},
	/*21*/{"<for>", {"for", "V", "from", "<E>", "to", "<E>", "<byE>", "do", "<Program>", "od", ";"}},
	/*22*/{"<byE>", {"by", "<E>"}},
	/*23*/{"<byE>", {"eps"}},
	/*24*/{"<if>", {"if", "(", "<Test>", ")", "<Program>", "else", "<Program>", "fi", ";"}},
	/*25*/{"<if>", {"if", "(", "<Test>", ")", "<Program>", "fi", ";"}},
	/*26*/{"<input>", {"input", "V", ";"}},
	/*27*/{"<print>", {"print", "<E>", ";"}},
	/*28*/{"<label>", {"L"}},
	/*29*/{"<transition>", {"gotoL", ";"}},
	/*30*/{"<select>", {"select", "<E>", "in", "<case>", "ni", ";"}},
	/*31*/{"<case>", {"case", "C", ":", "<Program>", "<case>"}},
	/*32*/{"<case>", {"case", "C", ":", "<Program>"}},
	/*33*/{"<case>", {"otherwise", ":", "<Program>"}},
	/*35*/{"<exception>", {"raise", ";"}},
	/*35*/{"<comment>", {"rem"}},
	/*36*/{"<E>", {"<E>", "+", "<T>"}},
	/*37*/{"<E>", {"<E>", "-", "<T>"}},
	/*38*/{"<E>", {"<T>"}},
	/*39*/{"<E>", {"(", "<E>", ")"}},
	/*40*/{"<T>", {"<T>", "*", "<F>"}},
	/*41*/{"<T>", {"<T>", "/", "<F>"}},
	/*42*/{"<T>", {"<T>", "%", "<F>"}},
	/*43*/{"<T>", {"<F>"}},
	/*44*/{"<F>", {"V"}},
	/*45*/{"<F>", {"C"}},
	/*46*/{"<F>", {"get", "(", "<E>", ",", "<E>", ")"}},
	/*47*/{"<Test>", {"<E>", "rel", "<E>"}},
	/*48*/{"<TYPE>", {"int"}},
	/*49*/{"<TYPE>", {"bignumber"}}

	};

	// Список нетерминалов грамматики
	vector<string> nonterminals = { "<S>", "<Ads>", "<ads>", "<Program>", "<Operation>", "<Assignment>", "<while>", "<for>", "<byE>", "<if>", "<input>", "<print>", "<label>", "<transition>", "<select>", "<case>", "<exception>", "<comment>", "<E>", "<T>", "<F>", "<Test>", "<TYPE>" };

	// Список терминалов грамматики eps = 14
	vector<string> terminals = { "declare", ";", "V", "as", ",", "=", "while", "rel", "do", "od", "for", "from", "to", "by", "eps", "if", "(", ")", "else", "fi", "input", "print", "L", "gotoL", "select", "in", "ni", "case", "C", ":", "otherwise", "raise", "rem", "+", "-", "*", "/", "%", "get", "int", "bignumber" };

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

	struct attribute_word
	{
		string word;
		vector<string> program;
		vector<string> labels;
		vector<set<variant<int, BigNumber>>::iterator> list_adress_constants;
		string end_label;
		int type; // int = 0, bignumber = 1
		string relation;
		list<string>::iterator adress_label;
		set<variant<int, BigNumber>>::iterator adress_const;
		set<variant<int, BigNumber>>::iterator adress_const2;
		map<string, variant<int, BigNumber>>::iterator adress_var;
		bool flag_otherwise = 0;
		int number_line; // Используется для обработки ошибок, чтобы выдавать номер строки

		attribute_word()
		{
			;
		}

		attribute_word(string word_)
		{
			word = word_;
		}

		bool operator==(const attribute_word& other) const
		{
			return word == other.word &&
				program == other.program &&
				labels == other.labels &&
				list_adress_constants == other.list_adress_constants &&
				end_label == other.end_label &&
				type == other.type &&
				relation == other.relation &&
				adress_const == other.adress_const &&
				adress_var == other.adress_var;
		}
	};

	vector<map<string, variant<int, BigNumber>>::iterator> declared_variables;
	list<pair<string, int>> List_tempered_labels;


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

	void Print_Stack(stack<attribute_word> st);

	//Переводим грамматику в стековый язык
	bool Translation_of_code(const string file_name, const string output_file_name);

	void Print_Stack_Program(const vector<string>& input);
	void Write_Stack_Program(const vector<string>& input, ofstream& file);

	void Print_Attribute_Stack(stack<attribute_word> input);
	void Write_Attribute_Stack(stack<attribute_word> input, ofstream& file);

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

	//Распознаем подаваемый токен как аттрибутное слово
	Sintax::attribute_word Token_processing(Token token_);

	// Выдает номер термилала из таблицы терминалов
	int Terminal_number(string terminal);

	// Выдает номер нетермилала из таблицы нетерминалов
	int Nonterminal_number(string nonterminal);

	int find_by_key_begin(const vector<pair<string, vector<string>>>& vec, const string& key);
	int find_by_key_end(const vector<pair<string, vector<string>>>& vec, const string& key);

	bool IsCellFull(int pos1, int pos2, string content);

	int FindRuleInRow(Sintax::canonical_table rule);

	bool S_more_I(string a);

	attribute_word Grouping_by_rule(stack<attribute_word>& attribute_stack, int number_rule);

	stack<attribute_word> del_rule(stack<attribute_word>& attribute_stack, int number_rule);

	stack<attribute_word> del_n_elements(stack<attribute_word>& attribute_stack, int n);

	int Count_rows_until_nonterminals(stack<attribute_word> attribute_stack);

	string NewLabel();

	vector<string> Creating_transitions_by_label(vector<string> stack_program);
};