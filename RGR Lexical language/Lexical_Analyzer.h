#pragma once
#include "BigNumber.h"
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <variant>
using namespace std;

vector<string> TokenTypeString
{
      "VARIABLE",
      "CONSTANT",
      "GET",
      "MARK",
      "GO_TO_MARK",
      "VARIABLE_TYPE",
      "ARITHMETIC_OPERATION",
      "RELATION",

      "EMPTY_OPERATOR",
      "DECLARING_VARIABLES",
      "AS",
      "ASSIGNMENT_OPERATOR",
      "WHILE",
      "DO",
      "OD",
      "FOR",
      "FROM",
      "TO",
      "BY",
      "IF",
      "ELSE",
      "FI",
      "SELECT",
      "IN",
      "CASE",
      "OTHERWISE",
      "NI",
      "INPUT",
      "PRINT",

      "RAISE",
      "COMMENT",
      "ERROR",
      "O_BRACE",
      "C_BRACE",

      "O_COMMENT",
      "C_COMMENT",
      "CASE_LISTING"
};

class TableToken
{

   enum SymbolicTokenType
   {
      start_S = -1,
      LETTER,
      DIGIT,
      ARITHMETIC_OPERATION_S,
      RELATION_S,
      SPACE,
      LF,
      SEMI_COLON,
      ERROR_S,
      O_BRACE_S,
      C_BRACE_S,
      UNDERLINING,
      END,
      END_MARKER,
      COMMA
   };

   enum TokenType
   {
      start = -1,
      VARIABLE,
      CONSTANT,
      GET,
      MARK,
      GO_TO_MARK,
      VARIABLE_TYPE,
      ARITHMETIC_OPERATION,
      RELATION,

      EMPTY_OPERATOR,
      DECLARING_VARIABLES,
      AS,
      ASSIGNMENT_OPERATOR,
      WHILE,
      DO,
      OD,
      FOR,
      FROM,
      TO,
      BY,
      IF,
      ELSE,
      FI,
      SELECT,
      IN,
      CASE,
      OTHERWISE,
      NI,
      INPUT,
      PRINT,

      RAISE,
      GET,
      COMMENT,
      ERROR,
      O_BRACE,
      C_BRACE,
      COMMA,

      O_COMMENT,
      C_COMMENT,
      CASE_LISTING

   };

   struct SymbolicToken
   {
      SymbolicTokenType token_class = start_S;
      int value = 0;
   };

   struct Token
   {
      TokenType token_class = (TokenType)start;
      //get<0> - это int значение для отношений и ариф. операций
      //get<1> - это ячейка для таблицы констант
      //get<2> - это ячейка для таблицы переменных
      //get<3> - это ячейка для таблицы меток
      //
      //get<0> is an int value for relations and arif. operations
      //get<1> is the cell for the table of constants.
      //get<2> is the cell for the variable table.
      //get<3> is the cell for the mark table.
      variant<string, set<variant<int, BigNumber>>::iterator, map<string, variant<int, BigNumber>>::iterator, vector<string>::iterator> value = " ";
      set<variant<int, BigNumber>>::iterator second_argument_get;
      int number_line = 0;
   };

   vector<SymbolicTokenType> types_for_names_variables
   {
      LETTER, DIGIT, UNDERLINING
   };

   //ТАБЛИЦЫ//
   //TABLES//

   //Таблица констант
   //Table of constants
   set<variant<int, BigNumber>> table_constants;

   //Таблица переменных
   //Table of variables
   map<string, variant<int, BigNumber>> table_variable;

   //Таблица меток
   //Table of lables(marks)
   vector<string> table_marks;


   //Таблица лексем для вывода
   //Вектор вариантов (int, set<variant<int, BigNumber>>::iterator, map<string, variant<int, BigNumber>>::iterator)
   //Table of tokens for output
   //Vector of variants (int, set<variant<int, BigNumber>>::iterator, map<string, variant<int, BigNumber>>::iterator)
   vector<Token> table_tokens;

