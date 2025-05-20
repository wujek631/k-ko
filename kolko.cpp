#include <iostream>
#include <vector>
#include <string>

using namespace std;

enum GameStatus {IN_PROGRESS, X_WON, O_WON, DRAW};

class Move {
 public:
  Move(int i, int j, int piece) : i(i), j(j), piece(piece) {}
  int getI() const { return i; }
  int getJ() const { return j; }
  int getPiece() const { return piece; }
 private:
  int i;
  int j;
  int piece;
};

class Board {
 public:
  Board(int N, int M) {
    table.resize(N, vector<int>(M, -1));
  }
  size_t getRows() const { return table.size(); }
  size_t getCols() const { return table[0].size(); }
  int get(int i, int j) const { return table[i][j]; }
  bool set(const Move& m) {
    if (table[m.getI()][m.getJ()] == -1) {
      table[m.getI()][m.getJ()] = m.getPiece();
      return true;
    }
    return false;
  }
  bool full() const {
    for (const auto& row : table)
      for (int cell : row)
        if (cell == -1)
          return false;
    return true;
  }
  void print() const {
    for (const auto& row : table) {
      for (int cell : row)
        cout << (cell == -1 ? '.' : (cell == 0 ? 'O' : 'X')) << " ";
      cout << "\n";
    }
  }
 private:
  vector<vector<int>> table;
};

class Player {
 public:
  Player(string name, char symbol) : name(name), symbol(symbol == 'O' ? 0 : 1) {}
  string getName() const { return name; }
  int getSymbol() const { return symbol; }
  void chooseMove(Board& b) const {
    int i, j;
    do {
      cout << "Your turn " << name << " (" << (symbol == 0 ? 'O' : 'X') << "): ";
      cin >> i >> j;
    } while (!b.set(Move(i, j, symbol)));
  }
 private:
  string name;
  int symbol;
};

class AI {
 public:
  AI(int symbol) : symbol(symbol) {}
  virtual Move chooseMove(const Board& b) = 0;
  int getSymbol() const { return symbol; }
  virtual ~AI() = default;
 private:
  int symbol;
};

class DumbAI : public AI {
 public:
  DumbAI(int symbol) : AI(symbol) {}
  Move chooseMove(const Board& b) override {
    for (size_t i = 0; i < b.getRows(); ++i)
      for (size_t j = 0; j < b.getCols(); ++j)
        if (b.get(i, j) == -1)
          return Move(i, j, getSymbol());
    return Move(0, 0, getSymbol());
  }
};

class CleverAI : public AI {
 public:
  CleverAI(int symbol) : AI(symbol) {}

  Move chooseMove(const Board& b) override {
    int aiSymbol = getSymbol();
    int playerSymbol = (aiSymbol == 0) ? 1 : 0;

    // 1. Try to win
    Move winMove = findWinningMove(b, aiSymbol);
    if (winMove.getI() != -1)
      return winMove;

    // 2. Try to block player
    Move blockMove = findWinningMove(b, playerSymbol);
    if (blockMove.getI() != -1)
      return Move(blockMove.getI(), blockMove.getJ(), aiSymbol);

    // 3. Else pick first free
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j)
        if (b.get(i, j) == -1)
          return Move(i, j, aiSymbol);

    return Move(0, 0, aiSymbol);
  }

 private:
  Move findWinningMove(const Board& b, int symbol) {
    for (int i = 0; i < 3; ++i) {
      int count = 0, empty = -1;
      for (int j = 0; j < 3; ++j) {
        if (b.get(i, j) == symbol) count++;
        else if (b.get(i, j) == -1) empty = j;
      }
      if (count == 2 && empty != -1)
        return Move(i, empty, symbol);
    }
    for (int j = 0; j < 3; ++j) {
      int count = 0, empty = -1;
      for (int i = 0; i < 3; ++i) {
        if (b.get(i, j) == symbol) count++;
        else if (b.get(i, j) == -1) empty = i;
      }
      if (count == 2 && empty != -1)
        return Move(empty, j, symbol);
    }

    int count = 0, empty = -1;
    for (int i = 0; i < 3; ++i) {
      if (b.get(i, i) == symbol) count++;
      else if (b.get(i, i) == -1) empty = i;
    }
    if (count == 2 && empty != -1)
      return Move(empty, empty, symbol);

    count = 0; empty = -1;
    for (int i = 0; i < 3; ++i) {
      if (b.get(i, 2 - i) == symbol) count++;
      else if (b.get(i, 2 - i) == -1) empty = i;
    }
    if (count == 2 && empty != -1)
      return Move(empty, 2 - empty, symbol);

    return Move(-1, -1, symbol);
  }
};

