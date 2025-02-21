#pragma once
#include "BigNumber.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <variant>
#include <map>
#include <set>
using namespace std;

vector<string> SymbolicTokenTypeString
{
   "LETTER",
   "DIGIT",
   "ARITHMETIC_OPERATION",
   "RELATION",
   "SPACE",
   "LF",
   "SEMI_COLON",
   "ERROR",
   "O_BRACE",
   "C_BRACE",
   "UNDERLINING",
   "END",
   "END_MARKER"
};

vector<string> TokenTypeString
{
   "EMPTY_OPERATOR",
   "DECLARING_VARIABLES",
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
   "MARK",
   "GO_TO_MARK",
   "RAISE",
   "COMMENT",
   "ERROR",
   "O_BRACE",
   "C_BRACE",
   "COMMA",
   "O_MARK",
   "C_MARK",
   "O_COMMENT",
   "C_COMMENT",
   "CASE_LISTING",
   "ARITHMETIC_OPERATION",
   "RELATION",
   "VARIABLE_TYPE",
   "VARIABLE",
   "CONSTANT"
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
      END_MARKER
   };

   enum TokenType
   {
      start = -1,
      EMPTY_OPERATOR,
      DECLARING_VARIABLES,
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
      MARK,
      GO_TO_MARK,
      RAISE,
      COMMENT,
      ERROR,
      O_BRACE,
      C_BRACE,
      COMMA,
      O_MARK,
      C_MARK,
      O_COMMENT,
      C_COMMENT,
      CASE_LISTING,
      ARITHMETIC_OPERATION,
      RELATION,
      VARIABLE_TYPE,
      VARIABLE,
      CONSTANT
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
      //get<2> - это ячейка дтя таблицы переменных
      variant<string, set<variant<int, BigNumber>>::iterator, map<string, variant<int, BigNumber>>::iterator> value = " ";
      int number_line = 0;
   };

   vector<SymbolicTokenType> types_for_names_variables
   {
      LETTER, DIGIT, UNDERLINING
   };

   //ТАБЛИЦЫ//

