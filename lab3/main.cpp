#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>

//Struktura w ktorej bedziemy przechowywac nasze elementy potrzebne do zastosowania algorytmu NEH i QNEH
struct data
{
	int rows = 0; //liczba wierszy w pliku danych
	int columns = 0; //liczba kolumn w liczbie danych
	int** L, ** R, ** time; //tablice dwywymiarowe, tablice "time" to tablica w ktorej przechowuje ile czasu zajmuje obrobka na danej maszynie, tablice "L" i "R" sa potrzebne do QNEHA
	int* all_machime_time; //tablica w ktorej przechowuje ile czasu zajmuje obrobka danego procesu na wszystkich maszynach
	std::vector<std::pair<int, int>> time_order; //vector w ktorym przechowuje jaki proces ma jaki czas wykonania na wszystkich maszynach od razu w kolejnosci malejacej

	std::vector<int> permutation; //wektor w ktorej przechowujemy permutacje procesow

	//cmax maksymalny
	int cmax = 0;

	//wyniki dzialania dla kazdej instacji
	int tab_wynik[121] = { 31,1286,1365,1159,1325,1305,1228,1278,1223,1291,1151,1680,1729,1557,1439,1502,1453,1562,1609,1647,1653,2410,2150,2411,2262,2397,2349,2362,2249,2320,2277,2733,2843,2640,2782,2868,2850,2758,2721,2576,2790,3135,3032,2986,3198,3160,3178,3277,3123,3002,3257,4082,3921,3927,3969,3835,3914,3952,3938,3952,4079,5519,5348,5219,5023,5266,5139,5259,5120,5489,5341,5846,5453,5824,5929,5679,5375,5704,5760,6032,5918,6541,6523,6639,6557,6695,6664,6632,6739,6677,6677,10942,10716,11025,11057,10645,10458,10989,10829,10574,10807,11594,11675,11852,11803,11685,11629,11833,11913,11673,11869,26670,27232,26848,27055,26727,26992,26797,27138,26631,26984 };

	//zmienne pomocnicze ktore powiedza nam ile wynikow dzialania algorytmu daje takie same wyniki/gorsze/lepsze jak w pliku tekstowym
	int worse = 0;
	int same = 0;
	int better = 0;
};

//Funkcja ktora wyczysci nam wszystkie tablice po tym jak dla danej instacji obliczymy permutacje i cmax
void clear_data(data& datas) //funkcja pomocnicza ktora kasuje wszystkie wczytane dane poniewaz wczytujemy 120 roznych pakietow danych
{
	for (int i = 0; i < datas.rows; i++)
	{
		delete[] datas.time[i];
		delete[] datas.L[i];
		delete[] datas.R[i];
	}
	delete[] datas.L;
	delete[] datas.R;
	delete[] datas.time;

	delete[] datas.all_machime_time;
	datas.permutation.clear();
	datas.time_order.clear();
	int cmax = 0;
}

//Funkcja pomocnicza dla algorytmu QNEH w ktorej na samym koncu obliczymy cmax poniewaz nie musimy go liczyc za kazdym razem tylko raz na koncu
void cmax(data& datas)
{
	int** tab;
	tab = new int* [datas.rows];


	for (int i = 0; i < datas.rows; i++)
	{
		tab[i] = new int[datas.columns];
	}

	for (int i = 0; i < datas.rows; i++)
	{
		for (int j = 0; j < datas.columns; j++)
		{
			if (i == 0)
			{
				if (j == 0)
				{
					tab[i][j] = datas.time[datas.permutation[i]][j];
				}
				if (j > 0)
				{
					tab[i][j] = tab[i][j - 1] + datas.time[datas.permutation[i]][j];
				}
			}
			if (i > 0)
			{
				if (j == 0)
				{
					tab[i][j] = tab[i - 1][j] + datas.time[datas.permutation[i]][j];
				}
				if (j > 0)
				{
					tab[i][j] = std::max(tab[i][j - 1], tab[i - 1][j]) + datas.time[datas.permutation[i]][j];
				}
			}
		}
	}

	datas.cmax = tab[datas.rows - 1][datas.columns - 1];


	for (int i = 0; i < datas.rows; i++)
	{
		delete[] tab[i];
	}
	delete[] tab;
}

