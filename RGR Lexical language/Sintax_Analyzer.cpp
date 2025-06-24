#include "Sintax_Analyzer.h"

// Конструктор класса Sintax
Sintax::Sintax(string file_name, string start_nonterminal_)
{
	ofstream File_for_writing("File for writing.txt");
	if (!File_for_writing.is_open())
	{
		cerr << "Error opening file: " << file_name << endl;
		return;
	}
	File_for_writing.clear();

	fstream file(file_name);
	if (!file.is_open())
	{
		cerr << "Error opening file: " << file_name << endl;
		return;
	}

	// Преобразование правил грамматики в структуру кода
	Rule_to_code(file);

	//Print_Rules();
	Write_Rules(File_for_writing);
	File_for_writing << endl;

	vector<auxiliary_table> tables;
	// Создание вспомогательных таблиц для синтаксического анализа
	tables = Create_Tables(start_nonterminal_);

	Write_Nonterminals(File_for_writing);
	File_for_writing << endl;

	Write_Terminals(File_for_writing);
	File_for_writing << endl;

	for (auto& i : tables)
	{
		Write_Small_Table(i, File_for_writing);
	}
	File_for_writing << endl;

	File_for_writing.close();
	file.close();
}

Sintax::~Sintax()
{
	map_rules.clear();
	nonterminals.clear();
	terminals.clear();
}

// Преобразует правила из файла в структуру map_rules
void Sintax::Rule_to_code(fstream& file)
{
	string word;
	string key;
	vector<string> rule;

	int number_string = 0;
	bool nonterminal_flag = 1;

	// Основной цикл по символам файла
	while (file.peek() != EOF)
	{
		// Поиск нетерминала в начале правила
		if (nonterminal_flag)
		{
			while (file.peek() != '<' && file.peek() != '\n' && file.peek() != EOF)
			{
				file.get();
				continue;
			}
			if (file.peek() == '<')
			{
				// Считываем нетерминал
				while (file.peek() != '>')
				{
					word += file.get();
				}
				word += file.get(); // захватываем '>'
				while (file.peek() == ' ')
					file.get();

				// Проверяем наличие '->' после нетерминала
				if (file.get() == '-' && file.get() == '>')
				{
					nonterminal_flag = 0;
					key = word; // первый считанный нетерминал — ключ
					if (find(nonterminals.begin(), nonterminals.end(), key) == nonterminals.end())
					{
						nonterminals.push_back(key);
					}
				}
				else
				{
					Rule_Error("Expected '->' after the nonterminal.", file);
					continue;
				}
				word.clear();
			}
			else
			{
				Rule_Error("Expected a nonterminal at the beginning of the rule.", file);
				continue;
			}
		}

		// Обработка различных символов в правиле
		switch (file.peek())
		{
		case '\n':
			file.get();
			number_string++;
			if (!word.empty())
			{
				rule.push_back(word);
			}
			if (map_rules.find(key) == map_rules.end())
				map_rules[key] = { rule };
			else
				map_rules[key].push_back(rule);

			if (!word.empty() && find(terminals.begin(), terminals.end(), word) == terminals.end())
				terminals.push_back(word);
			word.clear();
			rule.clear();
			nonterminal_flag = 1; // сброс флага для следующего правила
			break;

		case ' ':
		case '\t':
			file.get();
			break;

		case '[':
			// Обработка терминала в квадратных скобках
			if (!word.empty())
			{
				rule.push_back(word);
				if (find(terminals.begin(), terminals.end(), word) == terminals.end())
					terminals.push_back(word);
				word.clear();
			}
			while (file.peek() != ']')
			{
				word += file.get();
			}
			word += file.get(); // захватываем ']'
			if (find(terminals.begin(), terminals.end(), word) == terminals.end())
				terminals.push_back(word);
			rule.push_back(word);
			word.clear();
			break;

		case '<':
			// Обработка нетерминала в угловых скобках
			if (!word.empty())
			{
				if (find(terminals.begin(), terminals.end(), word) == terminals.end())
					terminals.push_back(word);
				rule.push_back(word);
				word.clear();
			}
			while (file.peek() != '>')
			{
				word += file.get();
			}
			word += file.get(); // захватываем '>'
			rule.push_back(word);
			word.clear();
			break;

		default:
			// Добавление символа к текущему слову
			word += file.get();
			break;
		}

		// Обработка конца файла
		if (file.peek() == EOF)
		{
			if (!word.empty())
			{
				rule.push_back(word);
			}
			if (map_rules.find(key) == map_rules.end())
				map_rules[key] = { rule };
			else
				map_rules[key].push_back(rule);

			if (!word.empty() && find(terminals.begin(), terminals.end(), word) == terminals.end())
				terminals.push_back(word);
			word.clear();
			rule.clear();
		}
	}

	Remove_Duplicate_Eps();
}

