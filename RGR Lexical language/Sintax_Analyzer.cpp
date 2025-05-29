#include "Sintax_Analyzer.h"

void Sintax::Rule_to_code(fstream file)
{
	string key;
	string word;
	map<list<list<string>>, vector<string>> rules;
	list<string> rule;

	int number_string = 0;

	bool nonterminal_flag = 1;

	while (file.peek() != EOF)
	{

		if (file.peek() == ' ')
			continue;

		if (file.peek() != '\n' || file.peek() != EOF)
		{
			if (nonterminal_flag)
			{
				if (file.peek() == '-')
				{
					file.get();
					if (file.peek() == '>')
					{
						nonterminal_flag = 0;
					}
					else
					{
						key.push_back('-');
					}
				}
				key.push_back(file.get());
			}
			else
			{
				if (file.peek() == '<')
				{
					word = file.get();
					while (file.peek() != '>')
					{
						word.push_back(file.get());
					}
				}
				else
				{
					while (file.peek() != ' ' && file.peek() != '<' && file.peek() != '\n' && file.peek() != EOF)
						word.push_back(file.get());
				}

				rule.push_back(word);

			}
		}
		else
		{
			if (map_rules.find(key) != map_rules.end())
			{
				rules.push_back(rule);
				map_rules.insert({ key, rules });
				rules.clear();
			}
			else
				map_rules[key].push_back(rule);
			rule.clear();
		}
	}
}