//Funkcja nieuzywana
void make_zero(data& datas)
{
	for (int i = 0; i < datas.rows; i++)
	{
		for (int j = 0; j < datas.columns; j++)
		{
			datas.L[i][j] = 0;
			datas.R[i][j] = 0;
		}
	}
}

//Funkcja pomocnicza ktora pomoze nam posortowac wektor w ktorym znajdujda sie procesy ustawione w kolejnosci malejacej
bool compare(const std::pair<int, int>& a, const std::pair<int, int>& b)
{
	if (a.second == b.second) //jesli dwa procesy zajmuja tyle samo czasu na wszystkich maszynach to pierwszy bedzie proces o mniejszym indeksie
	{
		return a.first < b.first;
	}
	else //jesli proces ma wiekszy czas wykonania na wszystkich maszynach to bedzie pierwszy
	{
		return a.second > b.second;
	}

}

//Funkcja ktora obliczy nam dla ile czasu zajmie obrobienie kazdego osobnego procesu przez wszystkie maszyny
void time_one_process(data& datas) //funkcja ktora oblicza jaki proces zajmuje czasu na wszystkich maszynach i ustawia potem procesy w kolejnosci malejacej
{
	std::pair<int, int> tmp_pair; //tworzenie pary, pierwsze miejsce to bedzie numer procesu a drugie miejsce to maksymalny czas przejscia przez wszystkie maszyny
	for (int i = 0; i < datas.rows; i++)
	{
		/*ustawienie na poczatku wartosci zero poniewaz w lini 134 dodajemy wartosci w petli a poniewaz czasem przy dynamicznej
		* tablicy wartosci w niej nie  ustawiaja sie na wartosc '0' tylko przyjmuja losowe wartosci to zeby tego uniknac nadajemy na poczatku wartoc '0'*/
		datas.all_machime_time[i] = 0;
		for (int k = 0; k < datas.columns; k++)
		{
			datas.all_machime_time[i] += datas.time[i][k]; //liczenie ile czasu zajmie danemu procesowi przejscie przez wszystkie maszyny
		}

		tmp_pair = { i,datas.all_machime_time[i] }; //dodanie wartosci do pary

		datas.time_order.push_back(tmp_pair); //dodanie pary do wektora

		tmp_pair = { 0,0 }; //ustawienie wartosci pary na (0,0)

	}
	std::sort(datas.time_order.begin(), datas.time_order.end(), compare); //posortowanie vectora na podstawie czasu przejscia procesu przez wszystkie maszyny w kolejnosci malejacej

}

