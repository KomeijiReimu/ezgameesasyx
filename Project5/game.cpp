
#include "all.h"

// ��������
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

// ��С�����㷨
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

// �޸�initAI����ʵ�֣���ӷ���ֵ
bool initAI(ChessAI *ai, int difficulty)
{
    // ������Ч�Լ��
    if (ai == nullptr)
    {
        printf("[AI ERROR] AIָ��Ϊ��\n");
        return false;
    }

    if (difficulty < 1 || difficulty > 5)
    {
        printf("[AI ERROR] ��Ч�Ѷ�: %d\n", difficulty);
        return false;
    }

    // ��ʼ��AI�ṹ��
    memset(ai, 0, sizeof(ChessAI)); // ������

    // �����Ѷ���ز���
    ai->difficulty = difficulty;
    ai->depth = difficulty;
    ai->color = black; // Ĭ��AIִ��

    // ��ʼ��Ȩ������
    const float baseWeights[] = {0, 1.0f, 3.0f, 3.1f, 5.0f, 9.0f}; // ��ӦEMPTY,PAWN,KNIGHT,BISHOP,ROOK,QUEEN
    float factor = 1.0f + (difficulty - 1) * 0.2f;

    for (int i = 0; i < 6; i++)
    {
        ai->weights[i] = baseWeights[i] * factor;
    }

    ai->learningRate = 0.1f;
    ai->experience = 0;

    printf("[AI] AI��ʼ���ɹ� �Ѷ�:%d ���:%d\n", difficulty, ai->depth);
    return true;
}

// ѵ��AI
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

// AI����
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

// AI��ʾ���� - �޸ĺ�汾
void showHint(ChessBoard *board)
{
    if (board->hintCount >= MAX_HINTS)
        return;

    int bestMove[4] = {0};
    AIMove(board, bestMove);

    if (bestMove[2] == 0 && bestMove[3] == 0)
    {
        MessageBox(NULL, _T("��ǰ����Ч��ʾ"), _T("��ʾ"), MB_OK);
        return;
    }

    highlightTarget(board, bestMove[2], bestMove[3]);
    board->hintCount++;
    board->needRedraw = true; // ������ѭ���ػ�
}

// ����Ŀ��λ�õĸ�������
void highlightTarget(ChessBoard *board, int toX, int toY)
{
    if (toX < 0 || toX >= 8 || toY < 0 || toY >= 8)
        return;

    // ���̸�����ʼ��(100,50)��ÿ������80����
    int screenX = 100 + toY * 80 + 1; // ��߽�+1��ֹ���Ǳ߿�
    int screenY = 50 + toX * 80 + 1;  // �ϱ߽�+1
    int screenWidth = 78;             // ���-2���ֱ߿�ɼ�
    int screenHeight = 78;

    setfillcolor(0xFFFFE060); // ʹ�ð�͸���Ļ�ɫ��ARGB��ʽ��
    solidrectangle(screenX, screenY, screenX + screenWidth, screenY + screenHeight);
}

// ���������������
void showEvaluation(ChessBoard* board)
{
    // ����˫���÷�
    float whiteScore = evaluateBoard(board, white);
    float blackScore = evaluateBoard(board, black);

    // ���崰��λ�����С
    const int winX = 250, winY = 250;
    const int winW = 400, winH = 400;
    setfillcolor(RGB(240, 240, 240));
    fillrectangle(winX, winY, winX + winW, winY + winH);

    // �����ı���ɫ
    settextcolor(BLACK);
    TCHAR buffer[100];

    // ���ơ��׷��÷֡���ˮƽ����
    _stprintf(buffer, _T("�׷��÷�: %.1f"), whiteScore);
    int tw = textwidth(buffer);
    int x = winX + (winW - tw) / 2;
    outtextxy(x, winY + 10, buffer);

    // ���ơ��ڷ��÷֡���ˮƽ����
    _stprintf(buffer, _T("�ڷ��÷�: %.1f"), blackScore);
    tw = textwidth(buffer);
    x = winX + (winW - tw) / 2;
    outtextxy(x, winY + 40, buffer);

    // ���ơ����ơ����֣��׷�ռ��/�ڷ�ռ��/���ƾ��⣩
    TCHAR status[50];
    if (whiteScore > blackScore)
        _stprintf(status, _T("�׷�ռ��"));
    else if (blackScore > whiteScore)
        _stprintf(status, _T("�ڷ�ռ��"));
    else
        _stprintf(status, _T("���ƾ���"));

    tw = textwidth(status);
    x = winX + (winW - tw) / 2;
    outtextxy(x, winY + 70, status);

    // ���ơ��رա���ť�������ڴ��ڵײ�
    const int btnW = 100, btnH = 40;
    int btnX = winX + (winW - btnW) / 2;
    int btnY = winY + winH - btnH - 10;
    setfillcolor(RGB(200, 200, 200));
    fillrectangle(btnX, btnY, btnX + btnW, btnY + btnH);
    // ��ť����
    _stprintf(buffer, _T("�ر�"));
    tw = textwidth(buffer);
    int tx = btnX + (btnW - tw) / 2;
    outtextxy(tx, btnY + 10, buffer);

    // �ȴ�������رա������ֽ�����Ӧ
    while (true)
    {
        if (MouseHit())
        {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN &&
                msg.x >= btnX && msg.x <= btnX + btnW &&
                msg.y >= btnY && msg.y <= btnY + btnH)
            {
                board->needRedraw = true;  // �����Ҫ�ػ�
                return;                    // �˳�����
            }
        }
        // ��������ϵͳ��Ϣ�������������Ӧ
        MSG winMsg;
        if (PeekMessage(&winMsg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&winMsg);
            DispatchMessage(&winMsg);
        }
        Sleep(10);  // ���� CPU ռ��
    }
}


