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

// ��������ö��
typedef enum {
    EMPTY,
    PAWN,   // ��
    ROOK,   // ��
    KNIGHT, // ��
    BISHOP, // ��
    QUEEN,  // ��
    KING    // ��
} PieceType;

// ������ɫö��
typedef enum
{
    white,
    black
}PieceColor;

// ��Ϸ״̬ö��
typedef enum {
    NORMAL,    // ������Ϸ״̬
    CHECK,     // ������״̬
    CHECKMATE, // ������״̬
    STALEMATE  // �ƺ�״̬
}GameState;

typedef enum {
    SINGLE_PLAYER,  // �˻���ս
    TWO_PLAYERS,    // ˫�˶�ս
    REPLAY_MODE     // �ط�ģʽ
}GameMode;

typedef struct MoveRecord MoveRecord;
typedef struct ChessPiece ChessPiece;
typedef struct ChessAI ChessAI;
typedef struct ChessBoard ChessBoard;
typedef struct GameRecord GameRecord;
typedef struct GameReview GameReview;
typedef struct GameNotation GameNotation;
typedef struct PossibleMoves PossibleMoves;

// ��Ϸ�߷���¼�ṹ��
struct MoveRecord {
    int fromX, fromY, toX, toY;
    PieceType movedPiece;
    PieceType capturedPiece;
};

// ���ӽṹ��
struct ChessPiece {
    PieceType type;
    PieceColor color;
    int hasMoved; // �����Ƿ��ƶ���(������������ж�)
};

struct ChessAI {
    int depth; // �������
    float weights[6]; // ����Ȩ��
    float learningRate;
    int experience;
    int difficulty; // �Ѷȵȼ�(1 - 5)
    PieceColor color;
};

// ���̽ṹ��
// ��ChessBoard�ṹ�������
struct ChessBoard {
    ChessPiece board[8][8];
    std::vector<std::pair<int, int>> whitePieces;  // �׷�����λ��
    std::vector<std::pair<int, int>> blackPieces;  // �ڷ�����λ��
    std::pair<int, int> whiteKingPos;  // ����λ��
    std::pair<int, int> blackKingPos;  // ����λ��
    int lastPawnJumpX;
    int lastPawnJumpY;
    int halfMoveClock;
    GameMode mode;
    ChessAI ai;
    vector<ChessBoard> history; // ��ʷ��¼
    vector<ChessBoard> redoHistory; // ������ʷ��¼
    int hintCount; // ��ʾ����
    int moveCount; // �ƶ�����
    int lastMove[4];  // ��¼��һ���ƶ�
    GameNotation* notation;
    RECT dirtyRect; // ���������
    bool needRedraw; // �Ƿ���Ҫ�ػ��־
};


// ��Ϸ��¼�ṹ��
struct GameRecord {
    ChessBoard board;
    int moveCount;
    // ������Ҫ��¼��״̬
};

struct GameReview {
    vector < ChessBoard > boards;  // ��¼ÿһ��������״̬
    int currentView;           // ��ǰ�鿴�Ĳ���
    int moveCount;             // �ܲ���
};

// ��Ϸ���׽ṹ��
struct GameNotation {
    ChessBoard board;
    vector<MoveRecord> moves;  // ��¼�����߷�
    int currentMove;           // ��ǰ�鿴�Ĳ���
};

struct PossibleMoves {
    int count;
    int moves[28][2]; // ���28������λ��(��)
};

// ��������
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

//����