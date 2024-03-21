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
#include <random>
#include <unistd.h>

// //tee device
// #include <boost/iostreams/tee.hpp>
// #include <boost/iostreams/stream.hpp>


#include "MatrixOperation.h"
#include "Data.h"
#include "ThreadPool.h"

const int max = 1024;

const int random_size = 40000;

//To turn off progress bar, comment out the line below
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

bool compare(std::pair<int, int> x, std::pair<int, int> y);

std::vector<std::array<int, 4>> triad_func(int init, int last, int *CSR_row, int *CSR_col, int *CSR_sign, int nodes);

int main(int argc, char* argv[])
{
    std::cout.setf(std::ios::unitbuf); // unbuffered output for live logging in nohup

    auto t_start = std::chrono::high_resolution_clock::now();

    int num_of_sub_processes = atoi(argv[1]);

    std::string fileOutPath = "../data/triad_result.txt";
    std::ofstream OutputFile(fileOutPath.data());

    if (!OutputFile.is_open())
    {
        std::cout << "OUTPUT FILE OPEN ERROR";
        exit(1);
    }

    // typedef boost::iostreams::tee_device<std::ostream, std::ofstream> TeeDevice;
    // typedef boost::iostreams::stream<TeeDevice> TeeStream;

    // TeeDevice output_tee(std::cout, OutputFile); 
    // TeeStream output_split(output_tee);

    GivenData* Data = new GivenData{"Twitter", 0, 52579682, 1963263820, "../data/interactions.txt", 0, 0, 0};

    time_t curTime = time(NULL);
    struct tm *pLocal = localtime(&curTime);

    char buf[1024];

    sprintf(buf, "\n\nStarted at %04d-%02d-%02dT%02d:%02d:%02d\n\n",  
        pLocal->tm_year + 1900, pLocal->tm_mon + 1, pLocal->tm_mday,  
        pLocal->tm_hour, pLocal->tm_min, pLocal->tm_sec);

    std::cout  << std::string(buf);
    OutputFile << std::string(buf);

    std::cout  << "[" << Data->nameOfData << "] Open File : " << Data->fileInPath << "\n\n";
    OutputFile << "[" << Data->nameOfData << "] Open File : " << Data->fileInPath << "\n\n";

    std::ifstream InputFile(Data->fileInPath.data());

    if(!InputFile.is_open())
    {
        std::cout  << "FILE OPEN ERROR\n";
        OutputFile << "FILE OPEN ERROR\n";
        exit(-1);
    }

    // Save spammers in list

    std::string LabelInPath = "/home/chanho/project/Bob/data/twitter/spammer_idlist.txt";

    std::ifstream LabelInputFile(LabelInPath.data());

    if (!LabelInputFile.is_open())
    {
        std::cout  << "LABEL INPUT FILE OPEN ERROR\n";
        OutputFile << "LABEL INPUT FILE OPEN ERROR\n";
        exit(1);
    }

    std::set<int> spammer_list;

    while(!LabelInputFile.eof())
    {
        int spammer;
        
        LabelInputFile >> spammer;

        spammer_list.insert(spammer);
    }


    std::string SampleFilePath = "../data/NodeSampled.txt";

    std::ofstream SampleFileOut;
    std::ifstream SampleFileIn;

    // i == 0 : exists
	// i == 1 : execute
	// i == 2 : write
	// i == 3 : read

    SampleFileIn.open(SampleFilePath);

    std::map<int, int> node_idx_m;

    // Save all nodes of graph in set
    std::set<int> node_idx;
    
    if (SampleFileIn)
    {
        std::cout  << "Sample File Exists\n";
        OutputFile << "Sample File Exists\n";

        while(!SampleFileIn.eof())
        {
            int node, index;
            SampleFileIn >> node >> index;
            node_idx_m.insert({node, index});
        }

        for (auto const &node_index : node_idx_m)
        {
            node_idx.insert(node_index.first);
        }
    }
    else
    {
        std::cout  << "Sample File Not Exists\n";
        OutputFile << "Sample File Not Exists\n";

        while(!InputFile.eof())
        {
            int from, to, sign;

            InputFile >> from >> to >> sign;

            node_idx.insert(from);
            node_idx.insert(to);
        }

        // Open Sample File as Output
        SampleFileOut.open(SampleFilePath);

        if (!SampleFileOut.is_open())
        {
            std::cout  << "LABEL INPUT FILE OPEN ERROR\n";
            OutputFile << "LABEL INPUT FILE OPEN ERROR\n";
            exit(1);
        }

        // Random Setting

        std::cout  << "\nRANDOM & NODE INDEXING\n";
        OutputFile << "\nRANDOM & NODE INDEXING\n";

        std::mt19937 gen(310);

        int num_of_nodes = Data->givenNodes;

        std::uniform_int_distribution<> pick_node(0, num_of_nodes - 1);

        int idx = 0;

        for (auto node : node_idx) // Indexing
        {
            if ((spammer_list.find(node) != spammer_list.end()) || (pick_node(gen) < 40000)) // in spammer list or sample target
            {
                node_idx_m.insert({node, idx});
                idx++;
            }
        }

        node_idx.clear();

        for (auto const &node_index : node_idx_m)
        {
            SampleFileOut << node_index.first << ' ' << node_index.second << '\n';
            node_idx.insert(node_index.first);
        }
    }

    int num_of_sampled_nodes = node_idx.size();

    int positive_edges = 0;
    int negative_edges = 0;

    // given form in the dataset is transposed matrix form of COO
    int **COO_T = new int *[Data->givenEdges];

    if (COO_T == NULL)
    {
        std::cout  << "MALLOC ERROR - COO_T\n";
        OutputFile << "MALLOC ERROR - COO_T\n";
        exit(1);
    }

    std::cout  << "start reading graph\n";
    OutputFile << "start reading graph\n";

    InputFile.clear();
    InputFile.seekg(0, std::ifstream::beg);

    std::cout  << "in\n";
    OutputFile << "in\n";

    int idx = 0;

    while (!InputFile.eof())
    {
        int from, to, sign;

        int *row = new int[3];

        if (row == NULL)
        {
            std::cout  << "MALLOC ERROR - row\n";
            OutputFile << "MALLOC ERROR - row\n";
            exit(1);
        }

        InputFile >> from >> to >> sign;

        bool from_in_sampled = (node_idx.find(from) != node_idx.end());
        bool to_in_sampled = (node_idx.find(to) != node_idx.end());

        // if both not exists (F,F) then not in ego network
        if (from_in_sampled || to_in_sampled)
        {
            row[0] = from;
            row[1] = to;
            row[2] = sign;

            COO_T[idx] = row;

            idx++;

            if (row[2] == 1) // sign is 1 (positive)
                positive_edges++;
            else
                negative_edges++;
        }
        else delete row;
    }

    int edges = positive_edges + negative_edges - 1;

    std::cout  << "edges : " << edges << '\n';
    OutputFile << "edges : " << edges << '\n';

    std::vector<int> node_idx_v(node_idx.begin(), node_idx.end());

    int progress_check = edges / 100;

    int q = 1;

    for (int i = 0; i < edges; i++)
    {
        auto row = COO_T[i];
        row[0] = node_idx_m[row[0]];
        row[1] = node_idx_m[row[1]];
    }

    InputFile.close();

    if (edges != Data->givenEdges)
    {
        std::cout  << edges << " " << Data->givenEdges << '\n';
        OutputFile << edges << " " << Data->givenEdges << '\n';
        std::cout  << "Counting Error\n";
        OutputFile << "Counting Error\n";
        //exit(1);
    }

    std::cout  << "nodes = " << num_of_sampled_nodes << "\nedges = " << edges
            << ", positive = " << float(positive_edges) / edges
            << "%, negative " << float(negative_edges) / edges << "%\n\n";
    OutputFile << "given_nodes = " << num_of_sampled_nodes << "\nedges = " << edges
            << ", positive = " << float(positive_edges) / edges
            << "%, negative " << float(negative_edges) / edges << "%\n\n";

    // COO
    int **COO = transpose(COO_T, edges, 3);

    // COO to CSR

    idx = 0;

    std::vector<int> CSR_row_vec;

    CSR_row_vec.push_back(0);

    for (int i = 0; i < edges; i++)
    {
        if (COO[0][i] == idx)
            continue;
        else
        {
            int times = COO[0][i] - idx - 1;

            for (int j = 0; j < times; j++)
            {
                // printf("%d %d", COO[0][i], idx);
                CSR_row_vec.push_back(-1);
                idx++;
            }

            CSR_row_vec.push_back(i);

            idx++;
        }
    }

    CSR_row_vec.push_back(edges);

    int size = CSR_row_vec.size();

    std::cout  << "size : " << size << '\n';
    OutputFile << "size : " << size << '\n';

    // if (size != (size_t)Datas[z]->givenNodes + 1)
    // {
    //     std::cout << "CSR ERROR";
    //     std::cout << "nodes = " << Datas[z]->givenNodes << ", CSR size = %d" << (int)size;
    //     exit(1);
    // }

    // for (size_t i = 0; i < size - z; i++)
    // {
    //     int k = COO[0][CSR_row_vec.at(i)];

    //     if ((i != k) && k)
    //     {
    //         std::cout << "ERROR : COO -> CSR conversion";
    //         OutputFile.close();
    //         exit(1);
    //     }
    // }

    int *CSR_row = &CSR_row_vec[0];

    delete COO[0];

    std::vector<int> CSR_col_vec(COO[1], COO[1] + edges);

    int *CSR_col = &CSR_col_vec[0];

    delete COO[1];

    std::vector<int> CSR_sign_vec(COO[2], COO[2] + edges);

    int *CSR_sign = &CSR_sign_vec[0];

    delete COO[2];

    delete COO;

    // //std::vector<std::array<int, 3>> triads;
    // std::set<std::array<std::pair<int, int>, 3>> t2;
    // std::set<std::array<std::pair<int, int>, 3>> t1;
    // std::set<std::array<std::pair<int, int>, 3>> t3;
    // std::set<std::array<std::pair<int, int>, 3>> t0;

    std::vector<std::vector<std::array<int, 4>>> DataList;

    // for (int i = 0; i < num_of_sub_processes; i++)
    // {
    //     std::vector<std::array<int, 4>> dataOfNodes(Datas[z]->givenNodes, {0, 0, 0, 0});
    //     DataList.push_back(dataOfNodes);
    // }

    // std::set<std::vector<int>> triads;

    // std::set<std::set<int>> triads;

    std::cout  << "Multi ..";
    OutputFile << "Multi ..";

    ThreadPool pool(std::thread::hardware_concurrency());

    int partial_size = size / num_of_sub_processes;

    std::vector<std::future<std::vector<std::array<int, 4>>>> results;

    for (int i = 0; i < num_of_sub_processes - 1; i++)
    {
        results.emplace_back(pool.enqueue(triad_func, partial_size * i, partial_size * (i + 1), CSR_row, CSR_col, CSR_sign, num_of_sampled_nodes));
    }

    results.emplace_back(pool.enqueue(triad_func, partial_size * (num_of_sub_processes - 1), size, CSR_row, CSR_col, CSR_sign, num_of_sampled_nodes)); 

    for (auto& r : results)
    {
        DataList.push_back(r.get());
    }

    std::cout  << "DONE\n";
    OutputFile << "DONE\n";

    // size_t triads_count = t0.size() + t1.size() + t2.size() + t3.size();

    // OutputFile << "triads = " << triads_count;
    // OutputFile << "\nT0 = " << t0.size() << ",T1 = " << t1.size() << ",T2 = " 
    // << t2.size() << ",T3 = " << t3.size() << "\n\n\n";

    //Extracting Data of Nodes
    std::string nodeFileOutPath = "../data/" + Data->nameOfData + "Nodes.txt";
    
    std::ofstream NodeOutputFile(nodeFileOutPath.data());

    if (!NodeOutputFile.is_open())
    {
        std::cout  << "NODE OUTPUT FILE OPEN ERROR";
        OutputFile << "NODE OUTPUT FILE OPEN ERROR";
        exit(1);
    }

    // Copied to up - sampling part
    // std::string LabelInPath = "/home/chanho/project/Bob/data/twitter/spammer_idlist.txt";
    
    // std::ifstream LabelInputFile(LabelInPath.data());

    // if (!LabelInputFile.is_open())
    // {
    //     std::cout << "LABEL INPUT FILE OPEN ERROR";
    //     exit(1);
    // }

    // std::set<int> spammer_list;

    // while(!LabelInputFile.eof())
    // {
    //     int spammer;
        
    //     LabelInputFile >> spammer;

    //     spammer_list.insert(spammer);
    // }

    for (int i = 0; i < num_of_sampled_nodes - 1; i++)
    {
        std::array<int, 4> node_triads = {0};

        for (int j = 0; j < num_of_sub_processes; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                node_triads[k] += DataList[j][i][k];
            }
        }

        int node_recovered = node_idx_v[i];

        int spammer = 0;

        if (spammer_list.find(node_recovered) != spammer_list.end()) spammer = 1;

        NodeOutputFile << node_recovered << ',' << node_triads[0] << ',' 
        << node_triads[1] << ',' << node_triads[2] << ',' 
        << node_triads[3] << ',' << spammer << '\n';
    }

    NodeOutputFile.close();

    auto t_end = std::chrono::high_resolution_clock::now();

    std::cout  << std::chrono::duration<double, std::milli>(t_end-t_start).count() << "sec" << '\n';
    OutputFile << std::chrono::duration<double, std::milli>(t_end-t_start).count() << "sec" << '\n';

    OutputFile.close();

    return 0;
}

