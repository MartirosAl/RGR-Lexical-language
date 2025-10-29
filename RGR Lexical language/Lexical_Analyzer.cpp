#include "Lexical_Analyzer.h"
#include "Sintax_Analyzer.h"

void TableToken::Add_Constant(string a)
{
    if (S_more_I(a))
    {
        if (!Is_this_constant(a))
            Add_Variable(a);
        else
            Add_Constant(from_string_to_vector_short(a));
        return;
    }

    if (!Is_this_constant(a))
    {
        Add_Variable(a);
        return;
    }

    int constant = stoi(a);

    table_constants.emplace(constant);

    register_indicator = table_constants.find(constant);
}

void TableToken::Add_Constant(vector<short> a)
{
    BigNumber temp(reverseDigits(a), a.size());

    table_constants.emplace(temp);

    register_indicator = table_constants.find(temp);
}

void TableToken::Add_Label(string a)
{
    if (find(table_labels.begin(), table_labels.end(), a) == table_labels.end())
    {
        table_labels.push_back(a);
    }
    register_indicator = find(table_labels.begin(), table_labels.end(), a);
}

void TableToken::Add_Variable(string word)
{

    if (!Is_this_variable(word))
    {
        Error_Handler_Variable(word);
        return;
    }

    if (table_variable.count(word) == 0)
    {
        table_variable[word] = 0;
    }

    register_indicator = table_variable.find(word);
    register_type_token = TokenType::VARIABLE;
}

void TableToken::Create_Token(string value_)
{
    Token result;

    if (register_type_token >= 3)
        result.value = value_;
    else if (register_indicator.index() == 1)
    {
        result.value = get<1>(register_indicator);
    }
    else if (register_indicator.index() == 0)
    {
        result.value = get<0>(register_indicator);
    }

    result.token_class = register_type_token;
    result.number_line = number_line;

    table_tokens.push_back(result);
}

void TableToken::Error_Handler(string error)
{
    register_type_token = TokenType::ERROR;

    cerr << "An error was found in the number line " << number_line << "; " << error << endl;
}
void TableToken::Error_Handler_Operation(string error)
{
    register_type_token = TokenType::ERROR;

    cerr << "An error was found in the number line " << number_line << "; This operation was not found \"" << error << "\"" << endl;
}

void TableToken::Error_Handler_Variable(string error)
{
    register_type_token = TokenType::ERROR;

    cerr << "An error was found in the number line " << number_line << "; Wrong variable \"" << error << "\"" << endl;
    cerr << "A variable cannot start with a digit." << endl;
}
bool TableToken::Is_in_variable(SymbolicTokenType a)
{
    return (find(types_for_names_variables.begin(), types_for_names_variables.end(), a) != types_for_names_variables.end());
}

bool TableToken::Is_this_variable(string word)
{
    if (!((word[0] >= 'a' && word[0] <= 'z') || (word[0] >= 'A' && word[0] <= 'Z') || word[0] == '_'))
        return false;
    for (auto i : word)
    {
        if (!((i >= '0' && i <= '9') || (i >= 'a' && i <= 'z') || (i >= 'A' && i <= 'Z') || (i == '_')))
            return false;
    }
    return true;
}

bool TableToken::Is_this_constant(string constant)
{
    for (auto i : constant)
    {
        if (!(i >= '0' && i <= '9'))
            return false;
    }
    return true;
}

vector<short> TableToken::reverseDigits(vector<short> num)
{
    vector<short> rev_num;
    for (size_t i = num.size(); i > 0; i--)
    {
        rev_num.push_back(num[i - 1]);
    }
    return rev_num;
}

bool TableToken::S_more_I(string a)
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

vector<short> TableToken::from_string_to_vector_short(string a)
{
    vector<short> b;
    for (auto i : a)
    {
        b.push_back(i - '0');
    }
    return b;
}