//Algorytm NEH 
void cmax_permutation_neh(data& datas)
{
	int* tmp_cmax = new int[datas.columns](); //inicjowanie tablicy dynamicznej dla kazdej maszyny
	int real_cmin = INT_MAX;
	int swapping = 0; //zmienna pomocnicza mowiaca nam na ktore miejsce przesuniemy nasz dodatkowy proces
	int tmp_swap = 0; //zmienna pomocnicza ktora zapamieta jaki bedzie tymczasowy cmax dla konkretnej permutacji procesow np (1,2,3,4) a potem sprawdzi (1,2,4,3) i zapamieta najmniejszy cmax

	if (datas.permutation.size() > 1)
	{
		for (int i = 0; i < datas.permutation.size(); i++) //petla w ktorej bedziemy umieszczac dodatkowy proces na miejscach o indeksie rownym i
		{
			for (int j = 0; j < datas.permutation.size(); j++) //petla po procesach
			{
				for (int k = 0; k < datas.columns; k++) //petla po maszynach
				{
					if (k == 0)
					{
						if (j < i)
						{
							tmp_cmax[k] = std::max(tmp_cmax[k], 0) + datas.time[datas.permutation[j]][k];
						}
						else if (j == i)
						{
							tmp_cmax[k] = std::max(tmp_cmax[k], 0) + datas.time[datas.permutation[datas.permutation.size() - 1]][k];
						}
						else if (j > i)
						{
							tmp_cmax[k] = std::max(tmp_cmax[k], 0) + datas.time[datas.permutation[j - 1]][k];
						}
					}
					else if (k > 0)
					{
						if (j < i)
						{
							tmp_cmax[k] = std::max(tmp_cmax[k], tmp_cmax[k - 1]) + datas.time[datas.permutation[j]][k];
						}
						else if (j == i)
						{
							tmp_cmax[k] = std::max(tmp_cmax[k], tmp_cmax[k - 1]) + datas.time[datas.permutation[datas.permutation.size() - 1]][k];
						}
						else if (j > i)
						{
							tmp_cmax[k] = std::max(tmp_cmax[k], tmp_cmax[k - 1]) + datas.time[datas.permutation[j - 1]][k];
						}
					}
				}
			}
			// sprawdzanie na ktorym miejscu bedzie pasowal najlepiej dodatkowy proces (np: mam procesy (4,3,2) oraz proces (1) 
			// i pytanie gdzie go najlepiej ulozyc zeby wartosc byla jak najmniejsza np: (1,4,3,2) albo (4,1,3,2)
			if (tmp_cmax[datas.columns - 1] < real_cmin)
			{
				swapping = i; //zapamietanie jaki to byl proces zeby wstawic go na miejsce "swapping"
				real_cmin = tmp_cmax[datas.columns - 1]; //zapamietanie najmniejszej wartosci uszeregowania
			}
			//ustawianie elementow w tablicy na wartosci zerowe poniewaz teraz proces dodatkowy w petli bedzie wpasowany w inne miejsce
			memset(tmp_cmax, 0, datas.columns * sizeof(int));
		}

		/*petla w ktorej przesuwamy procesy, zawsze przesuwamy ostatni element z wektora poniewaz on jest procesem "dodatkowym",
		mamy procesy (4,3,2,1) i proces (1) bedzie najlepiej pasowal na pozycji o indexie '0' w takim razie po przesunieciu bedziemy mieli (1,4,3,2)*/
		if (swapping < datas.permutation.size() - 1)
		{
			int tmp_swap = datas.permutation[datas.permutation.size() - 1];
			for (int i = datas.permutation.size() - 1; i > swapping; i--)
			{
				datas.permutation[i] = datas.permutation[i - 1];
			}
			datas.permutation[swapping] = tmp_swap;
		}


		//jesli mamy juz wszystkie procesy w wektorze permutacji to nasz cmax bedzie rowny najmniejszej wartosci uszeregowania
		if (datas.permutation.size() == datas.rows)
		{
			datas.cmax = real_cmin;
		}

	}
	delete[] tmp_cmax;
}

