#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// structure representing one of the 81 squares in a sudoku puzzle
typedef struct Square
{
    // value is the number in the square, or 0 if the value is still undetermined
    // i,j,k indicates position of the square
    int value, i, j, k;
    // poss[0] indicates the number of possibilities for the square if value is undetermined,
    // and poss[1]-poss[9] indicate whether the square could have the given value
    int poss[10];
} square;

// print the current state of the puzzle
void print_puzzle(square sudoku[9][3][3])
{
    for (int i = 0; i < 9; i++)
    {
        if ((i % 3) == 0)
        {
            printf("\n");
        }
        for (int j = 0; j < 3; j++)
        {
            printf("[%d %d %d] ", sudoku[i][j][0].value, sudoku[i][j][1].value, sudoku[i][j][2].value);
        }
        printf("\n");
    }
    printf("\n");
}

// indicate whether specified value val has already been used in the same row, col, or box
// as the given square unsolved; returns 1 if it has been used and 0 otherwise
int invalid(square unsolved, int val, int rows[9][10], int cols[9][10], int boxes[9][10])
{
    int i = unsolved.i;
    int j = unsolved.j;
    int k = unsolved.k;
    // check row
    if (rows[i][val])
    {
        return 1;
    }
    // check col
    if (cols[(3 * j) + k][val])
    {
        return 1;
    }
    // check box
    if (boxes[(3 * (i / 3)) + j][val])
    {
        return 1;
    }
    return 0;
}

// update rows, cols, and boxes when a square is filled in
void update_square(int i, int j, int k, int val, int rows[9][10], int cols[9][10], int boxes[9][10])
{
    rows[i][val] = 1;
    rows[i][0] += 1;
    cols[(3 * j) + k][val] = 1;
    cols[(3 * j) + k][0] += 1;
    boxes[(3 * (i / 3)) + j][val] = 1;
    boxes[(3 * (i / 3)) + j][0] += 1;
}

// update a given unsolved square, return 0 if still unsolved and 1 if solved
int update_unsolved(square *unsolved, int rows[9][10], int cols[9][10], int boxes[9][10])
{
    int i = unsolved->i;
    int j = unsolved->j;
    int k = unsolved->k;
    int *poss = unsolved->poss;
    // update the square's poss array based on which digits have already been used in the same row, col, or box
    for (int x = 1; x < 10; x++)
    {
        if (poss[x] > 0)
        {
            if (rows[i][x] || cols[(3 * j) + k][x] || boxes[(3 * (i / 3)) + j][x])
            {
                poss[x] = 0;
                poss[0]--;
            }
        }
    }
    // if only one possibility is left, fill in the square and return 1
    if (poss[0] == 1)
    {
        for (int x = 1; x < 10; x++)
        {
            if (poss[x])
            {
                unsolved->value = x;
                update_square(i, j, k, x, rows, cols, boxes);
                return 1;
            }
        }
    }
    // if multiple possibilities remain, return 0
    return 0;
}

// find all unsolved squares in a given row, col, or box and add them to the given array
// dim must be 'r' for row, 'c' for col, or 'b' for box, and num is the row/col/box number
void find_unsolved(char dim, int num, square sudoku[9][3][3], square *list_unsolved[9])
{
    int count = 0;
    if (dim == 'r')
    {
        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                square *curr_square = &sudoku[num][j][k];
                if (curr_square->value == 0)
                {
                    list_unsolved[count] = curr_square;
                    count++;
                }
            }
        }
    }
    else if (dim == 'c')
    {
        int j = num / 3;
        int k = num % 3;
        for (int i = 0; i < 9; i++)
        {
            square *curr_square = &sudoku[i][j][k];
            if (curr_square->value == 0)
            {
                list_unsolved[count] = curr_square;
                count++;
            }
        }
    }
    else if (dim == 'b')
    {
        int x = num / 3;
        int j = num % 3;
        for (int i = (3 * x); i < ((3 * x) + 3); i++)
        {
            for (int k = 0; k < 3; k++)
            {
                square *curr_square = &sudoku[i][j][k];
                if (curr_square->value == 0)
                {
                    list_unsolved[count] = curr_square;
                    count++;
                }
            }
        }
    }
}

