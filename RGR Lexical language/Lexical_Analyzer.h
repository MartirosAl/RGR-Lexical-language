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

class Lexical_Analyzer
{
   enum SymbolicTokenType
   {
      start = -1,
      LETTER,
      DIGIT,
      ARITHMETIC_OPERATION,
      RELATION,
      SPACE,
      LF,
      SEMI_COLON,
      ERROR,
      END_MARKER
   };

   enum TokenType
   {
      start = -1,
      EMPTY_OPERATOR = END_MARKER,
      DECLARING_VARIABLES,
      ASSIGNMENT_OPERATOR,
      WHILE,
      FOR,
      IF,
      INPUT,
      PRINT,
      MARK,
      GO_TO_MARK,
      IF_GO_TO_MARK,
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
      "FOR",
      "IF",
      "INPUT",
      "PRINT",
      "MARK",
      "GO_TO_MARK",
      "IF_GO_TO_MARK",
      "RAISE",
      "COMMENT",
      "ERROR",
      "ARITHMETIC_OPERATION",
      "RELATION",
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

   struct SymbolicToken
   {
      SymbolicTokenType token_class = start;
      int value = 0;
   };

   struct Token
   {
      TokenType token_class = (TokenType)start;
      //get<0> - ��� int �������� ��� ���������������, ��������� � ����. ��������
      //get<1> - ��� ������ ��� ������� ��������
      //get<2> - ��� ������ ��� ������� ����������
      variant<int, set<variant<int, BigNumber>>::iterator, map<string, variant<int, BigNumber>>::iterator> value = 0;
      int number_line = 0;
   };

   //0 - ������ ��������������
   //1 - ����������� ��������������
   //2 - ����� ��������������
   //3 - ��������� �������
   //4 - ��������� �����
   map<string, int> table_detection
   {
      {"declare", 0}, {"as", 1},
      {"while", 0}, {"do", 1}, {"od", 2},
      {"for", 0}, {"from", 1}, {"to", 1}, {"by",3}, {"do", 1}, {"od",2},
      {"if", 1}, {"else", 3}, {"fi", 2},
      {"input", 4},
      {"print", 4},
      {"goto", 4},
      {"select", 0}, {"in", 1}, {"case", 1}, {"otherwise", 3}, {"ni",2},
      {"raise", 4}
   };

   //�������//

//������� ��������
   set<variant<int, BigNumber>> table_constants;

   //������� ����������
   map<string, variant<int, BigNumber>> table_variable;

   //������� ������ ��� ������
   //������ ��������� (int, set<variant<int, BigNumber>>::iterator, map<string, variant<int, BigNumber>>::iterator)
   vector<Token> table_tokens;

   //������� ��� ����������� �������� ����
   const map<string, int> table_detection;

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
         result.token_class = SymbolicTokenType::RELATION;
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
      else
      {
         result.token_class = SymbolicTokenType::ERROR;
         result.value = (int)character;
      }
      return result;
   }

   bool Is_Keyword(string word)
   {
      return table_detection.contains(word);
   }


   //��������� ��������_���������
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

   //��������� ��������_����������
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

   //��������� �������_�������
   void Create_Token()
   {
      Token result;

      if (register_type_token == TokenType::RELATION)
         result.value = register_relation;
      else if (register_type_token == TokenType::ARITHMETIC_OPERATION)
         result.value = register_value;
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

   //��������� ��������� ������
   void Error_Handler()
   {
      register_type_token = TokenType::ERROR;
      Create_Token();
      cerr << "An error was found in the number line " << number_line << endl;
   }

   //����������//

   //������� ������ ������ ��� �������� ������ �������
   TokenType register_type_token = (TokenType)start;

   //������� ��������� �������� ��������� �� ������� ���
   //get<0> - ��������(���������) �� ������� ��������
   //get<1> - ��������(���������) �� ������� ����������
   variant<set<variant<int, BigNumber>>::iterator, map<string, variant<int, BigNumber>>::iterator> register_indicator;

   //������� ����� ������������ ��� ���������� ��������
   variant<int, vector<short>> register_number;

   //������� ��������� ������ ���������� � ������ ������� ���������
   int register_relation = 0;

   //������� ���������� �������� ��� ����������
   string register_variable;

   //������� �������� ������ �������� ������
   int register_value = -1;

   //����� ������ ������ ����� ������� ������ � ���������
   int number_line = 1;

   Token token;

   SymbolicToken symbolic_token;
};