#include "Sintax_Analyzer.h"

// Конструктор класса Sintax
Sintax::Sintax()
{
	ofstream File_for_writing("File for writing.txt");
	if (!File_for_writing.is_open())
	{
		cerr << "Error opening file: File for writing.txt" << endl;
		return;
	}
	File_for_writing.clear();

	//Print_Rules();
	Write_Rules(File_for_writing);
	File_for_writing << endl;

	//Создание всех First множеств
	Firsts = All_FIRSTs();

	Write_Firsts(Firsts, File_for_writing);

	Write_Nonterminals(File_for_writing);
	File_for_writing << endl;

	Write_Terminals(File_for_writing);
	File_for_writing << endl;

	// Создание вспомогательных таблиц для синтаксического анализа
	сanonical_table_system = Create_Tables();

	for (auto& i : сanonical_table_system)
	{
		Write_Сanonical_Table_System(i, File_for_writing);
	}
	File_for_writing << endl;

	File_for_writing << "Not included tables:" << endl;
	for (auto& i : not_included_tables)
	{
		File_for_writing << "A" << i.number_table << " = GOTO(A" << i.goto_from.number_table << "," << i.goto_from.symbol << ")" << endl;
	}
	File_for_writing << endl;

	// Инициализация табличного анализатора
	TabAn = tabular_analyzer((int)сanonical_table_system.size(), (int)terminals.size(), (int)nonterminals.size());

	try 
	{
		Tabular_analyzer(TabAn);
	}
	catch (string a) 
	{
		cerr << "Exception: " << a << endl;
		return;
	}

	Write_Tabular_analyzer(TabAn, File_for_writing);

	File_for_writing.close();
}

Sintax::~Sintax()
{
	vec_rules.clear();
	nonterminals.clear();
	terminals.clear();
}

// Сообщение об ошибке и завершение работы
void Sintax::Error(string error_text)
{
	cerr << "Error: " << error_text << endl;
	exit(EXIT_FAILURE);
}

// Создание вспомогательных таблиц для синтаксического анализа
vector<Sintax::auxiliary_table> Sintax::Create_Tables()
{
	string start_nonterminal = "<S>";
	vector<auxiliary_table> rules;
	vector<canonical_table> start_table;
	list<for_goto> table_goto_args;

	// Формирование стартовой таблицы
	start_table = Start_Table(start_nonterminal);

	rules.push_back(auxiliary_table(start_table, for_goto(-1, start_nonterminal), 0));

	// Поиск всех нетерминалов, следующих за точкой в стартовой таблице
	for (auto& i : Find_All_Goto(start_table, 0))
	{
		if (find(table_goto_args.begin(), table_goto_args.end(), i) == table_goto_args.end())
		{
			table_goto_args.push_back(i);
		}
	}

	// Основной цикл по созданию всех таблиц
	for (auto& i : table_goto_args)
	{
		vector<canonical_table> temp_vct;
		canonical_table temp_ct;

		// Пустые правила не трогаем
		if (i.symbol == "eps")
			continue;

		// Вычисление GOTO для текущего аргумента
		vector<canonical_table> goto_result = GOTO(i, rules[i.number_table].rules);

		// Проверка на наличие уже существующей таблицы с таким же содержимым
		auto find_b = find_if(
			rules.begin(),
			rules.end(),
			[&goto_result](const auxiliary_table& a) 
			{
				return a.rules == goto_result;
			});

		// Если такая таблица уже существует, пропускаем её
		if (find_b != rules.end())
		{
			not_included_tables.push_back(auxiliary_table(goto_result, i, (*find_b).number_table));
			continue;
		}
		// Если не существует, создаём новую таблицу
		rules.push_back(auxiliary_table(goto_result, i, rules.size()));
		

		// Высчитываение аргументов goto для последующих таблиц 
		list<canonical_table> result_list(goto_result.begin(), goto_result.end());
		for (auto& j : result_list)
		{
			if (j.rule.size() > j.dot)
			{
				if (find(table_goto_args.begin(), table_goto_args.end(), for_goto(rules.size() - 1, j.rule[j.dot])) == table_goto_args.end())
				{
					table_goto_args.push_back(for_goto(rules.size() - 1, j.rule[j.dot]));
				}
			}
			
		}
	}

	//Замена все пустышки в following на "eps"
	for (int i = 0; i < rules.size(); i++)
	{
		for (int j = 0; j < rules[i].rules.size(); j++)
		{
			for (int t = 0; t < rules[i].rules[j].following.size(); t++)
			{
				if (rules[i].rules[j].following[t][0] == '\0')
					rules[i].rules[j].following[t] = "eps";
			}
		}
	}

	return rules;
}

