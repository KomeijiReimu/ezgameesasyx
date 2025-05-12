
#include "all.h"

// 评估函数
float evaluateBoard(ChessBoard *board, PieceColor color)
{
    float score = 0;
    const auto &pieces = (color == white) ? board->whitePieces : board->blackPieces;
    for (const auto &pos : pieces)
    {
        ChessPiece *piece = &board->board[pos.first][pos.second];
        switch (piece->type)
        {
        case PAWN:
            score += 1.0;
            break;
        case KNIGHT:
            score += 3.0;
            break;
        case BISHOP:
            score += 3.1f;
            break;
        case ROOK:
            score += 5.0;
            break;
        case QUEEN:
            score += 9.0;
            break;
        }
    }
    return score;
}

// 极小极大算法
float minimax(ChessBoard *board, int depth, bool isMaximizing, float alpha, float beta)
{
    if (depth == 0)
    {
        return evaluateBoard(board, board->ai.color);
    }

    float bestValue = isMaximizing ? -INFINITY : INFINITY;
    const auto &pieces = isMaximizing ? (board->ai.color == white ? board->whitePieces : board->blackPieces) : (board->ai.color == white ? board->blackPieces : board->whitePieces);

    for (const auto &pos : pieces)
    {
        int fromX = pos.first;
        int fromY = pos.second;

        PossibleMoves moves = {0};
        getPossibleMoves(board, fromX, fromY, &moves);

        for (int i = 0; i < moves.count; i++)
        {
            int toX = moves.moves[i][0];
            int toY = moves.moves[i][1];

            if (isValidMove(board, fromX, fromY, toX, toY, false))
            {
                ChessBoard tempBoard = *board;
                tempBoard.board[toX][toY] = tempBoard.board[fromX][fromY];
                tempBoard.board[fromX][fromY].type = EMPTY;

                float value = minimax(&tempBoard, depth - 1, !isMaximizing, alpha, beta);

                if (isMaximizing)
                {
                    bestValue = fmax(bestValue, value);
                    alpha = fmax(alpha, bestValue);
                }
                else
                {
                    bestValue = fmin(bestValue, value);
                    beta = fmin(beta, bestValue);
                }

                if (beta <= alpha)
                    return bestValue;
            }
        }
    }
    return bestValue;
}

// 修改initAI函数实现，添加返回值
bool initAI(ChessAI *ai, int difficulty)
{
    // 参数有效性检查
    if (ai == nullptr)
    {
        printf("[AI ERROR] AI指针为空\n");
        return false;
    }

    if (difficulty < 1 || difficulty > 5)
    {
        printf("[AI ERROR] 无效难度: %d\n", difficulty);
        return false;
    }

    // 初始化AI结构体
    memset(ai, 0, sizeof(ChessAI)); // 先清零

    // 设置难度相关参数
    ai->difficulty = difficulty;
    ai->depth = difficulty;
    ai->color = black; // 默认AI执黑

    // 初始化权重数组
    const float baseWeights[] = {0, 1.0f, 3.0f, 3.1f, 5.0f, 9.0f}; // 对应EMPTY,PAWN,KNIGHT,BISHOP,ROOK,QUEEN
    float factor = 1.0f + (difficulty - 1) * 0.2f;

    for (int i = 0; i < 6; i++)
    {
        ai->weights[i] = baseWeights[i] * factor;
    }

    ai->learningRate = 0.1f;
    ai->experience = 0;

    printf("[AI] AI初始化成功 难度:%d 深度:%d\n", difficulty, ai->depth);
    return true;
}

// 训练AI
void trainAI(ChessAI *ai, float reward)
{
    ai->experience++;
    for (int i = 0; i < 5; i++)
    {
        ai->weights[i] += ai->learningRate * reward;
    }
    if (ai->experience % 10 == 0 && ai->depth < 5)
    {
        ai->depth++;
    }
}

// AI走棋
void AIMove(ChessBoard *board, int *bestMove)
{
    float bestScore = -INFINITY;
    const auto &pieces = (board->ai.color == white) ? board->whitePieces : board->blackPieces;

    for (const auto &pos : pieces)
    {
        int fromX = pos.first;
        int fromY = pos.second;

        PossibleMoves moves = {0};
        getPossibleMoves(board, fromX, fromY, &moves);

        for (int i = 0; i < moves.count; i++)
        {
            int toX = moves.moves[i][0];
            int toY = moves.moves[i][1];

            if (isValidMove(board, fromX, fromY, toX, toY, false))
            {
                ChessBoard tempBoard = *board;
                tempBoard.board[toX][toY] = tempBoard.board[fromX][fromY];
                tempBoard.board[fromX][fromY].type = EMPTY;

                float score = minimax(&tempBoard, board->ai.depth, false, -INFINITY, INFINITY);

                if (score > bestScore)
                {
                    bestScore = score;
                    bestMove[0] = fromX;
                    bestMove[1] = fromY;
                    bestMove[2] = toX;
                    bestMove[3] = toY;
                }
            }
        }
    }
}

