#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <random>
#include <chrono>

using namespace std;

// random number generator for int
int GetRandomNumber(int min, int max)
{
    // static aka only one instance is created
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}

// each unit has data saved in this structure
struct UnitData
{
    string name;
    float hp;
    int speed;
    int damage;
    int critChance;
    int dodgeChance;
    float attackRange;
};

// each unit can have one of these states
enum class State
{
    IDLE,
    ATTACK,
    DEAD,
    VICTORY
};

// a registry of all things in the engine
struct Registry
{
    vector<float> hps;
    vector<State> states;
    vector<int> teamIds;
    vector<const UnitData *> baseStats;

    vector<float> xs;
    vector<float> ys;
};

// grid structure, thats 20*20 squares, each square is 50*50 int big
struct Grid
{
    vector<vector<int>> cells;
    int cellSize = 50;
    int cols = 20;

    // set cells to 400, because 20*20 = 400
    Grid()
    {
        cells.resize(400);
    }

    // get an index of a cell
    int GetCellIndex(float x, float y)
    {
        return GetRow(y) * cols + GetCol(x);
    }

    int GetCol(float x)
    {
        int col = x / cellSize;
        return max(0, min(col, cols - 1));
    }

    int GetRow(float y)
    {
        int row = y / cellSize;
        return max(0, min(row, (int)(cells.size() / cols) - 1));
    }
};

// custom garbage collector, to delete dead units from Registry
void CleanupDeadUnits(Registry &world)
{
    for (int k = world.hps.size() - 1; k > 0; k--)
    {
        if (world.states[k] == State::DEAD)
        {
            world.hps[k] = world.hps.back();
            world.hps.pop_back();

            world.baseStats[k] = world.baseStats.back();
            world.baseStats.pop_back();

            world.states[k] = world.states.back();
            world.states.pop_back();

            world.teamIds[k] = world.teamIds.back();
            world.teamIds.pop_back();

            world.xs[k] = world.xs.back();
            world.xs.pop_back();
            world.ys[k] = world.ys.back();
            world.ys.pop_back();
        }
    }
};

// one of the most important functions, all units here fight in a 3x3 square radius around them, in a grid
void BattleSystem(Registry &world, Grid &grid)
{
    for (int i = 0; i < world.hps.size(); i++)
    {
        if (world.hps[i] <= 0)
        {
            world.states[i] = State::DEAD;
            continue;
        }
        switch (world.states[i])
        {
        case State::IDLE:
            world.states[i] = State::ATTACK;
            break;
        case State::ATTACK:
        {
            bool foundTarget = false;
            int myCol = grid.GetCol(world.xs[i]);
            int myRow = grid.GetRow(world.ys[i]);

            for (int row = myRow - 1; row < myRow + 1; row++)
            {
                if (row < 0 || row >= (grid.cells.size() / grid.cols))
                {
                    continue;
                }
                for (int col = myCol - 1; col < myCol + 1; col++)
                {
                    if (col < 0 || col > grid.cols)
                    {
                        continue;
                    }
                    int aroundCell = row * grid.cols + col;
                    for (int j : grid.cells[aroundCell])
                    {
                        if ((i == j) || (world.teamIds[j] == world.teamIds[i]) || (world.states[j] == State::DEAD))
                        {
                            continue;
                        }

                        float dx = world.xs[j] - world.xs[i];
                        float dy = world.ys[j] - world.ys[i];
                        float distSq = dx * dx + dy * dy;
                        float rangeSq = world.baseStats[i]->attackRange * world.baseStats[i]->attackRange;

                        if (distSq > rangeSq)
                        {
                            continue;
                        }

                        if (GetRandomNumber(1, 100) <= world.baseStats[j]->dodgeChance)
                        {
                            foundTarget = true;
                            break;
                        }

                        int actualDamage = world.baseStats[i]->damage;

                        if (GetRandomNumber(1, 100) <= world.baseStats[i]->critChance)
                        {
                            actualDamage *= 2;
                        }

                        if (world.hps[j] - actualDamage <= 0)
                        {
                            world.hps[j] = 0;
                            world.states[j] = State::DEAD;
                            foundTarget = true;
                            break;
                        }
                        world.hps[j] -= actualDamage;
                        foundTarget = true;
                        break;
                    }
                }
            }

            if (!foundTarget)
            {
                world.states[i] = State::VICTORY;
            }
            break;
        }

        case State::DEAD:
            break;

        case State::VICTORY:
            break;

        default:
            cout << "default case";
            break;
        }
    }
}

