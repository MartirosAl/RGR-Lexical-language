#include "Sintax_Analyzer.h"
using namespace std;

int main()
{
   //TableToken result;
   //result.Lexical_Analyzer("Test4.txt");
   //
   //result.Print_Table_Token();

   Sintax A("Grammar.txt");
   A.Print_Rules();
   A.Print_Nonterminals();
   A.Print_Terminals();

   return 0;
}