// check for numbers that only have one possible position in a given row, col, or box
// and return the number of squares that were solved during the execution of the function
int check_unique(square *list_unsolved[9], int rows[9][10], int cols[9][10], int boxes[9][10])
{
    int count = 0;
    // iterate through all digits
    for (int x = 1; x < 10; x++)
    {
        square *loc = 0;
        // iterate through all unsolved squares
        for (int index = 0; index < 9; index++)
        {
            square *unsolved = list_unsolved[index];
            if (unsolved == 0)
            {
                break;
            }
            // check if the current square could contain the current digit
            if (unsolved->poss[x] > 0)
            {
                if (invalid(*unsolved, x, rows, cols, boxes))
                {
                    unsolved->poss[x] = 0;
                    unsolved->poss[0]--;
                }
                // if this is the first square that could contain the current digit, mark it
                else if (loc == 0)
                {
                    loc = unsolved;
                }
                // if multiple possible locations are found, move on to the next digit
                else
                {
                    loc = 0;
                    break;
                }
            }
        }
        // if exactly one possible location was found, fill in that square and increment the count
        if (loc)
        {
            loc->value = x;
            update_square(loc->i, loc->j, loc->k, x, rows, cols, boxes);
            count++;
        }
    }
    // return the number of squares that were solved within this function call
    return count;
}

