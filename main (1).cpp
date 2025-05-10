#include "all (1).h"
#include <graphics.h>
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include "AI (1).cpp"
#pragma comment(lib, "winmm.lib")

int main() {
    // ��ʼ������
    printf("[INIT] ��ʼ��ʼ������...\n");
    ChessBoard board;
    initBoard(&board);
    printf("[INIT] ���̳�ʼ�����\n");

    printf("[INIT] ��ʼ��ͼ�ν���...\n");
    initGUI();
    printf("[INIT] ͼ�ν����ʼ�����\n");

    // ��ȡ���ھ��
    HWND hWnd = GetHWnd();
    printf("[INFO] ���ھ��: 0x%p\n", hWnd);

    // ˫�����ʼ��
    IMAGE buffer(1000, 900);
    printf("[INIT] ˫�����ʼ�����\n");

    setbkcolor(BROWN);
    cleardevice();
    // ģʽѡ��
    printf("[MODE] ����ģʽѡ�����...\n");
    SetWorkingImage(NULL);
    settextstyle(30, 0, _T("����"));
    settextcolor(BLACK);
    // �������ֿ�Ȳ�������ʾ
    int textWidth1 = textwidth(_T("1. ˫�˶�ս"));
    int textWidth2 = textwidth(_T("2. �˻���ս"));
    outtextxy((800 - textWidth1) / 2, 200, _T("1. ˫�˶�ս"));
    outtextxy((800 - textWidth2) / 2, 250, _T("2. �˻���ս"));

    int choice = 0;
    MSG msg;
    while (choice != 1 && choice != 2) {
        if (MouseHit()) {
            MOUSEMSG mouseMsg = GetMouseMsg();
            if (mouseMsg.uMsg == WM_LBUTTONDOWN) {
                printf("[DEBUG] �����: (%d,%d)\n", mouseMsg.x, mouseMsg.y);
                int textHeight = textheight(_T("1. ˫�˶�ս"));
                if (mouseMsg.y >= 200 && mouseMsg.y <= 200 + textHeight) {
                    if (mouseMsg.x >= (800 - textWidth1) / 2 && mouseMsg.x <= (800 - textWidth1) / 2 + textWidth1) {
                        choice = 1;
                        printf("[MODE] �û�ѡ��ģʽ: %d\n", choice);
                    }
                }
                else if (mouseMsg.y >= 250 && mouseMsg.y <= 250 + textHeight) {
                    if (mouseMsg.x >= (800 - textWidth2) / 2 && mouseMsg.x <= (800 - textWidth2) / 2 + textWidth2) {
                        choice = 2;
                        printf("[MODE] �û�ѡ��ģʽ: %d\n", choice);
                    }
                }
            }
        }
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(10);
    }

    // ��ʼ����Ϸģʽ
    board.mode = (choice == 1) ? TWO_PLAYERS : SINGLE_PLAYER;  // ��������Ϸģʽ

    if (board.mode == SINGLE_PLAYER) {
        printf("[AI] ��ʼ��AI...\n");
        // ��ӵ������
        printf("[DEBUG] ׼����ʾ�Ѷ�ѡ��˵�\n");
        int difficulty = showDifficultyMenu();
        printf("[DEBUG] ��ȡ���Ѷ�ѡ��: %d\n", difficulty);

        if (difficulty < 1 || difficulty > 5) {
            printf("[ERROR] ��Ч���Ѷȼ���: %d\n", difficulty);
            difficulty = 3; // ʹ��Ĭ���Ѷ�
        }
        if (!initAI(&board.ai, difficulty)) {
            printf("[ERROR] AI��ʼ��ʧ��\n");
            return -1;
        }
        board.ai.color = black;
        printf("[AI] AI��ʼ����ɣ��Ѷ�: %d\n", difficulty);
    }

    // ��Ϸ״̬����
    int selectedX = -1, selectedY = -1;
    bool gameRunning = true;

    printf("[UI] ��ʼ����ť����...\n");
    SetWorkingImage(&buffer);
    setfillcolor(RGB(200, 200, 200));
    fillrectangle(820, 100, 980, 140);  // ���尴ť
    fillrectangle(820, 160, 980, 200);  // ��ʾ��ť
    fillrectangle(820, 220, 980, 260);  // ������ť
    fillrectangle(820, 280, 980, 320);  // ��¼��ť

    settextstyle(20, 0, _T("����"));
    settextcolor(BLACK);
    setbkmode(TRANSPARENT);
    outtextxy(870, 105, _T("����"));
    outtextxy(870, 165, _T("��ʾ"));
    outtextxy(870, 225, _T("����"));
    outtextxy(870, 285, _T("��¼"));
    SetWorkingImage(NULL);

    // ����Ϸѭ��
    printf("[MAIN] ��������Ϸѭ��\n");
    while (gameRunning) {
        bool hasInput = false;

        // ���������¼�
        if (MouseHit()) {
            hasInput = true;
            MOUSEMSG mouseMsg = GetMouseMsg();
            printf("[INPUT] ����¼�: ����=0x%X ����=(%d,%d)\n",
                mouseMsg.uMsg, mouseMsg.x, mouseMsg.y);

            if (mouseMsg.uMsg == WM_LBUTTONDOWN) {
                // ����ť���
                if (mouseMsg.x >= 820 && mouseMsg.x <= 980) {
                    printf("[UI] ��ť������\n");
                    if (mouseMsg.y >= 100 && mouseMsg.y <= 140) {  // ���尴ť
                        if (!board.history.empty() && board.notation && !board.notation->moves.empty()) {
                            undoMove(&board);
                            board.needRedraw = true;
                            printf("[BUTTON] ����ɹ�\n");
                        }
                        else {
                            printf("[BUTTON] û�пɻڵ��岽��\n");
                            MessageBox(hWnd, _T("û�пɻڵ��岽��"), _T("��ʾ"), MB_OK);
                        }
                    }
                    else if (mouseMsg.y >= 160 && mouseMsg.y <= 200) {  // ��ʾ��ť
                        showHint(&board);
                        board.needRedraw = true;
                        printf("[BUTTON] ��ʾ��ť�������������ػ�\n");
                    }
                    else if (mouseMsg.y >= 220 && mouseMsg.y <= 260) {  // ������ť
                        showEvaluation(&board);
                        board.needRedraw = true;  // ȷ�������ػ�
                        printf("[BUTTON] ������ť���\n");
                    }
                    else if (mouseMsg.y >= 280 && mouseMsg.y <= 320 && board.notation) {  // ��¼��ť
                        showNotation(&board);
                        board.needRedraw = true;
                        printf("[BUTTON] ��¼��ť�������������ػ�\n");
                    }
                }


                // �������̵��
                int clickedX = (mouseMsg.y - 50) / 80;
                int clickedY = (mouseMsg.x - 100) / 80;

                if (clickedX >= 0 && clickedX < 8 && clickedY >= 0 && clickedY < 8) {
                    printf("[GAME] ���̵��: �߼�����(%d,%d)\n", clickedX, clickedY);

                    if (selectedX == -1) {
                        if (isValidSelection(&board, clickedX, clickedY)) {
                            selectedX = clickedX;
                            selectedY = clickedY;
                            board.needRedraw = true;

                            // ����������ʾ���ƶ�λ��
                            SetWorkingImage(&buffer);
                            drawBoard(&board, nullptr);
                            highlightMoves(&board, selectedX, selectedY);
                            SetWorkingImage(NULL);
                            putimage(0, 0, &buffer);
                        }
                    }
                    else {
                        // ����Ƿ�����Ч�ƶ�
                        if (isValidMove(&board, selectedX, selectedY, clickedX, clickedY, false)) {
                            // ������Ч�ƶ�
                            processMove(&board, selectedX, selectedY, clickedX, clickedY, false);
                            GameState state = checkGameState(&board);
                            if (state != NORMAL) {
                                gameRunning = false;
                            }
                        }
                        // �����ƶ��Ƿ���Ч��������ѡ��״̬
                        selectedX = selectedY = -1;
                        board.needRedraw = true;
                    }
                }
            }
        }

        if (board.mode == SINGLE_PLAYER && currentPlayer == board.ai.color) {
            hasInput = true;
            printf("[AI] AI ˼�����ӿ�ʼ\n");

            int bestMove[4] = { 0 };        // ����洢����ƶ�������
            AIMove(&board, bestMove);     // ���� 2 ��������ȡ����ƶ�

            if (bestMove[2] != 0 || bestMove[3] != 0) {  // ����Ƿ��ҵ���Ч�ƶ�
                processMove(&board, bestMove[0], bestMove[1], bestMove[2], bestMove[3], true);  // ִ���ƶ�
            }

            printf("[AI] AI �������\n");
        }


        // ����Windows��Ϣ
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            hasInput = true;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // ֻ�����������Ҫ�ػ�ʱ��Ⱦ
        if (hasInput || board.needRedraw) {
            if (board.needRedraw) {
                printf("[RENDER] ��ʼ��Ⱦ֡\n");
                SetWorkingImage(&buffer);
                clearrectangle(0, 0, 800, 700);
                drawBoard(&board, nullptr);

                if (selectedX != -1 && selectedY != -1) {
                    highlightMoves(&board, selectedX, selectedY);
                    printf("[RENDER] �������ƶ�λ�ã���ǰѡ��: (%d, %d)\n", selectedX, selectedY);
                }

                SetWorkingImage(NULL);
                putimage(0, 0, &buffer);
                printf("[RENDER] ��Ⱦ���\n");
                board.needRedraw = false;
            }
        }
        else {
            Sleep(50);
        }
    }
    printf("[MAIN] �˳���ѭ��\n");
    closegraph();
    if (board.notation != nullptr) {
        delete board.notation;
        board.notation = nullptr;
    }
    closegraph();
    return 0;
}