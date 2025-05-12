

#include "all.h"
const TCHAR* getPieceNameCN(PieceType type);

// 全局变量
PieceColor currentPlayer = white;

// 初始化棋盘
void initBoard(ChessBoard *board)
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            board->board[i][j].type = EMPTY;
            board->board[i][j].color = white;
            board->board[i][j].hasMoved = 0;
        }
    }

    // 初始化白方棋子
    board->board[0][0] = {ROOK, white, 0};
    board->board[0][1] = {KNIGHT, white, 0};
    board->board[0][2] = {BISHOP, white, 0};
    board->board[0][3] = {QUEEN, white, 0};
    board->board[0][4] = {KING, white, 0};
    board->board[0][5] = {BISHOP, white, 0};
    board->board[0][6] = {KNIGHT, white, 0};
    board->board[0][7] = {ROOK, white, 0};

    for (int i = 0; i < 8; i++)
    {
        board->board[1][i] = {PAWN, white, 0};
    }

    // 初始化黑方棋子
    board->board[7][0] = {ROOK, black, 0};
    board->board[7][1] = {KNIGHT, black, 0};
    board->board[7][2] = {BISHOP, black, 0};
    board->board[7][3] = {QUEEN, black, 0};
    board->board[7][4] = {KING, black, 0};
    board->board[7][5] = {BISHOP, black, 0};
    board->board[7][6] = {KNIGHT, black, 0};
    board->board[7][7] = {ROOK, black, 0};

    for (int i = 0; i < 8; i++)
    {
        board->board[6][i] = {PAWN, black, 0};
    }

    // 清空棋子位置记录
    board->whitePieces.clear();
    board->blackPieces.clear();

    // 初始化白方棋子位置
    board->whiteKingPos = {0, 4};
    board->whitePieces.push_back({0, 0}); // 车
    board->whitePieces.push_back({0, 1}); // 马
    board->whitePieces.push_back({0, 2}); // 象
    board->whitePieces.push_back({0, 3}); // 后
    board->whitePieces.push_back({0, 4}); // 王
    board->whitePieces.push_back({0, 5}); // 象
    board->whitePieces.push_back({0, 6}); // 马
    board->whitePieces.push_back({0, 7}); // 车
    for (int i = 0; i < 8; i++)
    {
        board->whitePieces.push_back({1, i}); // 兵
    }

    // 初始化黑方棋子位置
    board->blackKingPos = {7, 4};
    board->blackPieces.push_back({7, 0}); // 车
    board->blackPieces.push_back({7, 1}); // 马
    board->blackPieces.push_back({7, 2}); // 象
    board->blackPieces.push_back({7, 3}); // 后
    board->blackPieces.push_back({7, 4}); // 王
    board->blackPieces.push_back({7, 5}); // 象
    board->blackPieces.push_back({7, 6}); // 马
    board->blackPieces.push_back({7, 7}); // 车
    for (int i = 0; i < 8; i++)
    {
        board->blackPieces.push_back({6, i}); // 兵
    }

    // 初始化其他棋盘状态
    board->lastPawnJumpX = -1;
    board->lastPawnJumpY = -1;
    board->halfMoveClock = 0;
    board->history.clear();
    board->hintCount = 0;
    board->moveCount = 0;
    board->notation = nullptr;
    if (!board->notation)
    {
        board->notation = new GameNotation;
        board->notation->currentMove = 0;
        board->notation->moves.clear();
    }
}

bool isValidSelection(ChessBoard *board, int x, int y)
{
    // 检查坐标是否在棋盘范围内
    if (x < 0 || x >= 8 || y < 0 || y >= 8)
    {
        return false;
    }

    // 检查是否选择了棋子(非空)且颜色是当前玩家的
    ChessPiece *piece = &board->board[x][y];
    return (piece->type != EMPTY && piece->color == currentPlayer);
}

// 检查移动路径是否被阻挡
int isPathBlocked(ChessBoard *board, int fromX, int fromY, int toX, int toY)
{
    int dx = (toX > fromX) ? 1 : (toX < fromX) ? -1
                                               : 0;
    int dy = (toY > fromY) ? 1 : (toY < fromY) ? -1
                                               : 0;

    int x = fromX + dx;
    int y = fromY + dy;

    while (x != toX || y != toY)
    {
        if (board->board[x][y].type != EMPTY)
        {
            return 1;
        }
        x += dx;
        y += dy;
    }

    return 0;
}

// 检查王是否被将军
int isKingInCheck(ChessBoard *board, PieceColor kingColor)
{
    // 直接获取王的位置
    int kingX, kingY;
    if (kingColor == white)
    {
        kingX = board->whiteKingPos.first;
        kingY = board->whiteKingPos.second;
    }
    else
    {
        kingX = board->blackKingPos.first;
        kingY = board->blackKingPos.second;
    }

    // 直接遍历对方棋子
    const auto &opponentPieces = (kingColor == white) ? board->blackPieces : board->whitePieces;
    for (const auto &pos : opponentPieces)
    {
        int i = pos.first;
        int j = pos.second;
        ChessPiece *piece = &board->board[i][j];
        if (canAttack(board, i, j, kingX, kingY))
        {
            return 1;
        }
    }

    return 0;
}

