/*
  main.cpp
  <Implementing [Signed Network on Social Media]>
  Refactoring Epinion.cpp (created on 2021.7.7)
  Created by Chan Ho Park on 20217..28.
*/


#include <algorithm>
#include <array>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <utility>
#include <map>
#include <cstdlib>
#include <unistd.h>

int main(int argc, char* argv[])
{
    std::cout << "1..";

    // std::string fileOutPath = "../data/interactions_sampled.csv";
    // std::ofstream OutputFile(fileOutPath.data());

    // if (!OutputFile.is_open())
    // {
    //     exit(1);
    // }

    std::string nodeFileOutPath = "../data/nodeIndex.csv";
    std::ofstream NodeOutputFile(nodeFileOutPath.data());

    if (!NodeOutputFile.is_open())
    {
        exit(1);
    }

    std::string fileInPath = "../data/interactions.txt";
    std::ifstream InputFile(fileInPath.data());

    if(!InputFile.is_open())
    {
        exit(-1);
    }

    std::string SampleFilePath = "../data/NodeSampled.txt";

    std::ifstream SampleFileIn(SampleFilePath.data());

    if(!SampleFileIn.is_open())
    {
        exit(-1);
    }

    std::cout << "2..";

    std::set<int> node_idx;
    
    std::map<int, int> node_idx_m;
    
    while (!SampleFileIn.eof())
    {
        int node, index;
        SampleFileIn >> node >> index;
        node_idx.insert(node);
    }

    std::cout << "3..";

    while(!InputFile.eof())
    {
        int from, to, sign;
        
        InputFile >> from >> to >> sign;

        bool from_in_sampled = (node_idx.find(from) != node_idx.end());
        bool to_in_sampled = (node_idx.find(to) != node_idx.end());

        // if both not exists (F,F) then not in ego network
        if (from_in_sampled || to_in_sampled)
        {
            node_idx.insert(from);
            node_idx.insert(to);
        }
    }

    std::cout << "4..";

    int idx = 0;

    for (auto node : node_idx) // Indexing
    {
        node_idx_m.insert({node, idx});
        idx++;
    }

    for (auto node_index : node_idx_m)
    {
        NodeOutputFile << node_index.first << ',' << node_index.second << '\n';
    }


    // InputFile.clear();
    // InputFile.seekg(0, std::ifstream::beg);

    // while(!InputFile.eof())
    // {
    //     int from, to, sign;
        
    //     InputFile >> from >> to >> sign;

    //     bool from_in_sampled = (node_idx.find(from) != node_idx.end());
    //     bool to_in_sampled = (node_idx.find(to) != node_idx.end());

    //     // if both not exists (F,F) then not in ego network
    //     if (from_in_sampled && to_in_sampled)
    //     {
    //         OutputFile << node_idx_m.find(from)->second << ',' << node_idx_m.find(to)->second << ',' << sign << '\n';
    //     }
    // }

    std::cout << "Done";

    NodeOutputFile.close();
    // OutputFile.close();
    InputFile.close();
    SampleFileIn.close();

    return 0;
}