#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <string.h> // memcpy를 사용하기 위해 추가

// =========================================================
// 1. 전역 변수 및 상수 정의
// =========================================================

#define BOARD_HEIGHT 20
#define BOARD_WIDTH 10
#define BLOCK_SIZE 3 // 현재 블록 배열 크기

// 게임 보드: 0은 빈 공간, 1은 쌓인 블록
int board[BOARD_HEIGHT][BOARD_WIDTH] = {0};

// 보드 출력 시작 좌표 (화면 기준)
int board_x_offset = 35;
int board_y_offset = 2;

// 현재 움직이는 블록의 보드 내 위치 (행/열)
int current_block_r = 0; // Row
int current_block_c = 3; // Column

// 화면 좌표 (기존 코드에서 사용되었으나, 보드 기반 로직으로 변경)
// int x = 35, y = 12; // 더 이상 직접 사용하지 않음
// int inx = 0, iny = 0; // 사용자 입력 방향 (더 이상 직접 사용하지 않음)

// 현재 움직이는 블록의 모양 (main에서 초기화됨)
int current_shape[BLOCK_SIZE][BLOCK_SIZE];

// 새로운 블록을 위한 템플릿 (임의의 3x3 블록 정의)
int shape1[BLOCK_SIZE][BLOCK_SIZE] = {
    {0, 1, 0},
    {0, 1, 0},
    {1, 1, 1}
};

// =========================================================
// 2. 함수 선언
// =========================================================
void gotoxy(int x, int y);
void print_direction(void);
void rotation_right(int m[][BLOCK_SIZE]);
int check_collision(int m[][BLOCK_SIZE], int next_r, int next_c);
void lock_block(int m[][BLOCK_SIZE]);
int check_and_clear_lines(void);
void print_board_and_shape(int m[][BLOCK_SIZE]);
void move_shape_auto(int m[][BLOCK_SIZE]); // 자동 하강 처리
void move_control(int m[][BLOCK_SIZE]);


// =========================================================
// 3. main 함수
// =========================================================
int main(void)
{
    // 시작 시 현재 블록 모양 설정
    memcpy(current_shape, shape1, sizeof(shape1));
    
    // 게임 시작
    move_control(current_shape);
    
    return 0;
}


// =========================================================
// 4. 충돌 및 보드 관리 함수
// =========================================================

/**
 * @brief 블록 m이 (next_r, next_c) 위치에 놓일 때 보드와 충돌하는지 확인합니다.
 * @return 0: 충돌 없음, 1: 충돌 있음
 */
int check_collision(int m[][BLOCK_SIZE], int next_r, int next_c) {
    int i, j;
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            if (m[i][j] == 1) {
                int br = next_r + i; // 보드 행
                int bc = next_c + j; // 보드 열

                // 1. 보드 경계 충돌 체크 (아래 경계와 좌/우 경계)
                if (bc < 0 || bc >= BOARD_WIDTH || br >= BOARD_HEIGHT) {
                    return 1; // 경계 벗어남 -> 충돌
                }
                
                // 2. 쌓인 블록과의 충돌 체크 (보드 맨 위 경계는 예외적으로 허용하지 않음)
                // br < 0인 경우(생성 위치)는 경계 충돌에 포함하지 않지만, 쌓인 블록과 겹치는지 체크
                if (br >= 0 && board[br][bc] == 1) {
                    return 1; // 이미 쌓인 블록과 충돌
                }
            }
        }
    }
    return 0; // 충돌 없음
}

/**
 * @brief 현재 블록의 모양을 게임 보드에 고정합니다.
 */
void lock_block(int m[][BLOCK_SIZE]) {
    int i, j;
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            if (m[i][j] == 1) {
                int br = current_block_r + i;
                int bc = current_block_c + j;
                
                // 보드 범위 내에 있는 블록만 고정
                if (br >= 0 && br < BOARD_HEIGHT && bc >= 0 && bc < BOARD_WIDTH) {
                    board[br][bc] = 1;
                }
            }
        }
    }
}

/**
 * @brief 완성된 줄을 확인하고 제거하며, 위의 블록들을 아래로 내립니다.
 * @return 제거된 줄의 개수
 */
int check_and_clear_lines(void) {
    int lines_cleared = 0;
    int r, c, i, j;

    for (r = BOARD_HEIGHT - 1; r >= 0; r--) {
        int is_line_full = 1;
        
        // 1. 현재 줄이 가득 찼는지 확인
        for (c = 0; c < BOARD_WIDTH; c++) {
            if (board[r][c] == 0) {
                is_line_full = 0;
                break;
            }
        }

        if (is_line_full) {
            lines_cleared++;
            
            // 2. 줄 제거: 현재 줄 r을 지우고, 그 위의 모든 줄을 한 칸씩 아래로 내림
            for (i = r; i > 0; i--) {
                for (j = 0; j < BOARD_WIDTH; j++) {
                    board[i][j] = board[i - 1][j];
                }
            }
            // 맨 윗줄은 비움
            for (j = 0; j < BOARD_WIDTH; j++) {
                board[0][j] = 0;
            }

            // 줄이 제거되었으므로, 현재 행을 다시 검사하기 위해 r을 증가 (다음 반복에서 감소)
            r++; 
        }
    }
    return lines_cleared;
}


// =========================================================
// 5. 화면 출력 및 제어 함수
// =========================================================

/**
 * @brief 콘솔 커서 위치를 이동시킵니다.
 */