class Game {
 public:
  Game(bool twoPlayers, int difficulty = 1, string name1 = "Player1", char symbol1 = 'X', string name2 = "Player2")
    : board(3, 3), status(IN_PROGRESS), isTwoPlayers(twoPlayers),
      player1(name1, symbol1), player2(name2, symbol1 == 'X' ? 'O' : 'X') {

    if (!twoPlayers) {
      int aiSymbol = (symbol1 == 'O') ? 1 : 0;
      pAI = (difficulty == 1) ? static_cast<AI*>(new DumbAI(aiSymbol)) : static_cast<AI*>(new CleverAI(aiSymbol));
    } else {
      pAI = nullptr;
    }
  }

  void makeMovePlayer(bool first) {
    if (first)
      player1.chooseMove(board);
    else
      player2.chooseMove(board);
  }

  void makeMoveAI() {
    Move m = pAI->chooseMove(board);
    board.set(m);
  }

  GameStatus getStatus() const { return status; }
  void setStatus(GameStatus gameStatus) { status = gameStatus; }
  const Board& getBoard() const { return board; }
  void printBoard() const { board.print(); }

  ~Game() { delete pAI; }

  int victory() {
    for (int i = 0; i < 3; ++i) {
      if (board.get(i, 0) != -1 && board.get(i, 0) == board.get(i, 1) && board.get(i, 1) == board.get(i, 2))
        return board.get(i, 0);
      if (board.get(0, i) != -1 && board.get(0, i) == board.get(1, i) && board.get(1, i) == board.get(2, i))
        return board.get(0, i);
    }
    if (board.get(0, 0) != -1 && board.get(0, 0) == board.get(1, 1) && board.get(1, 1) == board.get(2, 2))
      return board.get(0, 0);
    if (board.get(0, 2) != -1 && board.get(0, 2) == board.get(1, 1) && board.get(1, 1) == board.get(2, 0))
      return board.get(0, 2);
    return -1;
  }

  bool isTwoPlayerGame() const { return isTwoPlayers; }

 private:
  Board board;
  Player player1, player2;
  AI* pAI;
  GameStatus status;
  bool isTwoPlayers;
};

class ConsoleRunner {
 public:
  ConsoleRunner(bool twoPlayers, int difficulty = 1, string name1 = "Player1", char symbol = 'X', string name2 = "Player2")
    : game(twoPlayers, difficulty, name1, symbol, name2) {}

  void play() {
    bool playerTurn = true;
    while (!game.getBoard().full() && game.victory() == -1) {
      game.printBoard();
      if (game.isTwoPlayerGame()) {
        game.makeMovePlayer(playerTurn);
      } else {
        if (playerTurn)
          game.makeMovePlayer(true);
        else
          game.makeMoveAI();
      }
      playerTurn = !playerTurn;
    }

    game.printBoard();
    if (game.getBoard().full() && game.victory() == -1)
      game.setStatus(DRAW);
    else if (game.victory() == 0)
      game.setStatus(O_WON);
    else if (game.victory() == 1)
      game.setStatus(X_WON);
  }

  void printResult() const {
    if (game.getStatus() == DRAW)
      cout << "Draw!\n";
    else if (game.getStatus() == O_WON)
      cout << "O won!\n";
    else
      cout << "X won!\n";
  }

 private:
  Game game;
};

int main() {
  int mode;
  cout << "Wybierz tryb gry:\n1 - Gracz vs AI\n2 - Gracz vs Gracz\n";
  cin >> mode;

  if (mode == 1) {
    int difficulty;
    cout << "Wybierz tryb AI (1 - slaby, 2 - inteligentny): ";
    cin >> difficulty;
    ConsoleRunner runner(false, difficulty, "Gracz", 'X');
    runner.play();
    runner.printResult();
  } else {
    ConsoleRunner runner(true, 1, "Gracz1", 'X', "Gracz2");
    runner.play();
    runner.printResult();
  }

  return 0;
}
