#pragma once
#include "Grid.h"

// ===================== MODULE 2: INPUT & PLACEMENT =====================
// Phụ trách: Thành viên B
// Trong bản SFML, "input" là chuột (kéo-thả) thay vì bàn phím như bản console,
// nhưng toàn bộ logic kiểm tra & đặt khối vẫn đúng như thiết kế ban đầu.

// Việc 4: kiểm tra có thể đặt khối tại (row, col) hay không
// (row, col) là vị trí của ô neo (0,0) của khối trên lưới.
bool canPlaceBlock(int grid[GRID_SIZE][GRID_SIZE], const Block& block, int row, int col);

// Việc 5: đặt khối vào lưới sau khi đã kiểm tra hợp lệ.
// bonusScore: điểm cộng thêm do hiệu ứng đặc biệt (vd: Bom phá vỡ các ô xung quanh).
void placeBlock(int grid[GRID_SIZE][GRID_SIZE], const Block& block, int row, int col, int& bonusScore);

// Việc 8 (mở rộng): gợi ý vị trí đặt tốt nhất cho khối đang chọn.
// Trả về true nếu tìm được vị trí hợp lệ, ghi kết quả vào outRow/outCol.
// Heuristic: ưu tiên vị trí giúp lấp đầy nhiều hàng/cột nhất (khả năng tạo combo).
bool giveHint(int grid[GRID_SIZE][GRID_SIZE], const Block& block, int& outRow, int& outCol);
