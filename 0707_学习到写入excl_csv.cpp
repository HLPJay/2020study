#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main()
{
	ofstream oFile;
	oFile.open("test.csv", ios::out|ios::trunc);
	oFile<<"test1"<<","<<"test2"<<","<<"test3"<<endl;
	oFile<<"111"<<","<<"123"<<","<<" 1 3 4"<<endl;
	oFile<<"111"<<","<<"123"<<","<<"  2; 5;7;"<<endl;
	oFile.close();
	return 0;
}