// Удаляет повторяющиеся [eps] из всех правил в map_rules
void Sintax::Remove_Duplicate_Eps()
{
	for (auto& [key, rules] : map_rules)
	{
		for (auto& rule : rules)
		{
			bool eps_found = false;
			// Используем erase-remove idiom для удаления всех повторных [eps]
			rule.erase(
				std::remove_if(rule.begin(), rule.end(),
					[&eps_found](const std::string& symbol) {
						if (symbol == "[eps]") {
							if (!eps_found) {
								eps_found = true;
								return false; // оставить первый [eps]
							}
							return true; // удалить повторные [eps]
						}
						return false;
					}),
				rule.end());
		}
	}
}

// Сообщение об ошибке в правиле и завершение работы
void Sintax::Rule_Error(string error_text, fstream& file)
{
	cerr << "Error: " << error_text << endl;
	while (file.peek() != '\n' && file.peek() != EOF)
		file.get(); // пропуск до конца строки
	exit(EXIT_FAILURE);
}

// Сообщение об ошибке и завершение работы
void Sintax::Error(string error_text)
{
	cerr << "Error: " << error_text << endl;
	exit(EXIT_FAILURE);
}

// Поиск всех нетерминалов в файле
void Sintax::Find_Nonterminals(fstream& file)
{
	int start_ptr = ios::cur;
	file.seekg(0, ios::beg);
	char cur_ch;

	while (file.peek() != EOF)
	{
		cur_ch = file.get();
		if (cur_ch == '<')
		{
			string nonterminal;
			while (cur_ch != '>')
			{
				cur_ch = file.get();
				if (cur_ch != '>' && cur_ch != '\n' && cur_ch != ' ')
					nonterminal += cur_ch;
			}
			if (find(nonterminals.begin(), nonterminals.end(), nonterminal) == nonterminals.end())
				nonterminals.push_back(nonterminal);
		}
	}

	file.seekg(start_ptr, ios::beg);
}

