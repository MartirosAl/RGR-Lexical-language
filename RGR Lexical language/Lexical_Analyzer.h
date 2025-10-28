#pragma once
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <variant>
#include <vector>
#include <list>

#include "BigNumber.h"
using namespace std;

class TableToken
{
protected:
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
        COMMA_S
    };

    vector<string> TokenTypeString
    {
          "VARIABLE",
          "CONSTANT",
          "GET",
          "LABEL",
          "GO_TO_LABEL",
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
          "COMMA"

          "O_COMMENT",
          "C_COMMENT",
          "CASE_LISTING"
    };

    enum TokenType
    {
        start = -1,
        VARIABLE,
        CONSTANT,
        GET,
        LABEL,
        GO_TO_LABEL,
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
        //get<3> is the cell for the label table.
        variant<string, set<variant<int, BigNumber>>::iterator, map<string, variant<int, BigNumber>>::iterator, list<string>::iterator> value = " ";
        // Итератор на второй аргумент функции get (для токенов типа GET, где требуется два аргумента-константы)
        set<variant<int, BigNumber>>::iterator second_argument_get;
        int number_line = 0;
    };

    vector<SymbolicTokenType> types_for_names_variables
    {
       LETTER, DIGIT, UNDERLINING
    };

public:
    vector<Token> Lexical_Analyzer(const string filename);

    void Print_Table_Token();

    void Print_Token(Token i);

protected:

    //ТАБЛИЦЫ//
    //TABLES//

    //Таблица констант
    //Table of constants
    set<variant<int, BigNumber>> table_constants;

    //Таблица переменных
    //Table of variables
    map<string, variant<int, BigNumber>> table_variable;

    //Таблица меток
    //Table of lables(labels)
    list<string> table_labels;

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
       {"int", TokenType::VARIABLE_TYPE}, {"BigNumber", TokenType::VARIABLE_TYPE},
       {"declare", TokenType::DECLARING_VARIABLES},                                           {"as", TokenType::AS},
       {"while", TokenType::WHILE},                                               {"do", TokenType::DO}, {"od", TokenType::OD},
       {"for", TokenType::FOR},           {"from", TokenType::FROM}, {"to", TokenType::TO}, {"by", TokenType::BY}, {"do", TokenType::DO}, {"od", TokenType::OD},
       {"if", TokenType::IF},                                                 {"else", TokenType::ELSE}, {"fi", TokenType::FI},
       {"input", TokenType::INPUT},
       {"print", TokenType::PRINT},
       {"goto", TokenType::GO_TO_LABEL},
       {"select", TokenType::SELECT},    {"in", TokenType::IN}, {"case", TokenType::CASE}, {"otherwise", TokenType::OTHERWISE}, {"ni", TokenType::NI},
       {"raise", TokenType::RAISE},
       {"get", TokenType::GET}
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
       {",", TokenType::COMMA}
    };


    //Процедура ДОБАВИТЬ_КОНСТАНТУ
    //Procedure TO ADD_CONSTANT
    void Add_Constant(string a);

    void Add_Constant(vector<short> a);

    //Процедура ДОБАВИТЬ_МЕТКУ
    //Procedure ADD_THE_LABEL
    void Add_Label(string a);

    //Процедура ДОБАВИТЬ_ПЕРЕМЕННУЮ
    //Procedure ADD_THE_VARIABLE
    void Add_Variable(string word);

    //Процедура СОЗДАТЬ_ЛЕКСЕМУ
    //Procedure CREATE_A_TOKEN
    void Create_Token(string value_ = " ");

    //Процедура обработки ошибок
    //Error handling procedure
    void Error_Handler(string error);

    void Error_Handler_Operation(string error);

    void Error_Handler_Variable(string error);


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
    //get<2> - iterator(pointer) to the table of labels
    variant<set<variant<int, BigNumber>>::iterator, map<string, variant<int, BigNumber>>::iterator, list<string>::iterator> register_indicator;

    //Номер строки хранит номер текущей строки в программе
    //Line number stores the number of the current line in the program
    int number_line = 1;

    Token token;

    SymbolicToken symbolic_token;

    bool Is_in_variable(SymbolicTokenType a);

    bool Is_this_variable(string word);

    bool Is_this_constant(string constant);

    vector<short> reverseDigits(vector<short> num);

    bool S_more_I(string a);

    vector<short> from_string_to_vector_short(string a);

    bool Is_this_start_comment(string a);

    bool Is_this_end_comment(string a);

    bool Is_this_o_braces(string a);

    bool Is_this_c_braces(string a);

    bool Is_this_empty_operators(string a);

    bool Is_this_o_label(string a);

    bool Is_this_c_label(string a);

    SymbolicToken Transliterator(int character);



};