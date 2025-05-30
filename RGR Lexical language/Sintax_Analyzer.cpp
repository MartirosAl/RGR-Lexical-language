#include "Sintax_Analyzer.h"

Sintax::Sintax(string file_name)
{
	Lexical_Analyzer(file_name);
	fstream file(file_name);

	if (!file.is_open())
	{
		cerr << "Error opening file: " << file_name << endl;
		return;
	}

	Rule_to_code(file);

	Create_Tables();
}

void Sintax::Rule_to_code(fstream& file)
{
	string word;
	string key;
	list<list<string>> rules;
	list<string> rule;

	int number_string = 0;

	bool nonterminal_flag = 1;

	//Find_Nonterminals(file);

	while (file.peek() != EOF)
	{
		if (nonterminal_flag)
		{
			while (file.peek() != '<' && file.peek() != '\n' && file.peek() != EOF)
			{
				file.get();
				continue;
			}
			if (file.peek() == '<')
			{
				while (file.peek() != '>')
				{
					word += file.get();
				}
				word += file.get(); // to get '>'
				while (file.peek() == ' ')
					file.get();
				
				if (file.get() == '-' && file.get() == '>') // to get '->'
				{
					nonterminal_flag = 0;
					key = word; // the first word is the key
					if (find(nonterminals.begin(), nonterminals.end(), key) == nonterminals.end())
					{
						nonterminals.push_back(key);
						//Rule_Error("Nonterminal '" + key + "' not found in the list of nonterminals.", file);
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
		

		switch (file.peek())
		{
		case '\n':
			file.get();
			number_string++;
			if (!word.empty())
			{
				rule.push_back(word);
			}

			rules.push_back(rule);
			if (map_rules.find(key) == map_rules.end())
			{
				map_rules[key] = rules;
			}
			if (find(terminals.begin(), terminals.end(), word) == terminals.end())
				terminals.push_back(word);
			word.clear();
			rule.clear();
			rules.clear();
			nonterminal_flag = 1; // reset the flag for the next rule
			break;

		case ' ':
		case '\t':
			file.get();
			break;

		case '[':
			if (!word.empty())
			{
				rule.push_back(word);
				word.clear();
			}

			while (file.peek() != ']')
			{
				word += file.get();
			}

			word += file.get(); // to get ']'
			if (find(terminals.begin(), terminals.end(), word) == terminals.end())
				terminals.push_back(word);
			rule.push_back(word);
			word.clear();
			break;

		case '<':
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
			word += file.get(); // to get '>'
			rule.push_back(word);
			word.clear();
			break;
		
		default:
			word += file.get();
			break;
		}



		if (file.peek() == EOF)
		{
			rule.push_back(word);
			rules.push_back(rule);
			if (map_rules.find(key) == map_rules.end())
			{
				map_rules[key] = rules;
			}
			if (find(terminals.begin(), terminals.end(), word) == terminals.end())
				terminals.push_back(word);
			word.clear();
			rule.clear();
			rules.clear();
		}
	}
}

void Sintax::Rule_Error(string error_text, fstream& file)
{
	cerr << "Error: " << error_text << endl;
	while (file.peek() != '\n' && file.peek() != EOF)
		file.get(); // skip to the end of the line
	exit(EXIT_FAILURE);
}

void Sintax::Error(string error_text)
{
	cerr << "Error: " << error_text << endl;
	exit(EXIT_FAILURE);
}

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

void Sintax::Create_Tables()
{
	list<list<list<string>>> Firsts;
	for (auto i = terminals.begin(); i != terminals.end(); i++)
	{
		Firsts.push_back(FIRST(1, (*i)));
	}

	string start_nonterminal = "<S>";// The beginning of grammar

	while (find(nonterminals.begin(), nonterminals.end(), start_nonterminal) == nonterminals.end()) 
	{
		cout << "Enter the starting nonterminal";
		cin >> start_nonterminal;
	}

	list<canonical_table> table;

	list<canonical_table> start_table;

	list<for_goto> table_goto_args;
	
	start_table = Start_Table(start_nonterminal, Firsts);
	
	table_goto_args.clear();


}

list<list<string>> Sintax::FIRST(int k, string nonterminal)
{
	list<list<string>> res;
	list<string> words;

	if (map_rules.size() == 0)
		Error("There are no rules");

	for (auto i = map_rules[nonterminal].begin(); i != map_rules[nonterminal].end(); i++) // Going through the rules
	{
		words.clear();
		for (auto j = (*i).begin(); j != (*i).end(); j++) // Going through terminals/nonterminals
		{
			
			if (IsTerminal(*j))
			{
				if ((*j) == "[eps]")
				{
					continue;
				}

				words.push_back(*j);

			}
			else
			{
				list<list<string>> temp;
				list<list<string>> temp2;
				temp = FIRST(k - words.size(), (*j));
				for (int i = 0; i < temp.size(); i++)
					temp2.push_back(words);
				
				res = Cartesian_Product(res, Cartesian_Product(temp2, temp));
				words.clear();
				break;
			}

			if (words.size() == k)
			{
				res.push_back(words);
				break;
			}
		}

		if (words.size() == 0)
			res.push_back({ "[eps]" });
		else if (words.size() < k)
			res.push_back(words);
	}

	return res;
}

list<list<string>> Sintax::Cartesian_Product(list<list<string>> to, list<list<string>> from)
{
	list<list<string>> result;

	for (const auto& prefix : to) 
	{
		for (const auto& suffix : from) 
		{
			list<string> combined = prefix;
			combined.insert(combined.end(), suffix.begin(), suffix.end());
			result.push_back(combined);
		}
	}

	return result;

}

list<list<string>> Sintax::Clipping(int n, list<list<string>> from)
{
	list<list<string>> result;

	for (const auto& i : from)
	{
		list<string> cliped = i;
		cliped.resize(n);
		result.push_back(cliped);
	}
	return result;
}

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

list<Sintax::canonical_table> Sintax::Start_Table(string start_nonterminal, list<list<list<string>>> Firsts)
{
	list<canonical_table> res;
	for (auto& i : map_rules[start_nonterminal])
		res.push_back(Sintax::canonical_table(start_nonterminal, 0, i, { "[eps]" }));

	for (auto& i : res)
	{
		auto t = i.rule.begin();
		if (IsNonterminal(*t))
		{
			for (auto& j : map_rules[*t])
			{
				res.push_back(Sintax::canonical_table((*t), 0, j, (*FIRST(1, (*t)).begin())));
				!!! Доделать стартовую таблицу
			}
		}
	}

	return res;
}

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

void Sintax::Print_Nonterminals()
{
	cout << "Nonterminals: ";
	for (const auto& nonterminal : nonterminals)
	{
		cout << nonterminal << " ";
	}
	cout << endl;
}

void Sintax::Print_Terminals()
{
	cout << "Terminals: ";
	for (const auto& terminal : terminals)
	{
		cout << terminal << " ";
	}
	cout << endl;
}

