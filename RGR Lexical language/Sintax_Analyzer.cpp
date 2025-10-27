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

void Sintax::Print_Stack(stack<attribute_word> st)
{
	stack<attribute_word> temp_st = st;
	while (!temp_st.empty())
	{
		cout << temp_st.top().word << " ";
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

Sintax::attribute_word Sintax::Token_processing(Token token_)
{
	attribute_word result;

	switch (token_.token_class)
	{
	case VARIABLE:
		result.word = "V";
		result.adress_var = get<2>(token_.value);
		break;

	case CONSTANT:
		result.word = "C";
		result.adress_const = get<1>(token_.value);
		result.type = (*(get<1>(token_.value))).index();
		break;

	case GET:
		result.word = "get";
		result.adress_const = get<1>(token_.value);
		result.adress_const2 = token_.second_argument_get;
		break;

	case LABEL:
		result.word = "L";
		result.adress_label = get<3>(token_.value);
		break;

	case GO_TO_LABEL:
		result.word = "gotoL";
		result.adress_label = get<3>(token_.value);
		break;

	case VARIABLE_TYPE:
		result.word = get<0>(token_.value);
		break;

	case ARITHMETIC_OPERATION:
		result.word = get<0>(token_.value);
		break;

	case RELATION:
		result.word = "rel";
		result.relation = get<0>(token_.value);
		break;

	case EMPTY_OPERATOR:
		result.word = ";";
		break;

	case DECLARING_VARIABLES:
		result.word = "declare";
		break;

	case AS:
		result.word = "as";
		break;

	case ASSIGNMENT_OPERATOR:
		result.word = "=";
		break;

	case WHILE:
		result.word = "while";
		break;

	case DO:
		result.word = "do";
		break;

	case OD:
		result.word = "od";
		break;

	case FOR:
		result.word = "for";
		break;

	case FROM:
		result.word = "from";
		break;

	case TO:
		result.word = "to";
		break;

	case BY:
		result.word = "by";
		break;

	case IF:
		result.word = "if";
		break;

	case ELSE:
		result.word = "else";
		break;

	case FI:
		result.word = "fi";
		break;

	case SELECT:
		result.word = "select";
		break;

	case IN:
		result.word = "in";
		break;

	case CASE:
		result.word = "case";
		break;

	case OTHERWISE:
		result.word = "otherwise";
		break;

	case NI:
		result.word = "ni";
		break;

	case INPUT:
		result.word = "input";
		break;

	case PRINT:
		result.word = "print";
		break;

	case RAISE:
		result.word = "raise";
		break;

	case COMMENT:
		result.word = "rem";
		break;

	case O_BRACE:
		result.word = "(";
		break;

	case C_BRACE:
		result.word = ")";
		break;

	case COMMA:
		result.word = ",";
		break;

	case CASE_LISTING:
		result.word = ":";
		break;
	default:
		throw (string("Error in Token processing"));
		
	}
	return result;
}



bool Sintax::Translation_of_code(const string file_name, const string output_file_name)
{
	Lexical_Analyzer(file_name);

	if (table_tokens.size() == 0)
		return true;

	ofstream file(output_file_name);
	if (!file)
		return -1;

	attribute_word att_word;
	vector<int> rules_used;
	stack<attribute_word> action_stack;
	int term_num;
	string action_cell;

	// Инициализация переменных
	int T = 0;
	action_stack.push(attribute_word("0"));
	att_word = Token_processing(table_tokens[0]);
	int token_index = 0;
	int size_program = 0;

	int TEST_COUNTER = 0;

	try
	{
		while (action_stack.size() >= 1)
		{
			if (token_index < table_tokens.size())
			{
				term_num = Terminal_number(Token_processing(table_tokens[token_index]).word);
				action_cell = TabAn.rows[T].f[term_num];
			}
			else
			{
				action_cell = TabAn.rows[T].f[Terminal_number("eps")];
			}

			//TEST//
			if (token_index < table_tokens.size())
				cout << "T" << action_stack.top().word << " | next: " << Token_processing(table_tokens[token_index]).word << " | action: " << action_cell << " | " << TEST_COUNTER << " | size: " << size_program << endl << "stack: ";
			else
				cout << "T" << action_stack.top().word << " | next: " << "eps" << " | action: " << action_cell << " | " << TEST_COUNTER << endl << "stack: ";
			Print_Attribute_Stack(action_stack);
			cout << endl;
			TEST_COUNTER++;
			//TEST//

			if (action_cell == "t")
			{
				
				token_index++;
				action_stack.push(att_word);
				T = TabAn.rows[T].g[nonterminals.size() + term_num - ((term_num < Terminal_number("eps")) ? 0 : 1)]; // В g нет eps
			}
			else if (action_cell == "a")
			{
				att_word = Grouping_by_rule(action_stack, 0, size_program);
				break;
			}
			else if (isdigit(action_cell[0]) && stoi(action_cell) >= 0 && stoi(action_cell) < TabAn.rows.size())
			{
				int rule_idx = stoi(action_cell);
				rules_used.push_back(rule_idx);

				att_word = Grouping_by_rule(action_stack, rule_idx, size_program);
				size_program = att_word.program.size();

				int temp_T = stoi(action_stack.top().word);
				action_stack.push(att_word);
				T = TabAn.rows[temp_T].g[Nonterminal_number(vec_rules[rule_idx].first)];
			}
			else
				throw(string("Wrong command"));

			action_stack.push(to_string(T));
			if (token_index < table_tokens.size())
			{
				att_word = Token_processing(table_tokens[token_index]);
			}
			else
			{
				att_word.word = "eps";
			}
		}
	}
	catch (string err)
	{
		cerr << "Error during syntactic analysis: " << err << endl;
		return false;
	}

	Write_Stack_Program(att_word.program, file);
	Print_Stack_Program(att_word.program);

	return true;

}

void Sintax::Print_Stack_Program(const vector<string>& input)
{
	for (auto& i : input)
	{
		cout << i << endl;
	}
}

void Sintax::Write_Stack_Program(const vector<string>& input, ofstream& file)
{
	for (auto& i : input)
	{
		file << i << endl;
	}
}

void Sintax::Print_Attribute_Stack(stack<attribute_word> input)
{
	stack<attribute_word> temp;
	while (!input.empty())
	{
		temp.push(input.top());
		input.pop();
	}
	while (!temp.empty())
	{
		cout << temp.top().word << " ";
		temp.pop();
	}
	cout << endl;
}

void Sintax::Write_Attribute_Stack(stack<attribute_word> input, ofstream& file)
{
	stack<attribute_word> temp;
	while (!input.empty())
	{
		temp.push(input.top());
		input.pop();
	}
	while (!temp.empty())
	{
		file << temp.top().word << " ";
		temp.pop();
	}
	file << endl;
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

Sintax::attribute_word Sintax::Grouping_by_rule(stack<attribute_word>& attribute_stack, int number_rule, int number_line)
{
	attribute_word result(vec_rules[number_rule].first);

	vector<string> p1, p2, p3, p4, p5;
	int type1, type2, type3, type4, type5;
	map<string, variant<int, BigNumber>>::iterator adress_var1, adress_var2, adress_var3, adress_var4, adress_var5;
	variant<int, BigNumber> adress_const1, adress_const2, adress_const3, adress_const4, adress_const5;
	set<variant<int, BigNumber>>::iterator adress_int_const1, adress_int_const2, adress_int_const3, adress_int_const4, adress_int_const5;
	string relation;
	int new_label1, new_label2, new_label3, new_label4;
	string name_label;
	int end_label;
	bool flag_otherwise;
	vector<set<variant<int, BigNumber>>::iterator> list_adress_constants;
	vector<int> labels;
	int temp_counter = 0;


	switch (number_rule)
	{
	case 0: // {"<S>", {"<Ads>", "<Program>"}}
		attribute_stack.pop(); // убираем T
		p1 = attribute_stack.top().program;
		attribute_stack.pop(); // <Program>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // <Ads>

		result.program = p1;

		break;

	case 1: // {"<Ads>", {"declare", "<ads>", ";", "<Ads>"}}
		del_rule(attribute_stack, number_rule);

		if (table_variable.size() > declared_variables.size())
			throw(string("Not all variables are declared"));
		break;

	case 2: // {"<Ads>", {"eps"}}
		del_rule(attribute_stack, number_rule);
		break;

	case 3: // {"<ads>", {"V", "as", "<TYPE>", ",", "<ads>"}}
		attribute_stack.pop(); // T
		attribute_stack.pop(); // <ads>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // ,
		attribute_stack.pop(); // T
		type1 = attribute_stack.top().type;
		attribute_stack.pop(); // <TYPE>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // as
		attribute_stack.pop(); // T
		adress_var2 = attribute_stack.top().adress_var;
		attribute_stack.pop(); // V

		if (find(declared_variables.begin(), declared_variables.end(), adress_var2) != declared_variables.end())
			throw (string("An attempt to repeat the announcement"));

		declared_variables.push_back(adress_var2);
		if (type1 == 0)
			(*adress_var2).second = int(0);
		else
			(*adress_var2).second = BigNumber(0);
		break;

	case 4: // {"<ads>", {"V", "as", "<TYPE>"}}
		attribute_stack.pop(); // T
		type1 = attribute_stack.top().type;
		attribute_stack.pop(); // <TYPE>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // as
		attribute_stack.pop(); // T
		adress_var2 = attribute_stack.top().adress_var;
		attribute_stack.pop(); // V

		if (find(declared_variables.begin(), declared_variables.end(), adress_var2) != declared_variables.end())
			throw (string("An attempt to repeat the announcement"));

		declared_variables.push_back(adress_var2);
		if (type1 == 0)
			(*adress_var2).second = int(0);
		else
			(*adress_var2).second = BigNumber(0);
		
		break;

	case 5: // {"<Program>", {"<Operation>", "<Program>"}}
		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		attribute_stack.pop(); // <Program>
		attribute_stack.pop(); // T
		p2 = attribute_stack.top().program;
		attribute_stack.pop(); // <Operation>

		result.program.insert(result.program.end(), p2.begin(), p2.end());
		result.program.insert(result.program.end(), p1.begin(), p1.end());

		break;

	case 6: // {"<Program>", {"eps"}}
		del_rule(attribute_stack, number_rule);

		result.program.push_back("");

		break;

	case 7: // {"<Operation>", {";"}


		del_rule(attribute_stack, number_rule);
		result.program.push_back("");
		break;

	case 8: // {"<Operation>", {"<Assignment>"}}


		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		attribute_stack.pop(); // <Assignment>

		result.program.insert(result.program.end(), p1.begin(), p1.end());
		break;


	case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17: case 18:// {"<Operation>", ...}
		
		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		attribute_stack.pop(); //<while> / <for> / <if> / <input> / <print> / <label> / <transition> / <select> / <exception> / <comment> 

		result.program.insert(result.program.end(), p1.begin(), p1.end());
		break;

	case 19: // {"<Assignment>", {"V", "=", "<E>", ";"}}
		attribute_stack.pop(); // T
		attribute_stack.pop(); // ;
		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		type1 = attribute_stack.top().type;
		attribute_stack.pop(); // <E>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // =
		attribute_stack.pop(); // T
		adress_var1 = attribute_stack.top().adress_var;
		attribute_stack.pop(); // V

		if (find(declared_variables.begin(), declared_variables.end(), adress_var1) == declared_variables.end())
			throw (string("The variable declaration is missing"));
		if ((*adress_var1).second.index() != type1)
			throw(string("Type mismatch in assignment"));

		result.program.insert(result.program.end(), p1.begin(), p1.end());
		result.program.push_back("pop " + (*adress_var1).first);
		break;

	case 20: // {"<while>", {"while", "<E>", "rel", "<E>", "do", "<Program>", "od", ";"}}
		attribute_stack.pop(); // T
		attribute_stack.pop(); // ;
		attribute_stack.pop(); // T
		attribute_stack.pop(); // od
		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		attribute_stack.pop(); // <Program>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // do
		attribute_stack.pop(); // T
		p2 = attribute_stack.top().program;
		type2 = attribute_stack.top().type;
		attribute_stack.pop(); // <E>
		attribute_stack.pop(); // T
		relation = attribute_stack.top().relation;
		attribute_stack.pop(); // rel
		attribute_stack.pop(); // T
		p3 = attribute_stack.top().program;
		type3 = attribute_stack.top().type;
		attribute_stack.pop(); // <E>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // while

		if (type2 != type3)
			throw(string("Type mismatch in while"));

		new_label1 = number_line;
		result.program.insert(result.program.end(), p3.begin(), p3.end());
		result.program.insert(result.program.end(), p2.begin(), p2.end());
		result.program.push_back(relation);
		new_label2 = number_line + result.program.size() + 2; // +2 тк после будет ji и jmp до самой метки
		result.program.push_back("ji " + to_string(new_label2));
		new_label3 = number_line + result.program.size() + 1 + p1.size() + 1; // до метки еще <Program> и jmp 
		result.program.push_back("jmp " + to_string(new_label3));
		result.program.insert(result.program.end(), p1.begin(), p1.end());
		result.program.push_back("jmp " + to_string(new_label1));
		break;

	case 21: // { "<for>", { "for", "V", "from", "<E>", "to", "<E>", "<byE>", "do", "<Program>", "od", ";" } }
		attribute_stack.pop(); // T
		attribute_stack.pop(); // ;
		attribute_stack.pop(); // T
		attribute_stack.pop(); // od
		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		attribute_stack.pop(); // <Program>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // do
		attribute_stack.pop(); // T
		p2 = attribute_stack.top().program;
		type2 = attribute_stack.top().type;
		attribute_stack.pop(); // <byE>
		attribute_stack.pop(); // T
		p3 = attribute_stack.top().program;
		type3 = attribute_stack.top().type;
		attribute_stack.pop(); // <E>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // to
		attribute_stack.pop(); // T
		p4 = attribute_stack.top().program;
		type4 = attribute_stack.top().type;
		attribute_stack.pop(); // <E>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // from
		attribute_stack.pop(); // T
		adress_var5 = attribute_stack.top().adress_var;
		attribute_stack.pop(); // V
		attribute_stack.pop(); // T
		attribute_stack.pop(); // for


		if (find(declared_variables.begin(), declared_variables.end(), adress_var5) == declared_variables.end())
			throw (string("The variable declaration is missing"));

		if (type2 != type3 || type2 != type4 || type2 != (*adress_var5).second.index())
			throw(string("Type mismatch in for"));

		if ((*adress_var5).second.index() == 0)
		{
			result.program.insert(result.program.end(), p4.begin(), p4.end());
			result.program.push_back("pop " + (*adress_var5).first);
			new_label1 = number_line + result.program.size();
			result.program.push_back("push " + (*adress_var5).first);
			result.program.insert(result.program.end(), p3.begin(), p3.end());
			result.program.push_back(">");
			new_label2 = number_line + result.program.size() + 3 + p2.size() + 3 + p1.size() + 1; // просчитываем все
			result.program.push_back("ji " + to_string(new_label2));
			new_label3 = number_line + result.program.size() + 2 + p2.size() + 3;
			result.program.push_back("jmp " + to_string(new_label3));
			new_label4 = number_line + result.program.size();
			result.program.push_back("push " + (*adress_var5).first);
			result.program.insert(result.program.end(), p2.begin(), p2.end());
			result.program.push_back("+");
			result.program.push_back("pop " + (*adress_var5).first);
			result.program.push_back("jmp " + to_string(new_label1));
			result.program.insert(result.program.end(), p1.begin(), p1.end());
			result.program.push_back("jmp " + to_string(new_label4));
		}
		else
		{
			result.program.insert(result.program.end(), p4.begin(), p4.end());
			result.program.push_back("pop " + (*adress_var5).first);
			new_label1 = number_line + result.program.size();
			result.program.push_back("pushbn " + (*adress_var5).first);
			result.program.insert(result.program.end(), p3.begin(), p3.end());
			result.program.push_back(">");
			new_label2 = number_line + result.program.size() + 3 + p2.size() + 3 + p1.size() + 1; // просчитываем все
			result.program.push_back("ji " + to_string(new_label2));
			new_label3 = number_line + result.program.size() + 2 + p2.size() + 3;
			result.program.push_back("jmp " + to_string(new_label3));
			new_label4 = number_line + result.program.size();
			result.program.push_back("pushbn " + (*adress_var5).first);
			result.program.insert(result.program.end(), p2.begin(), p2.end());
			result.program.push_back("+");
			result.program.push_back("pop " + (*adress_var5).first);
			result.program.push_back("jmp " + to_string(new_label1));
			result.program.insert(result.program.end(), p1.begin(), p1.end());
			result.program.push_back("jmp " + to_string(new_label4));
		}

		break;

	case 22: // {"<byE>", {"by", "<E>"}}
		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		type1 = attribute_stack.top().type;
		attribute_stack.pop(); // <E>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // by

		result.program.insert(result.program.end(), p1.begin(), p1.end());
		result.type = type1;
		break;

	case 23: // {"<byE>", {"eps"}}
		del_rule(attribute_stack, number_rule);

		result.program.push_back("push 1"); // Так как шаг не определен, то по умолчанию он равен 1
		result.type = 0;
		break;

	case 24: // {"<if>", {"if", "(", "<Test>", ")", "<Program>", "else", "<Program>", "fi", ";"}}
		attribute_stack.pop(); // T
		attribute_stack.pop(); // ;
		attribute_stack.pop(); // T
		attribute_stack.pop(); // fi
		p1 = attribute_stack.top().program;
		attribute_stack.pop(); // <Program>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // else
		attribute_stack.pop(); // T
		p2 = attribute_stack.top().program;
		attribute_stack.pop(); // <Program>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // )
		attribute_stack.pop(); // T
		p3 = attribute_stack.top().program;
		attribute_stack.pop(); // <Test>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // (
		attribute_stack.pop(); // T
		attribute_stack.pop(); // if

		result.program.insert(result.program.end(), p3.begin(), p3.end());
		new_label1 = number_line + result.program.size() + 1 + p2.size() + 1;
		result.program.push_back("ji " + to_string(new_label1));
		result.program.insert(result.program.end(), p2.begin(), p2.end());
		new_label2 = number_line + result.program.size() + 1 + p1.size();
		result.program.push_back("jmp " + to_string(new_label2));
		result.program.insert(result.program.end(), p1.begin(), p1.end());
		
		break;

	case 25: // {"<if>", {"if", "(", "<Test>", ")", "<Program>", "fi", ";"}}
		attribute_stack.pop(); // T
		attribute_stack.pop(); // ;
		attribute_stack.pop(); // T
		attribute_stack.pop(); // fi
		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		attribute_stack.pop(); // <Program>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // )
		attribute_stack.pop(); // T
		p2 = attribute_stack.top().program;
		attribute_stack.pop(); // <Test>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // (
		attribute_stack.pop(); // T
		attribute_stack.pop(); // if

		result.program.insert(result.program.end(), p2.begin(), p2.end());
		new_label1 = number_line + result.program.size() + 2;
		result.program.push_back("ji " + to_string(new_label1));
		new_label2 = number_line + result.program.size() + 1 + p1.size();
		result.program.push_back("jmp " + to_string(new_label2));
		result.program.insert(result.program.end(), p1.begin(), p1.end());

		break;

	case 26: // {"<input>", {"input", "V", ";"}}
		attribute_stack.pop(); // T
		attribute_stack.pop(); // ;
		attribute_stack.pop(); // T
		adress_var1 = attribute_stack.top().adress_var;
		attribute_stack.pop(); // V
		attribute_stack.pop(); // T
		attribute_stack.pop(); // input


		if (find(declared_variables.begin(), declared_variables.end(), adress_var1) != declared_variables.end())
			throw (string("The variable declaration is missing"));

		result.program.push_back("read");
		result.program.push_back("pop " + (*adress_var1).first);

		break;

	case 27: // {"<print>", {"print", "<E>", ";"}}
		attribute_stack.pop(); // T
		attribute_stack.pop(); // ;
		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		type1 = attribute_stack.top().type;
		attribute_stack.pop(); // <E>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // print

		result.program.insert(result.program.end(), p1.begin(), p1.end());
		result.program.push_back("write");
		break;

	case 28: // {"<label>", {"L"}}
		attribute_stack.pop(); // T
		name_label = *(attribute_stack.top().adress_label);
		attribute_stack.pop(); // gotoL

		if (find(number_lines_labels.begin(), number_lines_labels.end(), LabelInfo(name_label)) != number_lines_labels.end())
		{
			// Если сначала была goto, а только сейчас встретиласть label
			find( number_lines_labels.begin(), number_lines_labels.end(), LabelInfo(name_label) )->number_line = result.program.size();
		}
		else
		{
			number_lines_labels.push_back(LabelInfo(name_label, result.program.size()));
		}
		break;

	case 29: //{ "<transition>", { "gotoL", ";" } }
		attribute_stack.pop(); // T
		attribute_stack.pop(); // ;
		attribute_stack.pop(); // T
		name_label = *(attribute_stack.top().adress_label);
		attribute_stack.pop(); // gotoL

		if (find(number_lines_labels.begin(), number_lines_labels.end(), LabelInfo(name_label)) != number_lines_labels.end())
		{
			// Если сначала была метка
			result.program.push_back("jmp " + to_string(find(number_lines_labels.begin(), number_lines_labels.end(), LabelInfo(name_label))->number_line));
		}
		else
		{
			number_lines_labels.push_back(LabelInfo(name_label));
		}

		break;

	case 30: // {"<select>", {"select", "<E>", "in", "<case>", "ni", ";"}}
		attribute_stack.pop(); // T
		attribute_stack.pop(); // ;
		attribute_stack.pop(); // T
		attribute_stack.pop(); // ni
		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		flag_otherwise = attribute_stack.top().flag_otherwise;
		list_adress_constants = attribute_stack.top().list_adress_constants;
		labels = attribute_stack.top().labels;
		end_label = attribute_stack.top().end_label;
		attribute_stack.pop(); // <case>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // in
		attribute_stack.pop(); // T
		p2 = attribute_stack.top().program;
		type2 = attribute_stack.top().type;
		attribute_stack.pop(); // <E>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // select

		if (type2 == 1)
			throw(string("Type mismatch in case"));
		for (auto& i : list_adress_constants)
		{
			result.program.insert(result.program.end(), p2.begin(), p2.end());
			result.program.push_back("push " + get<0>((*i)));
			result.program.push_back("=");
			result.program.push_back("ji " + labels[temp_counter]);
			temp_counter++;
		}
		if (flag_otherwise == 0)
			result.program.push_back("jmp " + end_label);
		else
			result.program.push_back("jmp " + labels[labels.size() - 1]);
		result.program.insert(result.program.end(), p1.begin(), p1.end());

		break;

	case 31: // {"<case>", {"case", "C", ":", "<Program>", "<case>"}}
		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		flag_otherwise = attribute_stack.top().flag_otherwise;
		list_adress_constants = attribute_stack.top().list_adress_constants;
		labels = attribute_stack.top().labels;
		end_label = attribute_stack.top().end_label;
		attribute_stack.pop(); // <case>
		attribute_stack.pop(); // T
		p2 = attribute_stack.top().program;
		attribute_stack.pop(); // <Program>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // :
		attribute_stack.pop(); // T
		adress_int_const3 = attribute_stack.top().adress_const;
		type3 = attribute_stack.top().type;
		attribute_stack.pop(); // C
		attribute_stack.pop(); // T
		attribute_stack.pop(); // case

		if (type3 == 1)
			throw(string("Type mismatch in case"));

		//Проверка на наличее в списке одинаковых констант
		//if (find(list_adress_constants.begin(), list_adress_constants.end(), adress_const3) != list_adress_constants.end())
		//	throw("Uncertainty in the case");


		result.labels = labels;
		result.labels.push_back(result.program.size());
		result.list_adress_constants.push_back(adress_int_const3);
		result.program.insert(result.program.end(), p2.begin(), p2.end());
		result.program.push_back("jmp " + end_label);
		result.program.insert(result.program.end(), p1.begin(), p1.end());
		result.end_label = end_label;

		break;

	case 32: // {"<case>", {"case", "C", ":", "<Program>"}}
		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		attribute_stack.pop(); // <Program>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // :
		attribute_stack.pop(); // T
		adress_const2 = *attribute_stack.top().adress_const;
		type2 = attribute_stack.top().type;
		attribute_stack.pop(); // C
		attribute_stack.pop(); // T
		attribute_stack.pop(); // case

		if (type2 == 1)
			throw(string("Type mismatch in case"));

		result.labels.push_back(result.program.size());
		result.list_adress_constants.push_back(adress_int_const2);
		result.program.insert(result.program.end(), p1.begin(), p1.end());
		result.end_label = result.program.size();
		break;

	case 33: // {"<case>", {"otherwise", ":", "<Program>"}}
		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		attribute_stack.pop(); // <Program>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // :
		attribute_stack.pop(); // T
		attribute_stack.pop(); // otherwise

		result.flag_otherwise = true;
		result.labels.push_back(result.program.size());
		result.program.insert(result.program.end(), p1.begin(), p1.end());
		result.end_label = result.program.size();
		break;

	case 34: // {"<exception>", {"raise", ";"}}
		del_rule(attribute_stack, number_rule);

		result.program.push_back("end");
		break;

	case 35: // {"<comment>", {"rem"}}
		del_rule(attribute_stack, number_rule);
		result.program.push_back("");
		break;

	case 36: // {"<E>", {"<E>", "+", "<T>"}}
		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		type1 = attribute_stack.top().type;
		attribute_stack.pop(); // <T>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // +
		attribute_stack.pop(); // T
		p2 = attribute_stack.top().program;
		type2 = attribute_stack.top().type;
		attribute_stack.pop(); // <E>
		if (type1 == type2)
		{
			result.program.insert(result.program.end(), p2.begin(), p2.end());
			result.program.insert(result.program.end(), p1.begin(), p1.end());
			result.program.push_back("+");
			result.type = type1;
		}
		else
			throw(string("Type mismatch in addition"));
		break;

	case 37: // {"<E>", {"<E>", "-", "<T>"}}
		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		type1 = attribute_stack.top().type;
		attribute_stack.pop(); // <T>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // -
		attribute_stack.pop(); // T
		p2 = attribute_stack.top().program;
		type2 = attribute_stack.top().type;
		attribute_stack.pop(); // <E>
		if (type1 == type2)
		{
			result.program.insert(result.program.end(), p2.begin(), p2.end());
			result.program.insert(result.program.end(), p1.begin(), p1.end());
			result.program.push_back("-");
			result.type = type1;
		}
		else
			throw(string("Type mismatch in subtraction"));
		break;

	case 38: // {"<E>", {"<T>"}}
		attribute_stack.pop(); // T
		type1 = attribute_stack.top().type;
		p1 = attribute_stack.top().program;
		attribute_stack.pop(); // <T>
		result.program.insert(result.program.end(), p1.begin(), p1.end());
		result.type = type1;
		break;

	case 39: // {"<E>", {"(", "<E>", ")"}}
		attribute_stack.pop(); // T
		attribute_stack.pop(); // )
		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		type1 = attribute_stack.top().type;
		attribute_stack.pop(); // <E>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // (
		result.program.insert(result.program.end(), p1.begin(), p1.end());
		result.type = type1;
		break;

	case 40: // {"<T>", {"<T>", "*", "<F>"}}
		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		type1 = attribute_stack.top().type;
		attribute_stack.pop(); // <F>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // *
		attribute_stack.pop(); // T
		p2 = attribute_stack.top().program;
		type2 = attribute_stack.top().type;
		attribute_stack.pop(); // <T>
		if (type1 == type2)
		{
			result.program.insert(result.program.end(), p2.begin(), p2.end());
			result.program.insert(result.program.end(), p1.begin(), p1.end());
			result.program.push_back("*");
			result.type = type1;
		}
		else
			throw(string("Type mismatch in multiplication"));
		break;

	case 41: // {"<T>", {"<T>", "/", "<F>"}}
		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		type1 = attribute_stack.top().type;
		attribute_stack.pop(); // <F>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // /
		attribute_stack.pop(); // T
		p2 = attribute_stack.top().program;
		type2 = attribute_stack.top().type;
		attribute_stack.pop(); // <T>
		if (type1 == type2)
		{
			result.program.insert(result.program.end(), p2.begin(), p2.end());
			result.program.insert(result.program.end(), p1.begin(), p1.end());
			result.program.push_back("/");
			result.type = type1;
		}
		else
			throw(string("Type mismatch in division"));
		break;

	case 42: // {"<T>", {"<T>", "%", "<F>"}}
		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		type1 = attribute_stack.top().type;
		attribute_stack.pop(); // <F>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // %
		attribute_stack.pop(); // T
		p2 = attribute_stack.top().program;
		type2 = attribute_stack.top().type;
		attribute_stack.pop(); // <T>
		if (type1 == type2)
		{
			result.program.insert(result.program.end(), p2.begin(), p2.end());
			result.program.insert(result.program.end(), p1.begin(), p1.end());
			result.program.push_back("%");
			result.type = type1;
		}
		else
			throw(string("Type mismatch in modulus"));
		break;

	case 43: // {"<T>", {"<F>"}}
		attribute_stack.pop(); // T
		type1 = attribute_stack.top().type;
		p1 = attribute_stack.top().program;
		attribute_stack.pop(); // <F>
		result.program.insert(result.program.end(), p1.begin(), p1.end());
		result.type = type1;
		break;

	case 44: // {"<F>", {"V"}}
		attribute_stack.pop(); // T
		adress_var1 = attribute_stack.top().adress_var;
		attribute_stack.pop(); // V

		if ((*adress_var1).second.index() == 0)
			result.program.push_back("push " + (*adress_var1).first);
		else if ((*adress_var1).second.index() == 1)
			result.program.push_back("pushbn " + (*adress_var1).first);
		else 
			throw (string("The variable declaration is missing"));
		result.type = (*adress_var1).second.index();
		break;

	case 45: // {"<F>", {"C"}}
		attribute_stack.pop(); // T
		adress_const1 = *attribute_stack.top().adress_const;
		type1 = attribute_stack.top().type;
		attribute_stack.pop(); // C
		if (type1 == 0)
			result.program.push_back("push " + to_string(get<0>(adress_const1)));
		else if (type1 == 1)
			result.program.push_back("pushbn " + to_string(get<1>(adress_const1)));
		result.type = type1;
		break;

	case 46: // { "<F>", { "get", "(", "<E>", ",", "<E>", ")" } }
		attribute_stack.pop(); // T
		attribute_stack.pop(); // )
		attribute_stack.pop(); // T
		type2 = attribute_stack.top().type;
		p2 = attribute_stack.top().program;
		attribute_stack.pop(); // <E>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // ,
		attribute_stack.pop(); // T
		type1 = attribute_stack.top().type;
		p1 = attribute_stack.top().program;
		attribute_stack.pop(); // <E>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // (
		attribute_stack.pop(); // get
		if (type1 == 1 && type2 == 0)
		{
			result.program.insert(result.program.end(), p1.begin(), p1.end());
			result.program.insert(result.program.end(), p2.begin(), p2.end());
			result.program.push_back("getd");
			result.type = 0;
		}
		else
			throw(string("Type mismatch in get function"));
		break;

	case 47: // {"<Test>", {"<E>", "rel", "<E>"}}
		attribute_stack.pop(); // T
		p1 = attribute_stack.top().program;
		type1 = attribute_stack.top().type;
		attribute_stack.pop(); // <E>
		attribute_stack.pop(); // T
		attribute_stack.pop(); // rel
		relation = attribute_stack.top().relation;
		attribute_stack.pop(); // T
		p2 = attribute_stack.top().program;
		type2 = attribute_stack.top().type;
		attribute_stack.pop(); // <E>
		if (type1 == type2)
		{
			result.program.insert(result.program.end(), p2.begin(), p2.end());
			result.program.insert(result.program.end(), p1.begin(), p1.end());
			result.program.push_back(relation);
			result.type = 0;
		}
		else
			throw(string("Type mismatch in relational operation"));
		break;

	case 48: // { "<TYPE>", { "int" } }
		attribute_stack.pop(); // T
		attribute_stack.pop(); // int
		result.type = 0;
		break;

	case 49: // { "<TYPE>", { "bignumber" } }
		attribute_stack.pop(); // T
		attribute_stack.pop(); // bignumber
		result.type = 1;
		break;
	default:
		throw(string("Unknown rule number in grouping by rule"));
		break;
		
	}

	return result;

}

stack<Sintax::attribute_word> Sintax::del_rule(stack<attribute_word>& attribute_stack, int number_rule)
{
	if (vec_rules[number_rule].second[0] != "eps")
	{
		for (int i = 0; i < 2 * vec_rules[number_rule].second.size(); i++)
		{
			if (attribute_stack.empty())
				throw(string("Stack underflow during reduction"));
			attribute_stack.pop();
		}
	}
	return attribute_stack;
}

stack<Sintax::attribute_word> Sintax::del_n_elements(stack<attribute_word>& attribute_stack, int n)
{
	for (int i = 0; i < n; i++)
	{
		if (attribute_stack.empty())
			throw(string("Stack underflow during reduction"));
		attribute_stack.pop();
	}

	return attribute_stack;
}

bool Sintax::S_more_I(string a)
{
	string b = "2147483647";
	if (a.size() != b.size())
	{
		return (a.size() > b.size());
	}
	for (auto i : a)
	{
		if (a > b)
			return a > b;
	}
	return false;
}