   //Таблицы для обнаружения ключевых слов
   //Одинаковые элементы удалятся
   //Tables for keyword detection
   //Identical elements are deleted
   map<string, TokenType> table_detection
   {
      {"int", VARIABLE_TYPE}, {"BigNumber", VARIABLE_TYPE},
      {"declare", DECLARING_VARIABLES},                                           {"as", AS},
      {"while", WHILE},                                               {"do", DO}, {"od", OD},
      {"for", FOR},           {"from", FROM}, {"to", TO}, {"by", BY}, {"do", DO}, {"od", OD},
      {"if", IF},                                                 {"else", ELSE}, {"fi", FI},
      {"input", INPUT},
      {"print", PRINT},
      {"goto", GO_TO_MARK},
      {"select", SELECT},    {"in", IN}, {"case", CASE}, {"otherwise", OTHERWISE}, {"ni", NI},
      {"raise", RAISE},
      {"get", GET}
   };

   map<string, TokenType> table_operations
   {
      {";", TokenType::EMPTY_OPERATOR},       {"=", TokenType::ASSIGNMENT_OPERATOR},
      {"+", TokenType::ARITHMETIC_OPERATION}, {"-", TokenType::ARITHMETIC_OPERATION}, {"*", TokenType::ARITHMETIC_OPERATION},
      {"/", TokenType::ARITHMETIC_OPERATION}, {"%", TokenType::ARITHMETIC_OPERATION}, {"(", TokenType::O_BRACE}, 
      {")", TokenType::C_BRACE},              {"<", TokenType::RELATION},             {">", TokenType::RELATION}, 
      {"==", TokenType::RELATION},            {"<=", TokenType::RELATION},            {">=", TokenType::RELATION},
      {"!=", TokenType::RELATION},            
      {"<<<", TokenType::O_COMMENT},          {">>>", TokenType::C_COMMENT},          {":", TokenType::CASE_LISTING},
   };

   //Процедура ДОБАВИТЬ_КОНСТАНТУ
   //Procedure TO ADD_CONSTANT
   void Add_Constant(string a)
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

   void Add_Constant(vector<short> a)
   {
      BigNumber temp (reverseDigits(a), a.size());

      table_constants.emplace(temp);

      register_indicator = table_constants.find(temp);
   }
   
   //Процедура ДОБАВИТЬ_МЕТКУ
   //Procedure ADD_THE_MARK
   void Add_Mark(string a)
   {
      if (!count(table_marks.begin(), table_marks.end(), a))
      {
         table_marks.push_back(a);
      }
      register_indicator = find(table_marks.begin(), table_marks.end(), a);
   }

   //Процедура ДОБАВИТЬ_ПЕРЕМЕННУЮ
   //Procedure ADD_THE_VARIABLE
   void Add_Variable(string word)
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

   //Процедура СОЗДАТЬ_ЛЕКСЕМУ
   //Procedure CREATE_A_TOKEN
   void Create_Token(string value_ = " ")
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

   //Процедура обработки ошибок
   //Error handling procedure
   void Error_Handler(string error)
   {
      register_type_token = TokenType::ERROR;

      cerr << "An error was found in the number line " << number_line << "; " << error << endl;
   }


   void Error_Handler_Operation(string error)
   {
      register_type_token = TokenType::ERROR;

      cerr << "An error was found in the number line " << number_line << "; This operation was not found \"" << error << "\"" << endl;
   }

   void Error_Handler_Variable(string error)
   {
      register_type_token = TokenType::ERROR;

      cerr << "An error was found in the number line " << number_line << "; Wrong variable \"" << error << "\"" << endl;
      cerr << "A variable cannot start with a digit." << endl;
   }


   //ПЕРЕМЕННЫЕ//
   //VARIABLES//

   //Регистр класса служит для хранения класса лексемы
   //The class register is used to store the class of the token
   TokenType register_type_token = (TokenType)start;

   //Регистр указателя содержит указатель на таблицу имён
   //get<0> - итератор(указатель) на таблицу констант
   //get<1> - итератор(указатель) на таблицу переменных
   //get<2> - итератор(указатель) на таблицу меток
   // 
   //The pointer register contains a pointer to the table of names
   //get<0> - iterator(pointer) to the table of constants
   //get<1> - iterator(pointer) to the table of variables
   //get<2> - iterator(pointer) to the table of marks
   variant<set<variant<int, BigNumber>>::iterator, map<string, variant<int, BigNumber>>::iterator, vector<string>::iterator> register_indicator;