bool compare(std::pair<int, int> x, std::pair<int, int> y)
{
	return x.first < y.first;
}

std::vector<std::array<int, 4>> triad_func(int init, int last, int *CSR_row, int *CSR_col, int *CSR_sign, int nodes)
{
    std::vector<std::array<int, 4>> dataOfNodes(nodes, {0, 0, 0, 0});

    // CSR로 처리할 때

    // i는 인접 행렬에서의 행렬 값 (from)
    // CSR_row.at(i) 는 해당 행에 존재히는 원소 개수. 동시에 해당 행의 col을
    // 읽어들이는 시작 위치 col.at(j) 는 direction 도착 위치(to). 그리고 triad를
    // 찾기 위한 시작점 item.at(j) 는 i -> col.at(j) 의 sign 값
    for (int i = init; i < last; i++) // row의 마지막 원소는 총 edge 개수이므로, size - 1
    {
        int A = i;

        int Ath = CSR_row[A]; // CSR에서 A 노드가 시작하는 인덱스

        if (Ath == -1) // A 노드로 부터 시작하는 연결이 없다면
            continue;  // 다음 번호의 노드 탐색

        int Athp = CSR_row[A + 1]; // CSR에서 A 노드가 끝나는 지점 + 1 인덱스

        int Ap = 1;

        while (Athp == -1)              // A 다음 번호의 노드가 연결이 출발하지 않는 노드라면,
                                        // 출발하는 노드일 때 까지
            Athp = CSR_row[A + (++Ap)]; // 다음 번호의 노드로 이동

        for (int j = Ath; j < Athp; j++) // col에서 A 노드의 도착지점에 해당하는 인덱스들 탐색
        {
            int B = CSR_col[j]; // A에서 도착한 노드를 B라고 지정

            int _c = CSR_sign[j];

            int Bth = CSR_row[B];

            if (Bth == -1)
                continue;

            int Bthp = CSR_row[B + 1];

            int Bp = 1;

            while (Bthp == -1)
                Bthp = CSR_row[B + (++Bp)];

            for (int k = Bth; k < Bthp; k++)
            {
                int C = CSR_col[k];

                int _a = CSR_sign[k];

                if ((C == A) || (B == C) || (A == B))
                {
                    continue;
                }

                for (int l = Ath; l < Athp; l++)
                {
                    int D = CSR_col[l];

                    if (D == C)
                    {
                        int _b = CSR_sign[l];

                        std::pair<int, int> pair_AB = std::make_pair(A, B * _c);
                        std::pair<int, int> pair_BC = std::make_pair(B, C * _a);
                        std::pair<int, int> pair_AC = std::make_pair(A, C * _b);

                        std::array<std::pair<int, int>, 3> vertices{pair_AB, pair_BC, pair_AC};
                        std::sort(vertices.begin(), vertices.end(), compare);

                        int check = _c + _a + _b;

                        switch (check)
                        {
                        case 3:
                            dataOfNodes[A][3]++;
                            dataOfNodes[B][3]++;
                            dataOfNodes[C][3]++;
                            break;

                        case 1:
                            dataOfNodes[A][2]++;
                            dataOfNodes[B][2]++ ;
                            dataOfNodes[C][2]++;
                            break;

                        case -1:
                            dataOfNodes[A][1]++;
                            dataOfNodes[B][1]++;
                            dataOfNodes[C][1]++;
                            break;

                        case -3:
                            dataOfNodes[A][0]++;
                            dataOfNodes[B][0]++;
                            dataOfNodes[C][0]++;
                            break;

                        default:
                            break;
                        }
                    }
                }

                int Cth = CSR_row[C];

                if (Cth == -1)
                    continue;

                int Cthp = CSR_row[C + 1];

                int Cp = 1;

                while (Cthp == -1)
                    Cthp = CSR_row[C + (++Cp)];

                for (int l = Cth; l < Cthp; l++)
                {
                    int D = CSR_col[l];

                    if (D == A)
                    {
                        int _b = CSR_sign[l];

                        std::pair<int, int> pair_AB = std::make_pair(A, B * _c);
                        std::pair<int, int> pair_BC = std::make_pair(B, C * _a);
                        std::pair<int, int> pair_CA = std::make_pair(C, A * _b);

                        std::array<std::pair<int, int>, 3> vertices{pair_AB, pair_BC, pair_CA};
                        std::sort(vertices.begin(), vertices.end(), compare);

                        int check = _c + _a + _b;

                        switch (check)
                        {
                        case 3:
                            dataOfNodes[A][3]++;
                            dataOfNodes[B][3]++;
                            dataOfNodes[C][3]++;
                            break;

                        case 1:
                            dataOfNodes[A][2]++;
                            dataOfNodes[B][2]++;
                            dataOfNodes[C][2]++;
                            break;

                        case -1:
                            dataOfNodes[A][1]++;
                            dataOfNodes[B][1]++;
                            dataOfNodes[C][1]++;
                            break;

                        case -3:
                            dataOfNodes[A][0]++;
                            dataOfNodes[B][0]++;
                            dataOfNodes[C][0]++;
                            break;

                        default:
                            break;
                        }
                    }
                }
            }
        }
    }

    return dataOfNodes;
}