#pragma GCC optimize("O3,unroll-loops")
#pragma GCC target("bmi,bmi2,lzcnt,popcnt")

#include <bits/stdc++.h>

using namespace std;
using namespace chrono;

const int CHECKPOINT_1 = 16;
const int CHECKPOINT_2 = 64;

mt19937 rng(high_resolution_clock::now().time_since_epoch().count());

char rowNames[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I'};
char colNames[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'};

int board[9][9];
int block[9][9];
vector<pair<int, int>> tilesInBlock[9];

int rowMask[9], colMask[9], blockMask[9];

void init();

void parseBoard(const string& str);
void print();

bool backtrack(const int& t, vector<pair<int, int>>& emptyTiles, int& solutionsCount, const int& solutionsCountUntilHalt);
int getSolutionsCount(const int& solutionsCountUntilHalt);

int main()
{
    ios::sync_with_stdio(false);

    init();

    string input;
    cin >> input;

    if (input != "Start")
    {
        int i = input[0] - 'A', j = input[1] - 'a', value = input[2] - '1';
        board[i][j] = value;
        rowMask[i] ^= 1 << board[i][j];
        colMask[j] ^= 1 << board[i][j];
        blockMask[block[i][j]] ^= 1 << board[i][j];
        if (input.back() == '!')
            return 0;
    }

    vector<pair<int, int>> validTiles;
    vector<int> validValues;

    for (int turn = (input == "Start" ? 1 : 2); turn <= 81; turn += 2)
    {
        validTiles.clear();
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                if (board[i][j] == -1)
                    validTiles.emplace_back(i, j);

        if (true)
            shuffle(validTiles.begin(), validTiles.end(), rng);
        else
            sort(validTiles.begin(), validTiles.end(), [&](const pair<int, int>& p1, const pair<int, int>& p2) -> bool {
                return __builtin_popcount(rowMask[p1.first] & colMask[p1.second] & blockMask[block[p1.first][p1.second]])
                       > __builtin_popcount(rowMask[p2.first] & colMask[p2.second] & blockMask[block[p2.first][p2.second]]);
            });

        for (const auto& [i, j] : validTiles)
        {
            int candidateMask = rowMask[i] & colMask[j] & blockMask[block[i][j]];

            if (!candidateMask) continue;

            validValues.clear();
            while (candidateMask)
            {
                validValues.emplace_back(__builtin_ctz(candidateMask));
                candidateMask ^= 1 << validValues.back();
            }

            shuffle(validValues.begin(), validValues.end(), rng);

            bool madeMove = false;

            for (auto &value : validValues)
            {
                board[i][j] = value;
                rowMask[i] ^= 1 << board[i][j];
                colMask[j] ^= 1 << board[i][j];
                blockMask[block[i][j]] ^= 1 << board[i][j];

                int cnt = getSolutionsCount(turn <= CHECKPOINT_1 ? 1 : 2);

                if (cnt == 1 && turn > CHECKPOINT_1)
                {
                    cout << rowNames[i] << colNames[j] << value + 1 << '!' << endl;
                    return 0;
                }

                if (cnt)
                {
                    bool badMove = false;

                    if (turn > CHECKPOINT_2)
                        for (const auto& [_i, _j] : validTiles)
                            if (_i != i || _j != j)
                            {
                                int _candidateMask = rowMask[_i] & colMask[_j] & blockMask[block[_i][_j]];

                                if (!_candidateMask) continue;

                                validValues.clear();
                                while (_candidateMask)
                                {
                                    int _value = __builtin_ctz(_candidateMask);

                                    board[i][j] = value;
                                    rowMask[i] ^= 1 << board[i][j];
                                    colMask[j] ^= 1 << board[i][j];
                                    blockMask[block[i][j]] ^= 1 << board[i][j];

                                    if (getSolutionsCount(2) == 1)
                                    {
                                        badMove = true;
                                        break;
                                    }

                                    rowMask[i] ^= 1 << board[i][j];
                                    colMask[j] ^= 1 << board[i][j];
                                    blockMask[block[i][j]] ^= 1 << board[i][j];
                                    board[i][j] = -1;

                                    _candidateMask ^= 1 << _value;
                                }

                                if (badMove) break;
                            }

                    if (!badMove)
                    {
                        cout << rowNames[i] << colNames[j] << value + 1 << endl;
                        madeMove = true;
                        break;
                    }
                }

                rowMask[i] ^= 1 << board[i][j];
                colMask[j] ^= 1 << board[i][j];
                blockMask[block[i][j]] ^= 1 << board[i][j];
                board[i][j] = -1;

                candidateMask ^= 1 << value;
            }

            if (madeMove) break;
        }

        cin >> input;
        int i = input[0] - 'A', j = input[1] - 'a', value = input[2] - '1';
        board[i][j] = value;
        rowMask[i] ^= 1 << board[i][j];
        colMask[j] ^= 1 << board[i][j];
        blockMask[block[i][j]] ^= 1 << board[i][j];
        if (input.back() == '!')
            return 0;
    }
}

void init()
{
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
        {
            block[i][j] = i / 3 * 3 + j / 3;
            tilesInBlock[block[i][j]].emplace_back(i, j);
            board[i][j] = -1;
        }
    for (int t = 0; t < 9; t++)
        rowMask[t] = colMask[t] = blockMask[t] = (1 << 9) - 1;
}

void parseBoard(const string& str)
{
    for (int t = 0; t < 9; t++)
        rowMask[t] = colMask[t] = blockMask[t] = (1 << 9) - 1;
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            if ('1' <= str[i * 9 + j] && str[i * 9 + j] <= '9')
            {
                board[i][j] = str[i * 9 + j] - '1';
                rowMask[i] ^= 1 << board[i][j];
                colMask[j] ^= 1 << board[i][j];
                blockMask[block[i][j]] ^= 1 << board[i][j];
            }
}

void print()
{
    vector<string> result(35, string(35, ' '));

    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
        {
            int x = 4 * i, y = 4 * j;

            if (board[i][j] != -1)
            {
                for (int dx = 0; dx < 3; dx++)
                    for (int dy = 0; dy < 3; dy++)
                        result[x + dx][y + dy] = '*';
                result[x + 1][y + 1] = (char)('1' + board[i][j]);
            }
            else
            {
                for (int dx = 0; dx < 3; dx++)
                    for (int dy = 0; dy < 3; dy++)
                        if (rowMask[i] & colMask[j] & blockMask[block[i][j]] & (1 << (dx * 3 + dy)))
                            result[x + dx][y + dy] = (char)('1' + dx * 3 + dy);
            }
        }

    cout << "BEGIN OF BOARD\n";
    for (auto &row : result)
    {
        for (auto &chr : row)
            cout << chr << ' ';
        cout << '\n';
    }
    cout << "END OF BOARD\n" << endl;
}

bool backtrack(const int& t, vector<pair<int, int>>& emptyTiles, int& solutionsCount, const int& solutionsCountUntilHalt)
{
    if (solutionsCount == solutionsCountUntilHalt) return false;
    if (t == emptyTiles.size()) return ++solutionsCount < solutionsCountUntilHalt;

    nth_element(emptyTiles.begin() + t, emptyTiles.begin() + t, emptyTiles.end(), [&](const pair<int, int>& p1, const pair<int, int>& p2) -> bool {
        return __builtin_popcount(rowMask[p1.first] & colMask[p1.second] & blockMask[block[p1.first][p1.second]])
               < __builtin_popcount(rowMask[p2.first] & colMask[p2.second] & blockMask[block[p2.first][p2.second]]);
    });

    int i = emptyTiles[t].first, j = emptyTiles[t].second;

    int candidateMask = rowMask[i] & colMask[j] & blockMask[block[i][j]];

    while (candidateMask)
    {
        int value = __builtin_ctz(candidateMask);

        board[i][j] = value;
        rowMask[i] ^= 1 << board[i][j];
        colMask[j] ^= 1 << board[i][j];
        blockMask[block[i][j]] ^= 1 << board[i][j];

        bool toBeContinued = backtrack(t + 1, emptyTiles, solutionsCount, solutionsCountUntilHalt);

        rowMask[i] ^= 1 << board[i][j];
        colMask[j] ^= 1 << board[i][j];
        blockMask[block[i][j]] ^= 1 << board[i][j];
        board[i][j] = -1;

        if (!toBeContinued) return false;

        candidateMask ^= 1 << value;
    }

    return true;
}

int getSolutionsCount(const int& solutionsCountUntilHalt)
{
    vector<pair<int, int>> emptyTiles;
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            if (board[i][j] == -1)
                emptyTiles.emplace_back(i, j);

    int solutionsCount = 0;
    backtrack(0, emptyTiles, solutionsCount, solutionsCountUntilHalt);

    return solutionsCount;
}