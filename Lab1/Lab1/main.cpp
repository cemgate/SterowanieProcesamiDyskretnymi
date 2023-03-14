#include<iostream>
#include<vector>
#include<fstream>
#include<algorithm>
#include<string>

struct Process
{
	int r;
	int p;
	int q;
	bool is_taken = false;
};

std::vector<Process> load_data(std::string k)
{
	std::vector<Process> processes;
	std::string data;
	int r,p,q;
	std::ifstream datas("data" + k + ".txt");

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

int how_long(std::vector<Process> processes)
{
	int cmax = 0;
	int rp_time = 0;
	for (auto p : processes)
	{
		rp_time = std::max(rp_time, p.r ) + p.p ;
		cmax = std::max(cmax,rp_time+p.q);
	}
	return cmax;
}

void algorithm(std::vector<Process>& processes)
{
	std::vector<Process> new_vector;
	int min = INT_MAX;
	int where = 0;
	for (int i = 0; i < processes.size(); i++)
	{
		for (int k = 0; k < processes.size(); k++)
		{
			if (min > std::min(processes[k].r, min) && processes[k].is_taken == false)
			{
				min = std::min(processes[k].r, min);
				where = k;
			}
		}
		new_vector.push_back(processes[where]);
		processes[where].is_taken = true;
		min = INT_MAX;
	}
	processes = new_vector;
}

int main()
{
	std::vector <Process> processes;
	int max_time = 0;
	for (int i = 1; i < 5; i++)
	{  
		processes = load_data(std::to_string(i));
		algorithm(processes);
		std::cout << "Czas dla danych numer " << i << " wynosi:" << how_long(processes) << std::endl;
		max_time += how_long(processes);
	}
	std::cout << "Calkowity czas wynosi:" << max_time << std::endl;
	
}