#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<stdlib.h>
#include <easyx.h>
#include <graphics.h>
#include <conio.h>
#include <vector>
#include <math.h>
#include <windows.h>
#include<assert.h>

using namespace std;

#define MAX_UNDO 5
#define MAX_HINTS 3
#define MAX_HISTORY 100

// 棋子类型枚举
typedef enum {
    EMPTY,
    PAWN,   // 兵
    ROOK,   // 车
    KNIGHT, // 马
    BISHOP, // 象
    QUEEN,  // 后
    KING    // 王
} PieceType;

// 棋子颜色枚举
typedef enum
{
    white,
    black
}PieceColor;

// 游戏状态枚举
typedef enum {
    NORMAL,    // 正常游戏状态
    CHECK,     // 被将军状态
    CHECKMATE, // 被将死状态
    STALEMATE  // 逼和状态
}GameState;

typedef enum {
    SINGLE_PLAYER,  // 人机对战
    TWO_PLAYERS,    // 双人对战
    REPLAY_MODE     // 回放模式
}GameMode;

typedef struct MoveRecord MoveRecord;
typedef struct ChessPiece ChessPiece;
typedef struct ChessAI ChessAI;
typedef struct ChessBoard ChessBoard;
typedef struct GameRecord GameRecord;
typedef struct GameReview GameReview;
typedef struct GameNotation GameNotation;
typedef struct PossibleMoves PossibleMoves;

// 游戏走法记录结构体
struct MoveRecord {
    int fromX, fromY, toX, toY;
    PieceType movedPiece;
    PieceType capturedPiece;
};

// 棋子结构体
struct ChessPiece {
    PieceType type;
    PieceColor color;
    int hasMoved; // 棋子是否移动过(用于特殊规则判断)
};

struct ChessAI {
    int depth; // 搜索深度
    float weights[6]; // 评估权重
    float learningRate;
    int experience;
    int difficulty; // 难度等级(1 - 5)
    PieceColor color;
};

// 棋盘结构体
// 在ChessBoard结构体中添加
struct ChessBoard {
    ChessPiece board[8][8];
    std::vector<std::pair<int, int>> whitePieces;  // 白方棋子位置
    std::vector<std::pair<int, int>> blackPieces;  // 黑方棋子位置
    std::pair<int, int> whiteKingPos;  // 白王位置
    std::pair<int, int> blackKingPos;  // 黑王位置
    int lastPawnJumpX;
    int lastPawnJumpY;
    int halfMoveClock;
    GameMode mode;
    ChessAI ai;
    vector<ChessBoard> history; // 历史记录
    vector<ChessBoard> redoHistory; // 重做历史记录
    int hintCount; // 提示次数
    int moveCount; // 移动次数
    int lastMove[4];  // 记录上一次移动
    GameNotation* notation;
    RECT dirtyRect; // 脏矩形区域
    bool needRedraw; // 是否需要重绘标志
};


// 游戏记录结构体
struct GameRecord {
    ChessBoard board;
    int moveCount;
    // 其他需要记录的状态
};

struct GameReview {
    vector < ChessBoard > boards;  // 记录每一步的棋盘状态
    int currentView;           // 当前查看的步数
    int moveCount;             // 总步数
};

// 游戏记谱结构体
struct GameNotation {
    ChessBoard board;
    vector<MoveRecord> moves;  // 记录所有走法
    int currentMove;           // 当前查看的步数
};

struct PossibleMoves {
    int count;
    int moves[28][2]; // 最多28个可能位置(后)
};

// 函数声明
void initBoard(ChessBoard* board);
bool isValidSelection(ChessBoard* board, int x, int y);
int isPathBlocked(ChessBoard* board, int fromX, int fromY, int toX, int toY);
int isKingInCheck(ChessBoard* board, PieceColor kingColor);
int canAttack(ChessBoard* board, int fromX, int fromY, int toX, int toY);
int hasAnyLegalMove(ChessBoard* board, int fromX, int fromY);
int isValidMove(ChessBoard* board, int fromX, int fromY, int toX, int toY, bool debugPrint = true);
GameState checkGameState(ChessBoard* board);
void processMove(ChessBoard* board, int fromX, int fromY, int toX, int toY, bool isAIMove = false);
void initGUI();
void drawBoard(ChessBoard* board, RECT* dirtyRect);
bool RectInRect(const RECT* rect1, const RECT* rect2);
void reviewGame(ChessBoard* board, GameReview* review);
void editBoard(ChessBoard* board);
void highlightMoves(ChessBoard* board, int fromX, int fromY);
void checkAndHighlight(ChessBoard* board, int fromX, int fromY, int toX, int toY);
void undoMove(ChessBoard* board);
void showNotation(ChessBoard* board);
void showGameOver(GameState state);
void getPossibleMoves(ChessBoard* board, int fromX, int fromY, PossibleMoves* result);

float evaluateBoard(ChessBoard* board, PieceColor color);
float minimax(ChessBoard* board, int depth, bool isMaximizing, float alpha, float beta);
bool initAI(ChessAI* ai, int difficulty = 3);
void trainAI(ChessAI* ai, float reward);
void AIMove(ChessBoard* board, int* bestMove);
void showHint(ChessBoard* board);
void showEvaluation(ChessBoard* board);
void highlightTarget(ChessBoard* board, int toX, int toY);
int showDifficultyMenu();
void saveGameRecord(GameRecord* record);
bool loadGameRecord(GameRecord* record);
int showDifficultyMenu();

extern PieceColor currentPlayer;

//结束