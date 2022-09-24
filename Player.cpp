#include "Player.h"
#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <iostream>
#include <string>

using namespace std;


//========================================================================
// Timer t;                 // create a timer and start it
// t.start();               // start the timer
// double d = t.elapsed();  // milliseconds since timer was last started
//========================================================================

#include <chrono>

class Timer
{
public:
    Timer()
    {
        start();
    }
    void start()
    {
        m_time = std::chrono::high_resolution_clock::now();
    }
    double elapsed() const
    {
        std::chrono::duration<double, std::milli> diff =
            std::chrono::high_resolution_clock::now() - m_time;
        return diff.count();
    }
private:
    std::chrono::high_resolution_clock::time_point m_time;
};

//*********************************************************************
//  AwfulPlayer
//*********************************************************************

class AwfulPlayer : public Player
{
  public:
    AwfulPlayer(string nm, const Game& g);
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
                                                bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p);
  private:
    Point m_lastCellAttacked;
};

AwfulPlayer::AwfulPlayer(string nm, const Game& g)
 : Player(nm, g), m_lastCellAttacked(0, 0)
{}

bool AwfulPlayer::placeShips(Board& b)
{
      // Clustering ships is bad strategy
    for (int k = 0; k < game().nShips(); k++)
        if ( ! b.placeShip(Point(k,0), k, HORIZONTAL))
            return false;
    return true;
}

Point AwfulPlayer::recommendAttack()
{
    if (m_lastCellAttacked.c > 0)
        m_lastCellAttacked.c--;
    else
    {
        m_lastCellAttacked.c = game().cols() - 1;
        if (m_lastCellAttacked.r > 0)
            m_lastCellAttacked.r--;
        else
            m_lastCellAttacked.r = game().rows() - 1;
    }
    return m_lastCellAttacked;
}

void AwfulPlayer::recordAttackResult(Point /* p */, bool /* validShot */,
                                     bool /* shotHit */, bool /* shipDestroyed */,
                                     int /* shipId */)
{
      // AwfulPlayer completely ignores the result of any attack
}

void AwfulPlayer::recordAttackByOpponent(Point /* p */)
{
      // AwfulPlayer completely ignores what the opponent does
}

//*********************************************************************
//  HumanPlayer
//*********************************************************************

bool getLineWithTwoIntegers(int& r, int& c)
{
    bool result(cin >> r >> c);
    if (!result)
        cin.clear();  // clear error state so can do more input operations
    cin.ignore(10000, '\n');
    return result;
}


class HumanPlayer : public Player
{
public:
    HumanPlayer(string nm, const Game& g) : Player(nm, g) {}

    bool isHuman() const { return true; }

    // leave these alone v
    void recordAttackResult(Point p, bool validShot, bool shotHit,
        bool shipDestroyed, int shipId) {}
    void recordAttackByOpponent(Point p) {}
    // leave these alone ^

    bool placeShips(Board& b)
    {
        string direction = "";
        bool check = true;
        Direction d;
        int row, column;
        for (int i = 0; i < game().nShips(); i++)
        {
            cout << name() << " must place " << game().nShips() - i << " ships." << endl;
            b.display(false);
            while (check) 
            {
                cout << "Enter h or v for direction of " << game().shipName(i) << " (length " << game().shipLength(i) << "): ";
                getline(cin >> ws, direction);
                if (direction == "h" || direction == "v") { check = false; }
                else { cout << "Direction must be h or v." << endl; }
            }
            if (direction == "v")
                d = VERTICAL;
            else
                d = HORIZONTAL;
            while (!check) 
            {
                cout << "Enter row and column of ";
                if (d == VERTICAL) { cout << "top"; }
                else { cout << "left"; } 
                cout << "most cell (e.g., 3 5): ";
                getLineWithTwoIntegers(row, column);
                if (b.placeShip(Point(row, column), i, d))
                {
                    check = true;
                }
                else 
                {
                    cout << "The ship cannot be placed there." << endl;
                }
            }
            check = true;
        }
        return true;
    }