   //Номер строки хранит номер текущей строки в программе
   //Line number stores the number of the current line in the program
   int number_line = 1;

   Token token;

   SymbolicToken symbolic_token;

   bool Is_in_variable(SymbolicTokenType a)
   {
      return (find(types_for_names_variables.begin(), types_for_names_variables.end(), a) != types_for_names_variables.end());
   }

   bool Is_this_variable(string word)
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

   bool Is_this_constant(string constant)
   {
      for (auto i : constant)
      {
         if (!(i >= '0' && i <= '9'))
            return false;
      }
      return true;
   }

   vector<short> reverseDigits(vector<short> num)
   {
      vector<short> rev_num;
      for (size_t i = num.size(); i > 0; i--)
      {
         rev_num.push_back(num[i - 1]);
      }
      return rev_num;
   }

   bool S_more_I(string a)
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

   vector<short> from_string_to_vector_short(string a)
   {
      vector<short> b;
      for (auto i : a)
      {
         b.push_back(i - '0');
      }
      return b;
   }

   bool Is_this_start_comment(string a)
   {
      if (a.size() < 3)
         return false;
      if (a[0] == a[1] && a[1] == a[2] && a[0] == '<')
         return true;
      return false;
   }

   bool Is_this_end_comment(string a)
   {
      if (a.size() < 3)
         return false;
      if (a[a.size()-1] == a[a.size()-2] && a[a.size()-2] == a[a.size()-3] && a[a.size()-1] == '>')
         return true;
      return false;
   }

   bool Is_this_o_braces(string a)
   {
      if (a[0] == '(')
         return true;
      return false;
   }

   bool Is_this_c_braces(string a)
   {
      if (a[0] == ')')
         return true;
      return false;
   }

   bool Is_this_empty_operators(string a)
   {
      if (a[0] == ';')
         return true;
      return false;
   }

   bool Is_this_o_mark(string a)
   {
      if (a.size() < 2)
         return false;
      if (a[0] == a[1] && a[0] == '<')
         return true;
      return false;
   }

   bool Is_this_c_mark(string a)
   {
      if (a[a.size() - 1] == a[a.size() - 2] && a[a.size() - 1] == '>')
         return true;
      return false;
   }


   SymbolicToken Transliterator(int character)
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
         result.token_class = SymbolicTokenType::COMMA;
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

public:
   vector<Token> Lexical_Analyzer(const char* filename)
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
      bool flag_mark = false;
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
               Create_Token(accumulation_of_value);
            }

            continue;
         }

         if (flag_mark)
         {
            if (symbolic_token.token_class != LETTER && symbolic_token.token_class != DIGIT && symbolic_token.value != '>')
            {
               Error_Handler("Wrong symbol in mark");
               table_tokens[table_tokens.size() - 1].token_class = register_type_token;
            }
            if (symbolic_token.token_class == END || symbolic_token.token_class == LF)
            {
               Error_Handler("The mark is not closed");
               break;
            }
            accumulation_of_value += symbolic_token.value;
            if (Is_this_c_mark(accumulation_of_value))
            {
               flag_mark = false;

               accumulation_of_value.resize(accumulation_of_value.size() - 2);

               Add_Mark(accumulation_of_value);
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

         if (flag_get)
         {
            ;
         }

         if (prev_character == symbolic_token.token_class || (Is_in_variable(symbolic_token.token_class) && Is_in_variable(prev_character)))
         {
            accumulation_of_value += symbolic_token.value;
         }
         else if (prev_character == SymbolicTokenType::SPACE || prev_character == SymbolicTokenType::LF)
            accumulation_of_value = symbolic_token.value;
         else if (prev_token == GO_TO_MARK)
         {
            Add_Mark(accumulation_of_value);
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
               else if (Is_this_o_mark(accumulation_of_value))
               {
                  register_type_token = TokenType::MARK;
                  flag_mark = true;
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
      
      return table_tokens;
   }

   void Print_Table_Token()
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

   void Print_Token(Token i)
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

};