int canAttack(ChessBoard *board, int fromX, int fromY, int toX, int toY)
{
    ChessPiece *piece = &board->board[fromX][fromY];

    // 根据棋子类型检查攻击规则
    switch (piece->type)
    {
    case PAWN:
    {
        int direction = (piece->color == white) ? 1 : -1;
        return (abs(toY - fromY) == 1 && toX == fromX + direction);
    }
    case KNIGHT:
        return (abs(fromX - toX) == 2 && abs(fromY - toY) == 1) ||
               (abs(fromX - toX) == 1 && abs(fromY - toY) == 2);
    case BISHOP:
        return (abs(fromX - toX) == abs(fromY - toY)) &&
               !isPathBlocked(board, fromX, fromY, toX, toY);
    case ROOK:
        return (fromX == toX || fromY == toY) &&
               !isPathBlocked(board, fromX, fromY, toX, toY);
    case QUEEN:
        return (fromX == toX || fromY == toY || abs(fromX - toX) == abs(fromY - toY)) &&
               !isPathBlocked(board, fromX, fromY, toX, toY);
    case KING:
        return abs(fromX - toX) <= 1 && abs(fromY - toY) <= 1;
    default:
        return false;
    }
}

// 检查棋子是否有任何合法移动
int hasAnyLegalMove(ChessBoard* board, int fromX, int fromY)
{
    ChessPiece* piece = &board->board[fromX][fromY];

    // 如果是王，直接检查周围8个方向
    if (piece->type == KING)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            for (int dy = -1; dy <= 1; dy++)
            {
                if (dx == 0 && dy == 0)
                    continue;

                int toX = fromX + dx;
                int toY = fromY + dy;

                if (toX >= 0 && toX < 8 && toY >= 0 && toY < 8)
                {
                    if (isValidMove(board, fromX, fromY, toX, toY, false))
                    {
                        ChessBoard tempBoard = *board;
                        tempBoard.board[toX][toY] = *piece;
                        tempBoard.board[fromX][fromY].type = EMPTY;

                        // 更新王的位置
                        if (piece->color == white)
                        {
                            tempBoard.whiteKingPos = { toX, toY };
                        }
                        else
                        {
                            tempBoard.blackKingPos = { toX, toY };
                        }

                        if (!isKingInCheck(&tempBoard, piece->color))
                        {
                            return 1;
                        }
                    }
                }
            }
        }
        // 获取对方棋子位置列表
        const auto& opponentPieces = (piece->color == white) ? board->blackPieces : board->whitePieces;
        for (const auto& pos : opponentPieces)
        {
            if (canAttack(board, fromX, fromY, pos.first, pos.second))
            {
                ChessBoard tempBoard = *board;
                tempBoard.board[pos.first][pos.second] = tempBoard.board[fromX][fromY];
                tempBoard.board[fromX][fromY].type = EMPTY;
                if (!isKingInCheck(&tempBoard, piece->color))
                {
                    return 1;
                }
            }
        }
        return 0;
    }

    // 其他棋子使用棋子位置数组优化
    const auto& myPieces = (piece->color == white) ? board->whitePieces : board->blackPieces;
    for (const auto& pos : myPieces)
    {
        if (pos.first == fromX && pos.second == fromY)
        {
            // 获取所有可能移动
            PossibleMoves moves = { 0 };
            getPossibleMoves(board, fromX, fromY, &moves);

            // 检查每个可能的移动
            for (int i = 0; i < moves.count; i++)
            {
                int toX = moves.moves[i][0];
                int toY = moves.moves[i][1];

                if (isValidMove(board, fromX, fromY, toX, toY, false))
                {
                    ChessBoard tempBoard = *board;
                    tempBoard.board[toX][toY] = tempBoard.board[fromX][fromY];
                    tempBoard.board[fromX][fromY].type = EMPTY;

                    if (!isKingInCheck(&tempBoard, piece->color))
                    {
                        return 1;
                    }
                }
            }
            break;
        }
    }
    return 0;
}