// AI提示功能 - 修改后版本
void showHint(ChessBoard *board)
{
    if (board->hintCount >= MAX_HINTS)
        return;

    int bestMove[4] = {0};
    AIMove(board, bestMove);

    if (bestMove[2] == 0 && bestMove[3] == 0)
    {
        MessageBox(NULL, _T("当前无有效提示"), _T("提示"), MB_OK);
        return;
    }

    highlightTarget(board, bestMove[2], bestMove[3]);
    board->hintCount++;
    board->needRedraw = true; // 依赖主循环重绘
}

// 高亮目标位置的辅助函数
void highlightTarget(ChessBoard *board, int toX, int toY)
{
    if (toX < 0 || toX >= 8 || toY < 0 || toY >= 8)
        return;

    // 棋盘格子起始于(100,50)，每个格子80像素
    int screenX = 100 + toY * 80 + 1; // 左边界+1防止覆盖边框
    int screenY = 50 + toX * 80 + 1;  // 上边界+1
    int screenWidth = 78;             // 宽度-2保持边框可见
    int screenHeight = 78;

    setfillcolor(0xFFFFE060); // 使用半透明的黄色（ARGB格式）
    solidrectangle(screenX, screenY, screenX + screenWidth, screenY + screenHeight);
}

// 添加形势评估功能
void showEvaluation(ChessBoard* board)
{
    // 计算双方得分
    float whiteScore = evaluateBoard(board, white);
    float blackScore = evaluateBoard(board, black);

    // 定义窗口位置与大小
    const int winX = 250, winY = 250;
    const int winW = 400, winH = 400;
    setfillcolor(RGB(240, 240, 240));
    fillrectangle(winX, winY, winX + winW, winY + winH);

    // 设置文本颜色
    settextcolor(BLACK);
    TCHAR buffer[100];

    // 绘制“白方得分”，水平居中
    _stprintf(buffer, _T("白方得分: %.1f"), whiteScore);
    int tw = textwidth(buffer);
    int x = winX + (winW - tw) / 2;
    outtextxy(x, winY + 10, buffer);

    // 绘制“黑方得分”，水平居中
    _stprintf(buffer, _T("黑方得分: %.1f"), blackScore);
    tw = textwidth(buffer);
    x = winX + (winW - tw) / 2;
    outtextxy(x, winY + 40, buffer);

    // 绘制“形势”文字（白方占优/黑方占优/局势均衡）
    TCHAR status[50];
    if (whiteScore > blackScore)
        _stprintf(status, _T("白方占优"));
    else if (blackScore > whiteScore)
        _stprintf(status, _T("黑方占优"));
    else
        _stprintf(status, _T("局势均衡"));

    tw = textwidth(status);
    x = winX + (winW - tw) / 2;
    outtextxy(x, winY + 70, status);

    // 绘制“关闭”按钮，居中于窗口底部
    const int btnW = 100, btnH = 40;
    int btnX = winX + (winW - btnW) / 2;
    int btnY = winY + winH - btnH - 10;
    setfillcolor(RGB(200, 200, 200));
    fillrectangle(btnX, btnY, btnX + btnW, btnY + btnH);
    // 按钮文字
    _stprintf(buffer, _T("关闭"));
    tw = textwidth(buffer);
    int tx = btnX + (btnW - tw) / 2;
    outtextxy(tx, btnY + 10, buffer);

    // 等待点击“关闭”并保持界面响应
    while (true)
    {
        if (MouseHit())
        {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN &&
                msg.x >= btnX && msg.x <= btnX + btnW &&
                msg.y >= btnY && msg.y <= btnY + btnH)
            {
                board->needRedraw = true;  // 标记需要重绘
                return;                    // 退出函数
            }
        }
        // 处理其他系统消息，避免界面无响应
        MSG winMsg;
        if (PeekMessage(&winMsg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&winMsg);
            DispatchMessage(&winMsg);
        }
        Sleep(10);  // 降低 CPU 占用
    }
}


