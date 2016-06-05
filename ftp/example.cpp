
#include<iostream>
#include<fstream>

#include<string>

using namespace std;

int main()
{
	fstream fs;
	fs.open("/home/m/ftpFile.txt",ios::in);
    string tmp= fs.getline();
	cout<<tmp<<endl;
	return 0;
}