    Point recommendAttack()
    {
        int r, c;
        bool check = true;
        while (check) 
        {
            cout << "Enter row and column to attack (e.g. 3 5): ";
            if (!getLineWithTwoIntegers(r, c)) 
            { cout << "You must enter two integers." << endl; }
            else { check = false; }
        }
        Point p(r, c);
        return p;
    }
};

//*********************************************************************
//  MediocrePlayer
//*********************************************************************

class MediocrePlayer : public Player
{
public:
    MediocrePlayer(string nm, const Game& g) : Player(nm, g), mState(1)
    {
        board.resize(g.rows());
        for (int i = 0; i < g.rows(); i++)
        {
            board.at(i).resize(g.cols());
            for (int j = 0; j < g.cols(); j++)
            {
                board[i][j] = '.';
            }
        }
    }

    ~MediocrePlayer() {}

    bool placeShips(Board& b)
    {
        bool check = true;
        int i = 0;
        while (check && i < 50)
        {
            b.block();
            Point x(0, 0);
            if (helpPlaceShips(b, game().nShips(), x))
            {
                b.unblock();
                check = false;
            }
            b.unblock();
            i++;
        }
        if (!check)  { return true; }
        else { b.unblock(); return false; }
    }

    bool helpPlaceShips(Board& b, int nShips, Point p)
    {
        if (nShips == 0) { return true; }
        if (p.r >= game().rows()) { return false; }

        Direction d;
        if (b.placeShip(Point(p.r, p.c), nShips - 1, HORIZONTAL))
        { 
            d = HORIZONTAL;
        }
        else if (b.placeShip(Point(p.r, p.c), nShips - 1, VERTICAL))
        {
            d = VERTICAL;
        }
        else
        {
            Point t(p.r, p.c + 1);
            if (t.c >= game().cols()) { t.c = 0; t.r += 1; }
            return helpPlaceShips(b, nShips, t);
        }
        Point x(0, 0);
        if (helpPlaceShips(b, nShips - 1, x))
        {
            return true;
        }
        else
        {
            b.unplaceShip(Point(p.r, p.c), nShips - 1, d);
            Point t(p.r, p.c + 1);
            if (t.c >= game().cols())
            {
                t.c = 0; t.r += 1;
            }
            return helpPlaceShips(b, nShips, t);
        }
    }

    Point recommendAttack() 
    {
        if (mState == 1) 
        {
            bool check = true;
            while (check) 
            {
                Point temp(randInt(game().rows()), randInt(game().cols()));
                if (board[temp.r][temp.c] == '.') 
                { 
                check = false; 
                board[temp.r][temp.c] = '*'; 
                return temp; 
                }
            }
        }
        if (mState == 2) 
        {
            vector<Point> cross;
            int r = lastPointHit.r;
            int c = lastPointHit.c;
            int d = 1;
            while (d != 5)
            {
                if (r - d >= 0 && board[r - d][c] == '.')
                {
                    cross.push_back(Point(r - d, c));
                }
                if (r + d <= game().rows() - 1 && board[r + d][c] == '.')
                { 
                    cross.push_back(Point(r + d, c));
                }
                if (c - d >= 0 && board[r][c - d] == '.')
                { 
                    cross.push_back(Point(r, c - d));
                }
                if (c + d <= game().cols() - 1 && board[r][c + d] == '.')
                {
                    cross.push_back(Point(r, c + d));
                }
                d++;
            }
            if (cross.empty()) 
            { 
            mState = 1;             
            bool check = true;
            while (check)
            {
                Point temp(randInt(game().rows()), randInt(game().cols()));
                if (board[temp.r][temp.c] == '.')
                {
                    check = false;
                    board[temp.r][temp.c] = '*';
                    return temp;
                }
            }
            }
            int iter = randInt(cross.size());
            board[cross.at(iter).r][cross.at(iter).c] = '*';
            return cross.at(iter);
        }
        return Point(0, 0);
    }
    void recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId) 
    {
        if (!validShot)
        {
            mState = 1;
            return;
        }
        else 
        {
            if (shotHit) { board[p.r][p.c] = '*'; }
            if (!shotHit) { board[p.r][p.c] = '*'; }
            if (mState == 1)
            {
                if (!shotHit) { return; }
                if (shotHit && shipDestroyed) { return; }
                if (shotHit && !shipDestroyed)
                {
                    mState = 2;
                    lastPointHit = p;
                    return;
                }
            }
            else if (mState == 2)
            {
                if (!shotHit || (shotHit && !shipDestroyed)) { return; }
                if (shotHit && shipDestroyed) { mState = 1; return; }
            }
        }
    }
    void recordAttackByOpponent(Point p) {} // this does nothing   
