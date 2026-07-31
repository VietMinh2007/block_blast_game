#pragma once
#include "Grid.h"
#include <array>

// ===================== MODULE 4: SPAWN & GAME OVER =====================
// Phụ trách: Thành viên D

// Việc 1-2: sinh ngẫu nhiên 3 khối mới (đã tự động seed random trong main)
// grid: trạng thái lưới HIỆN TẠI, dùng để:
//   - Tăng tỉ lệ chọn khối "khớp vừa khít" các khoảng trống đang có trên lưới
//     (ăn trọn vẹn hàng/cột, không dư không thiếu), kể cả xuất hiện liên tiếp nhiều lượt.
//   - Đảm bảo mỗi khối sinh ra CHẮC CHẮN có ít nhất 1 vị trí đặt được trên lưới hiện tại,
//     tránh trường hợp dồn khối khiến người chơi thua ngay, đặc biệt lúc mới bắt đầu ván.
// score: điểm hiện tại của người chơi, dùng để tính độ khó (difficultyLevel trong Score.h):
//   - Độ khó càng cao -> giảm trợ giúp "khớp vừa khít", tăng tỉ lệ khối to (2x2/3x3/5 ô).
//   - Khi lưới đã đầy >= 1/2, có xác suất cố tình sinh 1 trong 3 khối KHÔNG đặt được
//     ngay lúc đó, buộc người chơi phải dùng 2 khối còn lại ăn hàng/cột để dọn chỗ.
//   - Khi lưới đầy >= 2/3, vào "chế độ giải đố": khối dự phòng đảm bảo đặt được sẽ ưu
//     tiên khớp CHÍNH XÁC vào khoảng trống hiện có thay vì random tự do, đòi hỏi người
//     chơi đặt đúng vị trí mới sống sót.
// classicHardMode: CHỈ truyền true khi đang chơi chế độ Classic. Khi bật, độ khó theo
//   điểm (khối to hơn, ít khớp vừa khít hơn, nhiều "bắt giải đố" hơn) tăng nhanh và
//   mạnh hơn hẳn mặc định - Time Attack/Survival KHÔNG bị ảnh hưởng vì luôn gọi hàm
//   này với classicHardMode = false (giữ nguyên hành vi cũ của 2 chế độ đó).
std::array<Block, 3> spawnNewBlocks(int grid[GRID_SIZE][GRID_SIZE], int score, bool classicHardMode = false);

// Việc 3: kiểm tra thua - true nếu không khối nào trong khay còn đặt được vào lưới
// `used[i] = true` nghĩa là khối ở vị trí i trong khay đã được dùng (bỏ qua khi kiểm tra)
bool isGameOver(int grid[GRID_SIZE][GRID_SIZE], const std::array<Block, 3>& tray, const std::array<bool, 3>& used);