int isValidMove(ChessBoard *board, int fromX, int fromY, int toX, int toY, bool debugPrint)
{
    if (debugPrint)
    {
        printf("[DEBUG] 详细检查移动 (%d,%d)->(%d,%d)\n", fromX, fromY, toX, toY);
        printf("[DEBUG] 棋子类型: %d, 颜色: %d\n",
               board->board[fromX][fromY].type,
               board->board[fromX][fromY].color);
    }

    // 先检查基本移动有效性
    if (fromX < 0 || fromX > 7 || fromY < 0 || fromY > 7 ||
        toX < 0 || toX > 7 || toY < 0 || toY > 7)
    {
        return 0;
    }

    ChessPiece *srcPiece = &board->board[fromX][fromY];
    ChessPiece *destPiece = &board->board[toX][toY];

    if (srcPiece->type == EMPTY)
    {
        return 0;
    }

    if (destPiece->type != EMPTY && destPiece->color == srcPiece->color)
    {
        return 0;
    }

    // 创建临时棋盘状态
    ChessBoard tempBoard = *board;
    tempBoard.board[toX][toY] = *srcPiece;
    tempBoard.board[fromX][fromY].type = EMPTY;

    if (srcPiece->type == KING)
    {
        if (srcPiece->color == white)
        {
            tempBoard.whiteKingPos = {toX, toY};
        }
        else
        {
            tempBoard.blackKingPos = {toX, toY};
        }
    }

    // 检查移动后是否会导致自己的王被将军
    if (isKingInCheck(&tempBoard, srcPiece->color))
    {
        printf("[DEBUG] 移动会导致将军，无效移动\n");
        return 0;
    }

    switch (srcPiece->type)
    {
    case PAWN:
    {
        printf("[DEBUG] 检查兵移动规则\n");
        int direction = (srcPiece->color == white) ? 1 : -1;

        // 1. 检查基本向前移动
        if (toY == fromY)
        {
            // 向前移动一步
            if (toX == fromX + direction && destPiece->type == EMPTY)
            {
                return 1;
            }
            // 初始位置可以向前两格
            if (!srcPiece->hasMoved && toX == fromX + 2 * direction &&
                destPiece->type == EMPTY && board->board[fromX + direction][fromY].type == EMPTY)
            {
                board->lastPawnJumpX = toX;
                board->lastPawnJumpY = toY;
                return 1;
            }
        }
        // 2. 检查吃子（包括吃过路兵）
        else if (abs(toY - fromY) == 1 && toX == fromX + direction)
        {
            // 正常吃子
            if (destPiece->type != EMPTY && destPiece->color != srcPiece->color)
            {
                return 1;
            }
            // 吃过路兵
            if (destPiece->type == EMPTY &&
                fromY != toY &&
                board->lastPawnJumpX == fromX &&
                board->lastPawnJumpY == toY)
            {
                ChessPiece *enPassantPawn = &board->board[fromX][toY];
                if (enPassantPawn->type == PAWN && enPassantPawn->color != srcPiece->color)
                {
                    return 1;
                }
            }
            break;
        }
    }

    case ROOK:
    {
        printf("[DEBUG] 检查车移动规则\n");
        if ((fromX == toX || fromY == toY) && !isPathBlocked(board, fromX, fromY, toX, toY))
        {
            return 1;
        }
        break;
    }

    case KNIGHT:
    {
        printf("[DEBUG] 检查马移动规则\n");
        if ((abs(fromX - toX) == 2 && abs(fromY - toY) == 1) ||
            (abs(fromX - toX) == 1 && abs(fromY - toY) == 2))
        {
            return 1;
        }
        break;
    }

    case BISHOP:
    {
        printf("[DEBUG] 检查象移动规则\n");
        if (abs(fromX - toX) == abs(fromY - toY) && !isPathBlocked(board, fromX, fromY, toX, toY))
        {
            return 1;
        }
        break;
    }

    case QUEEN:
    {
        printf("[DEBUG] 检查后移动规则\n");
        if ((fromX == toX || fromY == toY || abs(fromX - toX) == abs(fromY - toY)) &&
            !isPathBlocked(board, fromX, fromY, toX, toY))
        {
            return 1;
        }
        break;
    }

    case KING:
    {
        printf("[DEBUG] 检查王移动规则\n");
        // 普通移动
        if (abs(fromX - toX) <= 1 && abs(fromY - toY) <= 1)
        {
            return 1;
        }

        // 王车易位判断
        if (!srcPiece->hasMoved && abs(fromY - toY) == 2 && fromX == toX)
        {
            // 短易位(王翼)
            if (toY == 6)
            {
                ChessPiece *rook = &board->board[fromX][7];
                if (rook->type == ROOK && !rook->hasMoved)
                {
                    bool pathClear = true;
                    for (int y = 5; y < 7; y++)
                    {
                        if (board->board[fromX][y].type != EMPTY)
                        {
                            pathClear = false;
                            break;
                        }
                    }
                    if (pathClear && !isKingInCheck(board, srcPiece->color))
                    {
                        // 检查王经过的格子是否被将军
                        ChessBoard tempBoard = *board;
                        tempBoard.board[fromX][5] = *srcPiece;
                        tempBoard.board[fromX][fromY].type = EMPTY;
                        if (!isKingInCheck(&tempBoard, srcPiece->color))
                        {
                            return 1;
                        }
                    }
                }
            }
            // 长易位(后翼)
            else if (toY == 2)
            {
                ChessPiece *rook = &board->board[fromX][0];
                if (rook->type == ROOK && !rook->hasMoved)
                {
                    bool pathClear = true;
                    for (int y = 1; y < 4; y++)
                    {
                        if (board->board[fromX][y].type != EMPTY)
                        {
                            pathClear = false;
                            break;
                        }
                    }
                    if (pathClear && !isKingInCheck(board, srcPiece->color))
                    {
                        // 检查王经过的格子是否被将军
                        ChessBoard tempBoard = *board;
                        tempBoard.board[fromX][3] = *srcPiece;
                        tempBoard.board[fromX][fromY].type = EMPTY;
                        if (!isKingInCheck(&tempBoard, srcPiece->color))
                        {
                            return 1;
                        }
                    }
                }
            }
        }
        break;
    }
    }

    printf("[DEBUG] 未匹配任何有效移动规则\n");
    return 0;
}