vector<vector<string>> Sintax::All_FIRSTs()
{
	vector<vector<string>> result;
	for (auto& i : nonterminals)
	{
		vector<string> first_nont = FIRST_One(i, {});
		result.push_back(first_nont);
	}
	
	return result;
}

// Вычисление множества FIRST для нетерминала
vector<string> Sintax::FIRST_One(string nonterminal, set<string> visited)
{
	// Если уже были в этом нетерминале на этом пути — предотвращаем зацикливание
	if (visited.count(nonterminal))
		return {};

	visited.insert(nonterminal);

	vector<string> res;
	string words;

	if (vec_rules.size() == 0)
		Error("There are no rules");

	int temp_index = find_by_key_begin(vec_rules, nonterminal);
	for (int i = temp_index; i < find_by_key_end(vec_rules, nonterminal); i++)
	{
		words.clear();
		// Предпологаем, что лишние eps удалены 
		if (vec_rules[i].second.size() == 1 && vec_rules[i].second[0] == "eps")
			res.push_back({ "eps" });
		for (int j = 0; j < vec_rules[i].second.size(); j++)
		{
			if (IsTerminal(vec_rules[i].second[j]))
			{
				if (vec_rules[i].second[j] == "eps")
					continue;
				words = vec_rules[i].second[j];
			}
			else
			{
				// Передаём visited дальше!
				vector<string> temp = FIRST_One(vec_rules[i].second[j], visited);

				for (auto& v : temp)
					res.push_back(v);

				words.clear();
				break;
			}

			if (!words.empty())
			{
				res.push_back(words);
				break;
			}
		}		
	}

	// Удаление повторов в res
	res = Delete_Repetitions(res);

	return res;
}

void Sintax::Print_Firsts(vector<vector<string>> f)
{
	int count = 0;
	for (auto& i : f)
	{
		int temp_counter = 0;
		cout << "FIRST(" << nonterminals[count++] << ") = {";
		for (auto& j : i)
		{
			if (temp_counter++ < i.size() - 1)
				cout << j << ", ";
			else
				cout << j;
		}
		cout << "}" << endl;
	}
	cout << endl;
}

void Sintax::Write_Firsts(vector<vector<string>> f, ofstream& file)
{
	int count = 0;
	for (auto& i : f)
	{
		int temp_counter = 0;
		file << "FIRST(" << nonterminals[count++] << ") = {";
		for (auto& j : i)
		{
			if (temp_counter++ < i.size() - 1)
				file << j << ", ";
			else
				file << j;
		}
		file << "}" << endl;
	}
	file << endl;
}

