#pragma once
#include "LexicalAnalyzer.h"

void TableTokenStack::PrintStackOnEveryStep(bool a)
{
   print_stack_on_every_step = a;
}

void TableTokenStack::JUMP(int& number_token)
{
   for (int x = 0; x < table_tokens.size(); x++)
   {
      if (table_tokens[x].number_line == get<0>(*get<1>(table_tokens[number_token].value)))
      {
         number_token = x - 1;//Костыль, внутри for станет нужным значением
         return;
      }
   }
}

void TableTokenStack::Interpreter(stack<variant<int, BigNumber>>& stack_)
{
   //Проверка на наличее ERROR
   for (int i = 0; i < table_tokens.size(); i++)
   {
      if (table_tokens[i].token_class == ERROR)
      {
         cerr << "Incorrect commands are present" << endl;
         return;
      }
   }


   string name_variable = " ";
   int value_variable_int = 0;
   BigNumber value_variable_bn;
   variant<int, BigNumber> temp_var1 = 0;
   variant<int, BigNumber> temp_var2 = 0;
   BigNumber temp_var3 = 0;
   int a;
   int b;


   
   for (int number_token = 0; number_token < table_tokens.size(); number_token++)
   {
      if (print_stack_on_every_step)
      {
         cout << "- " << table_tokens[number_token].number_line << " " << TokenTypeString[table_tokens[number_token].token_class] << endl;
      }
      switch (table_tokens[number_token].token_class)
      {
      case (TokenType::PUSH):


         if (table_tokens[number_token].value.index() == 2)
         {
            stack_.push((*get<2>(table_tokens[number_token].value)).second);
         }
         else
            stack_.push(*get<1>(table_tokens[number_token].value));

         break;

      case (TokenType::PUSHBN):


         if (table_tokens[number_token].value.index() == 2)
         {
            stack_.push((*get<2>(table_tokens[number_token].value)).second);
         }
         else
            stack_.push(*get<1>(table_tokens[number_token].value));

         break;

      case (TokenType::POP):
         
         if (stack_.empty())
         {
            cerr << "Stack usage error " << table_tokens[number_token].number_line;
            return;
         }

         name_variable = get<2>(table_tokens[number_token].value)->first;
         if (stack_.top().index() == 0)
            (get<2>(table_tokens[number_token].value)->second) = get<0>(stack_.top());
         else 
            (get<2>(table_tokens[number_token].value)->second) = get<1>(stack_.top());

         stack_.pop();

         break;
         
      case (TokenType::ARITHMETIC_OPERATION):


         if (stack_.size() <= 1)
         {
            cerr << "Stack usage error " << table_tokens[number_token].number_line;
            return;
         }

         temp_var1 = stack_.top();
         stack_.pop();
         temp_var2 = stack_.top();
         stack_.pop();
         a = temp_var1.index();
         b = temp_var2.index();

         if (a == 0 && b == 0)
            switch (get<0>(table_tokens[number_token].value))
            {
            case ('+'):
               stack_.push(get<0>(temp_var2) + get<0>(temp_var1));
               break;
            case ('-'):
               stack_.push(get<0>(temp_var2) - get<0>(temp_var1));
               break;
            case('*'):
               stack_.push(get<0>(temp_var2) * get<0>(temp_var1));
               break;
            case('/'):
               if (get<0>(temp_var1) == 0)
               {
                  cerr << "Arithmetic error";
               }
               stack_.push(get<0>(temp_var2) / get<0>(temp_var1));
               break;
            case ('%'):
               if (get<0>(temp_var1) == 0)
               {
                  cerr << "Arithmetic error";
               }
               stack_.push(get<0>(temp_var2) % get<0>(temp_var1));
               break;
            }
         else if (a == 1 && b == 1)
            switch (get<0>(table_tokens[number_token].value))
            {
            case ('+'):
               stack_.push(get<1>(temp_var2) + get<1>(temp_var1));
               break;
            case ('-'):
               stack_.push(get<1>(temp_var2) - get<1>(temp_var1));
               break;
            case('*'):
               stack_.push(get<1>(temp_var2) * get<1>(temp_var1));
               break;
            case('/'):
               if (get<1>(temp_var1) == (BigNumber)0)
               {
                  cerr << "Arithmetic error";
               }
               stack_.push(get<1>(temp_var2) / get<1>(temp_var1));
               break;
            case ('%'):
               if (get<1>(temp_var1) == (BigNumber)0)
               {
                  cerr << "Arithmetic error";
               }
               stack_.push(get<1>(temp_var2) % get<1>(temp_var1));
               break;
            }
         else if (a == 0 && b == 1)
         {
            temp_var3 = get<0>(temp_var1);
            switch (get<0>(table_tokens[number_token].value))
            {
            case ('+'):
               
               stack_.push(get<1>(temp_var2) + temp_var3);
               break;
            case ('-'):
               stack_.push(get<1>(temp_var2) - temp_var3);
               break;
            case('*'):
               stack_.push(get<1>(temp_var2) * temp_var3);
               break;
            case('/'):
               if (get<0>(temp_var1) == 0)
               {
                  cerr << "Arithmetic error";
               }
               stack_.push(get<1>(temp_var2) / temp_var3);
               break;
            case ('%'):
               if (get<0>(temp_var1) == 0)
               {
                  cerr << "Arithmetic error";
               }
               stack_.push(get<1>(temp_var2) % temp_var3);
               break;
            }
         }
         else if (a == 1 && b == 0)
         {
            temp_var3 = get<0>(temp_var2);
            switch (get<0>(table_tokens[number_token].value))
            {
            case ('+'):
               stack_.push(temp_var3 + get<1>(temp_var1));
               break;
            case ('-'):
               stack_.push(temp_var3 - get<1>(temp_var1));
               break;
            case('*'):
               stack_.push(temp_var3 * get<1>(temp_var1));
               break;
            case('/'):
               if (get<1>(temp_var1) == (BigNumber)0)
               {
                  cerr << "Arithmetic error";
               }
               stack_.push(temp_var3 / get<1>(temp_var1));
               break;
            case ('%'):
               if (get<1>(temp_var1) == (BigNumber)0)
               {
                  cerr << "Arithmetic error";
               }
               stack_.push(temp_var3 % get<1>(temp_var1));
               break;
            }
         }
         break;

      case(TokenType::RELATION):

         if (stack_.size() <= 1)
         {
            cerr << "Stack usage error " << table_tokens[number_token].number_line;
            return;
         }

         temp_var1 = stack_.top();
         stack_.pop();
         temp_var2 = stack_.top();
         stack_.pop();
         a = temp_var1.index();
         b = temp_var2.index();

         if (a == 0 && b == 0)
            switch (get<0>(table_tokens[number_token].value))
            {
            case (Equal):
               stack_.push(get<0>(temp_var2) == get<0>(temp_var1));
               break;
            case (Not_equal):
               stack_.push(get<0>(temp_var2) != get<0>(temp_var1));
               break;
            case (Less_then):
               stack_.push(get<0>(temp_var2) < get<0>(temp_var1));
               break;
            case(More_then):
               stack_.push(get<0>(temp_var2) > get<0>(temp_var1));
               break;
            case(Less_or_equal_then):
               stack_.push(get<0>(temp_var2) <= get<0>(temp_var1));
               break;
            case (More_or_equal_then):
               stack_.push(get<0>(temp_var2) >= get<0>(temp_var1));
               break;
            }
         else if (a == 1 && b == 1)
            switch (get<0>(table_tokens[number_token].value))
            {
            case (Equal):
               stack_.push(get<1>(temp_var2) == get<1>(temp_var1));
               break;
            case (Not_equal):
               stack_.push(get<1>(temp_var2) != get<1>(temp_var1));
               break;
            case (Less_then):
               stack_.push(get<1>(temp_var2) < get<1>(temp_var1));
               break;
            case(More_then):
               stack_.push(get<1>(temp_var2) > get<1>(temp_var1));
               break;
            case(Less_or_equal_then):
               stack_.push(get<1>(temp_var2) <= get<1>(temp_var1));
               break;
            case (More_or_equal_then):
               stack_.push(get<1>(temp_var2) >= get<1>(temp_var1));
               break;
            }
         else if (a == 0 && b == 1)
         {
            temp_var3 = get<0>(temp_var1);
            switch (get<0>(table_tokens[number_token].value))
            {
            case (Equal):
               stack_.push(get<1>(temp_var2) == temp_var3);
               break;
            case (Not_equal):
               stack_.push(get<1>(temp_var2) != temp_var3);
               break;
            case (Less_then):
               stack_.push(get<1>(temp_var2) < temp_var3);
               break;
            case(More_then):
               stack_.push(get<1>(temp_var2) > temp_var3);
               break;
            case(Less_or_equal_then):
               stack_.push(get<1>(temp_var2) <= temp_var3);
               break;
            case (More_or_equal_then):
               stack_.push(get<1>(temp_var2) >= temp_var3);
               break;
            }
         }
         else if (a == 1 && b == 0)
         {
            temp_var3 = get<0>(temp_var2);
            switch (get<0>(table_tokens[number_token].value))
            {
            case (Equal):
               stack_.push(temp_var3 == get<1>(temp_var1));
               break;
            case (Not_equal):
               stack_.push(temp_var3 != get<1>(temp_var1));
               break;
            case (Less_then):
               stack_.push(temp_var3 < get<1>(temp_var1));
               break;
            case(More_then):
               stack_.push(temp_var3 > get<1>(temp_var1));
               break;
            case(Less_or_equal_then):
               stack_.push(temp_var3 <= get<1>(temp_var1));
               break;
            case (More_or_equal_then):
               stack_.push(temp_var3 >= get<1>(temp_var1));
               break;
            }
         }


         break;

      case(TokenType::JMP):
      
         JUMP(number_token);
         break;
      
      case(TokenType::JI):

         if (stack_.empty())
         {
            cerr << "Stack usage error " << table_tokens[number_token].number_line;
            return;
         }

         

         temp_var1 = stack_.top();
         stack_.pop();
         if (temp_var1.index() == 0)
         {
            if (get<0>(temp_var1) != 0)
               JUMP(number_token);
         }
         else
            if (get<1>(temp_var1) != (BigNumber)0)
               JUMP(number_token);


         break;
  
      case(TokenType::READ):

         cout << "Enter: ";
         cin >> a;
         stack_.push(a);
         

         break;

      case(TokenType::READBN):

         cin >> temp_var3;
         stack_.push(temp_var3);

         break;

      case(TokenType::WRITE):
         
         if (stack_.empty())
         {
            cerr << "Stack usage error " << table_tokens[number_token].number_line;
            return;
         }

         if (stack_.top().index() == 0)
            cout << get<0>(stack_.top()) << endl;
         else
            cout << get<1>(stack_.top()) << endl;
         stack_.pop();

         break;

      case (TokenType::GETD):

         if (stack_.top().index() == 0)
            break;

         temp_var3 = get<1>(stack_.top());
         stack_.pop();
         a = (int)temp_var3[get<0>(*get<1>(table_tokens[number_token].value))];
         stack_.push(a);
         break;

      case(TokenType::END):

         return;

      case(TokenType::END_MARKER):

         return;

      default:
         break;
      };

      //Печатать стек на каждом шаге
      if (print_stack_on_every_step)
      {
         
         if (stack_.empty())
            cout << "empty" << endl;
         for (std::stack<variant<int, BigNumber>> dump = stack_; !dump.empty(); dump.pop())
            if (dump.top().index() == 0)
               std::cout << get<0>(dump.top()) << '\n';
            else if (dump.top().index() == 1)
               std::cout << get<1>(dump.top()) << '\n';
         cout << "- " << table_tokens[number_token].number_line << " " << TokenTypeString[table_tokens[number_token].token_class] << endl;
      }

   }
}