// Создание вспомогательных таблиц для синтаксического анализа
vector<Sintax::auxiliary_table> Sintax::Create_Tables(string start_nonterminal_)
{
	int gcounter_test = 0;
	string start_nonterminal = start_nonterminal_; // Начальный нетерминал грамматики

	// Проверка наличия стартового нетерминала
	while (find(nonterminals.begin(), nonterminals.end(), start_nonterminal) == nonterminals.end())
	{
		cout << "Enter the starting nonterminal" << endl << "-> ";
		cin >> start_nonterminal;
	}

	vector<auxiliary_table> table;
	vector<canonical_table> start_table;
	list<for_goto> table_goto_args;

	// Формирование стартовой таблицы
	start_table = Start_Table(start_nonterminal);

	Formating_Table(start_table);

	table.push_back(auxiliary_table(start_table, for_goto(-1, start_nonterminal)));

	// Поиск всех нетерминалов, следующих за точкой в стартовой таблице
	for (auto& i : Find_All_Goto(start_table))
	{
		if (find(table_goto_args.begin(), table_goto_args.end(), i) == table_goto_args.end())
		{
			table_goto_args.push_back(i);
			//cout << "Find goto: " << i.number_table << " " << i.symbol << endl;
		}
	}

	int counter = 0;
	for (auto& i : table_goto_args)
	{
		vector<canonical_table> temp;
		for (auto& j : table)
		{
			for (auto& t : j.table)
			{
				if (t.number_table == i.number_table)
					temp.push_back(t);
			}
		}
		
		// Пустые правила не трогаем
		if (i.symbol == "[eps]")
			continue;
		
		vector<canonical_table> res = GOTO(i, temp, i.number_table);

		// Форматируем таблицу
		Formating_Table(res);

		// Проверка на наличие уже существующей таблицы с таким же содержимым
		auto find_b = find_if(
			table.begin(),
			table.end(),
			[&res](const auxiliary_table& a) 
			{
				return a.table == res;
			});

		// Если такая таблица уже существует, пропускаем её
		cout << gcounter_test++ << endl;
		if (find_b != table.end())
		{
			continue;
		}
		// Если не существует, создаём новую таблицу
		
		counter++;

		for (auto& j : res)
		{
			j.number_table = counter;
		}

		table.push_back(auxiliary_table(res, i));


		// Высчитываение аргументов goto для последующих таблиц 
		list<canonical_table> result_list(res.begin(), res.end());
		for (auto& j : result_list)
		{
			if (j.rule.size() > j.dot)
			{
				if (find(table_goto_args.begin(), table_goto_args.end(), for_goto(j.number_table, j.rule[j.dot])) == table_goto_args.end())
				{
					table_goto_args.push_back(for_goto(j.number_table, j.rule[j.dot]));
				}
			}
			
		}
	}

	return table;
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

	if (map_rules.size() == 0)
		Error("There are no rules");

	for (auto& i : map_rules[nonterminal])
	{
		words.clear();
		// Предпологаем, что лишние eps удалены 
		if (i.size() == 1 && i[0] == "[eps]")
			res.push_back({ "[eps]" });
		for (auto& j : i)
		{
			if (IsTerminal(j))
			{
				if (j == "[eps]")
					continue;
				words = j;
			}
			else
			{
				// Передаём visited дальше!
				vector<string> temp = FIRST_One(j, visited);

				res = Replacing_Eps(res, temp);

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

// Вычисляет множество FIRST для элементов, следующих за позицией t в правиле r
vector<vector<string>> Sintax::FIRST_One_for_next(const vector<string>::const_iterator it, const vector<string>& r)
{
	if (r.size() == 0)
		return {};
	vector<vector<string>> result;
	vector<string> prefix;

	auto t = it; // Создаём итератор для обхода
	++t; // Переходим к следующему элементу после t

	// Если достигли конца правила — возвращаем эпсилон
	if (t == r.end())
	{
		result.push_back({ "[eps]" });
		return result;
	}

	// Проходим по всем элементам после t
	while (t != r.end())
	{
		if (IsTerminal(*t))
		{
			// Если встретили терминал (кроме [eps]), добавляем его и завершаем
			if (*t != "[eps]")
			{
				prefix.push_back(*t);
				result.push_back(prefix);
				return result;
			}
			// Если [eps], просто пропускаем
		}
		else
		{
			// Для нетерминала вычисляем его FIRST
			vector<vector<string>> firstSet = { FIRST_One(*t, {}) };

			// Для каждого множества из FIRST добавляем к префиксу
			for (const auto& first : firstSet)
			{
				vector<string> combined = prefix;
				combined.insert(combined.end(), first.begin(), first.end());
				result.push_back(combined);
			}

			// Проверяем, есть ли [eps] среди FIRST
			bool hasEps = false;
			for (const auto& first : firstSet)
			{
				if (!first.empty() && first.front() == "[eps]")
				{
					hasEps = true;
					break;
				}
			}
			// Если нет [eps], дальше не идём
			if (!hasEps)
				return result;
		}
		++t;
	}

	// Если дошли до конца, добавляем [eps]
	result.push_back({ "[eps]" });
	return result;
}

void Sintax::Print_Firsts(vector<vector<vector<string>>> f)
{
	int count = 0;
	for (auto& i : f)
	{
		cout << "FIRST(" << nonterminals[count++] << ") = " << endl;
		for (auto& j : i)
		{
			for (auto& k : j)
			{
				cout << k << " ";
			}
			cout << endl;
		}
		cout << endl;
	}
	cout << endl;
}

// Декартово произведение двух списков списков строк
vector<string> Sintax::Cartesian_Product(vector<string> to, vector<string> from)
{
	vector<string> result;
	if (to.size() == 0)
	{
		result = from;
	}
	for (const auto& prefix : to)
	{
		for (const auto& suffix : from)
		{
			result.push_back(prefix + suffix);
		}
	}
	return result;
}

// Обрезка списков до длины n
vector<string> Sintax::Clipping(int n, vector<string> from)
{
	vector<string> result;
	for (const auto& str : from)
	{
		if (str.size() > n)
			result.push_back(str.substr(0, n));
		else
			result.push_back(str);
	}
	return result;
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
		if (i == "[eps]")
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
	if (s[0] == '<' && s[s.size() - 1] == '>')
		return true;
		//if (find(nonterminals.begin(), nonterminals.end(), s) != nonterminals.end())
		//	return true;
		//else
		//	Error("Missing nonterminal");
	else
		if (find(nonterminals.begin(), nonterminals.end(), s) != nonterminals.end())
			Error("Incorrect nonterminal");
	return false;
}

// Проверка, является ли строка терминалом
bool Sintax::IsTerminal(string s)
{
	if (!IsNonterminal(s))
		return true;
		//if (find(terminals.begin(), terminals.end(), s) != terminals.end())
		//	return true;
		//else
		//	Error("Missing terminal");
	else
		if (find(terminals.begin(), terminals.end(), s) != terminals.end())
			Error("Incorrect terminal");
	return false;
}

// Проверка, является ли строка ключевым словом
bool Sintax::IsKeyword(string s)
{
	if (s[0] == '[' && s[s.size() - 1] == ']')
		return true;
		/*if (find(Keywords.begin(), Keywords.end(), s) != Keywords.end())
			return true;
		else
			Error("Missing keyword");*/
	else
		if (find(Keywords.begin(), Keywords.end(), s) != Keywords.end())
			Error("Incorrect keyword");
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
	for (auto j : can_t)
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
	for (auto& i : map_rules[start_nonterminal])
		temp_res.push_back(canonical_table(start_nonterminal, 0, i, { "[eps]" }, 0));

	for (auto& i : temp_res)
	{
		if (i.rule.size() == 0)
			continue;
		auto t = i.rule.begin();

		if (IsNonterminal(*t))
		{
			for (auto& j : map_rules[*t])
			{
				if (i.rule.size() == 0)
					break;
				for (auto& f : FIRST_One_for_next(t, i.rule))
				{
					temp_res.push_back(canonical_table((*t), 0, j, f, 0));
				}
			}
		}
	}

	vector<canonical_table> res;
	for (auto& i : temp_res)
	{
		res.push_back(i);
	}

	return res;
}

vector<Sintax::for_goto> Sintax::Find_All_Goto(const vector<canonical_table>& can_t)
{
	vector<for_goto> res;

	for (auto& i : can_t)
	{
		if (i.rule.size() > i.dot)
			res.push_back(for_goto(i.number_table, i.rule[i.dot]));
	}

	return res;
}

Sintax::for_goto Sintax::Find_One_Goto(const canonical_table& can_t)
{
	if (can_t.rule.size() > can_t.dot)
		return (for_goto(can_t.number_table, can_t.rule[can_t.dot]));
	return for_goto(-1, " ");
}

void Sintax::Print_Small_Table(const auxiliary_table& can_t)
{
	
	cout << "A" << (*can_t.table.begin()).number_table << " = GOTO (" << "A" << can_t.goto_from.number_table << "," << can_t.goto_from.symbol << ")" << " = {" << endl;
	for (auto& i : can_t.table)
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

void Sintax::Write_Small_Table(const auxiliary_table& can_t, ofstream& file)
{
	if (can_t.goto_from.number_table != -1)
		file << "A" << (*can_t.table.begin()).number_table << " = GOTO (" << "A" << can_t.goto_from.number_table << "," << can_t.goto_from.symbol << ")" << " = {" << endl;
	else
		file << "A" << (*can_t.table.begin()).number_table << " = {" << endl;
	for (auto& i : can_t.table)
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
vector<Sintax::canonical_table> Sintax::GOTO(const for_goto& args, const vector<canonical_table>& can_t, int number_table)
{
	// Множество уникальных ситуаций
	vector<canonical_table> res;

	// Очередь для обработки новых ситуаций
	list<canonical_table> queue;

	// Сдвигаем точку вправо для всех ситуаций, где после точки стоит символ
	for (const auto& i : can_t)
	{
		if (i.number_table != number_table)
			continue;
		if (i.rule.size() <= i.dot)
			continue;

		// Проверяем, совпадает ли символ после точки с искомым символом
		if (i.rule[i.dot] == args.symbol)
		{
			canonical_table new_item(i.nonterminal, i.dot + 1, i.rule, i.following, i.number_table);

			// Проверяем, не добавлен ли уже этот элемент
			if (find(res.begin(), res.end(), new_item) == res.end()) 
			{
				res.push_back(new_item);
				queue.push_back(new_item);
			}
		}
	}

	// Обрабатываем только новые ситуации из очереди
	while (!queue.empty())
	{
		auto it = queue.front();
		queue.pop_front();

		// Проверяем, не достигли ли конца правила и не является ли следующий символ нетерминалом
		if (it.rule.size() > it.dot && IsNonterminal(it.rule[it.dot]))
		{
			string B = it.rule[it.dot];
			for (const auto& rule : map_rules[B])
			{
				for (auto firsts : FIRST_One_for_next(it.rule.begin() + it.dot, it.rule))
				{
					vector<string> temp = firsts;
					temp = Replacing_Eps(temp, it.following);//😦
					canonical_table new_item(B, 0, rule, temp, it.number_table);

					// Проверяем, не добавлен ли уже этот элемент
					if (std::find(res.begin(), res.end(), new_item) == res.end())
					{
						res.push_back(new_item);
						queue.push_back(new_item); // только новые!
					}

				}
			}
		}
	}

	return res;
}

// Печать всех правил грамматики
void Sintax::Print_Rules()
{
	for (auto it = map_rules.begin(); it != map_rules.end(); it++)
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
	for (auto it = map_rules.begin(); it != map_rules.end(); it++)
	{
		for (const auto& rule : it->second)
		{
			file << it->first << " -> ";
			for (const auto& word : rule)
			{
				file << word << " ";
			}
			file << endl;
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