// if unit won (has State::VICTORY), they randomly walk and try to find another enemy
void MovementSystem(Registry& world){
    for(int i = 0; i < world.hps.size(); i++){
        if(world.states[i] == State::DEAD){
            continue;
        }
        if(world.states[i] == State::VICTORY){
            world.xs[i] = clamp(world.xs[i] + GetRandomNumber(-world.baseStats[i]->speed, world.baseStats[i]->speed), 0.0f, 1000.0f);
            world.ys[i] = clamp(world.ys[i] + GetRandomNumber(-world.baseStats[i]->speed, world.baseStats[i]->speed), 0.0f, 1000.0f);

            world.states[i] = State::IDLE;
        }
    }
}

// main function, with a chrono timer, to calculate how long each 10 000 vs 10 000 units fights were long in ms
int main()
{
    // loading data from csv
    fstream file("units.csv");

    vector<UnitData> creatureData;

    string row;
    // first line skip
    getline(file, row);
    while (getline(file, row))
    {
        if(row.empty()) {continue;}
        stringstream ss(row);
        string nameStr, hpStr, speedStr, damageStr, critStr, dodgeStr, rangeStr;

        getline(ss, nameStr, ',');
        getline(ss, hpStr, ',');
        getline(ss, speedStr, ',');
        getline(ss, damageStr, ',');
        getline(ss, critStr, ',');
        getline(ss, dodgeStr, ',');
        getline(ss, rangeStr, ',');

        UnitData newUnit;
        newUnit.name = nameStr;
        newUnit.hp = stof(hpStr);
        newUnit.speed = stoi(speedStr);
        newUnit.damage = stoi(damageStr);
        newUnit.critChance = stoi(critStr);
        newUnit.dodgeChance = stoi(dodgeStr);
        newUnit.attackRange = stof(rangeStr);

        creatureData.push_back(newUnit);
    }

    // loading all 20 000 units
    Registry world;
    for (int i = 0; i < 10000; i++)
    {
        world.hps.push_back(creatureData[0].hp);
        world.states.push_back(State::IDLE);
        world.teamIds.push_back(1);
        world.baseStats.push_back(&creatureData[0]);
        world.xs.push_back(GetRandomNumber(0, 1000));
        world.ys.push_back(GetRandomNumber(0, 1000));

        world.hps.push_back(creatureData[1].hp);
        world.states.push_back(State::IDLE);
        world.teamIds.push_back(2);
        world.baseStats.push_back(&creatureData[1]);
        world.xs.push_back(GetRandomNumber(0, 1000));
        world.ys.push_back(GetRandomNumber(0, 1000));
    }

    // 10 rounds, 10 000 vs 10 000 enemies, who wins?
    for (int turn = 0; turn < 10; turn++)
    {
        cout << "--- ROUND " << turn + 1 << " ---" << endl;
        auto start = chrono::high_resolution_clock::now();
        Grid grid;
        for (int i = 0; i < world.hps.size(); i++)
        {
            if (world.states[i] != State::DEAD)
            {
                int cellIndex = grid.GetCellIndex(world.xs[i], world.ys[i]);
                grid.cells[cellIndex].push_back(i);
            }
        }
        BattleSystem(world, grid);
        MovementSystem(world);
        CleanupDeadUnits(world);
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double, std::milli> ms_double = end - start;
        cout << "Round " << turn + 1 << " was " << ms_double.count() << " ms." << endl;
    }

    int teamOne = 0;
    int teamTwo = 0;
    for (int i = 0; i < world.teamIds.size(); i++)
    {
        if (world.teamIds[i] == 1)
        {
            teamOne++;
        }
        else if (world.teamIds[i] == 2)
        {
            teamTwo++;
        }
    }
    if (teamOne > teamTwo)
    {
        cout << "TEAM ONE WON!!! " << teamOne << " : " << teamTwo << endl;
    }
    else if (teamOne == teamTwo)
    {
        cout << "IT WAS A DRAW!!! " << teamOne << " : " << teamTwo << endl;
    }
    else
    {
        cout << "TEAM TWO WON!!! " << teamTwo << " : " << teamOne << endl;
    }

    return 0;
};