//Таблица констант
   set<variant<int, BigNumber>> table_constants;

   //Таблица переменных
   map<string, variant<int, BigNumber>> table_variable;

   //Таблица лексем для вывода
   //Вектор вариантов (int, set<variant<int, BigNumber>>::iterator, map<string, variant<int, BigNumber>>::iterator)
   vector<Token> table_tokens;

   //Таблицы для обнаружения ключевых слов
   //Одинаковые элементы удалятся
   map<string, TokenType> table_detection
   {
      {"int", VARIABLE_TYPE}, {"BigNumber", VARIABLE_TYPE},
      {"while", WHILE}, {"do", DO}, {"od", OD},
      {"for", FOR}, {"from", FROM}, {"to", TO}, {"by", BY}, {"do", DO}, {"od", OD},
      {"if", IF}, {"else", ELSE}, {"fi", FI},
      {"input", INPUT},
      {"print", PRINT},
      {"goto", GO_TO_MARK},
      {"select", SELECT}, {"in", IN}, {"case", CASE}, {"otherwise", OTHERWISE}, {"ni", NI},
      {"raise", RAISE}
   };

   map<string, TokenType> table_operations
   {
      {";", TokenType::EMPTY_OPERATOR},       {"=", TokenType::ASSIGNMENT_OPERATOR},  {",", TokenType::COMMA},
      {"+", TokenType::ARITHMETIC_OPERATION}, {"-", TokenType::ARITHMETIC_OPERATION}, {"*", TokenType::ARITHMETIC_OPERATION},
      {"/", TokenType::ARITHMETIC_OPERATION}, {"%", TokenType::ARITHMETIC_OPERATION}, {"(", TokenType::O_BRACE}, 
      {")", TokenType::C_BRACE},              {"<", TokenType::RELATION},             {">", TokenType::RELATION}, 
      {"==", TokenType::RELATION},            {"<=", TokenType::RELATION},            {">=", TokenType::RELATION},
      {"!=", TokenType::RELATION},            {"<<", TokenType::O_MARK},              {">>", TokenType::C_MARK}, 
      {"<<<", TokenType::O_COMMENT},          {">>>", TokenType::C_COMMENT},          {":", TokenType::CASE_LISTING}
   };

   bool Is_Keyword(string word)
   {
      return table_detection.contains(word);
   }


   //Процедура ДОБАВИТЬ_КОНСТАНТУ
   void Add_Constant(int constant)
   {
      table_constants.emplace(constant);

      register_indicator = table_constants.find(constant);
   }

   void Add_Constant(string a)
   {
      if (!Is_this_constant(a))
         return Error_Handler_Constant(a);

      if (S_more_I(a))
      {
         Add_Constant(from_string_to_vector_short(a));
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

   //Процедура ДОБАВИТЬ_ПЕРЕМЕННУЮ
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
   }

   //Процедура СОЗДАТЬ_ЛЕКСЕМУ
   void Create_Token(string value_)
   {
      Token result;

      if (register_type_token >= 12 && register_type_token <= 37)
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

      Print_Token(result);
   }

   void Create_Token()
   {
      Token result;

      if (register_type_token >= 12 && register_type_token <= 32)
         result.value = " ";
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

      Print_Token(result);
   }

   //Процедура обработки ошибок
   void Error_Handler(ifstream& in)
   {
      register_type_token = TokenType::ERROR;
      Create_Token();
      cerr << "An error was found in the number line " << number_line << endl;
   }


   void Error_Handler_Operation(string error)
   {
      register_type_token = TokenType::ERROR;
      Create_Token();
      cerr << "An error was found in the number line " << number_line << "; Wrong operation \"" << error << "\"" << endl;
   }

   void Error_Handler_Variable(string error)
   {
      register_type_token = TokenType::ERROR;
      Create_Token();
      cerr << "An error was found in the number line " << number_line << "; Wrong variable \"" << error << "\"" << endl;
   }

   void Error_Handler_Constant(string error)
   {
      register_type_token = TokenType::ERROR;
      Create_Token();
      cerr << "An error was found in the number line " << number_line << "; Wrong constant with type int \"" << "\"" << endl;

   }


   //ПЕРЕМЕННЫЕ//

   //Регистр класса служит для хранения класса лексемы
   TokenType register_type_token = (TokenType)start;

   //Регистр указателя содержит указатель на таблицу имён
   //get<0> - итератор(указатель) на таблицу констант
   //get<1> - итератор(указатель) на таблицу переменных
   variant<set<variant<int, BigNumber>>::iterator, map<string, variant<int, BigNumber>>::iterator> register_indicator;

   //Регистр числа используется для вычисления констант
   variant<int, vector<short>> register_number;

   //Регистр отношения хранит информацию о первом символе отношения
   int register_relation = 0;

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
      if (!((word[0] >= 'a' && word[0] <= 'z') || (word[0] >= 'A' && word[0] <= 'Z')))
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

   bool Is_this_constant(vector<short> constant)
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
      prev_character = Transliterator(in.peek()).token_class;
      string accumulation_of_value;

      while (flag)
      {
         int character = in.get();
         symbolic_token = Transliterator(character);
         

         if (prev_character == symbolic_token.token_class || (Is_in_variable(symbolic_token.token_class) && Is_in_variable(prev_character)))
         {
            accumulation_of_value += symbolic_token.value;
         }
         else if (prev_character == SymbolicTokenType::SPACE || prev_character == SymbolicTokenType::LF)
            accumulation_of_value = symbolic_token.value;
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
                  //Проблема
                  Add_Variable(accumulation_of_value);
                  register_type_token = TokenType::VARIABLE;
               }
               break;

            case (SymbolicTokenType::DIGIT):

               //Проблема
               Add_Constant(accumulation_of_value);
               register_type_token = TokenType::CONSTANT;
               break;

            default:

               if (table_operations.count(accumulation_of_value))
               {
                  register_type_token = table_operations[accumulation_of_value];
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

         prev_character = symbolic_token.token_class;
      }
      
      return table_tokens;
   }

   void Print_Table_Token()
   {
      for (auto& i : table_tokens)
      {
         cout << i.number_line << " ";
         cout << TokenTypeString[i.token_class] << " ";
         if (i.token_class >= 0 && i.token_class <= 32)
            ;//Nothing
         else if (i.value.index() == 0)
            cout << get<0>(i.value);
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
      cout << "Test: ";
      cout << i.number_line << " ";
      cout << TokenTypeString[i.token_class] << " ";
      if (i.token_class >= 0 && i.token_class <= 32)
         ;//Nothing
      else if (i.value.index() == 0)
         cout << get<0>(i.value);
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

   friend ostream& operator<<(ostream& stream, const Token& object_)
   {
      stream << object_.number_line << " ";
      stream << TokenTypeString[object_.token_class] << " ";
      if (object_.token_class >= 0 && object_.token_class <= 32)
         ;//Nothing
      else if (object_.value.index() == 0)
         stream << get<0>(object_.value);
      else if (object_.value.index() == 2)
      {
         if (get<2>(object_.value)->second.index() == 0)
            stream << get<2>(object_.value)->first << " ";
         else
            stream << get<2>(object_.value)->first << " ";
      }
      else if (object_.value.index() == 1)
      {
         if (get<1>(object_.value)->index() == 0)
            stream << get<0>(*(get<1>(object_.value))) << " ";
         else
            stream << get<1>(*(get<1>(object_.value))) << " ";
      }
      stream << endl;
   }

   friend ostream& operator<<(ostream& stream, const vector<Token>& object_)
   {
      for (auto& i : object_)
      {
         stream << i.number_line << " ";
         stream << TokenTypeString[i.token_class] << " ";
         if (i.token_class >= 0 && i.token_class <= 32)
            ;//Nothing
         else if (i.value.index() == 0)
            stream << get<0>(i.value);
         else if (i.value.index() == 2)
         {
            if (get<2>(i.value)->second.index() == 0)
               stream << get<2>(i.value)->first << " ";
            else
               stream << get<2>(i.value)->first << " ";
         }
         else if (i.value.index() == 1)
         {
            if (get<1>(i.value)->index() == 0)
               stream << get<0>(*(get<1>(i.value))) << " ";
            else
               stream << get<1>(*(get<1>(i.value))) << " ";
         }
         stream << endl;
      }
   }
};