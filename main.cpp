#pragma GCC optimize("O3,unroll-loops")

#include <bits/stdc++.h>

using namespace std;
using namespace chrono;

const int CHECKPOINT_1 = 16;
const int CHECKPOINT_2 = 32;
const int TIME_LIMIT_PER_MOVE = 1500;

mt19937 rng(high_resolution_clock::now().time_since_epoch().count());

char rowNames[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I'};
char colNames[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'};

vector<int> values = {7, 5, 3, 1, 8, 6, 4, 2, 0};

int board[9][9];
int block[9][9];
vector<pair<int, int>> tilesOfBlock[9];

int rowMask[9], colMask[9], blockMask[9];

vector<tuple<int, int, int, int>> uniqueRectanglesUtil;

void init();

void parseBoard(const string& str);
void print();

void makeMove(const string& move);
void makeMove(const int& i, const int& j, const int& value);
void undoMove(const int& i, const int& j);

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
        makeMove(input);
        if (input.back() == '!') return 0;
    }

    vector<pair<int, int>> validTiles;

    for (int turn = (input == "Start" ? 1 : 2); turn <= 81; turn += 2)
    {
        const auto begin = high_resolution_clock::now();
        bool timeLimitExceeded = false;

        validTiles.clear();
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                if (board[i][j] == -1)
                    validTiles.emplace_back(i, j);

        if (turn <= CHECKPOINT_1)
            shuffle(validTiles.begin(), validTiles.end(), rng);
        else
            sort(validTiles.begin(), validTiles.end(), [&](const pair<int, int>& p1, const pair<int, int>& p2) -> bool {
                return __builtin_popcount(rowMask[p1.first] & colMask[p1.second] & blockMask[block[p1.first][p1.second]])
                       > __builtin_popcount(rowMask[p2.first] & colMask[p2.second] & blockMask[block[p2.first][p2.second]]);
            });

        bool madeMove = false;
        string potentialMove;

        for (const auto& [i, j] : validTiles)
        {
            int candidateMask = rowMask[i] & colMask[j] & blockMask[block[i][j]];

            for (auto &value : values)
            {
                if (!(candidateMask & (1 << value))) continue;

                makeMove(i, j, value);

                int cnt = getSolutionsCount(turn <= CHECKPOINT_1 ? 1 : 2);

                undoMove(i, j);

                if (cnt == 1 && turn > CHECKPOINT_1)
                {
                    cout << rowNames[i] << colNames[j] << value + 1 << '!' << endl;
                    return 0;
                }

                if (cnt && (potentialMove.empty() || rng() % 5 == 0))
                {
                    potentialMove = rowNames[i];
                    potentialMove += colNames[j];
                    potentialMove += '1' + value;

                    if (turn <= CHECKPOINT_1) break;
                }

                if (!potentialMove.empty() && duration_cast<milliseconds>(high_resolution_clock::now() - begin).count() > TIME_LIMIT_PER_MOVE)
                {
                    timeLimitExceeded = true;
                    break;
                }

                candidateMask ^= 1 << value;
            }

            if (madeMove || timeLimitExceeded) break;
        }

        if (!madeMove)
        {
            cout << potentialMove << endl;
            makeMove(potentialMove);
        }

        cin >> input;
        makeMove(input);
        if (input.back() == '!') return 0;
    }
}

void init()
{
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
        {
            board[i][j] = -1;
            block[i][j] = i / 3 * 3 + j / 3;
            tilesOfBlock[block[i][j]].emplace_back(i, j);
        }

    for (int t = 0; t < 9; t++)
        rowMask[t] = colMask[t] = blockMask[t] = (1 << 9) - 1;

    for (int i = 0; i < 9; i++)
        for (int _i = i; _i < 9; _i++)
            if (i / 3 == _i / 3)
                for (int j = 0; j < 9; j++)
                    for (int _j = j; _j < 9; _j++)
                        if (j / 3 != _j / 3)
                            uniqueRectanglesUtil.emplace_back(i, _i, j, _j);
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

void makeMove(const string& move)
{
    int i = move[0] - 'A', j = move[1] - 'a', value = move[2] - '1';
    board[i][j] = value;
    rowMask[i] ^= 1 << board[i][j];
    colMask[j] ^= 1 << board[i][j];
    blockMask[block[i][j]] ^= 1 << board[i][j];
}

void makeMove(const int &i, const int& j, const int& value)
{
    board[i][j] = value;
    rowMask[i] ^= 1 << board[i][j];
    colMask[j] ^= 1 << board[i][j];
    blockMask[block[i][j]] ^= 1 << board[i][j];
}

void undoMove(const int &i, const int& j)
{
    rowMask[i] ^= 1 << board[i][j];
    colMask[j] ^= 1 << board[i][j];
    blockMask[block[i][j]] ^= 1 << board[i][j];
    board[i][j] = -1;
}

bool backtrack(const int& t, vector<pair<int, int>>& emptyTiles, int& solutionsCount, const int& solutionsCountUntilHalt)
{
    if (false && solutionsCount && solutionsCount < solutionsCountUntilHalt)
    {
        // Unique Rectangles
        for (auto [i, _i, j, _j] : uniqueRectanglesUtil)
            if (__builtin_popcount( rowMask[i] & rowMask[_i] & colMask[j] & colMask[_j] & blockMask[block[i][j]] & blockMask[block[_i][_j]] ) >= 2
                || __builtin_popcount( rowMask[j] & rowMask[_j] & colMask[i] & colMask[_i] & blockMask[block[i][j]] & blockMask[block[_i][_j]] ) >= 2)
            {
                // assert(false);
                solutionsCount = solutionsCountUntilHalt;
                break;
            }
    }
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

        makeMove(i, j, value);

        bool toBeContinued = backtrack(t + 1, emptyTiles, solutionsCount, solutionsCountUntilHalt);

        undoMove(i, j);

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