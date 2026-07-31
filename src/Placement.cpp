#include "Placement.h"
#include "Score.h"
#include <climits>

bool canPlaceBlock(int grid[GRID_SIZE][GRID_SIZE], const Block& block, int row, int col) {
    for (auto& cell : block.cells) {
        int r = row + cell.first;
        int c = col + cell.second;

        // Việc 4: không vượt biên lưới
        if (r < 0 || r >= GRID_SIZE || c < 0 || c >= GRID_SIZE) return false;

        // Wildcard được đặt đè lên ô khối người chơi, nhưng KHÔNG được đè lên ô đá
        if (!block.isSpecial || block.id != WILDCARD_ID) {
            if (grid[r][c] != 0) return false;
        } else {
            if (grid[r][c] == OBSTACLE_COLOR) return false;
        }
    }
    return true;
}

void placeBlock(int grid[GRID_SIZE][GRID_SIZE], const Block& block, int row, int col, int& bonusScore) {
    bonusScore = 0;

    // copy dữ liệu khối vào đúng vị trí trên grid
    for (auto& cell : block.cells) {
        int r = row + cell.first;
        int c = col + cell.second;
        grid[r][c] = block.color;
    }

    // Việc 8 (mở rộng): hiệu ứng khối Bom - phá nổ vùng 3x3 quanh vị trí neo
    if (block.isSpecial && block.id == BOMB_ID) {
        for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
                int r = row + dr, c = col + dc;
                if (r >= 0 && r < GRID_SIZE && c >= 0 && c < GRID_SIZE) {
                    if (grid[r][c] != 0) {
                        grid[r][c] = 0;
                        bonusScore += 2; // thưởng điểm cho mỗi ô bị phá
                    }
                }
            }
        }
    }

    // Mở rộng (cực hiếm): khối Đại Bác - hiện dạng 3x3 nhưng khi đặt sẽ phá nổ
    // toàn bộ vùng 5x5 xung quanh TÂM của khối (không phải quanh ô neo góc trái-trên),
    // nên tâm nổ = (row + 1, col + 1) vì khối 3x3 có tâm lệch (1,1) so với ô neo.
    if (block.isSpecial && block.id == SUPERBOMB_ID) {
        int centerR = row + 1;
        int centerC = col + 1;
        for (int dr = -2; dr <= 2; dr++) {
            for (int dc = -2; dc <= 2; dc++) {
                int r = centerR + dr, c = centerC + dc;
                if (r >= 0 && r < GRID_SIZE && c >= 0 && c < GRID_SIZE) {
                    if (grid[r][c] != 0) {
                        grid[r][c] = 0;
                        bonusScore += 3; // hiếm hơn Bomb thường -> thưởng điểm cao hơn mỗi ô bị phá
                    }
                }
            }
        }
    }
}

bool giveHint(int grid[GRID_SIZE][GRID_SIZE], const Block& block, int& outRow, int& outCol) {
    int bestScore = -1;
    bool found = false;

    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            if (!canPlaceBlock(grid, block, r, c)) continue;

            // thử đặt tạm trên bản sao lưới để chấm điểm heuristic
            int temp[GRID_SIZE][GRID_SIZE];
            for (int i = 0; i < GRID_SIZE; i++)
                for (int j = 0; j < GRID_SIZE; j++)
                    temp[i][j] = grid[i][j];

            for (auto& cell : block.cells) {
                temp[r + cell.first][c + cell.second] = block.color;
            }

            ClearResult cr = checkFullLines(temp);
            int linesCleared = static_cast<int>(cr.rows.size() + cr.cols.size());

            // điểm heuristic = số ô lấp được + trọng số lớn cho khả năng tạo combo
            int heuristic = countCells(block) + linesCleared * 20;

            if (heuristic > bestScore) {
                bestScore = heuristic;
                outRow = r;
                outCol = c;
                found = true;
            }
        }
    }
    return found;
}
