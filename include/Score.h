#pragma once
#include "Grid.h"
#include <vector>
#include <string>

// ===================== MODULE 3: CLEAR LINES & SCORING =====================
// Phụ trách: Thành viên C

struct ClearResult {
    std::vector<int> rows; // danh sách hàng đầy đủ 8 ô
    std::vector<int> cols; // danh sách cột đầy đủ 8 ô
};

// Việc 1-2: kiểm tra các hàng/cột đã đầy
ClearResult checkFullLines(int grid[GRID_SIZE][GRID_SIZE]);

// Việc 3: xóa các hàng/cột đã đầy (gán 0)
void clearLines(int grid[GRID_SIZE][GRID_SIZE], const ClearResult& result);

// Số khối tối đa được phép đặt liên tiếp mà KHÔNG ăn hàng/cột nào trước khi mất combo.
// Yêu cầu: "đặt quá 3 khối mới mất combo" -> cho phép 3 khối "ân hạn", sang khối thứ 4
// liên tiếp không ăn dòng nào thì combo mới bị reset về 0.
const int COMBO_GRACE_BLOCKS = 3;

// Việc 4-6: tính điểm cho 1 lượt đặt khối
// cellsPlaced: số ô vừa đặt (+1 điểm / ô)
// linesCleared: tổng số hàng+cột bị xóa trong lượt này
// streak: cấp độ combo hiện tại (số lượt liên tiếp có ăn hàng/cột), được cập nhật bên trong hàm.
// missStreak: số khối liên tiếp vừa đặt mà KHÔNG ăn được hàng/cột nào kể từ lần ăn combo gần nhất
//             (dùng để hiển thị "còn bao nhiêu khối" trước khi mất combo). Được cập nhật bên trong hàm.
// Combo càng cao (streak càng lớn) thì điểm thưởng combo càng tăng mạnh.
// Mở rộng: hệ số nhân điểm theo SỐ DÒNG ăn cùng lúc được điều chỉnh tăng dần rõ rệt
// hơn cho 2, 3, 4, 5+ dòng (xem bảng multiplier trong Score.cpp) để khuyến khích
// người chơi dồn nhiều dòng ăn một lượt thay vì ăn lẻ tẻ từng dòng một.
// isExplosiveBlock: true nếu lượt đặt này dùng khối NỔ (Bom/Đại Bác). Khi true và
// KHÔNG ăn được hàng/cột nào (linesCleared == 0), lượt đặt này sẽ KHÔNG bị tính vào
// "khối cho phép bỏ lỡ" (missStreak giữ nguyên, không tăng) - khối nổ luôn được xem
// là 1 nước đi "miễn phí", không làm hao hụt số khối ân hạn trước khi mất combo.
// Trả về điểm cộng thêm trong lượt này.
int computeScoreForMove(int cellsPlaced, int linesCleared, int& streak, int& missStreak,
                         bool isExplosiveBlock = false);

// Việc 7: đọc / ghi điểm cao nhất
int loadHighScore(const std::string& path);
void saveHighScore(const std::string& path, int score);

// Mở rộng: thưởng "dọn sạch bàn cờ" (All Clear) - nếu 1 lượt ăn hàng/cột khiến
// lưới KHÔNG còn ô nào bị chiếm, cộng thêm 10% tổng điểm hiện có.
int fullClearBonus(int currentScore);

// Mở rộng: mốc độ khó tăng dần vô hạn theo điểm số - cứ mỗi 10.000 điểm
// (10k, 20k, 30k, ...) độ khó tăng thêm 1 cấp. Cấp độ này được Module 4 (Spawn)
// dùng để: giảm bớt trợ giúp "khớp vừa khít", tăng tỉ lệ khối to (2x2/3x3),
// và tăng tần suất các màn "bắt giải đố" khi lưới gần đầy.
int difficultyLevel(int score);

// Mở rộng: mốc độ khó hiển thị trên giao diện (nhãn + màu nền), theo NGƯỠNG ĐIỂM
// cố định (khác với difficultyLevel ở trên - dùng cho Spawn, tăng vô hạn mỗi 10k):
//   0 : 0           - 2.500      Yên bình   (siêu dễ)
//   1 : 2.500       - 10.000     Dễ
//   2 : 10.000      - 50.000     Bình thường
//   3 : 50.000      - 250.000    Khó
//   4 : 250.000     - 1.000.000  Cực đoan   (siêu khó)
//   5 : 1.000.000   - 2.000.000  Điên rồ
//   6 : 2.000.000+               Bậc thầy   (đỉnh cao)
int difficultyTier(int score);

// Việc 8 (mở rộng): Thành tựu & bảng xếp hạng
struct Achievement {
    std::string name;
    bool unlocked;
};

std::vector<Achievement> checkAchievements(int totalLinesCleared, int maxCombo, int score);
void updateLeaderboard(const std::string& path, const std::string& playerName, int score);