//Algorytm QNEH
void cmax_permutation_qneh(data& datas)
{
	int* tmp_cmax = new int[datas.columns](); //tablica pomocnicza
	int* tmp_connect = new int[datas.permutation.size()](); //tablica pomocnicza ktora bedzie nam mowic pod jakim indeksem jaka bedzie wartosc polaczenia procesow. Jesli dla i=2 nastapi najmniejszy czas dolaczenia procesu to przerzucimy nasz dodatkowy proces na miejsce o indeksie rownym 2
	int real_cmin = INT_MAX;
	int swapping = 0; //zmienna pomocnicza mowiaca nam na ktore miejsce przesuniemy nasz dodatkowy proces
	int tmp_swap = INT_MAX; //zmienna pomocnicza ktora zapamieta jaki bedzie tymczasowy cmax dla konkretnej permutacji procesow np (1,2,3,4) a potem sprawdzi (1,2,4,3) i zapamieta najmniejszy cmax


	//petla do obliczania tabli L i tablicy R ktore sa potrzebne do zobaczenia w jakim miejscu bedzie trzeba umiescic dany proces zeby bylo jak najszybciej
	if (datas.permutation.size() > 1)
	{
		for (int j = 0; j < datas.permutation.size() - 1; j++) //petla po procesach
		{

			for (int k = 0; k < datas.columns; k++) //petla po maszynach
			{
				if (k == 0)
				{
					if (j == 0)
					{
						datas.L[j][k] = datas.time[datas.permutation[j]][k];
						datas.R[datas.permutation.size() - 2 - j][datas.columns - k - 1] = datas.time[datas.permutation[datas.permutation.size() - 2 - j]][datas.columns - k - 1];
					}
					if (j > 0)
					{
						datas.L[j][k] = datas.L[j - 1][k] + datas.time[datas.permutation[j]][k];
						datas.R[datas.permutation.size() - 2 - j][datas.columns - k - 1] = datas.time[datas.permutation[datas.permutation.size() - 2 - j]][datas.columns - k - 1] + datas.R[datas.permutation.size() - 2 - j + 1][datas.columns - k - 1];
					}
				}
				if (k > 0)
				{
					if (j == 0)
					{
						datas.L[j][k] = datas.L[j][k - 1] + datas.time[datas.permutation[j]][k];
						datas.R[datas.permutation.size() - 2 - j][datas.columns - k - 1] = datas.R[datas.permutation.size() - 2 - j][datas.columns - k] + datas.time[datas.permutation[datas.permutation.size() - 2 - j]][datas.columns - k - 1];
					}
					if (j > 0)
					{
						datas.L[j][k] = std::max(datas.L[j - 1][k], datas.L[j][k - 1]) + datas.time[datas.permutation[j]][k];
						datas.R[datas.permutation.size() - 2 - j][datas.columns - k - 1] = std::max(datas.R[datas.permutation.size() - 2 - j + 1][datas.columns - k - 1], datas.R[datas.permutation.size() - 2 - j][datas.columns - k]) + datas.time[datas.permutation[datas.permutation.size() - 2 - j]][datas.columns - k - 1];
					}


				}
			}
		}


		//tutaj nastepuje obliczanie najlepszego ulozenia naszego dodatkowego procesu
		for (int i = 0; i < datas.permutation.size(); i++)
		{
			if (i == 0) //tutaj jesli dodajemy nasz proces na poczatku
			{
				for (int h = 0; h < datas.columns; h++)
				{
					if (h == 0)
					{
						tmp_cmax[h] = datas.time[datas.permutation[datas.permutation.size() - 1]][h];
						tmp_connect[i] = std::max(tmp_connect[i], tmp_cmax[h] + datas.R[i][h]);
					}
					if (h > 0)
					{
						tmp_cmax[h] = tmp_cmax[h - 1] + datas.time[datas.permutation[datas.permutation.size() - 1]][h];
						tmp_connect[i] = std::max(tmp_connect[i], tmp_cmax[h] + datas.R[i][h]);
					}
				}
			}

			if (i == datas.permutation.size() - 1) //tutaj jesli dodajemy nasz proces na samym koncu
			{
				for (int h = 0; h < datas.columns; h++)
				{
					if (h == 0)
					{
						tmp_cmax[h] = datas.L[i - 1][h] + datas.time[datas.permutation[datas.permutation.size() - 1]][h];
						tmp_connect[i] = tmp_cmax[h];
					}
					if (h > 0)
					{
						tmp_cmax[h] = std::max(tmp_cmax[h - 1], datas.L[i - 1][h]) + datas.time[datas.permutation[datas.permutation.size() - 1]][h];
						tmp_connect[i] = std::max(tmp_connect[i], tmp_cmax[h]);
					}
				}
			}


			if (i > 0 && i < datas.permutation.size() - 1) //tutaj kiedy dodajemy nasz dodatkowy proces gdzies po srodku
			{
				for (int h = 0; h < datas.columns; h++)
				{
					if (h == 0)
					{
						tmp_cmax[h] = datas.time[datas.permutation[datas.permutation.size() - 1]][h] + datas.L[i - 1][h];
						tmp_connect[i] = tmp_cmax[h] + datas.R[i][h];
					}
					if (h > 0)
					{
						tmp_cmax[h] = std::max(tmp_cmax[h - 1], datas.L[i - 1][h]) + datas.time[datas.permutation[datas.permutation.size() - 1]][h];
						tmp_connect[i] = std::max(tmp_connect[i], tmp_cmax[h] + datas.R[i][h]);
					}
				}
			}

			if (tmp_connect[i] < tmp_swap) //sprawdzamy czy dodanie procesu w danej kolejnosci np: proces dodany-P i mamy procesy (1,2,3,4) i sprawdzamy czy np (P,1,2,3,4) daje najlepsza mozliwosc
			{
				swapping = i; //jesli tak sie stanie to zapisujemy pozycje na ktora mamy przerzucic dany proces 
				tmp_swap = tmp_connect[i];
			}
		}


		if (swapping < datas.permutation.size() - 1) //petla do przerzucenia procesu w wektorze permutacji
		{
			int tmp_size = datas.permutation[datas.permutation.size() - 1];
			for (int i = datas.permutation.size() - 1; i > swapping; i--)
			{
				datas.permutation[i] = datas.permutation[i - 1];
			}
			datas.permutation[swapping] = tmp_size;
		}
	}
	delete[] tmp_connect;
	delete[] tmp_cmax;
}

