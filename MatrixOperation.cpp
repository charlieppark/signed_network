#include "MatrixOperation.h"

int **transpose(int **A, int row, int col)
{
    int **AT = new int *[col];

    if (AT == NULL)
    {
        std::cout << "MALLOC ERROR - AT";
        exit(1);
    }

    for (int i = 0; i < col; i++)
    {
        AT[i] = new int[row];

        if (AT[i] == NULL)
        {
            std::cout << "MALLOC ERROR - AT[i]";
            exit(1);
        }
    }

    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            AT[j][i] = A[i][j];
        }
        delete A[i];
    }

    delete A;

    return AT;
}