bool TableToken::Is_this_start_comment(string a)
{
    if (a.size() < 3)
        return false;
    if (a[0] == a[1] && a[1] == a[2] && a[0] == '<')
        return true;
    return false;
}
bool TableToken::Is_this_end_comment(string a)
{
    if (a.size() < 3)
        return false;
    if (a[a.size() - 1] == a[a.size() - 2] && a[a.size() - 2] == a[a.size() - 3] && a[a.size() - 1] == '>')
        return true;
    return false;
}
bool TableToken::Is_this_o_braces(string a)
{
    if (a[0] == '(')
        return true;
    return false;
}
bool TableToken::Is_this_c_braces(string a)
{
    if (a[0] == ')')
        return true;
    return false;
}

bool TableToken::Is_this_empty_operators(string a)
{
    if (a[0] == ';')
        return true;
    return false;
}
bool TableToken::Is_this_o_label(string a)
{
    if (a.size() < 2)
        return false;
    if (a[0] == a[1] && a[0] == '<')
        return true;
    return false;
}

bool TableToken::Is_this_c_label(string a)
{
    if (a[a.size() - 1] == a[a.size() - 2] && a[a.size() - 1] == '>')
        return true;
    return false;
}

TableToken::SymbolicToken TableToken::Transliterator(int character)
{
    SymbolicToken result;
    result.value = 0;
    if (character >= 'A' && character <= 'Z' || character >= 'a' && character <= 'z')
    {
        result.token_class = SymbolicTokenType::LETTER;
        result.value = (int)character;
    }
    else if (character >= '0' && character <= '9')
    {
        result.token_class = SymbolicTokenType::DIGIT;
        result.value = (int)character;
    }
    else if (character == '_')
    {
        result.token_class = SymbolicTokenType::UNDERLINING;
        result.value = (int)character;
    }
    else if (character == '+' || character == '-' || character == '*' || character == '/' || character == '%')
    {
        result.token_class = SymbolicTokenType::ARITHMETIC_OPERATION_S;
        result.value = (int)character;
    }
    else if (character == '<' || character == '>' || character == '=' || character == '!')
    {
        result.token_class = SymbolicTokenType::RELATION_S;
        result.value = (int)character;
    }
    else if (character == '(')
    {
        result.token_class = SymbolicTokenType::O_BRACE_S;
        result.value = (int)character;
    }
    else if (character == ')')
    {
        result.token_class = SymbolicTokenType::C_BRACE_S;
        result.value = (int)character;
    }
    else if (character == ' ' || character == '\t')
    {
        result.token_class = SymbolicTokenType::SPACE;
        result.value = (int)character;
    }
    else if (character == ';')
    {
        result.token_class = SymbolicTokenType::SEMI_COLON;
        result.value = (int)character;
    }
    else if (character == ',')
    {
        result.token_class = SymbolicTokenType::COMMA_S;
        result.value = (int)character;
    }
    else if (character == '\n')
    {
        result.token_class = SymbolicTokenType::LF;
        result.value = (int)character;
    }
    else if (character == EOF)
    {
        result.token_class = SymbolicTokenType::END;
        result.value = (int)character;
    }
    else
    {
        result.token_class = SymbolicTokenType::ERROR_S;
        result.value = (int)character;
    }
    return result;
}