void processMove(ChessBoard *board, int fromX, int fromY, int toX, int toY, bool isAIMove)
{
    if (board->notation == nullptr)
    {
        board->notation = new GameNotation();
    }
    if (!isAIMove)
    {
        printf("[PROCESS] 处理玩家移动 (%d,%d) -> (%d,%d)\n", fromX, fromY, toX, toY);
    }

    ChessPiece *srcPiece = &board->board[fromX][fromY];
    ChessPiece *destPiece = &board->board[toX][toY];

    // 更新棋子位置记录
    auto &myPieces = (srcPiece->color == white) ? board->whitePieces : board->blackPieces;
    auto &opponentPieces = (srcPiece->color == white) ? board->blackPieces : board->whitePieces;

    // 从原位置移除棋子
    myPieces.erase(
        std::remove(myPieces.begin(), myPieces.end(), std::make_pair(fromX, fromY)),
        myPieces.end());

    // 添加到新位置
    myPieces.push_back({toX, toY});

    // 如果是王则更新王的位置
    if (srcPiece->type == KING)
    {
        if (srcPiece->color == white)
        {
            board->whiteKingPos = {toX, toY};
        }
        else
        {
            board->blackKingPos = {toX, toY};
        }
    }

    // 处理吃子
    if (destPiece->type != EMPTY)
    {
        opponentPieces.erase(
            std::remove(opponentPieces.begin(), opponentPieces.end(), std::make_pair(toX, toY)),
            opponentPieces.end());
    }

    // 记录移动历史
    board->history.push_back(*board);
    if (board->history.size() > MAX_HISTORY)
    {
        board->history.erase(board->history.begin());
    }

    // 执行移动
    board->board[toX][toY] = *srcPiece;
    board->board[fromX][fromY].type = EMPTY;
    board->board[toX][toY].hasMoved = 1;

    // 处理吃过路兵
    if (srcPiece->type == PAWN && destPiece->type == EMPTY &&
        fromY != toY && board->lastPawnJumpX == toX && board->lastPawnJumpY == toY)
    {
        board->board[fromX][toY].type = EMPTY;
    }

    // 兵升变检查
    if (board->board[toX][toY].type == PAWN && (toX == 0 || toX == 7))
    {
        if (!isAIMove)
        {
            // 保存当前棋盘状态
            ChessBoard tempBoard = *board;

            // 绘制升变选择界面
            setfillcolor(RGB(240, 240, 240));
            fillrectangle(250, 250, 550, 450);
            settextstyle(20, 0, _T("宋体"));
            settextcolor(black);
            outtextxy(280, 260, _T("请选择兵升变的棋子类型:"));

            // 绘制选项按钮
            setfillcolor(RGB(200, 200, 200));
            fillrectangle(270, 300, 530, 340); // 后
            outtextxy(280, 310, _T("皇后 (Q)"));

            fillrectangle(270, 350, 530, 390); // 车
            outtextxy(280, 360, _T("车 (R)"));

            fillrectangle(270, 400, 530, 440); // 象
            outtextxy(280, 410, _T("象 (B)"));

            fillrectangle(270, 450, 530, 490); // 马
            outtextxy(280, 460, _T("马 (N)"));

            // 等待玩家选择
            bool selected = false;
            PieceType promoteTo = EMPTY;

            while (!selected)
            {
                if (MouseHit())
                {
                    MOUSEMSG msg = GetMouseMsg();
                    if (msg.uMsg == WM_LBUTTONDOWN)
                    {
                        if (msg.x >= 270 && msg.x <= 530)
                        {
                            if (msg.y >= 300 && msg.y <= 340)
                            {
                                promoteTo = QUEEN;
                                selected = true;
                            }
                            else if (msg.y >= 350 && msg.y <= 390)
                            {
                                promoteTo = ROOK;
                                selected = true;
                            }
                            else if (msg.y >= 400 && msg.y <= 440)
                            {
                                promoteTo = BISHOP;
                                selected = true;
                            }
                            else if (msg.y >= 450 && msg.y <= 490)
                            {
                                promoteTo = KNIGHT;
                                selected = true;
                            }
                        }
                    }
                }
            }

            // 执行升变
            board->board[toX][toY].type = promoteTo;

            // 更新棋子位置记录 - 从兵的位置列表中移除
            auto &myPieces = (srcPiece->color == white) ? board->whitePieces : board->blackPieces;
            myPieces.erase(
                std::remove(myPieces.begin(), myPieces.end(), std::make_pair(toX, toY)),
                myPieces.end());
            // 添加新棋子到位置列表
            myPieces.push_back({toX, toY});
            // 设置重绘标志
            board->needRedraw = true;

            // 重新绘制棋盘
            drawBoard(board, &board->dirtyRect);
        }
        else
        {
            // AI自动升变为皇后
            board->board[toX][toY].type = QUEEN;
        }
    }

    // 更新上次兵跳信息
    if (srcPiece->type == PAWN && abs(fromX - toX) == 2)
    {
        board->lastPawnJumpX = toX;
        board->lastPawnJumpY = toY;
    }
    else
    {
        board->lastPawnJumpX = -1;
        board->lastPawnJumpY = -1;
    }

    // 切换玩家
    currentPlayer = (currentPlayer == white) ? black : white;
    board->moveCount++;

    // 记录棋步
    if (board->notation)
    {
        MoveRecord record = {
            fromX, fromY, toX, toY,
            srcPiece->type,
            destPiece->type};
        board->notation->moves.push_back(record);
        board->notation->currentMove++;
    }

    // 检查游戏状态
    GameState state = checkGameState(board);
    if (state != NORMAL)
    {
        showGameOver(state);
    }

    // 如果是人机对战且不是AI移动，则触发AI回合
    if (board->mode == SINGLE_PLAYER && !isAIMove && currentPlayer == board->ai.color)
    {
        int bestMove[4] = {0};   // 声明存储最佳移动的数组（关键修正）
        AIMove(board, bestMove); // 传递数组指针给AIMove

        // 执行AI的最佳移动（需要调用processMove）
        if (bestMove[2] != 0 || bestMove[3] != 0)
        {                                                                                 // 检查是否找到有效移动
            processMove(board, bestMove[0], bestMove[1], bestMove[2], bestMove[3], true); // isAIMove设为true
        }
    }
    if (board->notation)
    {
        MoveRecord record = {
            fromX, fromY, toX, toY,
            srcPiece->type,
            destPiece->type};
        board->notation->moves.push_back(record);
        board->notation->currentMove++;
    }
    board->lastMove[0] = fromX;
    board->lastMove[1] = fromY;
    board->lastMove[2] = toX;
    board->lastMove[3] = toY;
}

// 初始化图形界面
void initGUI()
{
    initgraph(1000, 800);           // 修改窗口大小为1000x800
    setbkcolor(RGB(240, 240, 240)); // 设置背景色为浅灰色
    cleardevice();
    settextstyle(20, 0, _T("宋体"));
}