void Sintax::Write_Tabular_analyzer(Sintax::tabular_analyzer& TabAn, ofstream& file)
{
	int min_size = 3;

	file << "   |f";
	for (int i = 0; i < terminals.size() - 1; i++)
	{
		for (int j = 0; j < (terminals[i].size() > min_size ? terminals[i].size() : min_size); j++)
			file << " ";
		file << " ";
	}
	for (int j = 0; j < (terminals[terminals.size() - 1].size() > min_size ? terminals[terminals.size() - 1].size() : min_size); j++)
		file << " ";
	file << "||g" << endl;

	file << "   |";


	for (int i = 0; i < terminals.size(); i++)
	{
		file << terminals[i];
		for (int j = terminals[i].size(); j < min_size; j++)
			file << " ";
		file << "|";
	}

	file << "||";

	for (int i = 0; i < nonterminals.size(); i++)
	{
		file << nonterminals[i];
		for (int j = nonterminals[i].size(); j < min_size; j++)
			file << " ";
		file << "|";
	}

	for (int i = 0; i < terminals.size(); i++)
	{
		if (terminals[i] == "eps")
			continue;
		file << terminals[i];
		for (int j = terminals[i].size(); j < min_size; j++)
			file << " ";
		file << "|";
	}

	file << endl;

	for (int i = 0; i < TabAn.rows.size(); i++)
	{
		//Номер строки
		file << TabAn.rows[i].number_row;
		for (int v = to_string(TabAn.rows[i].number_row).size(); v < min_size; v++)
			file << " ";
		file << "|";

		//f
		for (int j = 0; j < TabAn.rows[i].f.size(); j++)
		{
			file << TabAn.rows[i].f[j];
			int spaces1 = terminals[j].size() > min_size ? terminals[j].size() : min_size;
			for (int v = TabAn.rows[i].f[j].size(); v < spaces1; v++)
				file << " ";
			file << "|";
		}

		file << "||";

		//Нетерминалы g
		for (int j = 0; j < nonterminals.size(); j++)
		{
			if (TabAn.rows[i].g[j] == -1)
				file << "- ";
			else
				file << TabAn.rows[i].g[j];
			int spaces2 = nonterminals[j].size() > min_size ? nonterminals[j].size() : min_size;
			for (int v = to_string(TabAn.rows[i].g[j]).size(); v < spaces2; v++)
				file << " ";
			file << "|";
		}

		//Терминалы g (без eps)
		for (int j = nonterminals.size(); j < (nonterminals.size() + terminals.size() - 1) /*без эпс*/; j++)
		{
			if (TabAn.rows[i].g[j] == -1)
				file << "- ";
			else
				file << TabAn.rows[i].g[j];
			int spaces3 = terminals[j - nonterminals.size() + (j - nonterminals.size() >= Terminal_number("eps") ? 1 : 0)].size() > min_size ? terminals[j - nonterminals.size() + (j - nonterminals.size() >= Terminal_number("eps") ? 1 : 0)].size() : min_size;
			for (int v = to_string(TabAn.rows[i].g[j]).size(); v < spaces3; v++)
				file << " ";
			file << "|";
		}
		file << endl;
	}
}

void Sintax::Print_Stack(stack<string> st)
{
	stack<string> temp_st = st;
	while (!temp_st.empty())
	{
		cout << temp_st.top() << " ";
		temp_st.pop();
	}
}

vector<string> Sintax::Delete_Repetitions(vector<string> from)
{
	std::vector<string> res;
	for (const auto& elem : from) {
		if (find(res.begin(), res.end(), elem) == res.end())
			res.push_back(elem);
	}
	return res;
}

vector<string> Sintax::Replacing_Eps(vector<string> to, vector<string> from)
{
	vector<string> res = to;

	if (from.empty())
		return res;

	if (to.empty())
		return from;

	auto it = from.begin();

	for (auto& i : res)
	{
		if (i == "eps")
		{
			i = *it;
			it++;

			while (it != from.end())
			{
				res.push_back(*it);
				it++;
			}
			break;
		}
	}

	return res;

}

// Проверка, является ли строка нетерминалом
bool Sintax::IsNonterminal(string s)
{
	if (find(nonterminals.begin(), nonterminals.end(), s) != nonterminals.end())
		return true;
	return false;
}

// Проверка, является ли строка терминалом
bool Sintax::IsTerminal(string s)
{
	if (find(terminals.begin(), terminals.end(), s) != terminals.end())
		return true;
	return false;
}

vector<Sintax::canonical_table> Sintax::Formating_Table(vector<canonical_table>& can_t)
{
	// Удаление дубликатов из can_t
	vector<canonical_table> unique_can_t;
	for (const auto& elem : can_t) {
		if (find(unique_can_t.begin(), unique_can_t.end(), elem) == unique_can_t.end())
			unique_can_t.push_back(elem);
	}
	can_t = move(unique_can_t);

	// Объединение правил с разными following в одно правило
	list<canonical_table> list_can_t;
	for (auto& j : can_t)
	{
		list_can_t.push_back(j);
	}
	can_t.clear();

	for (auto it1 = list_can_t.begin(); it1 != list_can_t.end(); )
	{
		auto it2 = std::next(it1);

		while (it2 != list_can_t.end())
		{
			if (it1->nonterminal == it2->nonterminal &&
				it1->rule == it2->rule &&
				it1->dot == it2->dot)
			{
				// Объединяем following
				it1->following.insert(it1->following.end(), it2->following.begin(), it2->following.end());

				// Удаляем дубликаты
				sort(it1->following.begin(), it1->following.end());
				it1->following.erase(std::unique(it1->following.begin(), it1->following.end()), it1->following.end());

				// Удаляем it2 и продолжаем
				it2 = list_can_t.erase(it2);
			}
			else
			{
				++it2;
			}
		}

		can_t.push_back(*it1);
		it1 = list_can_t.erase(it1);
	}

	list_can_t.clear();

	// Сортировка can_t по нетерминалу и правилу
	//for (int j = can_t.size() - 1; j > 0; j--)
	//{
	//	bool swapped = false;

	//	for (int k = 0; k < j; k++)
	//	{
	//		if (can_t[k].nonterminal > can_t[k + 1].nonterminal || can_t[k].nonterminal == can_t[k + 1].nonterminal && can_t[k].rule > can_t[k + 1].rule)
	//		{
	//			swap(can_t[k], can_t[k + 1]);
	//			swapped = true;
	//		}
	//	}

	//	if (swapped == false)
	//		break;
	//}

	return can_t;
}

