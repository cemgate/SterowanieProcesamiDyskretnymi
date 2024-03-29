#include<iostream>
#include<vector>
#include<fstream>
#include<algorithm>
#include<string>
#include<chrono>
#include<unordered_map>
#include<map>
#include<memory>

struct Process
{
	int p;
	int w;
	int d;
	int place;
};

struct witi_algorithm
{
	int witi=0;
	std::vector<int> permutation;
};

std::vector<Process> load_data(std::string k)
{
	std::vector<Process> processes;
	std::string data;
	int i = 1;
	int p, w, d;
	std::ifstream datas("data" + k + ".txt");

	while (datas >> p >> w >> d)
	{
		Process tmp_process{};
		tmp_process.p = p;
		tmp_process.w = w;
		tmp_process.d = d;
		tmp_process.place = i;
		processes.push_back(tmp_process);
		i++;
	}
	return processes;
}

int how_long(std::vector<Process> processes)
{
	int cmax = 0;
	for (auto& i : processes)
	{
		cmax += i.p;
	}
	return cmax;
}

std::vector<int> make_permutation(std::vector<int>& move_permutation, int add_to,int blank)
{
	if (blank == move_permutation.size() - 1)
	{
		move_permutation[blank] = add_to;
	}
	else
	{
		for (int i = 0; i < move_permutation.size()-1; i++)
		{
			std::swap(move_permutation[i], move_permutation[i + 1]);
		}
		move_permutation[move_permutation.size()-1] = add_to;
	}
	return move_permutation;
}






witi_algorithm algorithm(std::vector<Process>& processes, int blank, witi_algorithm& witi_solution,std::map<std::vector<int>,int>& hash, std::map<std::vector<int>, std::vector<int>>& hash2,std::vector<int> permutation)
{
	Process tmp_proces;
	Process blank_proces = { 0,0,0,0 };
	std::vector<Process> tmp_vector = processes;


	int witi = INT_MAX;
    int prev_witi = INT_MAX;


	std::vector<int> tmp_permutation;
	int tmp_add = 0;
	
	if (blank == processes.size())
	{
		witi_solution.witi = 0;
		return witi_solution;
	}
	
	{
		for (int i = 0; i < processes.size(); i++)
		{
			if(processes[i].p == 0)
			{
				continue;
			}

			tmp_proces = processes[i];
			processes[i] = blank_proces;
			permutation[i] = 0;
			
			if (how_long(tmp_vector) - tmp_proces.d >= 0)
			{
				if (hash.find(permutation) == hash.end())
				{
					witi = std::min(algorithm(processes, blank + 1, witi_solution, hash,hash2,permutation).witi + tmp_proces.w * (how_long(tmp_vector) - tmp_proces.d), witi);
					hash[permutation] = witi;
					if (witi < prev_witi)
					{
						if (hash2.find(permutation) == hash2.end())
						{
							tmp_permutation = permutation;
							tmp_add = tmp_proces.place;
						}
						else
						{
							
							tmp_permutation = hash2[permutation];
							tmp_add = tmp_proces.place;
						}
						tmp_add = tmp_proces.place;
						prev_witi = witi;
					}
				}
				else
				{
					witi = std::min(hash[permutation] + tmp_proces.w * (how_long(tmp_vector) - tmp_proces.d) , witi);
					if (witi < prev_witi)
					{
						if (hash2.find(permutation) == hash2.end())
						{
							tmp_permutation = permutation;
							tmp_add = tmp_proces.place;
						}
						else
						{
							tmp_permutation = hash2[permutation];
							tmp_add = tmp_proces.place;
						}
						tmp_add = tmp_proces.place;
						prev_witi = witi;
					}
				}
			}

			else
			{
				if (hash.find(permutation) == hash.end())
				{
					witi = std::min(algorithm(processes, blank + 1, witi_solution, hash,hash2,permutation).witi, witi);
					hash[permutation] = witi;
					if (witi < prev_witi)
					{
						if (hash2.find(permutation) == hash2.end())
						{
							tmp_permutation = permutation;
							tmp_add = tmp_proces.place;
						}
						else
						{
							tmp_permutation = hash2[permutation];
							tmp_add = tmp_proces.place;
						}
						tmp_add = tmp_proces.place;
						prev_witi = witi;
					}
				}
				else
				{
					witi = std::min(hash[permutation], witi);
				    if (witi < prev_witi)
					{
						if (hash2.find(permutation) == hash2.end())
						{
							tmp_permutation = permutation;
							tmp_add = tmp_proces.place;
						}
						else
						{
							tmp_permutation = hash2[permutation];
							tmp_add = tmp_proces.place;
						}
						tmp_add = tmp_proces.place;
						prev_witi = witi;
					}
				}	
			}

			processes[i] = tmp_proces;
			permutation[i] = tmp_proces.place;
		}
	}

	
	hash2[permutation] = make_permutation(tmp_permutation, tmp_add, blank);
	witi_solution.permutation = tmp_permutation;
	witi_solution.witi = witi;
	return witi_solution;

}


int main()
{


	witi_algorithm witi_solution;

	std::vector <Process> processes, tmp;
	std::map<std::vector<int>, int> hash;
	std::vector<int> permutation;
	std::map<std::vector<int>, std::vector<int>> hash2;

	int zero = 0;
	processes = load_data(std::to_string(1));

	for (int i = 0; i < processes.size(); i++)
	{
		witi_solution.permutation.push_back(zero);
		permutation.push_back(i + 1);
	}



	auto start_time = std::chrono::high_resolution_clock::now();
	algorithm(processes, 0, witi_solution, hash, hash2,permutation);
	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

	float czas = duration.count() / 1000000.0;


	std::cout << "Calkowity czas wynosi: " << czas <<" s" << std::endl;
	std::cout << "Optymalne kara: " << witi_solution.witi << std::endl;
	std::cout << "Optymalne ustawienie procesow: ";
	
	for (int i = 0; i < processes.size(); i++)
	{
		std::cout << witi_solution.permutation[i] << " ";
	}
}