vector<TableToken::Token> TableToken::Lexical_Analyzer(const string filename)
{
    ifstream in(filename);
    if (!in)
    {
        cout << "Couldn't open the file " << filename << endl;
        return table_tokens;
    }

    string word;

    bool flag = true;

    SymbolicTokenType prev_character;
    TokenType prev_token = start;
    prev_character = Transliterator(in.peek()).token_class;
    string accumulation_of_value;
    bool flag_comment = false;
    bool flag_label = false;
    bool flag_get = false;

    while (flag)
    {
        int character = in.get();
        symbolic_token = Transliterator(character);

        if (flag_comment)
        {
            if (symbolic_token.token_class == END)
            {
                Error_Handler("The comment is not closed");
                break;
            }
            if (symbolic_token.token_class == LF)
            {
                number_line++;
            }
            accumulation_of_value += symbolic_token.value;
            if (Is_this_end_comment(accumulation_of_value))
            {
                flag_comment = false;
                register_type_token = TokenType::C_COMMENT;
                prev_character = Transliterator(in.peek()).token_class;
                accumulation_of_value.clear();
                table_tokens[table_tokens.size() - 1].token_class = COMMENT;
            }

            continue;
        }

        if (flag_label)
        {
            if (symbolic_token.token_class != LETTER && symbolic_token.token_class != DIGIT && symbolic_token.value != '>')
            {
                Error_Handler("Wrong symbol in label");
                table_tokens[table_tokens.size() - 1].token_class = register_type_token;
            }
            if (symbolic_token.token_class == END || symbolic_token.token_class == LF)
            {
                Error_Handler("The label is not closed");
                break;
            }
            accumulation_of_value += symbolic_token.value;
            if (Is_this_c_label(accumulation_of_value))
            {
                flag_label = false;

                accumulation_of_value.resize(accumulation_of_value.size() - 2);

                Add_Label(accumulation_of_value);
                accumulation_of_value.clear();

                table_tokens[table_tokens.size() - 1].value = get<2>(register_indicator);

                prev_character = Transliterator(in.peek()).token_class;
            }

            continue;
        }


        if (flag_get)
        {
            if (prev_character != O_BRACE_S)
            {
                Error_Handler("The function argument is missing");
                flag_get = false;
                continue;
            }
            accumulation_of_value.clear();

            if (symbolic_token.token_class != DIGIT)
            {
                Error_Handler("The function argument is wrong");
                flag_get = false;
                continue;
            }
            while (symbolic_token.token_class == DIGIT)
            {
                accumulation_of_value += symbolic_token.value;
                character = in.get();
                symbolic_token = Transliterator(character);
            }

            if (symbolic_token.token_class != COMMA_S)
            {
                Error_Handler("The function argument is wrong");
                flag_get = false;
                continue;
            }
            Add_Constant(accumulation_of_value);
            table_tokens[table_tokens.size() - 1].value = get<0>(register_indicator);
            character = in.get();
            symbolic_token = Transliterator(character);
            if (symbolic_token.token_class != DIGIT)
            {
                Error_Handler("The function argument is wrong");
                flag_get = false;
                continue;
            }
            accumulation_of_value.clear();
            while (symbolic_token.token_class == DIGIT)
            {
                accumulation_of_value += symbolic_token.value;
                character = in.get();
                symbolic_token = Transliterator(character);
            }

            if (symbolic_token.token_class != C_BRACE_S)
            {
                Error_Handler("The function argument is wrong");
                flag_get = false;
                continue;
            }
            Add_Constant(accumulation_of_value);
            table_tokens[table_tokens.size() - 1].second_argument_get = get<0>(register_indicator);
            flag_get = false;
            accumulation_of_value.clear();
            prev_character = Transliterator(in.peek()).token_class;
            continue;
        }

        if (prev_character == symbolic_token.token_class || (Is_in_variable(symbolic_token.token_class) && Is_in_variable(prev_character)))
        {
            accumulation_of_value += symbolic_token.value;
        }
        else if (prev_character == SymbolicTokenType::SPACE || prev_character == SymbolicTokenType::LF)
            accumulation_of_value = symbolic_token.value;
        else if (prev_token == GO_TO_LABEL)
        {
            Add_Label(accumulation_of_value);
            table_tokens[table_tokens.size() - 1].value = get<2>(register_indicator);
            accumulation_of_value = symbolic_token.value;
            register_type_token = start;
        }
        else
        {

            switch (prev_character)
            {
            case (SymbolicTokenType::LETTER):

                if (table_detection.contains(accumulation_of_value))
                {
                    register_type_token = table_detection[accumulation_of_value];
                }
                else
                {
                    register_type_token = TokenType::VARIABLE;
                    Add_Variable(accumulation_of_value);
                }
                break;

            case (SymbolicTokenType::DIGIT):

                register_type_token = TokenType::CONSTANT;
                Add_Constant(accumulation_of_value);
                break;

            default:

                if (Is_this_start_comment(accumulation_of_value))
                {
                    register_type_token = TokenType::O_COMMENT;
                    flag_comment = true;
                }
                else if (Is_this_o_label(accumulation_of_value))
                {
                    register_type_token = TokenType::LABEL;
                    flag_label = true;
                }
                else if (table_operations.count(accumulation_of_value))
                {
                    register_type_token = table_operations[accumulation_of_value];
                }
                else if (Is_this_o_braces(accumulation_of_value))
                {
                    for (int i = 0; i < accumulation_of_value.size() - 1; i++)
                    {
                        register_type_token = TokenType::O_BRACE;
                        Create_Token(accumulation_of_value);
                    }
                }
                else if (Is_this_c_braces(accumulation_of_value))
                {
                    for (int i = 0; i < accumulation_of_value.size() - 1; i++)
                    {
                        register_type_token = TokenType::C_BRACE;
                        Create_Token(accumulation_of_value);
                    }
                }
                else if (Is_this_empty_operators(accumulation_of_value))
                {
                    for (int i = 0; i < accumulation_of_value.size() - 1; i++)
                    {
                        register_type_token = TokenType::EMPTY_OPERATOR;
                        Create_Token();
                    }
                }
                else
                    Error_Handler_Operation(accumulation_of_value);
            }

            Create_Token(accumulation_of_value);
            accumulation_of_value = symbolic_token.value;

        }

        if (symbolic_token.token_class == LF)
            number_line++;
        else if (symbolic_token.token_class == END)
            flag = false;

        if (register_type_token == GET)
            flag_get = true;

        prev_character = symbolic_token.token_class;
        prev_token = register_type_token;



    }
    
    in.close();
    return table_tokens;
}