void gotoxy(int x, int y)
{
    COORD Pos = {x * 2, y}; // 한글 2바이트 문자를 고려하여 x좌표를 *2
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}

/**
 * @brief 게임 조작법을 출력합니다.
 */
void print_direction(void)
{
    gotoxy(25, 1);
    printf("화살표: 이동, 스페이스키: 회전, ESC: 종료");
}

/**
 * @brief 보드와 움직이는 블록을 모두 출력합니다.
 */
void print_board_and_shape(int m[][BLOCK_SIZE])
{
    int i, j;
    
    // 1. 쌓여있는 보드 출력
    for (i = 0; i < BOARD_HEIGHT; i++) {
        gotoxy(board_x_offset, board_y_offset + i);
        for (j = 0; j < BOARD_WIDTH; j++) {
            if (board[i][j] == 1) {
                printf("■"); // 쌓인 블록
            } else {
                printf("  "); // 빈 공간
            }
        }
    }
    
    // 2. 현재 움직이는 블록 출력
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            if (m[i][j] == 1) {
                 // 보드 좌표를 화면 좌표로 변환하여 출력
                 gotoxy(board_x_offset + current_block_c * 2 + j * 2, board_y_offset + current_block_r + i);
                 printf("□");
            }
        }
    }
    print_direction();
}

/**
 * @brief 블록을 시계 방향으로 90도 회전시킵니다.
 */
void rotation_right(int m[][BLOCK_SIZE])
{
    int i, j;
    int temp[BLOCK_SIZE][BLOCK_SIZE];
    
    // 회전된 모양을 temp에 저장
    for(i = 0; i < BLOCK_SIZE; i++) {
        for(j = 0; j < BLOCK_SIZE; j++) {
            temp[j][BLOCK_SIZE - 1 - i] = m[i][j];
        }
    }
    
    // 회전된 블록이 충돌하는지 임시로 확인 (벽/다른 블록)
    if (check_collision(temp, current_block_r, current_block_c) == 0) {
        // 충돌이 없으면 m에 적용
        for(i = 0; i < BLOCK_SIZE; i++) {
            for(j = 0; j < BLOCK_SIZE; j++) {
                m[i][j] = temp[i][j];
            }
        }
    }
    // 충돌이 있으면 회전하지 않고 함수 종료
}

/**
 * @brief 블록을 자동으로 한 칸 하강시키거나, 착지 시 고정 및 줄을 제거합니다.
 */
void move_shape_auto(int m[][BLOCK_SIZE])
{
    int next_r = current_block_r + 1;
    
    // 1. 다음 위치로 하강 가능 여부 체크
    if (check_collision(m, next_r, current_block_c) == 0) {
        // 하강 가능: 위치 업데이트
        current_block_r = next_r;
    } else {
        // 하강 불가능 (착지):
        
        // 2. 블록을 보드에 고정
        lock_block(m); 
        
        // 3. 줄 제거 및 보드 업데이트
        check_and_clear_lines();
        
        // 4. 새로운 블록 생성 (여기서는 shape1로 임시 대체)
        memcpy(m, shape1, sizeof(shape1));
        current_block_r = 0;
        current_block_c = 3;

        // 5. 새 블록이 생성되자마자 충돌하면 게임 오버 (테트리스 로직에 따라 구현 필요)
        if (check_collision(m, current_block_r, current_block_c) == 1) {
            // 게임 오버 처리
            system("cls");
            gotoxy(board_x_offset, board_y_offset + BOARD_HEIGHT / 2);
            printf("GAME OVER");
            getch();
            exit(0);
        }
    }
}


/**
 * @brief 사용자 키 입력을 처리하고 블록을 이동/회전시킵니다.
 */
void move_control(int m[][BLOCK_SIZE])
{
    char key;
    
    do {
        // 1. 키 입력이 없을 때 자동 하강 및 화면 갱신
        while(!kbhit()) {
            system("cls");
            move_shape_auto(m);
            print_board_and_shape(m);
            Sleep(500); // 하강 속도 조절 (500ms마다 한 칸)
        }
        
        // 2. 키 입력 처리
        key = getch();
        int next_r = current_block_r;
        int next_c = current_block_c;
        
        if (key == 0 || key == 0xE0) { // 특수 키 (방향키)
            key = getch(); // 두 번째 바이트 읽기
            switch(key) {
                case 72 : /* Up (기존 코드와 달리 이동 없음. 즉시 바닥으로 내리기 기능을 추가할 수 있음) */ 
                    break;
                case 75 : /* Left */
                    next_c = current_block_c - 1;
                    break;
                case 77 : /* Right */
                    next_c = current_block_c + 1;
                    break;
                case 80 : /* Down (Fast Drop) */
                    next_r = current_block_r + 1;
                    break;
                default : break;
            }
        } else { // 일반 키
            switch(key) {
                case 32 : // Space: 회전
                    rotation_right(m);
                    break;
                default : break;
            }
        }

        // 3. 이동 가능한지 체크 및 위치 업데이트
        if (check_collision(m, next_r, next_c) == 0) {
            current_block_r = next_r;
            current_block_c = next_c;
        } else if (key == 80) { // 아래 화살표로 충돌 시 (착지)
            // 즉시 착지 로직을 추가할 수 있으나, 여기서는 단순 무시
        }

        // 4. 화면 갱신 (키 입력 후)
        system("cls");
        print_board_and_shape(m);
        
    } while (key != 27); // ESC 키로 종료
    
    printf("\n");
}