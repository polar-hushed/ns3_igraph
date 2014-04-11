#include <iostream>
#include <fstream>

using namespace std;

struct Node
{
   int nid;
   int devid;
}

struct packets
{
   int id;
   Node n;   
	 
}

int main()
{
	ifstream fin;
	float time;
	float p[70000];
	int node, id, device;
	string str;

/*
	for (int i = 0; i < 70000 ; ++i)
	for (int j = 0; j < 100 ; ++j)
	for (int k = 0; k < 100 ; ++k)
       {
	  p[i][j][k] = 0;	
	 cout << "Zeroing " << endl;
       }
	fin.open("outfile.txt");
	if (!fin)
	{
		cout << "Could not open file " << endl;
	}
	else
	{
		while(fin)
		{
			fin >> time >> node >> device >> str >> id;
			     p[id][node][device] += time;
	 			cout << "Reading " << endl;
		}
	}


	for (int i = 0; i < 70000 ; ++i)
	for (int j = 0; j < 100 ; ++j)
	for (int k = 0; k < 100 ; ++k)
       {
	  cout << p[i][j][k] << " " << j << " " << k << endl;
       }*/
	return 0;
}
