#include <iostream>
#include "BigNumber.h"
#include "Sintax_Analyzer.h"
#include "LexicalAnalyzer.h"
using namespace std;

int main()
{
   Sintax A;
   
   bool res = A.Translation_of_code("TestSeul.txt", "OUT.txt");

   if (res == 1)
   {
	   TableTokenStack tokens;
	   tokens.Lexical_Analyzer("OUT.txt");
	   stack<variant<int, BigNumber>> stack;
	   tokens.Interpreter(stack);
   }

   return 0;
}