// ����Ѷ�ѡ��˵�
int showDifficultyMenu()
{
    printf("[DEBUG] �����Ѷ�ѡ��˵�\n");
    // ���浱ǰ��ͼ״̬
    IMAGE buffer;
    DWORD *pBuf = GetImageBuffer(NULL); // ��ȡ��Ļ������
    if (pBuf == nullptr)
    {
        printf("[ERROR] �޷���ȡ��Ļ������\n");
        return -1;
    }

    // ���Ʋ˵�����
    setfillcolor(RGB(240, 240, 240));
    fillrectangle(200, 100, 600, 500);

    // ���Ʊ���
    settextstyle(30, 0, _T("����"));
    settextcolor(BLACK);
    TCHAR title[20] = _T("ѡ��AI�Ѷȼ���:");
    int titleWidth = textwidth(title);
    outtextxy(300 - titleWidth / 2, 120, title);
    const int btnWidth = 200;
    const int btnHeight = 40;
    const int startY = 180;
    const int spacing = 60;
    const TCHAR *difficultyTexts[] = {
        _T("���� (1)"),
        _T("�м� (2)"),
        _T("�߼� (3)"),
        _T("ר�� (4)"),
        _T("��ʦ (5)")};

    RECT btnRects[5];

    settextstyle(20, 0, _T("����"));
    for (int i = 0; i < 5; i++)
    {
        int btnX = 300 - btnWidth / 2;
        int btnY = startY + i * spacing;

        setfillcolor(RGB(200, 200, 200));
        btnRects[i] = {btnX, btnY, btnX + btnWidth, btnY + btnHeight};
        fillrectangle(btnX, btnY, btnX + btnWidth, btnY + btnHeight);

        int textWidth = textwidth(difficultyTexts[i]);
        int textX = btnX + (btnWidth - textWidth) / 2;                       // �������־���X����
        int textY = btnY + (btnHeight - textheight(difficultyTexts[i])) / 2; // �������־���Y����
        outtextxy(textX, textY, difficultyTexts[i]);
    }
    // ��������
    // settextstyle(30, 0, _T("����"));
    // settextcolor(BLACK);
    // TCHAR title[20] = _T("ѡ��AI�Ѷȼ���:");
    // int titleWidth = textwidth(title);
    // outtextxy(300 - titleWidth / 2, 120, title);

    // // ���尴ťλ�úʹ�С
    // const int btnWidth = 200;
    // const int btnHeight = 40;
    // const int startY = 180;
    // const int spacing = 60;

    // // �����Ѷ�ѡ�ť
    // const TCHAR *difficultyTexts[] = {
    //     _T("���� (1)"),
    //     _T("�м� (2)"),
    //     _T("�߼� (3)"),
    //     _T("ר�� (4)"),
    //     _T("��ʦ (5)")};

    // RECT btnRects[5];
    // for (int i = 0; i < 5; i++)
    // {
    //     int btnX = 300 - btnWidth / 2;
    //     int btnY = startY + i * spacing;

    //     // ���ư�ť����
    //     setfillcolor(RGB(200, 200, 200));
    //     btnRects[i] = {btnX, btnY, btnX + btnWidth, btnY + btnHeight};
    //     fillrectangle(btnX, btnY, btnX + btnWidth, btnY + btnHeight);

    //     // ���л��ư�ť����
    //     int textWidth = textwidth(difficultyTexts[i]);
    //     outtextxy(btnX + (btnWidth - textWidth) / 2, btnY + 10, difficultyTexts[i]);
    // }

    // �ȴ��û�ѡ��
    int choice = 0;
    while (choice < 1 || choice > 5)
    {
        if (MouseHit())
        {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN)
            {
                // ������Ƿ��ڰ�ť������
                for (int i = 0; i < 5; i++)
                {
                    if (msg.x >= btnRects[i].left && msg.x <= btnRects[i].right &&
                        msg.y >= btnRects[i].top && msg.y <= btnRects[i].bottom)
                    {
                        choice = i + 1;
                        printf("[DIFFICULTY] �û�ѡ���Ѷ�: %d\n", choice);
                        break;
                    }
                }
            }
        }
    }

    // �ָ���ͼ״̬
    SetWorkingImage(NULL);
    return choice;
}

void saveGameRecord(GameRecord *record)
{
    FILE *fp = fopen("chess_save.dat", "wb");
    if (!fp)
        return;

    // ��������״̬
    fwrite(&record->board, sizeof(ChessBoard), 1, fp);

    // �����ƶ�����
    fwrite(&record->moveCount, sizeof(int), 1, fp);

    fclose(fp);
}

bool loadGameRecord(GameRecord *record)
{
    FILE *fp = fopen("chess_save.dat", "rb");
    if (!fp)
        return false;

    // ��ȡ����״̬
    size_t read = fread(&record->board, sizeof(ChessBoard), 1, fp);

    // ��ȡ�ƶ�����
    fread(&record->moveCount, sizeof(int), 1, fp);

    fclose(fp);
    return read == 1;
}

// ����