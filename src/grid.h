#pragma once
#include <vector>
#include <utility>

// ===================== MODULE 1: GRID & BLOCK DATA =====================
// Phụ trách: Thành viên A
// Lưới 8x8: grid[row][col], 0 = trống, 1-6 = mã màu khối đang chiếm ô đó,
// -1 = ô vừa được đánh dấu để xóa (dùng nội bộ khi tính hiệu ứng).

const int GRID_SIZE = 8;
const int OBSTACLE_COLOR = -2; // giá trị ô chướng ngại trên lưới (Survival)

// id 2 khối thẳng dài 5 ô (mở rộng): cho phép ăn được những khoảng trống dài hơn
// và làm bộ khối đa dạng hơn ở độ khó cao.
const int ROW5_ID = 13; // hàng ngang 5 ô liền
const int COL5_ID = 14; // cột dọc 5 ô liền

// Mở rộng: thêm 5 loại khối mới theo yêu cầu (id 15-24, liền sau COL5_ID để
// pickWeightedNormalId() trong Spawn.cpp có thể dùng chung 1 mảng trọng số theo id).
const int T_ID       = 15; // chữ T, khung 2x3
const int Z_ID       = 16; // chữ Z, khung 2x3
const int S_ID       = 17; // chữ Z đảo ngược (chữ S), khung 2x3
const int DIAG2A_ID  = 18; // chéo trong ô vuông 2x2 (chéo chính)
const int DIAG2B_ID  = 19; // chéo trong ô vuông 2x2 (chéo phụ)
const int DIAG3A_ID  = 20; // chéo trong ô vuông 3x3 (chéo chính)
const int DIAG3B_ID  = 21; // chéo trong ô vuông 3x3 (chéo phụ)
const int L3_ID       = 22; // chữ L, khung 3x3
const int RECT23_ID   = 23; // chữ nhật đặc 2x3 (nằm ngang)
const int RECT32_ID   = 24; // chữ nhật đặc 3x2 (đứng)

// id khối đặc biệt (tính năng mở rộng - Power-up Blocks)
const int BOMB_ID = 90;      // Khối Bom: phá nổ vùng 3x3 quanh vị trí đặt
const int WILDCARD_ID = 91;  // Khối Cầu vồng: đặt được vào ô bất kỳ (kể cả ô đã có khối)
const int SUPERBOMB_ID = 92; // Khối Đại Bác (cực hiếm): hiện dạng khối 3x3, khi đặt
                              // phá nổ toàn bộ vùng 5x5 xung quanh tâm khối.

struct Block {
    int id = -1;
    std::vector<std::pair<int, int>> cells; // toạ độ (dRow, dCol) các ô được tô, offset từ (0,0)
    int color = 1;        // mã màu 1-6
    bool isSpecial = false; // true nếu là Bomb hoặc Wildcard
};

// Việc 4: gán toàn bộ lưới về 0
void resetGrid(int grid[GRID_SIZE][GRID_SIZE]);

// Mở rộng: true nếu TOÀN BỘ lưới đang trống (64/64 ô = 0). Dùng để thưởng
// điểm "dọn sạch bàn cờ" (Perfect/All Clear) sau khi ăn hàng/cột.
bool isGridEmpty(int grid[GRID_SIZE][GRID_SIZE]);

// Việc 5: in lưới ra console (debug/kiểm thử nhanh, dùng ký hiệu số)
void printGrid(int grid[GRID_SIZE][GRID_SIZE]);

// Việc 6: trả về hình dạng chuẩn của khối theo id (dùng chung cho cả nhóm)
Block getBlockShape(int id);

// Việc 7: đếm số ô có màu trong 1 khối
int countCells(const Block& b);

// Kích thước bounding box (số hàng, số cột) của 1 khối - phục vụ vẽ & đặt khối
std::pair<int, int> blockBoundingBox(const Block& b);