//Wstep do NEH
void NEH(data& datas)
{
	for (int i = 0; i < datas.time_order.size(); i++)
	{
		datas.permutation.push_back(datas.time_order[i].first); //dodanie do wektora permutacji procesow w kolejnosci malejacej
		cmax_permutation_neh(datas); //obliczenie cmaxa dla danej permutacji 
	}
}

//Wstep do QNEH
void QNEH(data& datas)
{

	for (int i = 0; i < datas.time_order.size(); i++)
	{
		datas.permutation.push_back(datas.time_order[i].first); //dodanie do wektora permutacji procesow w kolejnosci malejacej
		cmax_permutation_qneh(datas); //obliczenie permutacji 
	}
	cmax(datas); //obliczenie cmax jednorazowo na samym koncu programu
}

//Funkcja w ktorej bedziemy wczytywac dane z pliku tekstowego
void load_data(data& datas, int choice) //Funkcja ktora wczytuje ustala nam liczbie wierszy, kolumn oraz wczytuje dane do tablic
{
	std::ifstream file("data.txt"); // Otwarcie pliku
	std::string line; //zmienna pomocnicza do ktorej bedziemy wczytywac linie po kolei

	bool rows_columns = false; //zmienna pomocnicza ktora mowi nam ze nastepna wczytana linia bedzie zawierac informacje o liczbie wierszy i kolumn
	bool times = false; //zmienna pomocnicze mowiaca nam ze teraz wczytamy liczbe wierszy i liczbe kolumn np : (4,3) albo (100,50)
	int array_rows = 0; //zmienna pomocnicza mowiaca nam ktory mamy aktualnie wiersz
	int instance = 0; //zmienna pomocnicza mowiaca nam ktora instancje wlasnie sprawdzamy (0-120)


	while (std::getline(file, line))
	{

		// Czytanie pliku linijka po linijce
		if (line.find("data") != std::string::npos) //jesli znajdziemy linie w ktorej jest "data.xxx" to w nastepnej lini bedzie informacja o liczbie wierszy i kolumn
		{
			rows_columns = true; //zmienna pomocnicza ustawiona na true mowi nam ze w nastepnej lini znajdziemy informacje o liczbie wierszy i kolumn
			array_rows = 0; //wyzerowanie indeksu wierszy
			continue; //przejscie do nastepnej linii
		}


		std::istringstream iss(line); //
		if (rows_columns) //ustalenie ile bedzie wiereszy i kolumn
		{
			iss >> datas.rows >> datas.columns; // ustalenie liczby wierszy i kolumn czytajac z pliku np: 4 3 (4 liczba wierwszy, 3 liczba kolumn)

			//Inicjalizacja wierszy dla naszych tablic 
			datas.L = new int* [datas.rows];
			datas.R = new int* [datas.rows];
			datas.time = new int* [datas.rows];
			datas.all_machime_time = new int[datas.rows];

			for (int i = 0; i < datas.rows; i++)
			{
				//Inicjalizacja kolumn dla naszych tablic 
				datas.L[i] = new int[datas.columns];
				datas.R[i] = new int[datas.columns];
				datas.time[i] = new int[datas.columns];
			}
			rows_columns = false; //zmienna pomocnicza ustawiona na false poniewaz w nastepnych liniach nie bedzie informacji o liczbie wierwszy i kolumn
			times = true; //zmienna pomocnicza mowiaca nam ze w nastepnych liniach bedziemy wczytac jaki czas zajmuje jaki proces na jakiej maszynie
			continue; // przejscie do nastepnej lini
		}


		if (times)//wpisywanie jednego wiersza do tablicy
		{
			int num = 0;
			int counter = 0;
			while (iss >> num)
			{ // Wpisywanie liczb do tablicy dwuwymiarowej
				datas.time[array_rows][counter] = num;
				counter++; //inkrementacja licznika kolumn
			}

			array_rows++;  //inkremencja licznika wierszy zeby przy kolejnej lini wczytac do kolejnego wiersza

			if (array_rows == datas.rows)   //Sprawdzanie czy do wektora zostaly wpisane wszytkie wiersze
			{
				times = false;  //jesli sie wpisaly to ustawiamy zmienna "times" na false

				time_one_process(datas);//obliczamy czasy wykonania kazdego procesu na wszystkich maszynach i ustawiamy w kolejnosci rosnacej

				if (choice == 1) NEH(datas);
				else if(choice==2) QNEH(datas);


				//tutaj nastepuja sprawdzenie czy nasz wynik wyszedl lepszy/gorszy/taki sam jak w pliku tekstowym
				if (datas.cmax == datas.tab_wynik[instance])
				{
					datas.same++;
				}
				else if (datas.cmax < datas.tab_wynik[instance])
				{
					datas.better++;
				}
				else if (datas.cmax > datas.tab_wynik[instance])
				{
					datas.worse++;
				}

				instance++;

				clear_data(datas); //wyczyszczenie i usuniecie wszystkich tablic ktore powstaly zeby w nastepnym przejsciu petli stworzyc tablice od nowa 

			}
		}
	}
}