// Формирование стартовой таблицы для синтаксического анализа
vector<Sintax::canonical_table> Sintax::Start_Table(string start_nonterminal)
{
	list<canonical_table> temp_res;
	vector<canonical_table> res;

	for (int i = find_by_key_begin(vec_rules, start_nonterminal); i < find_by_key_end(vec_rules, start_nonterminal); i++)
	{
		temp_res.push_back(canonical_table(start_nonterminal, 0, vec_rules[i].second, { "eps" }));
	}

	for (auto& i : temp_res)
	{
		if (i.rule.size() == 0)
			continue;
		auto t = i.rule.begin();

		if (IsNonterminal(*t))
		{
			for (int j = find_by_key_begin(vec_rules, *t); j < find_by_key_end(vec_rules, *t); j++)
			{
				if (i.rule.size() == 0)
					break;
				temp_res.push_back(canonical_table((*t), 0, vec_rules[j].second, Firsts[Nonterminal_number(*(t + 1))]));
				
			}
		}
	}

	for (auto& i : temp_res)
	{
		res.push_back(i);
	}

	return Formating_Table(res);
}

vector<Sintax::for_goto> Sintax::Find_All_Goto(const vector<canonical_table>& can_t, int number_table)
{
	vector<for_goto> res;

	for (auto& i : can_t)
	{
		if (i.rule.size() > i.dot)
			res.push_back(for_goto(number_table, i.rule[i.dot]));
	}

	return res;
}

void Sintax::Print_Сanonical_Table_System(const auxiliary_table& can_t)
{
	
	cout << "A" << can_t.number_table << " = GOTO (" << "A" << can_t.goto_from.number_table << "," << can_t.goto_from.symbol << ")" << " = {" << endl;
	for (auto& i : can_t.rules)
	{

		cout << "[" << i.nonterminal << " -> ";
		int counter = 0;
		for (auto& j : i.rule)
		{
			if (counter++ == i.dot)
				cout << ". ";
			cout << j << " ";
		}
		if (counter == i.dot)
			cout << ". ";
		cout << ", ";
		for (auto& j : i.following)
		{
			if (j != *i.following.begin())
				cout << " | ";
			cout << j;
		}
		cout << "]" << endl;
	}
	cout << "}" << endl << endl;
}

void Sintax::Write_Сanonical_Table_System(const auxiliary_table& can_t, ofstream& file)
{
	if (can_t.goto_from.number_table != -1)
		file << "A" << can_t.number_table << " = GOTO (" << "A" << can_t.goto_from.number_table << "," << can_t.goto_from.symbol << ")" << " = {" << endl;
	else
		file << "A" << can_t.number_table << " = {" << endl;
	for (auto& i : can_t.rules)
	{

		file << "[" << i.nonterminal << " -> ";
		int counter = 0;
		for (auto& j : i.rule)
		{
			if (counter++ == i.dot)
				file << ". ";
			file << j << " ";
		}
		if (counter == i.dot)
			file << ". ";
		file << ", ";
		for (auto& j : i.following)
		{
			if (j != *i.following.begin())
				file << " | ";
			file << j;
		}
		file << "]" << endl;
	}
	file << "}" << endl << endl;
}