private:
    int mState;
    Point lastPointHit;
    vector<vector<char>> board;
};

//*********************************************************************
//  GoodPlayer
//*********************************************************************


class GoodPlayer : public Player
{
public:
    GoodPlayer(string nm, const Game& g) : Player(nm, g), mState(1), dir(HORIZONTAL)
    {
        board.resize(g.rows());
        for (int i = 0; i < g.rows(); i++) 
        {
            board.at(i).resize(g.cols());
            for (int j = 0; j < g.cols(); j++) 
            {
                availablePoints.push_back(Point(i, j));
                board[i][j] = '.';
            }
        }
    }
    ~GoodPlayer() {}
    bool placeShips(Board& b) 
    {
        int id = 0;
        bool check;
        int shipsLeft = game().nShips();
        while (shipsLeft != 0)
        {
            int i = randInt(availablePoints.size());
            Point p(availablePoints[i].r, availablePoints[i].c);
            check = b.placeShip(p, id, HORIZONTAL);
            if (!check)
                check = b.placeShip(p, id, VERTICAL);
            if (check)
            {
                for (vector<Point>::iterator i = availablePoints.begin(); i != availablePoints.end(); )
                {
                    if (i->r == p.r && i->c == p.c)
                        i = availablePoints.erase(i);
                    else
                        i++;
                }
                shipsLeft--;
                id++;
            }
        }
        return true;
    }


