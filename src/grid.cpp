#include "Grid.h"
#include <iostream>
#include <algorithm>
#include <string>

void resetGrid(int grid[GRID_SIZE][GRID_SIZE]) {
    for (int r = 0; r < GRID_SIZE; r++)
        for (int c = 0; c < GRID_SIZE; c++)
            grid[r][c] = 0;
}

bool isGridEmpty(int grid[GRID_SIZE][GRID_SIZE]) {
    for (int r = 0; r < GRID_SIZE; r++)
        for (int c = 0; c < GRID_SIZE; c++)
            if (grid[r][c] > 0) return false; // Chỉ quan tâm khối của người chơi (> 0), bỏ qua đá (-2)
    return true;
}

void printGrid(int grid[GRID_SIZE][GRID_SIZE]) {
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            std::cout << (grid[r][c] == 0 ? "." : std::to_string(grid[r][c])) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "-------------------------\n";
}

// Việc 3: khai báo đầy đủ hình dạng chuẩn cho >= 8 loại khối
// (1 ô, ngang/dọc 2-3-4 ô, vuông 2x2, vuông 3x3, chữ L với các biến thể xoay)
Block getBlockShape(int id) {
    Block b;
    b.id = id;

    switch (id) {
        case 0: // 1 ô
            b.cells = {{0,0}};
            break;
        case 1: // ngang 2 ô
            b.cells = {{0,0},{0,1}};
            break;
        case 2: // ngang 3 ô
            b.cells = {{0,0},{0,1},{0,2}};
            break;
        case 3: // ngang 4 ô
            b.cells = {{0,0},{0,1},{0,2},{0,3}};
            break;
        case 4: // dọc 2 ô
            b.cells = {{0,0},{1,0}};
            break;
        case 5: // dọc 3 ô
            b.cells = {{0,0},{1,0},{2,0}};
            break;
        case 6: // dọc 4 ô
            b.cells = {{0,0},{1,0},{2,0},{3,0}};
            break;
        case 7: // vuông 2x2
            b.cells = {{0,0},{0,1},{1,0},{1,1}};
            break;
        case 8: // vuông 3x3
            for (int r = 0; r < 3; r++)
                for (int c = 0; c < 3; c++)
                    b.cells.push_back({r,c});
            break;
        case 9: // chữ L - biến thể 1
            b.cells = {{0,0},{1,0},{2,0},{2,1}};
            break;
        case 10: // chữ L - biến thể 2 (xoay 90 độ)
            b.cells = {{0,0},{0,1},{0,2},{1,0}};
            break;
        case 11: // chữ L - biến thể 3 (xoay 180 độ)
            b.cells = {{0,0},{0,1},{1,1},{2,1}};
            break;
        case 12: // chữ L - biến thể 4 (xoay 270 độ)
            b.cells = {{1,0},{1,1},{1,2},{0,2}};
            break;
        case ROW5_ID: // mở rộng: hàng ngang 5 ô
            b.cells = {{0,0},{0,1},{0,2},{0,3},{0,4}};
            break;
        case COL5_ID: // mở rộng: cột dọc 5 ô
            b.cells = {{0,0},{1,0},{2,0},{3,0},{4,0}};
            break;
        case T_ID: // mở rộng: chữ T, khung 2x3
            // X X X
            // . X .
            b.cells = {{0,0},{0,1},{0,2},{1,1}};
            break;
        case Z_ID: // mở rộng: chữ Z, khung 2x3
            // X X .
            // . X X
            b.cells = {{0,0},{0,1},{1,1},{1,2}};
            break;
        case S_ID: // mở rộng: chữ Z đảo ngược (chữ S), khung 2x3
            // . X X
            // X X .
            b.cells = {{0,1},{0,2},{1,0},{1,1}};
            break;
        case DIAG2A_ID: // mở rộng: chéo trong ô vuông 2x2 (chéo chính \ )
            b.cells = {{0,0},{1,1}};
            break;
        case DIAG2B_ID: // mở rộng: chéo trong ô vuông 2x2 (chéo phụ / )
            b.cells = {{0,1},{1,0}};
            break;
        case DIAG3A_ID: // mở rộng: chéo trong ô vuông 3x3 (chéo chính \ )
            b.cells = {{0,0},{1,1},{2,2}};
            break;
        case DIAG3B_ID: // mở rộng: chéo trong ô vuông 3x3 (chéo phụ / )
            b.cells = {{0,2},{1,1},{2,0}};
            break;
        case L3_ID: // mở rộng: chữ L, khung 3x3
            // X . .
            // X . .
            // X X X
            b.cells = {{0,0},{1,0},{2,0},{2,1},{2,2}};
            break;
        case RECT23_ID: // mở rộng: chữ nhật đặc 2x3 (nằm ngang)
            b.cells = {{0,0},{0,1},{0,2},{1,0},{1,1},{1,2}};
            break;
        case RECT32_ID: // mở rộng: chữ nhật đặc 3x2 (đứng)
            b.cells = {{0,0},{0,1},{1,0},{1,1},{2,0},{2,1}};
            break;
        case BOMB_ID: // Việc 8 (mở rộng): khối Bom - hiển thị 1 ô, khi đặt sẽ phá nổ 3x3
            b.cells = {{0,0}};
            b.isSpecial = true;
            break;
        case WILDCARD_ID: // Việc 8 (mở rộng): khối Cầu vồng - đặt được vào ô bất kỳ
            b.cells = {{0,0}};
            b.isSpecial = true;
            break;
        case SUPERBOMB_ID: // mở rộng (cực hiếm): hiện dạng khối 3x3, nổ vùng 5x5 khi đặt
            for (int r = 0; r < 3; r++)
                for (int c = 0; c < 3; c++)
                    b.cells.push_back({r,c});
            b.isSpecial = true;
            break;
        default:
            b.cells = {{0,0}};
            break;
    }
    return b;
}

int countCells(const Block& b) {
    return static_cast<int>(b.cells.size());
}

std::pair<int, int> blockBoundingBox(const Block& b) {
    int maxR = 0, maxC = 0;
    for (auto& cell : b.cells) {
        maxR = std::max(maxR, cell.first);
        maxC = std::max(maxC, cell.second);
    }
    return {maxR + 1, maxC + 1};
}
