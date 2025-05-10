#include "all (1).h"
#include <graphics.h>
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include "AI (1).cpp"
#pragma comment(lib, "winmm.lib")

int main() {
    // 初始化部分
    printf("[INIT] 开始初始化棋盘...\n");
    ChessBoard board;
    initBoard(&board);
    printf("[INIT] 棋盘初始化完成\n");

    printf("[INIT] 初始化图形界面...\n");
    initGUI();
    printf("[INIT] 图形界面初始化完成\n");

    // 获取窗口句柄
    HWND hWnd = GetHWnd();
    printf("[INFO] 窗口句柄: 0x%p\n", hWnd);

    // 双缓冲初始化
    IMAGE buffer(1000, 900);
    printf("[INIT] 双缓冲初始化完成\n");

    setbkcolor(BROWN);
    cleardevice();
    // 模式选择
    printf("[MODE] 绘制模式选择界面...\n");
    SetWorkingImage(NULL);
    settextstyle(30, 0, _T("宋体"));
    settextcolor(BLACK);
    // 计算文字宽度并居中显示
    int textWidth1 = textwidth(_T("1. 双人对战"));
    int textWidth2 = textwidth(_T("2. 人机对战"));
    outtextxy((800 - textWidth1) / 2, 200, _T("1. 双人对战"));
    outtextxy((800 - textWidth2) / 2, 250, _T("2. 人机对战"));

    int choice = 0;
    MSG msg;
    while (choice != 1 && choice != 2) {
        if (MouseHit()) {
            MOUSEMSG mouseMsg = GetMouseMsg();
            if (mouseMsg.uMsg == WM_LBUTTONDOWN) {
                printf("[DEBUG] 鼠标点击: (%d,%d)\n", mouseMsg.x, mouseMsg.y);
                int textHeight = textheight(_T("1. 双人对战"));
                if (mouseMsg.y >= 200 && mouseMsg.y <= 200 + textHeight) {
                    if (mouseMsg.x >= (800 - textWidth1) / 2 && mouseMsg.x <= (800 - textWidth1) / 2 + textWidth1) {
                        choice = 1;
                        printf("[MODE] 用户选择模式: %d\n", choice);
                    }
                }
                else if (mouseMsg.y >= 250 && mouseMsg.y <= 250 + textHeight) {
                    if (mouseMsg.x >= (800 - textWidth2) / 2 && mouseMsg.x <= (800 - textWidth2) / 2 + textWidth2) {
                        choice = 2;
                        printf("[MODE] 用户选择模式: %d\n", choice);
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

    // 初始化游戏模式
    board.mode = (choice == 1) ? TWO_PLAYERS : SINGLE_PLAYER;  // 先设置游戏模式

    if (board.mode == SINGLE_PLAYER) {
        printf("[AI] 初始化AI...\n");
        // 添加调试输出
        printf("[DEBUG] 准备显示难度选择菜单\n");
        int difficulty = showDifficultyMenu();
        printf("[DEBUG] 获取到难度选择: %d\n", difficulty);

        if (difficulty < 1 || difficulty > 5) {
            printf("[ERROR] 无效的难度级别: %d\n", difficulty);
            difficulty = 3; // 使用默认难度
        }
        if (!initAI(&board.ai, difficulty)) {
            printf("[ERROR] AI初始化失败\n");
            return -1;
        }
        board.ai.color = black;
        printf("[AI] AI初始化完成，难度: %d\n", difficulty);
    }

    // 游戏状态变量
    int selectedX = -1, selectedY = -1;
    bool gameRunning = true;

    printf("[UI] 初始化按钮界面...\n");
    SetWorkingImage(&buffer);
    setfillcolor(RGB(200, 200, 200));
    fillrectangle(820, 100, 980, 140);  // 悔棋按钮
    fillrectangle(820, 160, 980, 200);  // 提示按钮
    fillrectangle(820, 220, 980, 260);  // 评估按钮
    fillrectangle(820, 280, 980, 320);  // 记录按钮

    settextstyle(20, 0, _T("宋体"));
    settextcolor(BLACK);
    setbkmode(TRANSPARENT);
    outtextxy(870, 105, _T("悔棋"));
    outtextxy(870, 165, _T("提示"));
    outtextxy(870, 225, _T("评估"));
    outtextxy(870, 285, _T("记录"));
    SetWorkingImage(NULL);

    // 主游戏循环
    printf("[MAIN] 进入主游戏循环\n");
    while (gameRunning) {
        bool hasInput = false;

        // 处理输入事件
        if (MouseHit()) {
            hasInput = true;
            MOUSEMSG mouseMsg = GetMouseMsg();
            printf("[INPUT] 鼠标事件: 类型=0x%X 坐标=(%d,%d)\n",
                mouseMsg.uMsg, mouseMsg.x, mouseMsg.y);

            if (mouseMsg.uMsg == WM_LBUTTONDOWN) {
                // 处理按钮点击
                if (mouseMsg.x >= 820 && mouseMsg.x <= 980) {
                    printf("[UI] 按钮区域点击\n");
                    if (mouseMsg.y >= 100 && mouseMsg.y <= 140) {  // 悔棋按钮
                        if (!board.history.empty() && board.notation && !board.notation->moves.empty()) {
                            undoMove(&board);
                            board.needRedraw = true;
                            printf("[BUTTON] 悔棋成功\n");
                        }
                        else {
                            printf("[BUTTON] 没有可悔的棋步！\n");
                            MessageBox(hWnd, _T("没有可悔的棋步！"), _T("提示"), MB_OK);
                        }
                    }
                    else if (mouseMsg.y >= 160 && mouseMsg.y <= 200) {  // 提示按钮
                        showHint(&board);
                        board.needRedraw = true;
                        printf("[BUTTON] 提示按钮点击，标记棋盘重绘\n");
                    }
                    else if (mouseMsg.y >= 220 && mouseMsg.y <= 260) {  // 评估按钮
                        showEvaluation(&board);
                        board.needRedraw = true;  // 确保触发重绘
                        printf("[BUTTON] 评估按钮点击\n");
                    }
                    else if (mouseMsg.y >= 280 && mouseMsg.y <= 320 && board.notation) {  // 记录按钮
                        showNotation(&board);
                        board.needRedraw = true;
                        printf("[BUTTON] 记录按钮点击，标记棋盘重绘\n");
                    }
                }


                // 处理棋盘点击
                int clickedX = (mouseMsg.y - 50) / 80;
                int clickedY = (mouseMsg.x - 100) / 80;

                if (clickedX >= 0 && clickedX < 8 && clickedY >= 0 && clickedY < 8) {
                    printf("[GAME] 棋盘点击: 逻辑坐标(%d,%d)\n", clickedX, clickedY);

                    if (selectedX == -1) {
                        if (isValidSelection(&board, clickedX, clickedY)) {
                            selectedX = clickedX;
                            selectedY = clickedY;
                            board.needRedraw = true;

                            // 立即高亮显示可移动位置
                            SetWorkingImage(&buffer);
                            drawBoard(&board, nullptr);
                            highlightMoves(&board, selectedX, selectedY);
                            SetWorkingImage(NULL);
                            putimage(0, 0, &buffer);
                        }
                    }
                    else {
                        // 检查是否是有效移动
                        if (isValidMove(&board, selectedX, selectedY, clickedX, clickedY, false)) {
                            // 处理有效移动
                            processMove(&board, selectedX, selectedY, clickedX, clickedY, false);
                            GameState state = checkGameState(&board);
                            if (state != NORMAL) {
                                gameRunning = false;
                            }
                        }
                        // 无论移动是否有效，都重置选中状态
                        selectedX = selectedY = -1;
                        board.needRedraw = true;
                    }
                }
            }
        }

        if (board.mode == SINGLE_PLAYER && currentPlayer == board.ai.color) {
            hasInput = true;
            printf("[AI] AI 思考落子开始\n");

            int bestMove[4] = { 0 };        // 定义存储最佳移动的数组
            AIMove(&board, bestMove);     // 传递 2 个参数获取最佳移动

            if (bestMove[2] != 0 || bestMove[3] != 0) {  // 检查是否找到有效移动
                processMove(&board, bestMove[0], bestMove[1], bestMove[2], bestMove[3], true);  // 执行移动
            }

            printf("[AI] AI 落子完成\n");
        }


        // 处理Windows消息
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            hasInput = true;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 只在有输入或需要重绘时渲染
        if (hasInput || board.needRedraw) {
            if (board.needRedraw) {
                printf("[RENDER] 开始渲染帧\n");
                SetWorkingImage(&buffer);
                clearrectangle(0, 0, 800, 700);
                drawBoard(&board, nullptr);

                if (selectedX != -1 && selectedY != -1) {
                    highlightMoves(&board, selectedX, selectedY);
                    printf("[RENDER] 高亮可移动位置，当前选中: (%d, %d)\n", selectedX, selectedY);
                }

                SetWorkingImage(NULL);
                putimage(0, 0, &buffer);
                printf("[RENDER] 渲染完成\n");
                board.needRedraw = false;
            }
        }
        else {
            Sleep(50);
        }
    }
    printf("[MAIN] 退出主循环\n");
    closegraph();
    if (board.notation != nullptr) {
        delete board.notation;
        board.notation = nullptr;
    }
    closegraph();
    return 0;
}