    Point recommendAttack() 
    {
        if (mState == 1)
        {
            bool check = true;
            while (check)
            {
                Point temp(randInt(game().rows()), randInt(game().cols()));
                if (board[temp.r][temp.c] == '.')
                {
                    check = false;
                    board[temp.r][temp.c] = '*';
                    return temp;
                }
            }
        }
        if (mState == 2)
        {
            int r = lastPointHit.r;
            int c = lastPointHit.c;
            if (r - 1 >= 0 && board[r - 1][c] == '.')
            {
                cross.push_back(Point(r - 1, c));
            }
            if (r + 1 <= game().rows() - 1 && board[r + 1][c] == '.')
            {
                cross.push_back(Point(r + 1, c));
             }
            if (c - 1 >= 0 && board[r][c - 1] == '.')
            {
                cross.push_back(Point(r, c - 1));
            }
            if (c + 1 <= game().cols() - 1 && board[r][c + 1] == '.')
            {
                cross.push_back(Point(r, c + 1));
            }
            if (cross.empty()) 
            { 
                mState = 1; 
                bool check = true;
                while (check)
                {
                    Point temp(randInt(game().rows()), randInt(game().cols()));
                    if (board[temp.r][temp.c] == '.')
                    {
                        check = false;
                        board[temp.r][temp.c] = '*';
                        return temp;
                    }
                }
            }
            int i = randInt(cross.size());
            if (cross.at(i).r == r) 
            {
                dir = HORIZONTAL;
            }
            if (cross.at(i).c == c) 
            {
                dir = VERTICAL;
            }
            board[cross.at(i).r][cross.at(i).c] = '*';
            Point temp(cross.at(i).r, cross.at(i).c);
            cross.clear();
            return temp;
        }
        if (mState == 3) // we know it is a horizontal ship (mostly)
        {
            int r = lastPointHit.r;
            int c = lastPointHit.c;
            int d = 1;
            while (d != 5) 
            {
                if (c - d >= 0 && board[r][c - d] == '.')
                {
                    cross.push_back(Point(r, c - d));
                }
                if (c + d <= game().cols() - 1 && board[r][c + d] == '.')
                {
                    cross.push_back(Point(r, c + d));
                }
                d++;
            }
            if (cross.empty()) 
            { 
                mState = 1;
                bool check = true;
                while (check)
                {
                    Point temp(randInt(game().rows()), randInt(game().cols()));
                    if (board[temp.r][temp.c] == '.')
                    {
                        check = false;
                        board[temp.r][temp.c] = '*';
                        return temp;
                    }
                }
            }
            int i = randInt(cross.size());
            board[cross.at(i).r][cross.at(i).c] = '*';
            Point temp(cross.at(i).r, cross.at(i).c);
            cross.clear();
            return temp;
        }
        if (mState == 4) 
        {
            int r = lastPointHit.r;
            int c = lastPointHit.c;
            int d = 1;
            while (d != 5) 
            {
                if (r - d >= 0 && board[r - d][c] == '.')
                {
                    cross.push_back(Point(r - d, c));
                }
                if (r + d <= game().cols() - 1 && board[r + d][c] == '.')
                {
                    cross.push_back(Point(r + d, c));
                }
                d++;
            }
            if (cross.empty()) 
            { 
                mState = 1;
                bool check = true;
                while (check)
                {
                    Point temp(randInt(game().rows()), randInt(game().cols()));
                    if (board[temp.r][temp.c] == '.')
                    {
                        check = false;
                        board[temp.r][temp.c] = '*';
                        return temp;
                    }
                }
            }
            int i = randInt(cross.size());
            board[cross.at(i).r][cross.at(i).c] = '*';
            Point temp(cross.at(i).r, cross.at(i).c);
            cross.clear();
            return temp;
        }
        return Point(0, 0);
    }
    void recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId)
    {
        if (!validShot)
        {
            mState = 1;
            return;
        }
        else
        {
            if (shotHit) { board[p.r][p.c] = 'X'; }
            if (!shotHit) { board[p.r][p.c] = 'o'; }
            if (mState == 1)
            {
                if (!shotHit) { return; }
                if (shotHit && shipDestroyed) { return; }
                if (shotHit && !shipDestroyed)
                {
                    mState = 2;
                    lastPointHit = p;
                    return;
                }
            }
            else if (mState == 2)
            {
                if ((shotHit && !shipDestroyed) && dir == HORIZONTAL) { mState = 3; return; }
                else if ((shotHit && !shipDestroyed) && dir == VERTICAL) { mState = 4; return; }
                if (shotHit && shipDestroyed) { mState = 1; return; }
            }
            else if (mState == 3) // we know it is a horizontal ship
            {
                if (shotHit && shipDestroyed) { mState = 1; return; }
            }
            else if (mState == 4) // we know it is a vertical ship
            {
                if (shotHit && shipDestroyed) { mState = 1; return; }
            }
        }
    }
    void recordAttackByOpponent(Point p) {} // does nothing imo
private:
    vector<vector<char>> board;
    int mState;
    Point lastPointHit;
    vector<Point> availablePoints;
    Direction dir;
    vector<Point> cross;
};


//*********************************************************************
//  createPlayer
//*********************************************************************

Player* createPlayer(string type, string nm, const Game& g)
{
    static string types[] = {
        "human", "awful", "mediocre", "good"
    };
    
    int pos;
    for (pos = 0; pos != sizeof(types)/sizeof(types[0])  &&
                                                     type != types[pos]; pos++)
        ;
    switch (pos)
    {
      case 0:  return new HumanPlayer(nm, g);
      case 1:  return new AwfulPlayer(nm, g);
      case 2:  return new MediocrePlayer(nm, g);
      case 3:  return new GoodPlayer(nm, g);
      default: return nullptr;
    }
}
