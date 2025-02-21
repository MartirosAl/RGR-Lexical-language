#include <iostream>
#include "Lexical_Analyzer.h"
using namespace std;

int main()
{
   TableToken result;
   result.Lexical_Analyzer("Test4.txt");
   result.Print_Table_Token();
   return 0;
}