// 绘制棋盘
void drawBoard(ChessBoard *board, RECT *dirtyRect)
{
    // 如果没有指定脏矩形，则绘制整个棋盘
    bool fullRedraw = (dirtyRect == nullptr ||
                       (dirtyRect->left == 0 && dirtyRect->top == 0 &&
                        dirtyRect->right == 0 && dirtyRect->bottom == 0));

    // 绘制棋盘背景 - 修改为80x80像素每格，从(100,50)开始
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            RECT cellRect = {j * 80 + 100, i * 80 + 50, (j + 1) * 80 + 100, (i + 1) * 80 + 50};

            // 只绘制在脏矩形区域内的格子
            if (fullRedraw || RectInRect(&cellRect, dirtyRect))
            {
                // 交替绘制棋盘格子颜色
                setfillcolor((i + j) % 2 ? RGB(210, 180, 140) : RGB(180, 140, 90)); // 浅木色和深木色
                fillrectangle(cellRect.left, cellRect.top, cellRect.right, cellRect.bottom);

                // 绘制棋子（如果有）
                if (board->board[i][j].type != EMPTY)
                {
                    // 修改棋子颜色设置
                    settextcolor(board->board[i][j].color == white ? RGB(255, 255, 255) : RGB(0, 0, 0)); // 白棋用纯白色，黑棋用纯黑色

                    // 获取棋子符号
                    TCHAR pieceText[2] = {0};
                    switch (board->board[i][j].type)
                    {
                    case KING:
                        _tcscpy(pieceText, _T("K"));
                        break;
                    case QUEEN:
                        _tcscpy(pieceText, _T("Q"));
                        break;
                    case ROOK:
                        _tcscpy(pieceText, _T("R"));
                        break;
                    case BISHOP:
                        _tcscpy(pieceText, _T("B"));
                        break;
                    case KNIGHT:
                        _tcscpy(pieceText, _T("N"));
                        break;
                    case PAWN:
                        _tcscpy(pieceText, _T("P"));
                        break;
                    }

                    // 居中绘制棋子符号
                    int textX = cellRect.left + (cellRect.right - cellRect.left - textwidth(pieceText)) / 2;
                    int textY = cellRect.top + (cellRect.bottom - cellRect.top - textheight(pieceText)) / 2;
                    outtextxy(textX, textY, pieceText);
                }
            }
        }
    }

    // 绘制右侧控制面板 (200x800)
    setfillcolor(RGB(240, 240, 240));
    fillrectangle(800, 0, 1000, 800);

    // 绘制按钮区域
    setfillcolor(RGB(200, 200, 200));
    fillrectangle(820, 100, 980, 140); // 悔棋按钮
    fillrectangle(820, 160, 980, 200); // 提示按钮
    fillrectangle(820, 220, 980, 260); // 评估按钮
    fillrectangle(820, 280, 980, 320); // 记录按钮

    // 绘制按钮文字
    settextcolor(BLACK);
    settextstyle(20, 0, _T("宋体"));
    outtextxy(870, 110, _T("悔棋"));
    outtextxy(870, 170, _T("提示"));
    outtextxy(870, 230, _T("评估"));
    outtextxy(870, 290, _T("记录"));

    // 绘制状态信息
    TCHAR statusText[100];
    _stprintf(statusText, _T("回合: %s"), currentPlayer == white ? _T("白方") : _T("黑方"));
    outtextxy(820, 400, statusText);
}

bool RectInRect(const RECT *rect1, const RECT *rect2)
{
    return !(rect1->right < rect2->left ||
             rect1->left > rect2->right ||
             rect1->bottom < rect2->top ||
             rect1->top > rect2->bottom);
}

// 添加查看对局功能
void reviewGame(ChessBoard *board, GameReview *review)
{
    review->currentView = review->moveCount; // 从最后一步开始查看
    *board = review->boards[review->currentView];

    while (true)
    {
        drawBoard(board, &board->dirtyRect);

        if (MouseHit())
        {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN)
            {
                if (msg.x >= 700 && msg.x <= 780)
                {
                    if (msg.y >= 300 && msg.y <= 340)
                    {
                        // 退出查看模式
                        return;
                    }
                    else if (msg.y >= 350 && msg.y <= 390 && review->currentView > 0)
                    {
                        // 上一步
                        review->currentView--;
                        *board = review->boards[review->currentView];
                    }
                    else if (msg.y >= 400 && msg.y <= 440 && review->currentView < review->moveCount)
                    {
                        // 下一步
                        review->currentView++;
                        *board = review->boards[review->currentView];
                    }
                    else if (msg.y >= 450 && msg.y <= 490)
                    {
                        // 编辑当前局面
                        editBoard(board);
                        review->boards[review->currentView] = *board;
                    }
                }
            }
        }
    }
}

// 添加编辑棋盘功能
void editBoard(ChessBoard *board)
{
    PieceType selectedPiece = EMPTY;
    PieceColor selectedColor = white;

    // 绘制编辑工具栏
    setfillcolor(RGB(240, 240, 240));
    fillrectangle(0, 0, 800, 60);

    // 绘制棋子选择按钮
    PieceType pieces[] = {EMPTY, PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING};
    for (int i = 0; i < 7; i++)
    {
        fillrectangle(i * 50 + 100, 10, i * 50 + 140, 50);
        // 绘制棋子图标...
    }

    while (true)
    {
        if (MouseHit())
        {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN)
            {
                // 检查工具栏点击
                if (msg.y >= 10 && msg.y <= 50)
                {
                    for (int i = 0; i < 7; i++)
                    {
                        if (msg.x >= i * 50 + 100 && msg.x <= i * 50 + 140)
                        {
                            selectedPiece = pieces[i];
                            break;
                        }
                    }
                }

                // 棋盘点击处理
                int clickedX = (msg.y - 20) / 80;
                int clickedY = (msg.x - 80) / 80;

                if (clickedX >= 0 && clickedX < 8 && clickedY >= 0 && clickedY < 8)
                {
                    if (selectedPiece != EMPTY)
                    {
                        board->board[clickedX][clickedY].type = selectedPiece;
                        board->board[clickedX][clickedY].color = selectedColor;
                    }
                }

                // 切换颜色按钮
                if (msg.x >= 700 && msg.x <= 780 && msg.y >= 10 && msg.y <= 50)
                {
                    selectedColor = (selectedColor == white) ? black : white;
                }
            }
        }

        // 按ESC退出编辑模式
        if (GetAsyncKeyState(VK_ESCAPE))
        {
            break;
        }
    }
}