//Funkcja pomocnicza sluzaca do wyswietlenia statow
void display_stats(data& datas,float real_time)
{
	std::cout << "Wynik taki sam= " << datas.same << std::endl;
	std::cout << "Wynik lepszy= " << datas.better << std::endl;
	std::cout << "Wynik gorszy= " << datas.worse << std::endl;
	std::cout << "Czas wykonania= " << real_time << "s" << std::endl;
	datas.same = datas.better = datas.worse = 0;
}

//Funkcja odpalacaja program
void run()
{
	data datas{};
	int choice = 0;
	
	std::cout << "Wybierz opcje: 1.Algorytm NEH" << std::endl;
	std::cout << "               2.Algorytm QNEH" << std::endl;
	std::cout << "               3.Koniec dzialania programu" << std::endl;
	
	while (choice!=3)
	{
		std::cin >> choice;
		switch (choice)
		{
		case 1:
		{
			auto start = std::chrono::high_resolution_clock::now();
			load_data(datas, choice);
			auto end = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
			float real_time = duration.count() / 1000.0;
			display_stats(datas, real_time);
			break;
		}
		case 2:
		{
			auto start = std::chrono::high_resolution_clock::now();
			load_data(datas, choice);
			auto end = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
			float real_time = duration.count() /1000.0;
			display_stats(datas, real_time);
			break;
		}
		case 3:
		{
			std::cout << "Naura";
			break;
		}

		default:
		{
			std::cout << "Nie ma takiego wyboru prosze wybrac raz jeszcze:";
			std::cin >> choice;
			break;
		}
		};
	}

}


int main()
{
	run();
}