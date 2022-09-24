#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>

using namespace std;

class BoardImpl
{
  public:
    BoardImpl(const Game& g);
    void clear();
    void block();
    void unblock();
    bool placeShip(Point topOrLeft, int shipId, Direction dir);
    bool unplaceShip(Point topOrLeft, int shipId, Direction dir);
    void display(bool shotsOnly) const;
    bool attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId);
    bool allShipsDestroyed() const;

  private:
    struct ship
    {
        int mLength = 0;
        char mSymbol = ' ';
        int shipId = 1000;
        int hits = 0;
    };
    const Game& m_game;
    vector<vector<char>> board;
    vector<int> Id;
    vector<ship> placedShips;
};

BoardImpl::BoardImpl(const Game& g)
    : m_game(g)
{
    board.resize(g.rows());
    for (int i = 0; i < g.rows(); i++)
    {
        board[i].resize(g.cols());
        for (int j = 0; j < g.cols(); j++)
        {
            board[i][j] = '.';
        }
    }

}

void BoardImpl::clear()
{
    for (int i = 0; i < m_game.rows(); i++)
    {
        for (int j = 0; j < m_game.cols(); j++)
        {
            board[i][j] = '.';
        }
    }
}

void BoardImpl::block()
{
    int R = m_game.rows();
    int C = m_game.cols();
    int amount = R * C / 2;
    for (int i = 0; i < amount; i++) 
    {
        int randomR = randInt(R);
        int randomC = randInt(C);
        if (board[randomR][randomC] == '.') 
        {
            board[randomR][randomC] = '#';
        }
        else 
        {
            i--;
        }
    }
}

void BoardImpl::unblock()
{
    int R = m_game.rows();
    int C = m_game.cols();
    for (int i = 0; i < R; i++)
    {
        for (int j = 0; j < C; j++)
        {
            if (board[i][j] == '#')
            { board[i][j] = '.'; }
        }
    }
}

bool BoardImpl::placeShip(Point topOrLeft, int shipId, Direction dir)
{
    if (shipId < 0 || shipId > m_game.nShips() - 1) { return false; }
    if (topOrLeft.r < 0 || topOrLeft.r > m_game.rows() - 1 || topOrLeft.c < 0 || topOrLeft.c > m_game.cols() - 1) { return false; }
    if (find(Id.begin(), Id.end(), shipId) != Id.end()) { return false; }
    int length = m_game.shipLength(shipId);
    char sym = m_game.shipSymbol(shipId);
    if (dir == 0) // horizontal
    {
        if ((topOrLeft.c + length) > m_game.cols()) { return false; }
        for (int i = topOrLeft.c; i < length + topOrLeft.c; i++)
        {
            if (board[topOrLeft.r][i] != '.') { return false; }
        }
        for (int j = topOrLeft.c; j < length + topOrLeft.c; j++)
        {
            board[topOrLeft.r][j] = sym;
        }
    }
    if (dir == 1) // vertical
    {
        if ((topOrLeft.r + length) > m_game.rows()) { return false; }
        for (int i = topOrLeft.r; i < length + topOrLeft.r; i++)
        {
            if (board[i][topOrLeft.c] != '.') { return false; }
        }
        for (int j = topOrLeft.r; j < length + topOrLeft.r; j++)
        {
            board[j][topOrLeft.c] = sym;
        }
    }
    ship s;
    s.mLength = length;
    s.mSymbol = sym;
    s.shipId = shipId;
    placedShips.push_back(s);
    return true;
}

