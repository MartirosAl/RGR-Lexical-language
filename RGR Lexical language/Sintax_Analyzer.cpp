#include "Sintax_Analyzer.h"

// ����������� ������ Sintax
Sintax::Sintax(string file_name)
{
	fstream file(file_name);
	if (!file.is_open())
	{
		cerr << "Error opening file: " << file_name << endl;
		return;
	}

	// �������������� ������ ���������� � ��������� ����
	Rule_to_code(file);

	for (auto& [key, value] : map_rules)
	{
		cout << key << " -> ";
		for (const auto& rule : value)
		{
			for (const auto& word : rule)
			{
				cout << word << " ";
			}
			cout << "| ";
		}
		cout << endl;
	}

	Print_Nonterminals();
	Print_Terminals();

	// �������� ��������������� ������ ��� ��������������� �������
	Create_Tables();
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

			if (find(terminals.begin(), terminals.end(), word) == terminals.end())
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
			rule.push_back(word);
			if (map_rules.find(key) == map_rules.end())
				map_rules[key] = { rule };
			else
				map_rules[key].push_back(rule);

			if (find(terminals.begin(), terminals.end(), word) == terminals.end())
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
void Sintax::Create_Tables()
{
	vector<vector<vector<string>>> Firsts;
	// ��� ������� ����������� ��������� ��������� FIRST
	for (auto i = nonterminals.begin(); i != nonterminals.end(); i++)
	{
		Firsts.push_back(FIRST_One((*i), { }));
	}

	Print_Firsts(Firsts);

	string start_nonterminal = "<S>"; // ��������� ���������� ����������

	// �������� ������� ���������� �����������
	while (find(nonterminals.begin(), nonterminals.end(), start_nonterminal) == nonterminals.end())
	{
		cout << "Enter the starting nonterminal";
		cin >> start_nonterminal;
	}

	vector<canonical_table> table;
	vector<canonical_table> start_table;
	list<for_goto> table_goto_args;

	// ������������ ��������� �������
	start_table = Start_Table(start_nonterminal);

	Print_Canonical_Table(start_table);

	table = start_table;

	// ����� ���� ������������, ��������� �� ������ � ��������� �������
	for (auto& i : Find_All_Goto(table))
	{
		if (table_goto_args.empty() || find(table_goto_args.begin(), table_goto_args.end(), i) == table_goto_args.end())
		{
			table_goto_args.push_back(i);
			cout << "Find goto: " << i.number_table << " " << i.symbol << endl;
		}
	}

	int counter = 0;
	for (auto& i : table_goto_args)
	{
		counter++;
		cout << "GOTO(" << i.number_table << ", " << i.symbol << ") = { ";
		vector<canonical_table> result = GOTO(i, table, i.number_table);
		for (auto& j : result)
		{
			cout << "[" << j.nonterminal << " -> ";
			int rule_counter = 0;
			for (auto& k : j.rule)
			{
				if (rule_counter++ == j.dot)
					cout << ". ";
				cout << k << " ";
			}
			cout << "] ";
		}
		cout << "}" << endl;
	}

	

	


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
				for (int i = 0; i < temp.size(); i++)
					temp2.push_back(words);

				if (res.empty())
					res = Cartesian_Product(temp2, temp);
				else
					res = Cartesian_Product(res, Cartesian_Product(temp2, temp));
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
vector<vector<string>> Sintax::FIRST_One_for_next(vector<string>::iterator t, const vector<string>& r)
{
	if (r.size() == 0)
		return {};
	vector<vector<string>> result;
	vector<string> prefix;

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
		if (find(nonterminals.begin(), nonterminals.end(), s) != nonterminals.end())
			return true;
		else
			Error("Missing nonterminal");
	else
		if (find(nonterminals.begin(), nonterminals.end(), s) != nonterminals.end())
			Error("Incorrect nonterminal");
	return false;
}

// ��������, �������� �� ������ ����������
bool Sintax::IsTerminal(string s)
{
	if (!IsNonterminal(s))
		if (find(terminals.begin(), terminals.end(), s) != terminals.end())
			return true;
		else
			Error("Missing terminal");
	else
		if (find(terminals.begin(), terminals.end(), s) != terminals.end())
			Error("Incorrect terminal");
	return false;
}

// ��������, �������� �� ������ �������� ������
bool Sintax::IsKeyword(string s)
{
	if (s[0] == '[' && s[s.size() - 1] == ']')
		if (find(Keywords.begin(), Keywords.end(), s) != Keywords.end())
			return true;
		else
			Error("Missing keyword");
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
	if (can_t.rule.size() < can_t.dot)
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
		cout << ", ";
		for (auto& j : i.following)
		{
			if (j != *i.following.begin())
				cout << " | ";
			cout << j;
		}
		cout << "]" << endl;
	}
	cout << "}" << endl;
}

vector<Sintax::canonical_table> Sintax::GOTO(const for_goto& args, const vector<canonical_table>& can_t, int number_table)
{
	for (auto& i : can_t)
	{
		if (i.number_table != number_table)
			continue; // ���� ����� ������� �� ���������, ����������
		if (i.rule.size() <= i.dot)
			continue; // ���� ����� � ����� �������, ����������
		if (i.rule[i.dot] == args.symbol) // ���� ��������� ������ ��������� � ������� ������������
		{
			vector<canonical_table> res;
			res.push_back(i); // ��������� ������� ��������� � ���������
			return res; // ���������� ��������� ���������
		}
	}
	return {}; // ���� ������ �� �������, ���������� ������ ������
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