// 高亮可移动位置
void highlightMoves(ChessBoard *board, int fromX, int fromY)
{
    // 添加前置检查
    if (!board || fromX < 0 || fromX >= 8 || fromY < 0 || fromY >= 8)
    {
        return;
    }

    printf("[DEBUG] 开始高亮——1\n");
    printf("[DEBUG] 选中棋子位置: (%d,%d), 类型: %d\n", fromX, fromY, board->board[fromX][fromY].type);

    // 保存当前棋盘状态
    ChessBoard tempBoard = *board;

    // 先绘制棋盘
    drawBoard(board, nullptr);
    printf("[DEBUG] 开始高亮——2\n");

    // 设置高亮颜色
    setfillcolor(RGB(144, 238, 144, 128)); // 半透明浅绿色

    // 获取可能的移动位置
    PossibleMoves moves = {0};
    getPossibleMoves(board, fromX, fromY, &moves);
    int highlightCount = 0;

    for (int i = 0; i < moves.count; i++)
    {
        int toX = moves.moves[i][0];
        int toY = moves.moves[i][1];

        if (isValidMove(board, fromX, fromY, toX, toY, false))
        {
            fillrectangle(toY * 80 + 101, toX * 80 + 51,
                          (toY + 1) * 80 + 99, (toX + 1) * 80 + 49);
            highlightCount++;
        }
    }

    printf("[DEBUG] 共高亮 %d 个位置\n", highlightCount);
    *board = tempBoard;
}

// 新增辅助函数
void checkAndHighlight(ChessBoard *board, int fromX, int fromY, int toX, int toY)
{
    if (toX >= 0 && toX < 8 && toY >= 0 && toY < 8 &&
        (isValidMove(board, fromX, fromY, toX, toY, false)))
    {
        fillrectangle(toY * 80 + 101, toX * 80 + 51,
                      (toY + 1) * 80 + 99, (toX + 1) * 80 + 49);
    }
}

// 悔棋功能 - 修改后版本
void undoMove(ChessBoard *board)
{
    // 双重检查历史记录和棋谱记录
    if (!board->history.empty() && board->notation && !board->notation->moves.empty())
    {
        // 保存当前状态到重做历史
        board->redoHistory.push_back(*board);

        // 恢复上一步棋盘状态
        *board = board->history.back();
        board->history.pop_back();

        // 同步更新棋谱记录
        board->notation->moves.pop_back();
        board->notation->currentMove--;

        // 切换玩家回合
        currentPlayer = (currentPlayer == white) ? black : white;
    }
    else
    {
        printf("[ERROR] 无法悔棋：历史记录或棋谱为空！\n");
    }
}

void showNotation(ChessBoard *board)
{
    if (!board->notation)
        return;

    // 1. 保存当前界面状态
    IMAGE tempBuffer;
    getimage(&tempBuffer, 0, 0, getwidth(), getheight());

    // 2. 显式设置中文字体（避免被其他代码覆盖）
    LOGFONT oldFont;
    gettextstyle(&oldFont); // 保存原字体
    LOGFONT newFont = oldFont;
    _tcscpy(newFont.lfFaceName, _T("宋体"));
    newFont.lfHeight = 20;
    settextstyle(&newFont);

    // 3. 绘制记录窗口
    setfillcolor(RGB(240, 240, 240));
    fillrectangle(100, 100, 700, 600);
    settextcolor(BLACK);
    outtextxy(300, 110, _T("棋谱记录"));

    // 4. 显示棋谱（修复字符运算和变量引用）
    int start = max(0, board->notation->currentMove - 20);
    int yPos = 150;
    int maxX = getwidth(); // 获取窗口宽度

    for (int i = start; i < board->notation->currentMove; i++)
    {
        MoveRecord move = board->notation->moves[i];
        TCHAR moveText[50];
        wchar_t fromCol = L'a' + (move.fromY % 8);
        wchar_t toCol = L'a' + (move.toY % 8);
        _stprintf(
            moveText,
            _T("%d. %s %c%d → %c%d"),
            i + 1,
            getPieceNameCN(move.movedPiece),
            fromCol,
            8 - move.fromX,
            toCol,
            8 - move.toX);

        int textWidth = textwidth(moveText);
        int textX = (maxX - textWidth) / 2;
        outtextxy(textX, yPos, moveText);

        yPos += 30;

        // 如果yPos超出了窗口高度，则停止绘制
        if (yPos > 550)
            break;
    }

    // int start = max(0, board->notation->currentMove - 20);
    // int yPos = 150;
    // for (int i = start; i < board->notation->currentMove; i++)
    // {
    //     MoveRecord move = board->notation->moves[i];
    //     TCHAR moveText[50];
    //     wchar_t fromCol = L'a' + (move.fromY % 8); // 安全计算列字符
    //     wchar_t toCol = L'a' + (move.toY % 8);
    //     _stprintf(
    //         moveText,
    //         _T("%d. %s %c%d → %c%d"),
    //         i + 1,
    //         getPieceNameCN(move.movedPiece),
    //         fromCol,
    //         8 - move.fromX,
    //         toCol,
    //         8 - move.toX // ✅ 正确引用
    //     );
    //     outtextxy(120, yPos, moveText);
    //     yPos += 30;
    //     if (yPos > 550)
    //         break;
    // }

    // 5. 绘制关闭按钮
    setfillcolor(RGB(200, 200, 200));
    fillrectangle(300, 570, 500, 610);
    outtextxy(380, 580, _T("返回"));

    // 6. 处理事件（严格限制作用域）
    bool exitFlag = false;
    while (!exitFlag)
    {
        if (MouseHit())
        {
            MOUSEMSG msg = GetMouseMsg(); // 局部变量
            if (msg.uMsg == WM_LBUTTONDOWN &&
                msg.x >= 300 && msg.x <= 500 &&
                msg.y >= 570 && msg.y <= 610)
            {
                exitFlag = true;
            }
        }

        // 处理Windows消息队列
        MSG winMsg;
        while (PeekMessage(&winMsg, NULL, 0, 0, PM_REMOVE))
        {
            if (winMsg.message == WM_QUIT)
                exitFlag = true;
            TranslateMessage(&winMsg);
            DispatchMessage(&winMsg);
        }
        Sleep(10);
    }

    // 7. 恢复原始界面并释放资源
    putimage(0, 0, &tempBuffer);
    settextstyle(&oldFont); // 恢复原字体

    board->needRedraw = true;
}