// check for pairs that must go in a certain pair of boxes, return 1 if any pairs found and 0 otherwise
int check_pairs(square sudoku[9][3][3])
{
    int progress_made = 0;
    // check every pair of two distinct digits
    for (int x = 2; x < 10; x++)
    {
        for (int y = 1; y < x; y++)
        {
            // check for pairs in each row
            for (int i = 0; i < 9; i++)
            {
                square *poss[2] = {0};
                int count = 0;
                for (int j = 0; j < 3; j++)
                {
                    if (count == 5)
                    {
                        break;
                    }
                    for (int k = 0; k < 3; k++)
                    {
                        square *curr_square = &sudoku[i][j][k];
                        if ((curr_square->value == 0) && (curr_square->poss[x] || curr_square->poss[y]))
                        {
                            if (count > 1)
                            {
                                count++;
                                break;
                            }
                            poss[count] = curr_square;
                            count++;
                        }
                        else if ((curr_square->value == x) || (curr_square->value == y))
                        {
                            count = 5;
                            break;
                        }
                    }
                }
                if (count == 2)
                {
                    // if pair found, neither square can have any other value
                    for (int q = 1; q < 10; q++)
                    {
                        for (int w = 0; w < 2; w++)
                        {
                            if ((poss[w]->poss[q] > 0) && (q != x) && (q != y))
                            {
                                progress_made = 1;
                                poss[w]->poss[q] = 0;
                                poss[w]->poss[0]--;
                            }
                        }
                    }
                    // if pair are both in same box, no other square in that box can have either value
                    if (poss[0]->j == poss[1]->j)
                    {
                        int j = poss[0]->j;
                        for (int i1 = (3 * (i / 3)); i1 < ((3 * (i / 3)) + 3); i1++)
                        {
                            if (i != i1)
                            {
                                for (int k = 0; k < 3; k++)
                                {
                                    square *curr_square = &sudoku[i1][j][k];
                                    if (curr_square->value == 0)
                                    {
                                        if (curr_square->poss[x] > 0)
                                        {
                                            curr_square->poss[x] = 0;
                                            curr_square->poss[0]--;
                                            progress_made = 1;
                                        }
                                        if (curr_square->poss[y] > 0)
                                        {
                                            curr_square->poss[y] = 0;
                                            curr_square->poss[0]--;
                                            progress_made = 1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            // check for pairs in each col
            for (int j = 0; j < 3; j++)
            {
                for (int k = 0; k < 3; k++)
                {
                    square *poss[2] = {0};
                    int count = 0;
                    for (int i = 0; i < 9; i++)
                    {
                        square *curr_square = &sudoku[i][j][k];
                        if ((curr_square->value == 0) && (curr_square->poss[x] || curr_square->poss[y]))
                        {
                            if (count > 1)
                            {
                                count++;
                                break;
                            }
                            poss[count] = curr_square;
                            count++;
                        }
                        else if ((curr_square->value == x) || (curr_square->value == y))
                        {
                            count = 5;
                            break;
                        }
                    }

                    if (count == 2)
                    {
                        // if pair found, neither square can have any other value
                        for (int q = 1; q < 10; q++)
                        {
                            for (int w = 0; w < 2; w++)
                            {
                                if ((poss[w]->poss[q] > 0) && (q != x) && (q != y))
                                {
                                    progress_made = 1;
                                    poss[w]->poss[q] = 0;
                                    poss[w]->poss[0]--;
                                }
                            }
                        }
                        // if pair are both in same box, no other square in that box can have either value
                        if ((poss[0]->i) / 3 == (poss[1]->i) / 3)
                        {
                            int i = poss[0]->i;
                            for (int i1 = (3 * (i / 3)); i1 < ((3 * (i / 3)) + 3); i1++)
                            {
                                for (int k1 = 0; k1 < 3; k1++)
                                {
                                    if (k != k1)
                                    {
                                        square *curr_square = &sudoku[i1][j][k1];
                                        if (curr_square->value == 0)
                                        {
                                            if (curr_square->poss[x] > 0)
                                            {
                                                curr_square->poss[x] = 0;
                                                curr_square->poss[0]--;
                                                progress_made = 1;
                                            }
                                            if (curr_square->poss[y] > 0)
                                            {
                                                curr_square->poss[y] = 0;
                                                curr_square->poss[0]--;
                                                progress_made = 1;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            // check for pairs in each box
            for (int i1 = 0; i1 < 3; i1++)
            {
                for (int j = 0; j < 3; j++)
                {
                    square *poss[2] = {0};
                    int count = 0;
                    for (int i2 = 0; i2 < 3; i2++)
                    {
                        if (count == 5)
                        {
                            break;
                        }
                        for (int k = 0; k < 3; k++)
                        {
                            square *curr_square = &sudoku[(3 * i1) + i2][j][k];
                            if ((curr_square->value == 0) && (curr_square->poss[x] || curr_square->poss[y]))
                            {
                                if (count > 1)
                                {
                                    count++;
                                    break;
                                }
                                poss[count] = curr_square;
                                count++;
                            }
                            else if ((curr_square->value == x) || (curr_square->value == y))
                            {
                                count = 5;
                                break;
                            }
                        }
                    }
                    if (count == 2)
                    {
                        // if pair found, neither square can have any other value
                        for (int q = 1; q < 10; q++)
                        {
                            for (int w = 0; w < 2; w++)
                            {
                                if ((poss[w]->poss[q] > 0) && (q != x) && (q != y))
                                {
                                    progress_made = 1;
                                    poss[w]->poss[q] = 0;
                                    poss[w]->poss[0]--;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return progress_made;
}

// check for triples that must go in a certain triple of boxes, return 1 if any triples found and 0 otherwise
int check_triples(square sudoku[9][3][3])
{
    int progress_made = 0;
    // check every triple of three distinct digits
    for (int x = 3; x < 10; x++)
    {
        for (int y = 2; y < x; y++)
        {
            for (int z = 1; z < y; z++)
            {
                // check for triples in each row
                for (int i = 0; i < 9; i++)
                {
                    square *poss[3] = {0};
                    int count = 0;
                    for (int j = 0; j < 3; j++)
                    {
                        if (count == 5)
                        {
                            break;
                        }
                        for (int k = 0; k < 3; k++)
                        {
                            square *curr_square = &sudoku[i][j][k];
                            if ((curr_square->value == 0) && (curr_square->poss[x] || curr_square->poss[y] || curr_square->poss[z]))
                            {
                                if (count > 2)
                                {
                                    count++;
                                    break;
                                }
                                poss[count] = curr_square;
                                count++;
                            }
                            else if ((curr_square->value == x) || (curr_square->value == y) || (curr_square->value == z))
                            {
                                count = 5;
                                break;
                            }
                        }
                    }
                    if (count == 3)
                    {
                        // if triple found, none of the squares can have any other value
                        for (int q = 1; q < 10; q++)
                        {
                            for (int w = 0; w < 3; w++)
                            {
                                if ((poss[w]->poss[q] > 0) && (q != x) && (q != y) && (q != z))
                                {
                                    progress_made = 1;
                                    poss[w]->poss[q] = 0;
                                    poss[w]->poss[0]--;
                                }
                            }
                        }
                        // if triple are all in same box, no other square in that box can have any of the three values
                        int j = poss[0]->j;
                        if ((j == poss[1]->j) && (j == poss[2]->j))
                        {
                            for (int i1 = (3 * (i / 3)); i1 < ((3 * (i / 3)) + 3); i1++)
                            {
                                if (i != i1)
                                {
                                    for (int k = 0; k < 3; k++)
                                    {
                                        square *curr_square = &sudoku[i1][j][k];
                                        if (curr_square->value == 0)
                                        {
                                            if (curr_square->poss[x] > 0)
                                            {
                                                curr_square->poss[x] = 0;
                                                curr_square->poss[0]--;
                                                progress_made = 1;
                                            }
                                            if (curr_square->poss[y] > 0)
                                            {
                                                curr_square->poss[y] = 0;
                                                curr_square->poss[0]--;
                                                progress_made = 1;
                                            }
                                            if (curr_square->poss[z] > 0)
                                            {
                                                curr_square->poss[z] = 0;
                                                curr_square->poss[0]--;
                                                progress_made = 1;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                // check for triples in each col
                for (int j = 0; j < 3; j++)
                {
                    for (int k = 0; k < 3; k++)
                    {
                        square *poss[3] = {0};
                        int count = 0;
                        for (int i = 0; i < 9; i++)
                        {
                            square *curr_square = &sudoku[i][j][k];
                            if ((curr_square->value == 0) && (curr_square->poss[x] || curr_square->poss[y] || curr_square->poss[z]))
                            {
                                if (count > 2)
                                {
                                    count++;
                                    break;
                                }
                                poss[count] = curr_square;
                                count++;
                            }
                            else if ((curr_square->value == x) || (curr_square->value == y) || (curr_square->value == z))
                            {
                                count = 5;
                                break;
                            }
                        }

                        if (count == 3)
                        {
                            // if triple found, none of the three squares can have any other value
                            for (int q = 1; q < 10; q++)
                            {
                                for (int w = 0; w < 3; w++)
                                {
                                    if ((poss[w]->poss[q] > 0) && (q != x) && (q != y) && (q != z))
                                    {
                                        progress_made = 1;
                                        poss[w]->poss[q] = 0;
                                        poss[w]->poss[0]--;
                                    }
                                }
                            }
                            // if triple are all in same box, no other square in that box can have any of the three values
                            int i = poss[0]->i;
                            if ((i / 3 == (poss[1]->i) / 3) && (i / 3 == (poss[2]->i) / 3))
                            {
                                for (int i1 = (3 * (i / 3)); i1 < ((3 * (i / 3)) + 3); i1++)
                                {
                                    for (int k1 = 0; k1 < 3; k1++)
                                    {
                                        if (k != k1)
                                        {
                                            square *curr_square = &sudoku[i1][j][k1];
                                            if (curr_square->value == 0)
                                            {
                                                if (curr_square->poss[x] > 0)
                                                {
                                                    curr_square->poss[x] = 0;
                                                    curr_square->poss[0]--;
                                                    progress_made = 1;
                                                }
                                                if (curr_square->poss[y] > 0)
                                                {
                                                    curr_square->poss[y] = 0;
                                                    curr_square->poss[0]--;
                                                    progress_made = 1;
                                                }
                                                if (curr_square->poss[z] > 0)
                                                {
                                                    curr_square->poss[z] = 0;
                                                    curr_square->poss[0]--;
                                                    progress_made = 1;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                // check for triples in each box
                for (int i1 = 0; i1 < 3; i1++)
                {
                    for (int j = 0; j < 3; j++)
                    {
                        square *poss[3] = {0};
                        int count = 0;
                        for (int i2 = 0; i2 < 3; i2++)
                        {
                            if (count == 5)
                            {
                                break;
                            }
                            for (int k = 0; k < 3; k++)
                            {
                                square *curr_square = &sudoku[(3 * i1) + i2][j][k];
                                if ((curr_square->value == 0) && (curr_square->poss[x] || curr_square->poss[y] || curr_square->poss[z]))
                                {
                                    if (count > 2)
                                    {
                                        count++;
                                        break;
                                    }
                                    poss[count] = curr_square;
                                    count++;
                                }
                                else if ((curr_square->value == x) || (curr_square->value == y) || (curr_square->value == z))
                                {
                                    count = 5;
                                    break;
                                }
                            }
                        }
                        if (count == 3)
                        {
                            // if triple found, none of the three squares can have any other value
                            for (int q = 1; q < 10; q++)
                            {
                                for (int w = 0; w < 3; w++)
                                {
                                    if ((poss[w]->poss[q] > 0) && (q != x) && (q != y) && (q != z))
                                    {
                                        progress_made = 1;
                                        poss[w]->poss[q] = 0;
                                        poss[w]->poss[0]--;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return progress_made;
}

// solve the puzzle
void solve_puzzle()
{
    char sudoku_str[82] = {0};
    square sudoku[9][3][3] = {0};

    // get puzzle as user input
    printf("\nPlease enter a Sudoku puzzle as a string, one row at a time, with a 0 to represent each blank space.\n");
    printf("Each line must include exactly 9 numeric characters and no other characters.\n\n");

    for (int i = 0; i < 9; i++)
    {
        printf("Enter row %d: ", i + 1);
        scanf("%9s", (sudoku_str + (9 * i)));
        fflush(stdin);
        if (strlen(sudoku_str) != (9 * (i + 1)))
        {
            printf("\nError: each line must contain exactly 9 characters.\n");
            return;
        }
        for (int j = (9 * i); j < (9 * (i + 1)); j++)
        {
            if ((sudoku_str[j] < 48) || (sudoku_str[j] > 57))
            {
                printf("\nError: each line must only contain numeric characters.\n");
                return;
            }
        }
    }
    printf("\n");

    // hardcoded example puzzle for testing
    // strcpy(sudoku_str, "090032040000000000700590306000000053008020400130000000902073008000000000050940060");

    // start timer
    clock_t start_time = clock();

    // convert input to matrix
    for (int i = 0; i < 81; i++)
    {
        int row = i / 9;
        int col = i % 9;
        int col1 = col / 3;
        int col2 = col % 3;
        sudoku[row][col1][col2].value = (int)sudoku_str[i] - 48;
        sudoku[row][col1][col2].i = row;
        sudoku[row][col1][col2].j = col1;
        sudoku[row][col1][col2].k = col2;
    }

    // print original puzzle
    printf("Original puzzle:");
    print_puzzle(sudoku);

    // create lists to indicate what has been solved already in each row, column, and box
    // within these lists, each nested list indicates a particular row, column, or box in the puzzle
    // the element in position 0 of a nested list counts the number of solved positions within the row/column/box
    // the elements is positions 1-9 of a nested list indicate whether the corresponding number has been placed yet
    int rows[9][10] = {0};
    int cols[9][10] = {0};
    int boxes[9][10] = {0};

    // fill in rows, cols, and boxes based on original puzzle
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                int val = sudoku[i][j][k].value;
                if (val)
                {
                    update_square(i, j, k, val, rows, cols, boxes);
                }
            }
        }
    }

    // count unsolved squares and update poss for all unsolved squares
    int count_unsolved = 0;
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                if (sudoku[i][j][k].value == 0)
                {
                    // determine which values are possible
                    int *poss = sudoku[i][j][k].poss;
                    for (int x = 1; x < 10; x++)
                    {
                        if ((rows[i][x] == 0) && (cols[(3 * j) + k][x] == 0) && (boxes[(3 * (i / 3)) + j][x] == 0))
                        {
                            poss[x] = 1;
                            poss[0] += 1;
                        }
                    }
                    // if there is only one possibility, fill in the square's value
                    if (poss[0] == 1)
                    {
                        for (int x = 1; x < 10; x++)
                        {
                            if (poss[x])
                            {
                                sudoku[i][j][k].value = x;
                                update_square(i, j, k, x, rows, cols, boxes);
                                break;
                            }
                        }
                    }
                    // otherwise count the square as unsolved
                    else
                    {
                        count_unsolved++;
                    }
                }
            }
        }
    }

    // solve the puzzle
    while (count_unsolved)
    {
        int prev_count = -1;
        while (count_unsolved && (count_unsolved != prev_count))
        {
            prev_count = count_unsolved;
            // update possibilities for all unsolved squares
            for (int i = 0; i < 9; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    for (int k = 0; k < 3; k++)
                    {
                        square *curr_square = &sudoku[i][j][k];
                        if (curr_square->value == 0)
                        {
                            count_unsolved -= update_unsolved(curr_square, rows, cols, boxes);
                        }
                    }
                }
            }
            // check for numbers which have only one possible position in any row, col, or box
            for (int num = 0; num < 9; num++)
            {
                // check rows
                square *row_unsolved[9] = {0};
                find_unsolved('r', num, sudoku, row_unsolved);
                count_unsolved -= check_unique(row_unsolved, rows, cols, boxes);
                // check cols
                square *col_unsolved[9] = {0};
                find_unsolved('c', num, sudoku, col_unsolved);
                count_unsolved -= check_unique(col_unsolved, rows, cols, boxes);
                // check boxes
                square *box_unsolved[9] = {0};
                find_unsolved('b', num, sudoku, box_unsolved);
                count_unsolved -= check_unique(box_unsolved, rows, cols, boxes);
            }
        }
        if (count_unsolved == 0)
        {
            break;
        }
        // if methods above can't solve puzzle, check for pairs
        if (check_pairs(sudoku) == 0)
        {
            // if checking pairs does not make progress, check for triples
            if (check_triples(sudoku) == 0)
            {
                // if none of the above methods made any progress, this program is unable to solve the puzzle
                break;
            }
        }
    }

    // print solution
    if (count_unsolved)
    {
        printf("Unable to solve puzzle. Partial solution:");
    }
    else
    {
        printf("Solution:");
    }
    print_puzzle(sudoku);

    // stop timer and print time
    clock_t stop_time = clock();
    printf("Execution time: %f seconds\n\n", (double)(stop_time - start_time) / CLOCKS_PER_SEC);
}

// check if user wants to continue, return 1 if yes and 0 if no
int check_continue()
{
    while (1)
    {
        char user_input[2] = {0};
        printf("Would you like to enter another puzzle? Enter 'y' or 'n'.\n");
        scanf("%1s", user_input);
        fflush(stdin);
        if ((user_input[0] == 'y') || (user_input[0] == 'Y'))
        {
            return 1;
        }
        if ((user_input[0] == 'n') || (user_input[0] == 'N'))
        {
            return 0;
        }
        printf("Invalid input\n");
    }
}

int main()
{
    int keep_playing = 1;
    char user_input[2] = {0};
    while (keep_playing)
    {
        solve_puzzle();
        keep_playing = check_continue();
    }
}