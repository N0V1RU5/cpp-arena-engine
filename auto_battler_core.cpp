#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <random>
#include <chrono>

using namespace std;

int GetRandomNumber(int min, int max)
{
    // Staticke promenne se vytvori jen jednou pri prvnim zavolani
    static std::random_device rd;
    static std::mt19937 gen(rd()); // Standardni AAA generator (Mersenne Twister)
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);

}

struct UnitData
{
    string name;
    float hp;
    int speed;
    int damage;
    int critChance;
    int dodgeChance;

};

enum class State
{
    IDLE,
    ATTACK,
    DEAD,
    VICTORY

};

struct Registry{
    vector<float> hps;
    vector<State> states;
    vector<int> teamIds;
    vector<const UnitData*> baseStats;

};

void CleanupDeadUnits(Registry& world){
    for(int k = world.hps.size() - 1; k > 0; k--){
        if(world.states[k] == State::DEAD){
            world.hps[k] = world.hps.back();
            world.hps.pop_back();

            world.baseStats[k] = world.baseStats.back();
            world.baseStats.pop_back();

            world.states[k] = world.states.back();
            world.states.pop_back();

            world.teamIds[k] = world.teamIds.back();
            world.teamIds.pop_back();
        }
    }
};

void BattleSystem(Registry& world){
    for(int i = 0; i < world.hps.size(); i++){
        if (world.hps[i] <= 0)
        {
            world.states[i] = State::DEAD;
            continue;
        }
        switch (world.states[i])
        {
        case State::IDLE:
            world.states[i] = State::ATTACK;
            // cout << baseStats->name << " is getting ready to attack." << endl;
            break;
        case State::ATTACK:
        {
            bool foundTarget = false;
            for(int j = 0; j < world.hps.size(); j++)
            {
                if ((i == j) || (world.teamIds[j] == world.teamIds[i]) || (world.states[j] == State::DEAD))
                {
                    continue;
                }

                if (GetRandomNumber(1, 100) <= world.baseStats[j]->dodgeChance)
                {
                    // cout << target->baseStats->name << " DODGED!" << endl;
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
                    // cout << baseStats->name << " dealt finishing blow to " << target->baseStats->name << ".\n";
                    foundTarget = true;
                    break;
                }
                world.hps[j] -= actualDamage;
                if (actualDamage > world.baseStats[i]->damage)
                {
                    // cout << baseStats->name << " CRITICAL HIT " << target->baseStats->name << " for " << actualDamage << " damage. " << target->currentHp << '/' << target->baseStats->hp << "HP." << endl;
                }
                else
                {
                    // cout << baseStats->name << " hit " << target->baseStats->name << " for " << baseStats->damage << " damage. " << target->currentHp << '/' << target->baseStats->hp << "HP." << endl;
                }
                foundTarget = true;
                break;
            }
            if (!foundTarget)
            {
                world.states[i] = State::VICTORY;
            }
            break;
        }

        case State::DEAD:
            // cout << "I am dead." << endl;
            break;

        case State::VICTORY:
            // cout << "Chilling after win!" << endl;
            break;

        default:
            cout << "default case";
            break;
        }
    }
}

int main()
{
    fstream file("units.csv");

    vector<UnitData> creatureData;

    string row;
    // preskoceni prvniho raadku
    getline(file, row);
    while (getline(file, row))
    {
        stringstream ss(row);
        string nameStr, hpStr, speedStr, damageStr, critStr, dodgeStr;

        getline(ss, nameStr, ',');
        getline(ss, hpStr, ',');
        getline(ss, speedStr, ',');
        getline(ss, damageStr, ',');
        getline(ss, critStr, ',');
        getline(ss, dodgeStr, ',');

        UnitData newUnit;
        newUnit.name = nameStr;
        newUnit.hp = stof(hpStr);
        newUnit.speed = stoi(speedStr);
        newUnit.damage = stoi(damageStr);
        newUnit.critChance = stoi(critStr);
        newUnit.dodgeChance = stoi(dodgeStr);

        creatureData.push_back(newUnit);
    }

    Registry world;
    for(int i = 0; i < 10000; i++){
        world.hps.push_back(creatureData[0].hp);
        world.states.push_back(State::IDLE);
        world.teamIds.push_back(1);
        world.baseStats.push_back(&creatureData[0]);

        world.hps.push_back(creatureData[1].hp);
        world.states.push_back(State::IDLE);
        world.teamIds.push_back(2);
        world.baseStats.push_back(&creatureData[1]);

    }

    for (int turn = 0; turn < 10; turn++)
    {
        cout << "--- ROUND " << turn + 1 << " ---" << endl;
        auto start = chrono::high_resolution_clock::now();
        BattleSystem(world);
        CleanupDeadUnits(world);
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double, std::milli> ms_double = end - start;
        cout << "Round " << turn + 1 << " was " << ms_double.count() << " ms." << endl;
    }
    
    return 0;
};