// Реализация функции GOTO для LR(1)-анализатора
vector<Sintax::canonical_table> Sintax::GOTO(const for_goto& args, const vector<canonical_table>& can_t)
{
	vector<canonical_table> res;
	list<canonical_table> queue;

	// Сдвигаем точку на символ args.symbol
	for (const auto& item : can_t)
	{
		if (item.dot < item.rule.size() && item.rule[item.dot] == args.symbol)
		{
			canonical_table moved = item;
			moved.dot++;
			if (find(res.begin(), res.end(), moved) == res.end())
			{
				res.push_back(moved);
				queue.push_back(moved);
			}
		}
	}

	// Closure
	while (!queue.empty())
	{
		canonical_table current = queue.front();
		queue.pop_front();

		if (current.dot < current.rule.size() && IsNonterminal(current.rule[current.dot]))
		{
			string B = current.rule[current.dot];

			// Вычисляем lookahead для новых правил B
			vector<string> lookahead;
			if (current.dot + 1 < current.rule.size())
			{
				// β = символы после B
				vector<string> beta(current.rule.begin() + current.dot + 1, current.rule.end());

				// FIRST(β)
				for (const auto& sym : beta)
				{
					if (IsTerminal(sym))
					{
						lookahead.push_back(sym);
						break;
					}
					else
					{
						for (const auto& x : Firsts[Nonterminal_number(sym)])
							lookahead.push_back(x);

						// если FIRST(sym) не содержит eps, дальше не идём
						if (find(Firsts[Nonterminal_number(sym)].begin(),
							Firsts[Nonterminal_number(sym)].end(),
							"eps") == Firsts[Nonterminal_number(sym)].end())
							break;
					}
				}

				// Если β может породить eps, добавляем lookahead из текущего
				if (find(lookahead.begin(), lookahead.end(), "eps") != lookahead.end())
				{
					lookahead.erase(remove(lookahead.begin(), lookahead.end(), "eps"), lookahead.end());
					lookahead.insert(lookahead.end(), current.following.begin(), current.following.end());
				}
			}
			else
			{
				// Если после B ничего нет, lookahead = current.following
				lookahead = current.following;
			}

			// Удаляем дубликаты
			lookahead = Delete_Repetitions(lookahead);

			// Добавляем все правила вида B → γ
			for (int r = find_by_key_begin(vec_rules, B); r < find_by_key_end(vec_rules, B); r++)
			{
				canonical_table new_item(B, 0, vec_rules[r].second, lookahead);

				if (find(res.begin(), res.end(), new_item) == res.end())
				{
					res.push_back(new_item);
					queue.push_back(new_item);
				}
			}
		}
	}

	return Formating_Table(res);
}



