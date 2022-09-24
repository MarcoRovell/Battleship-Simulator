#include "Game.h"
#include "Board.h"
#include "Player.h"
#include "globals.h"
#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>

using namespace std;

class GameImpl
{
  public:
    GameImpl(int nRows, int nCols);
    int rows() const;
    int cols() const;
    bool isValid(Point p) const;
    Point randomPoint() const;
    bool addShip(int length, char symbol, string name);
    int nShips() const;
    int shipLength(int shipId) const;
    char shipSymbol(int shipId) const;
    string shipName(int shipId) const;
    Player* play(Player* p1, Player* p2, Board& b1, Board& b2, bool shouldPause);
private:
    struct ship 
    {
        int mLength = 0;
        char mSymbol = ' ';
        string mName = "";
        int shipId = -1;
    };
    vector<ship> shipVector;
    int mRows;
    int mCols;
};

void waitForEnter()
{
    cout << "Press enter to continue: ";
    cin.ignore(10000, '\n');
}

GameImpl::GameImpl(int nRows, int nCols) : mRows(nRows), mCols(nCols)
{}

int GameImpl::rows() const
{
    return mRows;
}

int GameImpl::cols() const
{
    return mCols;
}

bool GameImpl::isValid(Point p) const
{
    return p.r >= 0  &&  p.r < rows()  &&  p.c >= 0  &&  p.c < cols();
}

Point GameImpl::randomPoint() const
{
    return Point(randInt(rows()), randInt(cols()));
}

bool GameImpl::addShip(int length, char symbol, string name)  
{
    if (length > 0 && (symbol != 'X' && symbol != 'o' && symbol != '.') && isprint(symbol))
    {
        ship s;
        s.mLength = length;
        s.mSymbol = symbol;
        s.mName = name;
        shipVector.push_back(s);
        s.shipId = shipVector.size() - 1;
        return true;
    }
    return false; 
}

int GameImpl::nShips() const
{
    return shipVector.size();
}

int GameImpl::shipLength(int shipId) const
{
    return shipVector.at(shipId).mLength; 
}

char GameImpl::shipSymbol(int shipId) const
{
    return shipVector.at(shipId).mSymbol;  
}

string GameImpl::shipName(int shipId) const
{
    return shipVector.at(shipId).mName;
}

Player* GameImpl::play(Player* p1, Player* p2, Board& b1, Board& b2, bool shouldPause)
{
    if (!p1->placeShips(b1) || !p2->placeShips(b2)) { return nullptr; }
    // game play starts
    int k = 0;
    while (!b1.allShipsDestroyed() && !b2.allShipsDestroyed()) 
    {
        bool shotHit = false;
        bool shipDestroyed = false;
        bool valid = false;
        int shipId = -1;
        if (k % 2 == 0) //p1 plays
        {
            bool isHuman = p1->isHuman();
            cout << p1->name() << "'s turn. Board for " << p2->name() << ":" << endl;
            b2.display(isHuman);
            Point p = p1->recommendAttack();
            valid = b2.attack(p, shotHit, shipDestroyed, shipId);
            p1->recordAttackResult(p, valid, shotHit, shipDestroyed, shipId);
            if (p1->isHuman() && valid == false)
            { cout << p1->name() << " wasted a shot at (" << p.r << "," << p.c << ")." << endl; }
            else 
            {
                cout << p1->name() << " attacked (" << p.r << "," << p.c << ") and ";
                if (shotHit)
                {
                    if (shipDestroyed) { cout << "destroyed the " << this->shipName(shipId); }
                    else { cout << "hit something"; }
                }
                else { cout << "missed"; }
                cout << " , resulting in:" << endl;
            }
            b2.display(isHuman);
        }
        if (k % 2 == 1) //p2 plays
        {
            bool isHuman = p2->isHuman();
            cout << p2->name() << "'s turn. Board for " << p1->name() << ":" << endl;
            b1.display(isHuman);
            Point p = p2->recommendAttack();
            valid = b1.attack(p, shotHit, shipDestroyed, shipId);
            p2->recordAttackResult(p, valid, shotHit, shipDestroyed, shipId);
            if (p2->isHuman() && valid == false)
            { cout << p2->name() << " wasted a shot at (" << p.r << "," << p.c << ")." << endl; }
            else
            {
                cout << p2->name() << " attacked (" << p.r << "," << p.c << ") and ";
                if (shotHit)
                {
                    if (shipDestroyed) { cout << "destroyed the " << this->shipName(shipId); }
                    else { cout << "hit something"; }
                }
                else { cout << "missed"; }
                cout << " , resulting in:" << endl;
            }
            b1.display(isHuman);
        }
        if (shouldPause) { waitForEnter(); }
        k++;
    }
    if (b1.allShipsDestroyed()) { cout << p2->name() << " wins!" << endl; return p2; }
    else if (b2.allShipsDestroyed()) { cout << p1->name() << " wins!" << endl; return p1; }
    else { return nullptr; }
}


//******************** Game functions *******************************

// These functions for the most part simply delegate to GameImpl's functions.
// You probably don't want to change any of the code from this point down.

Game::Game(int nRows, int nCols)
{
    if (nRows < 1  ||  nRows > MAXROWS)
    {
        cout << "Number of rows must be >= 1 and <= " << MAXROWS << endl;
        exit(1);
    }
    if (nCols < 1  ||  nCols > MAXCOLS)
    {
        cout << "Number of columns must be >= 1 and <= " << MAXCOLS << endl;
        exit(1);
    }
    m_impl = new GameImpl(nRows, nCols);
}

Game::~Game()
{
    delete m_impl;
}

int Game::rows() const
{
    return m_impl->rows();
}

int Game::cols() const
{
    return m_impl->cols();
}

bool Game::isValid(Point p) const
{
    return m_impl->isValid(p);
}

Point Game::randomPoint() const
{
    return m_impl->randomPoint();
}

bool Game::addShip(int length, char symbol, string name)
{
    if (length < 1)
    {
        cout << "Bad ship length " << length << "; it must be >= 1" << endl;
        return false;
    }
    if (length > rows()  &&  length > cols())
    {
        cout << "Bad ship length " << length << "; it won't fit on the board"
             << endl;
        return false;
    }
    if (!isascii(symbol)  ||  !isprint(symbol))
    {
        cout << "Unprintable character with decimal value " << symbol
             << " must not be used as a ship symbol" << endl;
        return false;
    }
    if (symbol == 'X'  ||  symbol == '.'  ||  symbol == 'o')
    {
        cout << "Character " << symbol << " must not be used as a ship symbol"
             << endl;
        return false;
    }
    int totalOfLengths = 0;
    for (int s = 0; s < nShips(); s++)
    {
        totalOfLengths += shipLength(s);
        if (shipSymbol(s) == symbol)
        {
            cout << "Ship symbol " << symbol
                 << " must not be used for more than one ship" << endl;
            return false;
        }
    }
    if (totalOfLengths + length > rows() * cols())
    {
        cout << "Board is too small to fit all ships" << endl;
        return false;
    }
    return m_impl->addShip(length, symbol, name);
}

int Game::nShips() const
{
    return m_impl->nShips();
}

int Game::shipLength(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipLength(shipId);
}

char Game::shipSymbol(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipSymbol(shipId);
}

string Game::shipName(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipName(shipId);
}

Player* Game::play(Player* p1, Player* p2, bool shouldPause)
{
    if (p1 == nullptr  ||  p2 == nullptr  ||  nShips() == 0)
        return nullptr;
    Board b1(*this);
    Board b2(*this);
    return m_impl->play(p1, p2, b1, b2, shouldPause);
}

