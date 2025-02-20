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
      SPACE,
      LF,
      SEMI_COLON,
      ERROR,
      O_BRACE,
      C_BRACE,
      UNDERLINING,
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
      RELATION,
      VARIABLE_TYPE,
      O_BRACE,
      C_BRACE,
      COMMA,
      O_MARK,
      C_MARK,
      O_COMMENT,
      C_COMMENT,
      CASE_LISTING
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
      "ARITHMETIC_OPERATION",
      "RELATION",
      "VARIABLE_TYPE",
      "O_BRACE",
      "C_BRACE",
      "COMMA",
      "O_MARK",
      "C_MARK",
      "O_COMMENT",
      "C_COMMENT",
      "CASE_LISTING"
   };

   struct SymbolicToken
   {
      SymbolicTokenType token_class = start;
      int value = 0;
   };

   struct Token
   {
      TokenType token_class = (TokenType)start;
      //get<0> - ��� int �������� ��� ��������� � ����. ��������
      //get<1> - ��� ������ ��� ������� ��������
      //get<2> - ��� ������ ��� ������� ����������
      variant<int, set<variant<int, BigNumber>>::iterator, map<string, variant<int, BigNumber>>::iterator> value = 0;
      int number_line = 0;
   };

   vector<SymbolicTokenType> types_for_names_variables
   {
      LETTER, DIGIT, UNDERLINING
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
   //���������� �������� ��������
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


   //��������� ��������_���������
   void Add_Constant(int constant)
   {
      table_constants.emplace(constant);

      register_value = constant;

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

      register_value = constant;

      register_indicator = table_constants.find(constant);
   }

   void Add_Constant(vector<short> a)
   {
      BigNumber temp (reverseDigits(a), a.size());

      table_constants.emplace(temp);

      register_indicator = table_constants.find(temp);
   }

   //��������� ��������_����������
   void Add_Variable(string word)
   {

      if (Is_this_variable(word))
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

   //��������� �������_�������
   void Create_Token()
   {
      Token result;

      if (register_type_token >= 12 && register_type_token <= 38)
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
      cerr << "An error was found in the number line " << number_line << "; Wrong operation " << error << endl;
   }

   void Error_Handler_Variable(string error)
   {
      register_type_token = TokenType::ERROR;
      Create_Token();
      cerr << "An error was found in the number line " << number_line << "; Wrong variable " << error << endl;
   }

   void Error_Handler_Constant(string error)
   {
      register_type_token = TokenType::ERROR;
      Create_Token();
      cerr << "An error was found in the number line " << number_line << "; Wrong constant with type int " << error << endl;;

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

   //������� �������� ������ �������� ������
   int register_value = -1;

   //����� ������ ������ ����� ������� ������ � ���������
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
         result.value = (int)character - '0';
      }
      else if (character == '_')
      {
         result.token_class = SymbolicTokenType::UNDERLINING;
         result.value = (int)character;
      }
      else if (character == '+' || character == '-' || character == '*' || character == '/' || character == '%')
      {
         result.token_class = SymbolicTokenType::ARITHMETIC_OPERATION;
         result.value = (int)character;
      }
      else if (character == '<' || character == '>' || character == '=' || character == '!')
      {
         result.token_class = SymbolicTokenType::RELATION;
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
         cout << "�� ������� ������� ���� " << filename << endl;
         return table_tokens;
      }

      string word;

      bool flag = true;

      SymbolicTokenType prev_character;
      prev_character = Transliterator(in.peek()).token_class;
      string accumulation_of_value;
      bool flag_big_number = false;

      while (true)
      {
         int character = in.get();
         symbolic_token = Transliterator(character);

         if (prev_character == symbolic_token.token_class || (Is_in_variable(symbolic_token.token_class) && Is_in_variable(prev_character)))
         {
            accumulation_of_value += symbolic_token.value;
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
                  Add_Variable(accumulation_of_value);
               }
               break;

            case (SymbolicTokenType::DIGIT):

               Add_Constant(accumulation_of_value);
               break;

            default:

               if (table_operations.count(accumulation_of_value))
               {
                  register_type_token = table_operations[accumulation_of_value];
               }
               else
                  Error_Handler_Operation(accumulation_of_value);
            }
            
            Create_Token();
         }
      }
   }
};