Sintax::tabular_analyzer Sintax::Tabular_analyzer(Sintax::tabular_analyzer& TabAn)
{
	int find_rule = -1;
	//Создание f
	for (int i = 0; i < сanonical_table_system.size(); i++)
	{
		for (int j = 0; j < terminals.size(); j++)
		{
			for (auto& v : сanonical_table_system[i].rules)
			{
				if (v.dot == v.rule.size() && v.nonterminal == "<S>")
				{
					//Допуск, скорее всего following будет один = eps, если это так, то оптимизировать!
					for (auto& w : v.following)
					{
						IsCellFull(i, Terminal_number(w), "a");
						TabAn.rows[i].f[Terminal_number(w)] = "a";
					}
				}
				else if (v.dot + 1 == v.rule.size() && v.rule[v.dot] == "eps")
				{
					find_rule = FindRuleInRow(v);

					for (auto& w : v.following)
					{
						TabAn.rows[i].f[Terminal_number(w)] = to_string(find_rule);
					}
				}
				else if (v.dot == v.rule.size())
				{
					find_rule = FindRuleInRow(v);

					for (auto& k : v.following)
					{
						int pos = Terminal_number(k);
						if (pos == terminals.size())
						{
							throw(string("Error in finding terminal"));
						}
							
						IsCellFull(i, pos, to_string(find_rule));

						TabAn.rows[i].f[pos] = to_string(find_rule);
					}
				}
				else if (IsNonterminal(v.rule[v.dot]))
				{
					//Переход
					for (auto& w : Firsts[Nonterminal_number(v.rule[v.dot])])
					{
						if (w == "eps")
							continue;
						IsCellFull(i, Terminal_number(w), "t");
						TabAn.rows[i].f[Terminal_number(w)] = "t";
					}
				}
				else if (IsTerminal(v.rule[v.dot]) && v.rule[v.dot] == terminals[j])
				{
					//Переход
					IsCellFull(i, j, "t");
					TabAn.rows[i].f[j] = "t";
				}
				else
				{
					//Ничего
				}
			}
		}
	}

	//Создание g
	for (int i = 0; i < nonterminals.size(); i++)
	{
		// С 1 тк в <S> нет переходов
		for (int j = 1; j < сanonical_table_system.size(); j++)
		{
			if (nonterminals[i] == сanonical_table_system[j].goto_from.symbol)
			{
				TabAn.rows[сanonical_table_system[j].goto_from.number_table].g[i] = j;
			}
		}
		for (int j = 0; j < not_included_tables.size(); j++)
		{
			if (nonterminals[i] == not_included_tables[j].goto_from.symbol)
			{
				TabAn.rows[not_included_tables[j].goto_from.number_table].g[i] = not_included_tables[j].number_table;
			}
		}
		
	}
	bool flag_eps = 0;
	for (int i = nonterminals.size(); i < nonterminals.size() + terminals.size(); i++)
	{
		if (terminals[i - nonterminals.size()] == "eps")
		{
			flag_eps = 1;
		}

		// С 1 тк в <S> нет переходов
		for (int j = 1; j < сanonical_table_system.size() - 1; j++)
		{
			if (terminals[i - nonterminals.size()] == сanonical_table_system[j].goto_from.symbol)
			{
				TabAn.rows[сanonical_table_system[j].goto_from.number_table].g[i - flag_eps] = j;
			}
		}
		for (int j = 0; j < not_included_tables.size(); j++)
		{
			if (terminals[i - nonterminals.size()] == not_included_tables[j].goto_from.symbol)
			{
				TabAn.rows[not_included_tables[j].goto_from.number_table].g[i - flag_eps] = not_included_tables[j].number_table;
			}
		}
	}


	return TabAn;
}

string Sintax::Word_processing(string word)
{
	string result;

	if (word.empty())
		throw (string("Empty word"));

	if (IsTerminal(word))
	{
		result = word;
	}
	else if (!isdigit(word[0]))
	{
		result = "V";
	}
	else
	{
		result = "C";
	}

	return result;
}

bool Sintax::Processing_incoming_code(const string file_name)
{
	string word;
	fstream file(file_name);
	if (!file.is_open())
	{
		cerr << "Error opening file: " << file_name << endl;
		return false;
	}
	int T = 0;
	if (file.eof())
		return true;
	file >> word;
	stack<string> action_stack;
	action_stack.push("0");
	vector<int> rules_used;

	///TEST///
	try 
	{
		while (action_stack.size() >= 1)
		{
			
			word = Word_processing(word);
			///TEST///
			cout << "." << word << " " << T << " || ";
			Print_Stack(action_stack);
			cout << endl;
			///TEST///

			int term_num = Terminal_number(word);
			string action_cell = TabAn.rows[T].f[term_num];

			if (action_cell == "t")
			{
				action_stack.push(word);
				T = TabAn.rows[T].g[nonterminals.size() + term_num - ((term_num < Terminal_number("eps")) ? 0 : 1)]; // В g нет eps
				action_stack.push(to_string(T));
				if (file.eof())
					word = "eps";
				else
					file >> word;
			}
			else if (action_cell == "a")
				return true;
			else if (isdigit(action_cell[0]) && stoi(action_cell) >= 0 && stoi(action_cell) < TabAn.rows.size())
			{
				int rule_idx = stoi(action_cell);
				//Удаляем из стека 2 * |правило|
				if (vec_rules[rule_idx].second[0] != "eps")
				{
					for (int i = 0; i < 2 * vec_rules[rule_idx].second.size(); i++)
					{
						if (action_stack.empty())
							throw(string("Stack underflow during reduction"));
						action_stack.pop();
					}
				}

				int temp_T = stoi(action_stack.top());
				action_stack.push(vec_rules[rule_idx].first);
				T = TabAn.rows[temp_T].g[Nonterminal_number(vec_rules[rule_idx].first)];
				action_stack.push(to_string(T));
			}
			else
				throw(string("Wrong command"));

			///TEST///
			cout << ".." << word << " " << T << " || ";
			Print_Stack(action_stack);
			cout << endl;
			///TEST///

		}		

	}
	catch (string err)
	{
		cerr << "Word " << word << " can't be recognized" << endl;
		cerr << err << endl;
		return false;
	}
	return false;
}

