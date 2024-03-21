#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <array>
#include <utility>

typedef struct _given_data
{
    const std::string nameOfData;
    const int linesOfMetaData;
    const int givenNodes;
    const int givenEdges;
    const std::string fileInPath;
    int positiveEdges;
    int negativeEdges;
    int foundEdges;
} GivenData;

typedef struct _triads
{
    std::set<std::array<std::pair<int, int>, 3>> t3;
    std::set<std::array<std::pair<int, int>, 3>> t2;
    std::set<std::array<std::pair<int, int>, 3>> t1;
    std::set<std::array<std::pair<int, int>, 3>> t0;
} Triads;