// 新增辅助函数：获取棋子中文名
const TCHAR *getPieceNameCN(PieceType type)
{
    switch (type)
    {
    case PAWN:
        return _T("兵");
    case KNIGHT:
        return _T("马");
    case BISHOP:
        return _T("象");
    case ROOK:
        return _T("车");
    case QUEEN:
        return _T("后");
    case KING:
        return _T("王");
    default:
        return _T("未知");
    }
}

void showGameOver(GameState state)
{
    // 窗口位置与大小
    const int winX = 250, winY = 250;
    const int winW = 300, winH = 200;
    // 绘制背景
    setfillcolor(RGB(240, 240, 240));
    fillrectangle(winX, winY, winX + winW, winY + winH);

    // 设置文字样式
    settextcolor(BLACK);
    settextstyle(25, 0, _T("宋体"));

    // 计算并绘制游戏结束提示文字
    TCHAR msgText[20];
    if (state == CHECKMATE) {
        // currentPlayer 为刚下完棋后的玩家，需要提示对方胜利
        _stprintf(msgText, _T("%s胜利！"),
            currentPlayer == white ? _T("黑方") : _T("白方"));
    }
    else {  // STALEMATE
        _stprintf(msgText, _T("和棋！"));
    }
    int msgW = textwidth(msgText);
    int msgH = textheight(msgText);
    // 水平居中：winX + (winW - msgW)/2；垂直大致居中：winY + winH/3
    int msgX = winX + (winW - msgW) / 2;
    int msgY = winY + winH / 3 - msgH / 2;
    outtextxy(msgX, msgY, msgText);

    // 绘制“关闭”按钮
    const int btnW = 100, btnH = 40;
    int btnX = winX + (winW - btnW) / 2;
    int btnY = winY + winH - btnH - 20;  // 底部留 20px 间距
    setfillcolor(RGB(200, 200, 200));
    fillrectangle(btnX, btnY, btnX + btnW, btnY + btnH);
    // 按钮文字
    TCHAR btnLabel[] = _T("关闭");
    int lblW = textwidth(btnLabel);
    int lblH = textheight(btnLabel);
    int lblX = btnX + (btnW - lblW) / 2;
    int lblY = btnY + (btnH - lblH) / 2;
    outtextxy(lblX, lblY, btnLabel);

    // 消息循环：等待点击“关闭”并保持界面响应
    while (true)
    {
        // 鼠标点击检查
        if (MouseHit())
        {
            MOUSEMSG m = GetMouseMsg();
            if (m.uMsg == WM_LBUTTONDOWN &&
                m.x >= btnX && m.x <= btnX + btnW &&
                m.y >= btnY && m.y <= btnY + btnH)
            {
                return;  // 点击“关闭”后退出函数
            }
        }
        // 处理系统其他消息，避免界面无响应
        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(10);  // 降低 CPU 占用
    }
}


