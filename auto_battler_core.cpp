#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <random>

using namespace std;

int GetRandomNumber(int min, int max) {
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

struct ArenaUnit
{
    float currentHp;
    State currentState;
    const UnitData *baseStats;
    int teamId;

    void Update(vector<ArenaUnit*>& allUnits)
    {
        if (currentHp <= 0)
        {
            currentState = State::DEAD;
            return;
        }
        switch (currentState)
        {
        case State::IDLE:
            currentState = State::ATTACK;
            cout << baseStats->name << " is getting ready to attack." << endl;
            break;
        case State::ATTACK:
        {
            if (allUnits.empty())
            {
                break;
            }

            bool foundTarget = false;
            for (ArenaUnit* target : allUnits)
            {
                if (target->currentState == State::DEAD || (target->teamId == teamId))
                {
                    continue;
                }

                if(GetRandomNumber(1, 100) <= target->baseStats->dodgeChance){
                    cout << target->baseStats->name << " DODGED!" << endl;
                    foundTarget = true;
                    break;
                }

                int actualDamage = baseStats->damage;

                if(GetRandomNumber(1,100) <= baseStats->critChance){
                    actualDamage *= 2;
                }

                if (target->currentHp - actualDamage <= 0)
                {
                    target->currentHp = 0;
                    target->currentState = State::DEAD;
                    cout << baseStats->name << " dealt finishing blow to " << target->baseStats->name << ".\n";
                    foundTarget = true;
                    break;
                }
                target->currentHp -= actualDamage;
                if(actualDamage > baseStats->damage){
                    cout << baseStats->name << " CRITICAL HIT " << target->baseStats->name << " for " << actualDamage << " damage. " << target->currentHp << '/' << target->baseStats->hp << "HP." << endl;
                } else {
                    cout << baseStats->name << " hit " << target->baseStats->name << " for " << baseStats->damage << " damage. " << target->currentHp << '/' << target->baseStats->hp << "HP." << endl;
                }
                foundTarget = true;
                break;
            }
            if (!foundTarget)
            {
                currentState = State::VICTORY;
            }
            break;
        }

        case State::DEAD:
            cout << "I am dead." << endl;
            break;

        case State::VICTORY:
            cout << "Chilling after win!" << endl;
            break;
        default:
            cout << "default case";
        }
    }
};

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

    vector<ArenaUnit> teamA;
    for (int i = 0; i < 10; i++)
    {
        ArenaUnit goblin;
        goblin.baseStats = &creatureData[0];
        goblin.currentHp = goblin.baseStats->hp;
        goblin.currentState = State::IDLE;
        goblin.teamId = 1;
        teamA.push_back(goblin);
    }

    vector<ArenaUnit> teamB;
    for (int i = 0; i < 3; i++)
    {
        ArenaUnit orc;
        orc.baseStats = &creatureData[1];
        orc.currentHp = orc.baseStats->hp;
        orc.currentState = State::IDLE;
        orc.teamId = 2;
        teamB.push_back(orc);
    }

    vector<ArenaUnit*> turnOrder;
    for(ArenaUnit& unitA : teamA){
        turnOrder.push_back(&unitA);
    }
    for(ArenaUnit& unitB : teamB){
        turnOrder.push_back(&unitB);
    }

    sort(turnOrder.begin(), turnOrder.end(), [](ArenaUnit* a, ArenaUnit* b){
        return a->baseStats->speed > b->baseStats->speed;
    });

    for (int turn = 0; turn < 10; turn++)
    {
        int aliveTeamA = teamA.size();
        int aliveTeamB = teamB.size();
        cout << "--- ROUND " << turn + 1 << " ---" << endl;
        for (ArenaUnit* unit : turnOrder)
        {
            if(unit->currentState == State::DEAD && unit->teamId == 1){
                aliveTeamA -= 1;
            }
            if(unit->currentState == State::DEAD && unit->teamId == 2){
                aliveTeamB -= 1;
            }
            unit->Update(turnOrder);
        }

        if(aliveTeamA <= 0 || aliveTeamB <=0){
            break;
        }
    }

    return 0;
};