int Sintax::Terminal_number(string terminal)
{
	int res = find(terminals.begin(), terminals.end(), terminal) - terminals.begin();

	return res;
}

int Sintax::Nonterminal_number(string nonterminal)
{
	int res = find(nonterminals.begin(), nonterminals.end(), nonterminal) - nonterminals.begin();

	return res;
}

int Sintax::find_by_key_begin(const vector<pair<string, vector<string>>>& vec, const string& key)
{
	for (int i = 0; i < vec.size(); i++)
	{
		if (vec[i].first == key)
			return i;
	}

	return -1;
}

int Sintax::find_by_key_end(const vector<pair<string, vector<string>>>& vec, const string& key)
{
	for (int i = vec.size() - 1; i >= 0; i--)
	{
		if (vec[i].first == key)
			return i + 1;
	}
	return -1;
}

bool Sintax::IsCellFull(int pos1, int pos2, string сontent)
{
	if (TabAn.rows[pos1].f[pos2] != сontent && TabAn.rows[pos1].f[pos2] != "-" && TabAn.rows[pos1].f[pos2].size() != 0)
	{
		cerr << "Conflict in table f: T" << pos1 << " on " << terminals[pos2] << " | " << TabAn.rows[pos1].f[pos2] << " and " << сontent << endl;
		return true;
	}
	return false;
}

int Sintax::FindRuleInRow(canonical_table rule)
{
	int find_rule = -1;
	for (int counter_rules = 0; counter_rules < vec_rules.size(); counter_rules++)
	{
		if (rule.nonterminal == vec_rules[counter_rules].first && rule.rule == vec_rules[counter_rules].second)
		{
			find_rule = counter_rules;
			break;
		}
	}
	if (find_rule == -1)
	{
		throw(string("Error in finding rules"));
	}
	return find_rule;
}

// Печать всех правил грамматики
void Sintax::Print_Rules()
{
	for (auto it = vec_rules.begin(); it != vec_rules.end(); it++)
	{
		for (const auto& rule : it->second)
		{
			cout << it->first << " -> ";
			for (const auto& word : rule)
			{
				cout << word << " ";
			}
			cout << endl;
		}
		cout << endl;
	}
}

// Печать всех правил грамматики
void Sintax::Write_Rules(ofstream& file)
{
	for (auto it = vec_rules.begin(); it != vec_rules.end(); it++)
	{
		file << it->first << " -> ";
		for (const auto& word : it->second)
		{
			file << word << " ";
		}
		file << endl;
	}
}

// Печать всех нетерминалов
void Sintax::Print_Nonterminals()
{
	cout << "Nonterminals: ";
	for (const auto& nonterminal : nonterminals)
	{
		cout << nonterminal << " ";
	}
	cout << endl;
}

// Печать всех нетерминалов
void Sintax::Write_Nonterminals(ofstream& file)
{
	file << "Nonterminals: ";
	for (const auto& nonterminal : nonterminals)
	{
		file << nonterminal << " ";
	}
	file << endl;
}

// Печать всех терминалов
void Sintax::Print_Terminals()
{
	cout << "Terminals: ";
	for (const auto& terminal : terminals)
	{
		cout << terminal << " ";
	}
	cout << endl;
}

// Печать всех терминалов
void Sintax::Write_Terminals(ofstream& file)
{
	file << "Terminals: ";
	for (const auto& terminal : terminals)
	{
		file << terminal << " ";
	}
	file << endl;
}

void Sintax::Print_GO_TO_args(list<for_goto> go_to_args)
{
	for (auto& i : go_to_args)
	{
		cout << "(" << i.number_table << " " << i.symbol << ")" << endl;
	}
}

void Sintax::Write_GO_TO_args(list<for_goto> go_to_args, ofstream& file)
{
	for (auto& i : go_to_args)
	{
		file << "(" << i.number_table << " " << i.symbol << ")" << endl;
	}
}