// 检查游戏状态
GameState checkGameState(ChessBoard *board)
{
    bool inCheck = isKingInCheck(board, currentPlayer);

    // 新增：检查所有己方棋子是否有解除将军的合法移动
    bool hasEscape = false;
    const auto &myPieces = (currentPlayer == white) ? board->whitePieces : board->blackPieces;
    for (const auto &pos : myPieces)
    {
        if (hasAnyLegalMove(board, pos.first, pos.second))
        {
            hasEscape = true;
            break;
        }
    }

    if (inCheck)
    {
        return hasEscape ? CHECK : CHECKMATE; // 关键修改点
    }
    return hasEscape ? NORMAL : STALEMATE;
}
void getPossibleMoves(ChessBoard *board, int fromX, int fromY, PossibleMoves *result)
{

    // 添加安全检查
    assert(board && fromX >= 0 && fromX < 8 && fromY >= 0 && fromY < 8);

    ChessPiece *piece = &board->board[fromX][fromY];

    switch (piece->type)
    {
    case PAWN:
    {
        int direction = (piece->color == white) ? 1 : -1;
        // 基本移动
        if (fromX + direction >= 0 && fromX + direction < 8)
        {
            if (board->board[fromX + direction][fromY].type == EMPTY)
            {
                (*result).moves[(*result).count][0] = fromX + direction;
                (*result).moves[(*result).count][1] = fromY;
                (*result).count++;

                // 1.2 初始位置可以向前移动两格（独立判断）
                if (!piece->hasMoved && board->board[fromX + 2 * direction][fromY].type == EMPTY)
                {
                    (*result).moves[(*result).count][0] = fromX + 2 * direction;
                    (*result).moves[(*result).count][1] = fromY;
                    (*result).count++;
                }
            }

            // 吃子移动
            for (int dy = -1; dy <= 1; dy += 2)
            {
                if (fromY + dy >= 0 && fromY + dy < 8)
                {
                    // 正常吃子
                    if (board->board[fromX + direction][fromY + dy].type != EMPTY &&
                        board->board[fromX + direction][fromY + dy].color != piece->color)
                    {
                        (*result).moves[(*result).count][0] = fromX + direction;
                        (*result).moves[(*result).count][1] = fromY + dy;
                        (*result).count++;
                    }
                    // 吃过路兵
                    else if (fromX == board->lastPawnJumpX &&
                             abs(fromY - board->lastPawnJumpY) == 1)
                    {
                        ChessPiece *enPassantPawn = &board->board[fromX][fromY + dy];
                        if (enPassantPawn->type == PAWN && enPassantPawn->color != piece->color)
                        {
                            (*result).moves[(*result).count][0] = fromX + direction;
                            (*result).moves[(*result).count][1] = fromY + dy;
                            (*result).count++;
                        }
                    }
                }
            }
        }
        break;
    }
    case KNIGHT:
    {
        const int knightMoves[8][2] = {{2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};
        for (int i = 0; i < 8; i++)
        {
            int toX = fromX + knightMoves[i][0];
            int toY = fromY + knightMoves[i][1];
            if (toX >= 0 && toX < 8 && toY >= 0 && toY < 8)
            {
                (*result).moves[(*result).count][0] = toX;
                (*result).moves[(*result).count][1] = toY;
                (*result).count++;
            }
        }
        break;
    }
    case BISHOP:
    {
        for (int dx = -1; dx <= 1; dx += 2)
        {
            for (int dy = -1; dy <= 1; dy += 2)
            {
                for (int step = 1; step < 8; step++)
                {
                    int toX = fromX + dx * step;
                    int toY = fromY + dy * step;
                    if (toX < 0 || toX >= 8 || toY < 0 || toY >= 8)
                        break;

                    (*result).moves[(*result).count][0] = toX;
                    (*result).moves[(*result).count][1] = toY;
                    (*result).count++;

                    if (board->board[toX][toY].type != EMPTY)
                        break;
                }
            }
        }
        break;
    }
    case ROOK:
    {
        const int rookDirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (int i = 0; i < 4; i++)
        {
            for (int step = 1; step < 8; step++)
            {
                int toX = fromX + rookDirs[i][0] * step;
                int toY = fromY + rookDirs[i][1] * step;
                if (toX < 0 || toX >= 8 || toY < 0 || toY >= 8)
                    break;

                (*result).moves[(*result).count][0] = toX;
                (*result).moves[(*result).count][1] = toY;
                (*result).count++;

                if (board->board[toX][toY].type != EMPTY)
                    break;
            }
        }
        break;
    }
    case QUEEN:
    {
        // 组合车和象的移动
        const int queenDirs[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
        for (int i = 0; i < 8; i++)
        {
            for (int step = 1; step < 8; step++)
            {
                int toX = fromX + queenDirs[i][0] * step;
                int toY = fromY + queenDirs[i][1] * step;
                if (toX < 0 || toX >= 8 || toY < 0 || toY >= 8)
                    break;

                (*result).moves[(*result).count][0] = toX;
                (*result).moves[(*result).count][1] = toY;
                (*result).count++;

                if (board->board[toX][toY].type != EMPTY)
                    break;
            }
        }
        break;
    }
    case KING:
    {
        // 普通移动
        for (int dx = -1; dx <= 1; dx++)
        {
            for (int dy = -1; dy <= 1; dy++)
            {
                if (dx == 0 && dy == 0)
                    continue;

                int toX = fromX + dx;
                int toY = fromY + dy;
                if (toX >= 0 && toX < 8 && toY >= 0 && toY < 8)
                {
                    (*result).moves[(*result).count][0] = toX;
                    (*result).moves[(*result).count][1] = toY;
                    (*result).count++;
                }
            }
        }

        // 王车易位
        if (!piece->hasMoved)
        {
            // 短易位(王翼)
            if (board->board[fromX][7].type == ROOK && !board->board[fromX][7].hasMoved)
            {
                bool canCastle = true;
                for (int y = 5; y < 7; y++)
                {
                    if (board->board[fromX][y].type != EMPTY)
                    {
                        canCastle = false;
                        break;
                    }
                }
                if (canCastle)
                {
                    (*result).moves[(*result).count][0] = fromX;
                    (*result).moves[(*result).count][1] = 6; // g1/g8
                    (*result).count++;
                }
            }
            // 长易位(后翼)
            if (board->board[fromX][0].type == ROOK && !board->board[fromX][0].hasMoved)
            {
                bool canCastle = true;
                for (int y = 1; y < 4; y++)
                {
                    if (board->board[fromX][y].type != EMPTY)
                    {
                        canCastle = false;
                        break;
                    }
                }
                if (canCastle)
                {
                    (*result).moves[(*result).count][0] = fromX;
                    (*result).moves[(*result).count][1] = 2; // c1/c8
                    (*result).count++;
                }
            }
        }
        break;
    }
    }

    return;
}

// 结束