#include "Sintax_Analyzer.h"

// ����������� ������ Sintax
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

	// �������������� ������ ���������� � ��������� ����
	Rule_to_code(file);

	//Print_Rules();
	Write_Rules(File_for_writing);
	File_for_writing << endl;

	vector<vector<canonical_table>> tables;
	// �������� ��������������� ������ ��� ��������������� �������
	tables = Create_Tables(start_nonterminal_);

	Write_Nonterminals(File_for_writing);
	File_for_writing << endl;

	Write_Terminals(File_for_writing);
	File_for_writing << endl;

	for (auto& i : tables)
	{
		Write_Canonical_Table(i, File_for_writing);
	}
	File_for_writing << endl;

	File_for_writing.close();
	file.close();
}

// ����������� ������� �� ����� � ��������� map_rules
void Sintax::Rule_to_code(fstream& file)
{
	string word;
	string key;
	vector<string> rule;

	int number_string = 0;
	bool nonterminal_flag = 1;

	// �������� ���� �� �������� �����
	while (file.peek() != EOF)
	{
		// ����� ����������� � ������ �������
		if (nonterminal_flag)
		{
			while (file.peek() != '<' && file.peek() != '\n' && file.peek() != EOF)
			{
				file.get();
				continue;
			}
			if (file.peek() == '<')
			{
				// ��������� ����������
				while (file.peek() != '>')
				{
					word += file.get();
				}
				word += file.get(); // ����������� '>'
				while (file.peek() == ' ')
					file.get();

				// ��������� ������� '->' ����� �����������
				if (file.get() == '-' && file.get() == '>')
				{
					nonterminal_flag = 0;
					key = word; // ������ ��������� ���������� � ����
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

		// ��������� ��������� �������� � �������
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
			nonterminal_flag = 1; // ����� ����� ��� ���������� �������
			break;

		case ' ':
		case '\t':
			file.get();
			break;

		case '[':
			// ��������� ��������� � ���������� �������
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
			word += file.get(); // ����������� ']'
			if (find(terminals.begin(), terminals.end(), word) == terminals.end())
				terminals.push_back(word);
			rule.push_back(word);
			word.clear();
			break;

		case '<':
			// ��������� ����������� � ������� �������
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
			word += file.get(); // ����������� '>'
			rule.push_back(word);
			word.clear();
			break;

		default:
			// ���������� ������� � �������� �����
			word += file.get();
			break;
		}

		// ��������� ����� �����
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

// ������� ������������� [eps] �� ���� ������ � map_rules
void Sintax::Remove_Duplicate_Eps()
{
	for (auto& [key, rules] : map_rules)
	{
		for (auto& rule : rules)
		{
			bool eps_found = false;
			// ���������� erase-remove idiom ��� �������� ���� ��������� [eps]
			rule.erase(
				std::remove_if(rule.begin(), rule.end(),
					[&eps_found](const std::string& symbol) {
						if (symbol == "[eps]") {
							if (!eps_found) {
								eps_found = true;
								return false; // �������� ������ [eps]
							}
							return true; // ������� ��������� [eps]
						}
						return false;
					}),
				rule.end());
		}
	}
}

// ��������� �� ������ � ������� � ���������� ������
void Sintax::Rule_Error(string error_text, fstream& file)
{
	cerr << "Error: " << error_text << endl;
	while (file.peek() != '\n' && file.peek() != EOF)
		file.get(); // ������� �� ����� ������
	exit(EXIT_FAILURE);
}

// ��������� �� ������ � ���������� ������
void Sintax::Error(string error_text)
{
	cerr << "Error: " << error_text << endl;
	exit(EXIT_FAILURE);
}

// ����� ���� ������������ � �����
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

// �������� ��������������� ������ ��� ��������������� �������
vector<vector<Sintax::canonical_table>> Sintax::Create_Tables(string start_nonterminal_)
{
	string start_nonterminal = start_nonterminal_; // ��������� ���������� ����������

	// �������� ������� ���������� �����������
	while (find(nonterminals.begin(), nonterminals.end(), start_nonterminal) == nonterminals.end())
	{
		cout << "Enter the starting nonterminal" << endl << "-> ";
		cin >> start_nonterminal;
	}

	vector<vector<canonical_table>> table;
	vector<canonical_table> start_table;
	list<for_goto> table_goto_args;

	// ������������ ��������� �������
	start_table = Start_Table(start_nonterminal);

	table.push_back(start_table);

	// ����� ���� ������������, ��������� �� ������ � ��������� �������
	for (auto& i : Find_All_Goto(*table.begin()))
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
			for (auto& t : j)
			{
				if (t.number_table == i.number_table)
					temp.push_back(t);
			}
		}
		
		vector<canonical_table> res = GOTO(i, temp, i.number_table);

		// �������� ���������� �� res
		std::vector<canonical_table> unique_res;
		for (const auto& elem : res) {
			if (find(unique_res.begin(), unique_res.end(), elem) == unique_res.end())
				unique_res.push_back(elem);
		}
		res = move(unique_res);

		for (int j = res.size() - 1; j > 0; j--)
		{
			bool swapped = false;

			for (int k = 0; k < j; k++)
			{
				if (res[k].nonterminal > res[k + 1].nonterminal || res[k].nonterminal == res[k + 1].nonterminal && res[k].rule > res[k + 1].rule)
				{
					swap(res[k], res[k + 1]);
					swapped = true;
				}
			}

			if (swapped == false)
				break;
		}

		!!���-�� ���������� � ��������� � �� ��������� ��������� ������. ����� ��������� ���� ��������� �������. ���� �������������, ��� ��� ���������� ��-�� 
			������������� �������� ������ �� ��������� �� � ����������� �������
		auto find_a = find(table.begin(), table.end(), res);

		if (find_a != table.end())
		{
			Print_Canonical_Table(*find_a);
			continue;
		}
		counter++;

		for (auto& j : res)
		{
			j.number_table = counter;
		}

		table.push_back(res);


		// ������������� ���������� goto ��� ����������� ������ 
		list<canonical_table> result_list(res.begin(), res.end());
		for (auto& j : result_list)
		{
			j.number_table = counter;
			if (j.rule.size() > j.dot)
			{
				if (find(table_goto_args.begin(), table_goto_args.end(), for_goto(j.number_table, j.rule[j.dot])) == table_goto_args.end())
				{
					table_goto_args.push_back(for_goto(j.number_table, j.rule[j.dot]));
					//cout << "Added goto: " << j.number_table << " " << j.rule[j.dot] << endl;
				}
			}
			
		}
	}

	//Print_GO_TO_args(table_goto_args);

	return table;
}

// ���������� ��������� FIRST ��� �����������
vector<vector<string>> Sintax::FIRST_One(string nonterminal, set<string> visited)
{
	// ���� ��� ���� � ���� ����������� �� ���� ���� � ������������� ������������
	if (visited.count(nonterminal))
		return {};

	visited.insert(nonterminal);

	vector<vector<string>> res;
	vector<string> words;

	if (map_rules.size() == 0)
		Error("There are no rules");

	for (auto i = map_rules[nonterminal].begin(); i != map_rules[nonterminal].end(); i++)
	{
		words.clear();
		// ������������, ��� �������������� eps ������� 
		if ((*i).size() == 1 && (*i)[0] == "[eps]")
			res.push_back({ "[eps]" });
		for (auto j = (*i).begin(); j != (*i).end(); j++)
		{
			if (IsTerminal(*j))
			{
				if ((*j) == "[eps]")
					continue;
				words.push_back(*j);
			}
			else
			{
				// ������� visited ������!
				vector<vector<string>> temp = FIRST_One(*j, visited);
				vector<vector<string>> temp2;

				if (!words.empty())
					for (int i = 0; i < temp.size(); i++)
						temp2.push_back(words);


				if (!temp2.empty())
					temp = Cartesian_Product(temp, temp2);

				res = Cartesian_Product(res, temp);

				words.clear();
				break;
			}

			if (words.size() == 1)
			{
				res.push_back(words);
				break;
			}
		}		
	}

	sort(res.begin(), res.end());
	res.erase(unique(res.begin(), res.end()), res.end());

	for (auto& i : res)
	{
		sort(i.begin(), i.end());
		i.erase(unique(i.begin(), i.end()), i.end());
	}

	return res;
}

// ��������� ��������� FIRST ��� ���������, ��������� �� �������� t � ������� r
vector<vector<string>> Sintax::FIRST_One_for_next(const vector<string>::const_iterator it, const vector<string>& r)
{
	if (r.size() == 0)
		return {};
	vector<vector<string>> result;
	vector<string> prefix;

	auto t = it; // ������ �������� ��� ������
	++t; // ��������� � ���������� �������� ����� t

	// ���� �������� ����� ������� � ���������� �������
	if (t == r.end())
	{
		result.push_back({ "[eps]" });
		return result;
	}

	// �������� �� ���� ��������� ����� t
	while (t != r.end())
	{
		if (IsTerminal(*t))
		{
			// ���� ��������� �������� (����� [eps]), ��������� ��� � ���������
			if (*t != "[eps]")
			{
				prefix.push_back(*t);
				result.push_back(prefix);
				return result;
			}
			// ���� [eps], ������ ����������
		}
		else
		{
			// ��� ����������� ��������� ��� FIRST
			vector<vector<string>> firstSet = FIRST_One(*t, {});

			// ��� ������� ��������� �� FIRST ��������� � ��������
			for (const auto& first : firstSet)
			{
				vector<string> combined = prefix;
				combined.insert(combined.end(), first.begin(), first.end());
				result.push_back(combined);
			}

			// ���������, ���� �� [eps] ����� FIRST
			bool hasEps = false;
			for (const auto& first : firstSet)
			{
				if (!first.empty() && first.front() == "[eps]")
				{
					hasEps = true;
					break;
				}
			}
			// ���� ��� [eps], ������ �� ���
			if (!hasEps)
				return result;
		}
		++t;
	}

	// ���� ����� �� �����, ��������� [eps]
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

// ��������� ������������ ���� ������� ������� �����
vector<vector<string>> Sintax::Cartesian_Product(vector<vector<string>> to, vector<vector<string>> from)
{
	vector<vector<string>> result;

	for (const auto& prefix : to)
	{
		for (const auto& suffix : from)
		{
			vector<string> combined = prefix;
			combined.insert(combined.end(), suffix.begin(), suffix.end());
			result.push_back(combined);
		}
	}

	return result;
}

// ������� ������� �� ����� n
vector<vector<string>> Sintax::Clipping(int n, vector<vector<string>> from)
{
	vector<vector<string>> result;

	for (const auto& i : from)
	{
		vector<string> cliped = i;
		cliped.resize(n);
		result.push_back(cliped);
	}
	return result;
}

// ��������, �������� �� ������ ������������
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

// ��������, �������� �� ������ ����������
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

// ��������, �������� �� ������ �������� ������
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

// ������������ ��������� ������� ��� ��������������� �������
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

	for (int j = res.size() - 1; j > 0; j--)
	{
		bool swapped = false;

		for (int k = 0; k < j; k++)
		{
			if (res[k].nonterminal > res[k + 1].nonterminal || res[k].nonterminal == res[k + 1].nonterminal && res[k].rule > res[k + 1].rule)
			{
				swap(res[k], res[k + 1]);
				swapped = true;
			}
		}

		if (swapped == false)
			break;
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

void Sintax::Print_Canonical_Table(const vector<canonical_table>& can_t)
{
	int prev_number = -1;
	for (auto& i : can_t)
	{
		if (prev_number != i.number_table)
		{
			cout << "A" << i.number_table << " = {" << endl;
			prev_number = i.number_table;
		}

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

void Sintax::Write_Canonical_Table(const vector<canonical_table>& can_t, ofstream& file)
{
	int prev_number = -1;
	for (auto& i : can_t)
	{
		if (prev_number != i.number_table)
		{
			file << "A" << i.number_table << " = {" << endl;
			prev_number = i.number_table;
		}

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

// ���������� ������� GOTO ��� LR(1)-�����������
vector<Sintax::canonical_table> Sintax::GOTO(const for_goto& args, const vector<canonical_table>& can_t, int number_table)
{
	// ��������� ���������� ��������
	std::vector<canonical_table> res;

	// ������� ��� ��������� ����� ��������
	std::list<canonical_table> queue;

	// 1. �������� ����� ������ ��� ���� ��������, ��� ����� ����� ����� args.symbol
	for (const auto& i : can_t)
	{
		if (i.number_table != number_table)
			continue;
		if (i.rule.size() <= i.dot)
			continue;
		if (i.rule[i.dot] == args.symbol)
		{
			canonical_table new_item(i.nonterminal, i.dot + 1, i.rule, i.following, i.number_table);
			if (std::find(res.begin(), res.end(), new_item) == res.end()) {
				res.push_back(new_item);
				queue.push_back(new_item);
			}
		}
	}

	// 2. closure: ������������ ������ ����� �������� �� �������
	while (!queue.empty())
	{
		auto it = queue.front();
		queue.pop_front();

		if (it.rule.size() > it.dot && IsNonterminal(it.rule[it.dot]))
		{
			string B = it.rule[it.dot];
			for (const auto& rule : map_rules[B])
			{
				for (const auto& lookahead : it.following)
				{
					auto firsts = FIRST_One_for_next(it.rule.begin() + it.dot, it.rule);
					std::set<std::string> first_beta;
					for (const auto& f : firsts)
						for (const auto& s : f)
							first_beta.insert(s);

					for (const auto& f : first_beta)
					{
						canonical_table new_item(B, 0, rule, {f}, it.number_table);
						if (std::find(res.begin(), res.end(), new_item) == res.end()) {
							res.push_back(new_item);
							queue.push_back(new_item); // ������ �����!
						}
					}
				}
			}
		}
	}

	return res;
}

// ������ ���� ������ ����������
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

// ������ ���� ������ ����������
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

// ������ ���� ������������
void Sintax::Print_Nonterminals()
{
	cout << "Nonterminals: ";
	for (const auto& nonterminal : nonterminals)
	{
		cout << nonterminal << " ";
	}
	cout << endl;
}

// ������ ���� ������������
void Sintax::Write_Nonterminals(ofstream& file)
{
	file << "Nonterminals: ";
	for (const auto& nonterminal : nonterminals)
	{
		file << nonterminal << " ";
	}
	file << endl;
}

// ������ ���� ����������
void Sintax::Print_Terminals()
{
	cout << "Terminals: ";
	for (const auto& terminal : terminals)
	{
		cout << terminal << " ";
	}
	cout << endl;
}

// ������ ���� ����������
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
