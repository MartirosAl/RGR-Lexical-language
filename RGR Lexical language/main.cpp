#include "Sintax_Analyzer.h"
using namespace std;

int main()
{
   //TableToken result;
   //result.Lexical_Analyzer("Test4.txt");
   //
   //result.Print_Table_Token();

   Sintax A;
   
   bool res = A.Processing_incoming_code("TestL3.txt");
   cout << endl << res << endl;
   return 0;
}
