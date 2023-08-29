#include <bits/stdc++.h>

using namespace std;
using namespace chrono;

mt19937 rng(high_resolution_clock::now().time_since_epoch().count());

int board[9][9];
int group[9][9];
vector<pair<int, int>> tilesInGroup[9];

int violations[9][9][9];
int candidatesCount[9][9];
int solutions;

vector<pair<int, int>> moves;

void init();
void reset();

void fill(const int& i, const int& j, const int& value);
void erase(const int& i, const int& j);

void parseBoard(const string& str);
void print();
void rollback(const int& t);
bool backtrack(const int& filled);

int main()
{
    ios::sync_with_stdio(false);

    init();

    // string initBoard(81, '0');
    string initBoard = "008317000004205109000040070327160904901450000045700800030001060872604000416070080";

    parseBoard(initBoard);

    print();

    solutions = 0;
    backtrack(81 - (int)count(initBoard.begin(), initBoard.end(), '0'));
}

void init()
{
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
        {
            board[i][j] = -1;
            group[i][j] = i / 3 * 3 + j / 3;
            tilesInGroup[group[i][j]].emplace_back(i, j);
        }
}

void reset()
{
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
        {
            for (int value = 0; value < 9; value++)
                violations[i][j][value] = 0;
            candidatesCount[i][j] = 9;
        }
}

void fill(const int& i, const int& j, const int& value)
{
    assert(!violations[i][j][value]);

    moves.emplace_back(i, j);

    board[i][j] = value;

    candidatesCount[i][j]--;
    violations[i][j][value]++;

    for (int _j = 0; _j < 9; _j++)
        if (_j != j)
            if (violations[i][_j][value]++ == 0)
                candidatesCount[i][_j]--;

    for (int _i = 0; _i < 9; _i++)
        if (_i != i)
            if (violations[_i][j][value]++ == 0)
                candidatesCount[_i][j]--;

    for (const auto& [_i, _j] : tilesInGroup[group[i][j]])
        if (_i != i && _j != j)
            if (violations[_i][_j][value]++ == 0)
                candidatesCount[_i][_j]--;
}

void erase(const int& i, const int& j)
{
    assert(board[i][j] != -1);

    moves.pop_back();

    int value = board[i][j];

    board[i][j] = -1;

    candidatesCount[i][j]++;
    violations[i][j][value]--;

    for (int _j = 0; _j < 9; _j++)
        if (_j != j)
            if (--violations[i][_j][value] == 0)
                candidatesCount[i][_j]++;

    for (int _i = 0; _i < 9; _i++)
        if (_i != i)
            if (--violations[_i][j][value] == 0)
                candidatesCount[_i][j]++;

    for (const auto& [_i, _j] : tilesInGroup[group[i][j]])
        if (_i != i && _j != j)
            if (--violations[_i][_j][value] == 0)
                candidatesCount[_i][_j]++;
}

void parseBoard(const string& str)
{
    reset();
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            if (str[i * 9 + j] != '0')
                fill(i, j, str[i * 9 + j] - '1');
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
                        if (!violations[i][j][dx * 3 + dy])
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

void rollback(const int& t)
{
    while (moves.size() > t)
    {
        int i = moves.back().first, j = moves.back().second;
        erase(i, j);
    }
}

bool backtrack(const int& filled)
{
    if (solutions == 1) return false;

    if (filled == 81)
    {
        print();
        return ++solutions < 1;
    }

    int singleCandidatesCount = 0;
    int t = (int)moves.size();
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            if (board[i][j] == -1 && candidatesCount[i][j] == 1)
                for (int value = 0; value < 9; value++)
                    if (!violations[i][j][value])
                    {
                        singleCandidatesCount++;
                        fill(i, j, value);
                        break;
                    }

    if (singleCandidatesCount)
    {
        if (!backtrack(filled + singleCandidatesCount))
            return false;
        rollback(t);
        return true;
    }

    priority_queue<pair<int, pair<int, int>>> kweue;

    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            if (board[i][j] == -1)
                kweue.push({- candidatesCount[i][j], {i, j}});

    while (!kweue.empty())
    {
        int i = kweue.top().second.first, j = kweue.top().second.second;
        kweue.pop();

        for (int value = 0; value < 9; value++)
            if (!violations[i][j][value])
            {
                fill(i, j, value);
                if (!backtrack(filled + 1))
                    return false;
                erase(i, j);
            }
    }

    return true;
}