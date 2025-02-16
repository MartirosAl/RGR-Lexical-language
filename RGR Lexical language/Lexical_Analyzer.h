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

class TableToken
{
   enum SymbolicTokenType
   {
      start = -1,
      LETTER,
      DIGIT,
      ARITHMETIC_OPERATION,
      RELATION,
      EQUAL,
      SPACE,
      LF,
      SEMI_COLON,
      ERROR,
      O_BRACE,
      C_BRACE,
      END,
      END_MARKER
   };

   enum TokenType
   {
      start = -1,
      EMPTY_OPERATOR = END_MARKER,
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
      ARITHMETIC_OPERATION,
      RELATION
   };

   vector<string> SymbolicTokenTypeString
   {
      "LETTER",
      "DIGIT",
      "ARITHMETIC_OPERATION",
      "RELATION",
      "SPACE",
      "LF",
      "SEMI_COLON",
      "COMMENT",
      "ERROR",
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
      "ARITHMETIC_OPERATION",
      "RELATION"
   };

   enum Relation
   {
      Equal,
      Not,
      Less_then,
      More_then,
      Not_equal,
      Less_or_equal_then,
      More_or_equal_then
   };

   vector<string> RelationString
   {
      "Equal",
      "Not",
      "Less_then",
      "More_then",
      "Not_equal",
      "Less_or_equal_then",
      "More_or_equal_then"
   };

   enum VariableType
   {
      Int,
      Big_Number
   };

   vector<string> VariableTypeString
   {
      "int",
      "BigNumber"
   };

   struct SymbolicToken
   {
      SymbolicTokenType token_class = start;
      int value = 0;
   };

   struct Token
   {
      TokenType token_class = (TokenType)start;
      //get<0> - это int значение для отношений и ариф. операций
      //get<1> - это ячейка для таблицы констант
      //get<2> - это ячейка дтя таблицы переменных
      variant<int, set<variant<int, BigNumber>>::iterator, map<string, variant<int, BigNumber>>::iterator> value = 0;
      int number_line = 0;
   };

   //Одинаковые элементы удалятся
   map<string, TokenType> table_detection
   {
      {"while", WHILE}, {"do", DO}, {"od", OD},
      {"for", FOR}, {"from", FROM}, {"to", TO}, {"by", BY}, {"do", DO}, {"od", OD},
      {"if", IF}, {"else", ELSE}, {"fi", FI},
      {"input", INPUT},
      {"print", PRINT},
      {"goto", GO_TO_MARK},
      {"select", SELECT}, {"in", IN}, {"case", CASE}, {"otherwise", OTHERWISE}, {"ni", NI},
      {"raise", RAISE}
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
   const map<string, int> table_detection;

   bool Is_Keyword(string word)
   {
      return table_detection.contains(word);
   }


   //Процедура ДОБАВИТЬ_КОНСТАНТУ
   void Add_Constant()
   {
      if (register_number.index() == 0)
      {
         table_constants.emplace(get<0>(register_number));

         register_value = get<0>(register_number);

         register_indicator = table_constants.find(get<0>(register_number));

         get<0>(register_number) = -1;
      }
      else
      {
         reverse(get<1>(register_number).begin(), get<1>(register_number).end());
         BigNumber temp(get<1>(register_number).data(), get<1>(register_number).size());
         table_constants.emplace(temp);

         register_indicator = table_constants.find(temp);

         get<1>(register_number).clear();
      }
   }

   //Процедура ДОБАВИТЬ_ПЕРЕМЕННУЮ
   void Add_Variable()
   {

      if (Is_Keyword(register_variable))
      {
         Error_Handler();
         return;
      }

      if (table_variable.count(register_variable) == 0)
      {
         table_variable[register_variable] = 0;
      }

      register_indicator = table_variable.find(register_variable);
   }

   //Процедура СОЗДАТЬ_ЛЕКСЕМУ
   void Create_Token(ifstream& in)
   {
      Token result;
      if (register_type_token == TokenType::RELATION)
         result.value = register_relation;
      else if (register_type_token == TokenType::ARITHMETIC_OPERATION)
         result.value = register_value;
      else if (register_type_token == TokenType::DECLARING_VARIABLES)
      {
         int flag = true;
         string name_variable;
         string word;
         while (flag)
         {
            name_variable = V(in);
            cin >> word;

            if (word != "as")
               Error_Handle(in); //Ошибка операции

            in >> word;

            if (word[word.size() - 1] == ';')
            {
               word.resize(word.size() - 1);
               flag = false;
            }
            else if (word[word.size() - 1] == ',')
               word.resize(word.size() - 1);
            else
            {
               Error_Handle(in); //Ошибка операции объявления
            }


            if (word == VariableTypeString[0])
            {
               table_variable[name_variable] = (int)0;
            }
            else if (word == VariableTypeString[1])
            {
               table_variable[name_variable] = (BigNumber)0;
            }
            else
               Error_Handle(); //Ошибка типа переменной

            result.value = table_variable.find(register_variable);

            table_tokens.push_back(result);
         }
         return;
      }
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

      register_value = -1;

   }

   //Процедура обработки ошибок
   void Error_Handler(ifstream& in)
   {
      register_type_token = TokenType::ERROR;
      Create_Token(in);
      cerr << "An error was found in the number line " << number_line << endl;
   }

   void Error_Handler(char variable, ifstream& in)
   {
      register_type_token = TokenType::ERROR;
      Create_Token(in);
      cerr << "An error was found in the number line " << number_line << "; Wrong variable name with symbol" << variable << endl;
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

   //Регистр переменной содержит имя переменной
   string register_variable;

   //Регистр значения хранит значения лексем
   int register_value = -1;

   //Номер строки хранит номер текущей строки в программе
   int number_line = 1;

   Token token;

   SymbolicToken symbolic_token;

   string V(ifstream& in)
   {
      string word;
      in >> word;
      for (int i = 0; i < word.size(); i++)
      {
         if (!((word[i] >= '0' && word[i] <= '9') || (word[i] >= 'a' && word[i] <= 'z') || (word[i] >= 'A' && word[i] <= 'Z')))
         {
            Error_Handler(word[i], in);
         }
      }
      return word;
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
         result.value = (int)character - '0';
      }
      else if (character == '+' || character == '-' || character == '*' || character == '/' || character == '%')
      {
         result.token_class = SymbolicTokenType::ARITHMETIC_OPERATION;
         result.value = (int)character;
      }
      else if (character == '<')
      {
         result.token_class = SymbolicTokenType::RELATION;
         result.value = Less_then;
      }
      else if (character == '>')
      {
         result.token_class = SymbolicTokenType::RELATION;
         result.value = More_then;
      }
      else if (character == '=')
      {
         result.token_class = SymbolicTokenType::EQUAL;
         result.value = Equal;
      }
      else if (character == '!')
      {
         result.token_class = SymbolicTokenType::RELATION;
         result.value = Not;
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
         result.token_class = SymbolicTokenType::ERROR;
         result.value = (int)character;
      }
      return result;
   }

   vector<Token> Lexical_Analyzer(const char* filename)
   {
      ifstream in(filename);
      if (!in)
      {
         cout << "Не удалось открыть файл " << filename << endl;
         return table_tokens;
      }

      string word;

      bool flag = true;
      while (true)
      {
         int character = in.get();
         symbolic_token = Transliterator(character);

         
         
      }
      
   }
};