void TableToken::Print_Table_Token()
{
    for (auto& i : table_tokens)
    {
        cout << i.number_line << " ";
        cout << TokenTypeString[i.token_class] << " ";
        if (i.token_class >= 7)
            ;//Nothing
        else if (i.token_class == GET)
        {
            if (get<1>(i.value)->index() == 0)
                cout << get<0>(*(get<1>(i.value))) << " ";
            else
                cout << get<1>(*(get<1>(i.value))) << " ";

            if (i.second_argument_get->index() == 0)
                cout << get<0>(*(i.second_argument_get)) << " ";
            else
                cout << get<1>(*(i.second_argument_get)) << " ";
        }
        else if (i.value.index() == 0)
            cout << get<0>(i.value);
        else if (i.value.index() == 3)
        {
            cout << *get<3>(i.value) << " ";
        }
        else if (i.value.index() == 2)
        {
            if (get<2>(i.value)->second.index() == 0)
                cout << get<2>(i.value)->first << " ";
            else
                cout << get<2>(i.value)->first << " ";
        }
        else if (i.value.index() == 1)
        {
            if (get<1>(i.value)->index() == 0)
                cout << get<0>(*(get<1>(i.value))) << " ";
            else
                cout << get<1>(*(get<1>(i.value))) << " ";
        }
        cout << endl;
    }
}

void TableToken::Print_Token(Token i)
{
    cout << i.number_line << " ";
    cout << TokenTypeString[i.token_class] << " ";
    if (i.token_class >= 7)
        ;//Nothing
    else if (i.value.index() == 0)
        cout << get<0>(i.value);
    else if (i.value.index() == 3)
    {
        cout << *get<3>(i.value) << " ";
    }
    else if (i.value.index() == 2)
    {
        if (get<2>(i.value)->second.index() == 0)
            cout << get<2>(i.value)->first << " ";
        else
            cout << get<2>(i.value)->first << " ";
    }
    else if (i.value.index() == 1)
    {
        if (get<1>(i.value)->index() == 0)
            cout << get<0>(*(get<1>(i.value))) << " ";
        else
            cout << get<1>(*(get<1>(i.value))) << " ";
    }
    cout << endl;
}