bool BoardImpl::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
    int length = m_game.shipLength(shipId);
    if (shipId > m_game.nShips()) { return false; }
    if (dir == HORIZONTAL) 
    {
        for (int i = topOrLeft.c; i < m_game.shipLength(shipId) + topOrLeft.c; i++)
        {
            if (board[topOrLeft.r][i] == '.') { return false; }
        }
        for (int j = topOrLeft.c; j < m_game.shipLength(shipId) + topOrLeft.c; j++)
        {
            board[topOrLeft.r][j] = '.';
        }
    }
    if (dir == VERTICAL) 
    {
        for (int i = topOrLeft.r; i < m_game.shipLength(shipId) + topOrLeft.r; i++)
        {
            if (board[i][topOrLeft.c] == '.') { return false; }
        }
        for (int j = topOrLeft.r; j < m_game.shipLength(shipId) + topOrLeft.r; j++)
        {
            board[j][topOrLeft.c] = '.';
        }
    }
    for (int i = 0; i < placedShips.size(); i++) 
    {
        if (placedShips.at(i).mSymbol == m_game.shipSymbol(shipId))
        {
            placedShips.erase(placedShips.begin() + i);
        }
    }
    return true;
}

bool BoardImpl::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
    if (p.r < 0 || p.r > m_game.rows() || p.c < 0 || p.c > m_game.rows())
    {
        shotHit = false;
        shipDestroyed = false;
        return false;
    }

    if (board[p.r][p.c] == 'o' || board[p.r][p.c] == 'X')
    {
        shotHit = false;
        shipDestroyed = false;
        return false;
    }
    else if (board[p.r][p.c] == '.')
    {
        shotHit = false;    
        shipDestroyed = false;
        board[p.r][p.c] = 'o';
        return true;
    }
    else
    {
        for (int i = 0; i < placedShips.size(); i++)
        {
            if (placedShips.at(i).mSymbol == board[p.r][p.c])
            {
                shipId = placedShips.at(i).shipId;
                placedShips[i].hits++;
                if (placedShips[i].hits == placedShips[i].mLength)
                {
                    shipDestroyed = true;
                }
                continue;
            }
        }
        board[p.r][p.c] = 'X';
        shotHit = true;
        return true;
    }
    return true;
}

void BoardImpl::display(bool shotsOnly) const
{
    string spaces = "  ";
    cout << spaces;
    for (int i = 0; i < m_game.cols(); i++) { cout << i; }
    cout << endl;
    int k = 0;
    while (k < m_game.rows()) 
    {
        cout << k << " ";
        for (int j = 0; j < m_game.cols(); j++) 
        {
            if (shotsOnly) 
            {
                switch (board[k][j])
                {
                case '.':
                case 'X':
                case 'o':
                    cout << board[k][j];
                    break;
                case '#':
                    cout << '#';
                    break;
                default:
                    cout << '.';
                    break;
                }
            }
            if (!shotsOnly) 
            {
                switch (board[k][j])
                {
                case '.':
                case 'X':
                case 'o':
                    cout << board[k][j];
                    break;
                case '#':
                    cout << '#';
                    break;
                default:
                    if (isalpha(board[k][j])) { cout << board[k][j]; }
                    else {
                        cout << '.';
                    }
                    break;
                }
            }
        }
        k++;
        cout << endl;
    }
}


bool BoardImpl::allShipsDestroyed() const
{
    for (int i = 0; i < m_game.rows(); i++) 
    {
        for (int j = 0; j < m_game.cols(); j++)
        {
            if (board[i][j] != 'o' && board[i][j] != 'X' && isalpha(board[i][j]) )
            {
                return false;
            }
        }
    }
    return true;
}

//******************** Board functions ********************************

// These functions simply delegate to BoardImpl's functions.
// You probably don't want to change any of this code.

Board::Board(const Game& g)
{
    m_impl = new BoardImpl(g);
}

Board::~Board()
{
    delete m_impl;
}

void Board::clear()
{
    m_impl->clear();
}

void Board::block()
{
    return m_impl->block();
}

void Board::unblock()
{
    return m_impl->unblock();
}

bool Board::placeShip(Point topOrLeft, int shipId, Direction dir)
{
    return m_impl->placeShip(topOrLeft, shipId, dir);
}

bool Board::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
    return m_impl->unplaceShip(topOrLeft, shipId, dir);
}

void Board::display(bool shotsOnly) const
{
    m_impl->display(shotsOnly);
}

bool Board::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
    return m_impl->attack(p, shotHit, shipDestroyed, shipId);
}

bool Board::allShipsDestroyed() const
{
    return m_impl->allShipsDestroyed();
}
