#include<iostream>
#include<vector>
#include<fstream>

struct Process
{
	int r;
	int p;
	int q;
	bool is_taken = false;
};

std::vector<Process> load_data()
{
	std::vector<Process> processes;
	std::string data;
	int r,p,q;
	std::cout << "Ktory zestaw danych wybrac? 1,2,3,4" << std::endl;
	std::cin >> data;
	std::ifstream datas("data" + data + ".txt");

	while (datas >> r >> p >> q)
	{
		Process tmp_process;
		tmp_process.r = r;
		tmp_process.p = p;
		tmp_process.q = q;
		processes.push_back(tmp_process);
	}
	return processes;
}

int main()
{
	std::vector <Process> processes;
	processes=load_data();
	for (auto p : processes)
	{
		std::cout << p.r << " " << p.p <<" " << p.q << std::endl;
	}
}