// 添加难度选择菜单
int showDifficultyMenu()
{
    printf("[DEBUG] 进入难度选择菜单\n");
    // 保存当前绘图状态
    IMAGE buffer;
    DWORD *pBuf = GetImageBuffer(NULL); // 获取屏幕缓冲区
    if (pBuf == nullptr)
    {
        printf("[ERROR] 无法获取屏幕缓冲区\n");
        return -1;
    }

    // 绘制菜单界面
    setfillcolor(RGB(240, 240, 240));
    fillrectangle(200, 100, 600, 500);

    // 绘制标题
    settextstyle(30, 0, _T("宋体"));
    settextcolor(BLACK);
    TCHAR title[20] = _T("选择AI难度级别:");
    int titleWidth = textwidth(title);
    outtextxy(300 - titleWidth / 2, 120, title);
    const int btnWidth = 200;
    const int btnHeight = 40;
    const int startY = 180;
    const int spacing = 60;
    const TCHAR *difficultyTexts[] = {
        _T("初级 (1)"),
        _T("中级 (2)"),
        _T("高级 (3)"),
        _T("专家 (4)"),
        _T("大师 (5)")};

    RECT btnRects[5];

    settextstyle(20, 0, _T("宋体"));
    for (int i = 0; i < 5; i++)
    {
        int btnX = 300 - btnWidth / 2;
        int btnY = startY + i * spacing;

        setfillcolor(RGB(200, 200, 200));
        btnRects[i] = {btnX, btnY, btnX + btnWidth, btnY + btnHeight};
        fillrectangle(btnX, btnY, btnX + btnWidth, btnY + btnHeight);

        int textWidth = textwidth(difficultyTexts[i]);
        int textX = btnX + (btnWidth - textWidth) / 2;                       // 计算文字居中X坐标
        int textY = btnY + (btnHeight - textheight(difficultyTexts[i])) / 2; // 计算文字居中Y坐标
        outtextxy(textX, textY, difficultyTexts[i]);
    }
    // 绘制文字
    // settextstyle(30, 0, _T("宋体"));
    // settextcolor(BLACK);
    // TCHAR title[20] = _T("选择AI难度级别:");
    // int titleWidth = textwidth(title);
    // outtextxy(300 - titleWidth / 2, 120, title);

    // // 定义按钮位置和大小
    // const int btnWidth = 200;
    // const int btnHeight = 40;
    // const int startY = 180;
    // const int spacing = 60;

    // // 绘制难度选项按钮
    // const TCHAR *difficultyTexts[] = {
    //     _T("初级 (1)"),
    //     _T("中级 (2)"),
    //     _T("高级 (3)"),
    //     _T("专家 (4)"),
    //     _T("大师 (5)")};

    // RECT btnRects[5];
    // for (int i = 0; i < 5; i++)
    // {
    //     int btnX = 300 - btnWidth / 2;
    //     int btnY = startY + i * spacing;

    //     // 绘制按钮背景
    //     setfillcolor(RGB(200, 200, 200));
    //     btnRects[i] = {btnX, btnY, btnX + btnWidth, btnY + btnHeight};
    //     fillrectangle(btnX, btnY, btnX + btnWidth, btnY + btnHeight);

    //     // 居中绘制按钮文字
    //     int textWidth = textwidth(difficultyTexts[i]);
    //     outtextxy(btnX + (btnWidth - textWidth) / 2, btnY + 10, difficultyTexts[i]);
    // }

    // 等待用户选择
    int choice = 0;
    while (choice < 1 || choice > 5)
    {
        if (MouseHit())
        {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN)
            {
                // 检查点击是否在按钮区域内
                for (int i = 0; i < 5; i++)
                {
                    if (msg.x >= btnRects[i].left && msg.x <= btnRects[i].right &&
                        msg.y >= btnRects[i].top && msg.y <= btnRects[i].bottom)
                    {
                        choice = i + 1;
                        printf("[DIFFICULTY] 用户选择难度: %d\n", choice);
                        break;
                    }
                }
            }
        }
    }

    // 恢复绘图状态
    SetWorkingImage(NULL);
    return choice;
}

void saveGameRecord(GameRecord *record)
{
    FILE *fp = fopen("chess_save.dat", "wb");
    if (!fp)
        return;

    // 保存棋盘状态
    fwrite(&record->board, sizeof(ChessBoard), 1, fp);

    // 保存移动计数
    fwrite(&record->moveCount, sizeof(int), 1, fp);

    fclose(fp);
}

bool loadGameRecord(GameRecord *record)
{
    FILE *fp = fopen("chess_save.dat", "rb");
    if (!fp)
        return false;

    // 读取棋盘状态
    size_t read = fread(&record->board, sizeof(ChessBoard), 1, fp);

    // 读取移动计数
    fread(&record->moveCount, sizeof(int), 1, fp);

    fclose(fp);
    return read == 1;
}

// 结束