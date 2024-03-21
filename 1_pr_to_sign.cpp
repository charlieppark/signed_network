#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <set>
#include <map>
#include <utility>
#include <random>
#include <ctime>
#include <queue>

#define PROGRESS_ON

#ifdef PROGRESS_ON
#define PROGRESS(x) printProgress(x);
#endif

#ifndef PROGRESS_ON
#define PROGRESS(x) x;
#endif


#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

void printProgress(double percentage)
{
    int val = (int) (percentage * 100);
    int lpad = (int) (percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}


#define CSRFPSTR "./data/TwitterNodes.txt"

#define GIVENNODES 999999948
#define GIVENEDGES 1963263821

bool compare(std::pair<int, int> x, std::pair<int, int> y);

int main()
{
    std::clock_t start, finish;
    double duration;
    start = clock();

    std::string GraphFilePath = "/home/chanho/project/Bob/data/twitter/twitter_follownet.txt";
    std::ifstream GraphFile(GraphFilePath.data());
    if(!GraphFile.is_open())
    {
        std::cout << "FILE OPEN ERROR";
        exit(-1);
    }


    std::string PageRankFilePath = "/home/chanho/project/Bob/data/twitter/twitter-PR100.txt";
    std::ifstream PageRankFile(PageRankFilePath.data());
    if(!PageRankFile.is_open())
    {
        std::cout << "FILE OPEN ERROR";
        exit(-1);
    }

    std::string CSVFilePath = "../data/interactions.csv";
    std::ofstream CSVFileOut(CSVFilePath.data());
    if(!CSVFileOut.is_open())
    {
        std::cout << "FILE OPEN ERROR";
        exit(-1);
    }

    std::string TXTFilePath = "../data/interactions.txt";
    std::ofstream TXTFileOut(TXTFilePath.data());
    if(!TXTFileOut.is_open())
    {
        std::cout << "FILE OPEN ERROR";
        exit(-1);
    }

    std::map<int, double> PRList;

    int zzz = 0;

    std::cout << "PR" << '\n';

    while(!PageRankFile.eof())
    {
        zzz++;

        int node;
        double pr;
        PageRankFile >> node >> pr;
        PRList.insert(std::pair<int, double> (node, pr));
    }
    PROGRESS(1)

    // COO_T to CSR

    std::vector<int> CSR_row_vec;

    CSR_row_vec.push_back(0);

    std::mt19937 gen(310);

    double alpha = 0.2;

    int numNegativeEdges = alpha * GIVENEDGES;

    std::cout << numNegativeEdges;

    std::cout << "\nRANDOM\n";

    std::uniform_int_distribution<> easyDist(1, 2);
    PROGRESS(1)

    int idx = 0;

    int i = -1;

    std::cout << "\nCSR\n";

    int percentage = 20000000;
    int by = 1;

    CSVFileOut << "Src,Dst,Weight\n";

    while (!GraphFile.eof())
    {
        i++;

        int from, to;

        GraphFile >> from >> to;

        int sign = 0;

        auto j = PRList.find(from);

        auto k = PRList.find(to);

        if (numNegativeEdges > 0)
        {
            if ((j != PRList.end()) && (k != PRList.end()))
            {
                if ((j->second > k->second) && (easyDist(gen) == 2))
                {
                    sign = -1;
                    numNegativeEdges--;
                }
            }
        }

        if (sign == 0)
        {
            sign = 1;
        }

        CSVFileOut << from << ',' << to << ',' << sign << '\n';
        TXTFileOut << from << ' ' << to << ' ' << sign << '\n';

        // if(from == idx)
        //     continue;
        // else
        // {
        //     int times = from - idx - 1;

        //     for (int j = 0; j < times; j++)
        //     {
        //         CSR_row_vec.push_back(-1);
        //         idx++;
        //     }

        //     CSR_row_vec.push_back(i);

        //     idx++;
        // }

        if (i > percentage * by)
        {
            double progress = (double)i/2000000000;
            //PROGRESS(progress)
            by++;
        }
    }
    PROGRESS(1)

    std::cout << numNegativeEdges;

    // for (auto i = CSR_row_vec.begin(); i < CSR_row_vec.end(); i++)
    // {
    //     CSRFileOut << (*i) << " ";
    // }

    int nodes = idx;
    int edges = i + 1;

    std::cout << nodes << " " << edges << '\n';

    CSVFileOut.close();
    TXTFileOut.close();
    GraphFile.close();
    PageRankFile.close();

    finish = clock();

    duration = (double)(finish - start) / CLOCKS_PER_SEC;

    std::cout << duration << "sec" << '\n';

    return 0;
}

bool compare(std::pair<int, int> x, std::pair<int, int> y)
{
	return x.first < y.first;
}