#include "Spawn.h"
#include "Placement.h"
#include "Score.h"
#include <cstdlib>
#include <vector>
#include <random>
#include <algorithm>

// id các khối "đường thẳng" theo độ dài 1..5, dùng để lấp vừa khít 1 khoảng trống
// (ăn trọn vẹn hàng/cột, không dư không thiếu). Xem thứ tự id trong Grid.cpp::getBlockShape.
static const int LINE_H_IDS[5] = {0, 1, 2, 3, ROW5_ID}; // ngang: 1,2,3,4,5 ô
static const int LINE_V_IDS[5] = {0, 4, 5, 6, COL5_ID}; // dọc:   1,2,3,4,5 ô

// Ghi nhớ giữa các lượt gọi: số lượt liên tiếp gần đây đã sinh được khối "khớp vừa khít"
// -> dùng để tăng tỉ lệ xuất hiện liên tiếp của combo khớp vừa khít.
static int s_perfectFitStreak = 0;

// Bộ sinh số ngẫu nhiên có độ chính xác cao (dùng cho các xác suất lẻ như 0,2007%
// của khối Đại Bác cực hiếm) - rand()%N không đủ mịn cho các tỉ lệ dưới 1%.
static std::mt19937& rng() {
    static std::mt19937 engine(std::random_device{}());
    return engine;
}
static double rollUnit() { // trả về số thực ngẫu nhiên trong [0, 1)
    static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng());
}

// Kiểm tra 1 khối có thể đặt được ở BẤT KỲ vị trí nào trên lưới hiện tại không.
static bool canPlaceAnywhere(int grid[GRID_SIZE][GRID_SIZE], const Block& b) {
    for (int r = 0; r < GRID_SIZE; r++)
        for (int c = 0; c < GRID_SIZE; c++)
            if (canPlaceBlock(grid, b, r, c)) return true;
    return false;
}

// Thông tin 1 khoảng trống liên tục (1-4 ô) trên 1 hàng hoặc 1 cột, phần còn lại của
// hàng/cột đó đã được lấp đầy -> nếu đặt đúng khối đường thẳng dài bằng đúng khoảng trống
// này thì hàng/cột sẽ được ăn trọn vẹn, không dư không thiếu.
struct GapInfo { bool horizontal; int index; int start; int len; };

static std::vector<GapInfo> findFillableGaps(int grid[GRID_SIZE][GRID_SIZE]) {
    std::vector<GapInfo> gaps;

    // các hàng ngang
    for (int r = 0; r < GRID_SIZE; r++) {
        int emptyCount = 0, start = -1;
        for (int c = 0; c < GRID_SIZE; c++) {
            if (grid[r][c] == 0) { emptyCount++; if (start == -1) start = c; }
        }
        if (emptyCount >= 1 && emptyCount <= 5) {
            bool contiguous = true;
            for (int c = start; c < start + emptyCount; c++) {
                if (grid[r][c] != 0) { contiguous = false; break; }
            }
            if (contiguous) gaps.push_back({true, r, start, emptyCount});
        }
    }

    // các cột dọc
    for (int c = 0; c < GRID_SIZE; c++) {
        int emptyCount = 0, start = -1;
        for (int r = 0; r < GRID_SIZE; r++) {
            if (grid[r][c] == 0) { emptyCount++; if (start == -1) start = r; }
        }
        if (emptyCount >= 1 && emptyCount <= 5) {
            bool contiguous = true;
            for (int r = start; r < start + emptyCount; r++) {
                if (grid[r][c] != 0) { contiguous = false; break; }
            }
            if (contiguous) gaps.push_back({false, c, start, emptyCount});
        }
    }

    return gaps;
}

// Chọn 1 id "khối thường" (0..NORMAL_ID_COUNT-1) theo trọng số, ưu tiên các khối to
// (2x2 id=7, 3x3 id=8, đường thẳng 5 ô id=ROW5_ID/COL5_ID) nhiều hơn khối nhỏ, và ưu
// tiên mạnh hơn nữa khi độ khó (difficulty) tăng -> lưới nhanh chật hơn, thử thách hơn.
static int pickWeightedNormalId(int difficulty) {
    // id 0..14 (khối gốc + ROW5_ID/COL5_ID) + id 15..24 (5 loại khối mới: T, Z/S,
    // chéo 2x2/3x3, L 3x3, chữ nhật 2x3/3x2) => tổng cộng 25 id liền nhau.
    const int NORMAL_ID_COUNT = 25;
    static const int BASE_W = 10;

    std::vector<int> w(NORMAL_ID_COUNT, BASE_W);
    w[7] = 16;                                   // vuông 2x2: phổ biến hơn 1 chút
    w[8] = std::min(14 + difficulty * 3, 34);    // vuông 3x3: to hơn, cấp độ càng cao càng hay gặp
    w[ROW5_ID] = std::min(7 + difficulty * 2, 20);
    w[COL5_ID] = std::min(7 + difficulty * 2, 20);

    // Khối mới: độ khó càng cao thì các khối to (L 3x3, chữ nhật) càng hay gặp hơn,
    // còn khối chéo (khó đặt khít) giữ tần suất thấp - vừa đủ tạo biến hoá, không
    // làm bàn cờ quá rối.
    w[T_ID] = 13;
    w[Z_ID] = 12;
    w[S_ID] = 12;
    w[DIAG2A_ID] = 7;
    w[DIAG2B_ID] = 7;
    w[DIAG3A_ID] = std::min(5 + difficulty, 12);
    w[DIAG3B_ID] = std::min(5 + difficulty, 12);
    w[L3_ID] = std::min(12 + difficulty * 2, 26);
    w[RECT23_ID] = std::min(11 + difficulty * 2, 24);
    w[RECT32_ID] = std::min(11 + difficulty * 2, 24);

    int total = 0;
    for (int x : w) total += x;
    int roll = rand() % total;
    for (int id = 0; id < NORMAL_ID_COUNT; id++) {
        if (roll < w[id]) return id;
        roll -= w[id];
    }
    return 0; // không bao giờ tới đây, phòng hờ
}

std::array<Block, 3> spawnNewBlocks(int grid[GRID_SIZE][GRID_SIZE], int score, bool classicHardMode) {
    std::array<Block, 3> tray;

    // Mở rộng: mốc độ khó tăng dần vô hạn theo điểm (10k, 20k, 30k, ...).
    int difficulty = difficultyLevel(score);

    // Mở rộng CLASSIC: chỉ khi classicHardMode = true (đang chơi Classic) VÀ người chơi
    // đã lên tới tier "Cực đoan" trở lên (difficultyTier >= 4, tức từ 250.000 điểm), mới
    // tăng tốc độ leo thang độ khó rõ rệt so với mặc định - có sàn ngay từ đầu (+2) và
    // nhân đôi theo điểm, khiến các khối to (2x2/3x3/L/chữ nhật...) xuất hiện dày hơn,
    // nhanh hơn hẳn. Ở các tier thấp hơn (Yên bình/Dễ/Bình thường/Khó), Classic dùng
    // đúng độ khó mặc định như Time Attack/Survival - KHÔNG bị tăng thêm nữa, để những
    // tier này chơi nhẹ nhàng, dễ chịu hơn như yêu cầu. classicBoost = cờ dùng chung cho
    // mọi điều chỉnh "khó thêm riêng cho Classic" bên dưới (challengeRound, puzzleMode,
    // perfectFitChance, specialChance...).
    bool classicBoost = classicHardMode && difficultyTier(score) >= 4;
    if (classicBoost) difficulty = difficulty * 2 + 2;

    // Mở rộng: tỉ lệ lấp lưới hiện tại, dùng để kích hoạt các cơ chế "bắt giải đố".
    int filledCells = 0;
    for (int r = 0; r < GRID_SIZE; r++)
        for (int c = 0; c < GRID_SIZE; c++)
            if (grid[r][c] != 0) filledCells++;
    double occupancy = filledCells / static_cast<double>(GRID_SIZE * GRID_SIZE);

    // >= 1/2 lưới bị chiếm: có xác suất cố tình để lại 1 khối KHÔNG đặt được ngay,
    // buộc người chơi phải dùng 2 khối còn lại ăn hàng/cột trước để dọn chỗ cho nó.
    // Ở Classic từ tier Cực đoan trở lên (classicBoost), ngưỡng kích hoạt sớm hơn
    // (>= 40% lưới) và xác suất xảy ra cao hơn hẳn (50% thay vì 35%). Ở các tier thấp
    // hơn của Classic, dùng lại ngưỡng mặc định giống Time Attack/Survival cho nhẹ nhàng.
    bool challengeRound = classicBoost
        ? (occupancy >= 0.40 && occupancy < (2.0 / 3.0)) && (rollUnit() < 0.50)
        : (occupancy >= 0.5 && occupancy < (2.0 / 3.0)) && (rollUnit() < 0.35);
    // >= 2/3 lưới bị chiếm: "chế độ giải đố" - khối dự phòng (an toàn) sẽ ưu tiên khớp
    // CHÍNH XÁC vào khoảng trống hiện có thay vì random tự do -> đòi hỏi đặt chuẩn xác.
    // Ở Classic từ tier Cực đoan trở lên, ngưỡng vào "giải đố" sớm hơn (>= 58% thay vì
    // 2/3 ~ 66.7%). Các tier thấp hơn của Classic dùng lại ngưỡng mặc định.
    bool puzzleMode = classicBoost ? (occupancy >= 0.58) : (occupancy >= (2.0 / 3.0));

    // Tìm các khoảng trống có thể lấp vừa khít trên lưới hiện tại.
    std::vector<GapInfo> gaps = findFillableGaps(grid);

    // Tỉ lệ cơ bản (%) để cố gắng chọn khối khớp vừa khít cho MỖI khối trong khay.
    // Tăng theo số lượt liên tiếp gần đây đã có khớp vừa khít (hiệu ứng "xuất hiện liên tiếp"),
    // nhưng GIẢM dần theo độ khó để lưới khó lấp gọn hơn ở cấp cao.
    int perfectFitChance = 42 - std::min(difficulty * 2, 20); // tối thiểu 22
    // Ở Classic từ tier Cực đoan trở lên, giảm thêm trợ giúp "khớp vừa khít" để đòi hỏi
    // người chơi tính toán kỹ hơn. Các tier thấp hơn của Classic giữ trợ giúp như mặc định.
    if (classicBoost) perfectFitChance -= 12;
    perfectFitChance += std::min(s_perfectFitStreak * 12, 30); // tối đa +30%
    int perfectFitFloor = classicBoost ? 8 : 15;
    if (perfectFitChance < perfectFitFloor) perfectFitChance = perfectFitFloor;
    bool usedPerfectFitThisRound = false;

    for (int i = 0; i < 3; i++) {
        Block b;
        bool assigned = false;

        // Mở rộng (cực hiếm, khoảng 0,2007%): khối Đại Bác - hiện dạng 3x3, nổ vùng 5x5.
        if (rollUnit() < 0.002007) {
            b = getBlockShape(SUPERBOMB_ID);
            assigned = true;
        }

        // Ưu tiên chọn khối "đường thẳng" khớp vừa khít 1 khoảng trống đang có trên lưới.
        if (!assigned && !gaps.empty() && (rand() % 100) < perfectFitChance) {
            const GapInfo& g = gaps[rand() % gaps.size()];
            int id = g.horizontal ? LINE_H_IDS[g.len - 1] : LINE_V_IDS[g.len - 1];
            b = getBlockShape(id);
            assigned = true;
            usedPerfectFitThisRound = true;
        }

        if (!assigned) {
            int roll = rand() % 100; // 0-99
            // Việc 8 (mở rộng): tăng nhẹ tỉ lệ khối đặc biệt so với bản gốc (5% -> 7%),
            // tăng thêm chút theo độ khó (tối đa +3%).
            // Ở Classic từ tier Cực đoan trở lên, nới trần lên hẳn (tối đa +6%) để khối
            // bom/wildcard xuất hiện dày hơn - buộc người chơi xử lý tình huống khó hơn.
            // Các tier thấp hơn của Classic dùng trần mặc định như Time Attack/Survival.
            int specialChance = classicBoost ? std::min(7 + difficulty, 13) : std::min(7 + difficulty, 10);
            int id;
            if (roll < specialChance) {
                id = (roll < specialChance / 2) ? BOMB_ID : WILDCARD_ID;
            } else {
                id = pickWeightedNormalId(difficulty);
            }
            b = getBlockShape(id);
        }

        b.color = 1 + (rand() % 6); // mã màu 1-6

        // Đảm bảo khối chắc chắn đặt được ở đâu đó trên lưới hiện tại: không dồn khối
        // khiến người chơi thua ngay, đặc biệt là lúc mới bắt đầu ván / đầu mỗi đợt 3 khối.
        // NGOẠI LỆ: nếu đây là khối thứ 3 (i==2) và "challengeRound" đang kích hoạt, CỐ Ý
        // bỏ qua đảm bảo này -> khối có thể chưa đặt được ngay, buộc dùng 2 khối trước ăn dòng.
        bool skipGuarantee = (i == 2 && challengeRound);

        if (!skipGuarantee) {
            int guard = 0;
            while (!canPlaceAnywhere(grid, b) && guard < 20) {
                int id;
                if (puzzleMode && !gaps.empty()) {
                    // Chế độ giải đố: khối "cứu cánh" cũng phải khớp đúng khoảng trống hiện
                    // có (không random tự do), buộc người chơi đặt càng lúc càng chuẩn xác.
                    const GapInfo& g = gaps[rand() % gaps.size()];
                    id = g.horizontal ? LINE_H_IDS[g.len - 1] : LINE_V_IDS[g.len - 1];
                } else {
                    id = pickWeightedNormalId(difficulty);
                }
                b = getBlockShape(id);
                b.color = 1 + (rand() % 6);
                guard++;
            }
            // Trường hợp cực hiếm lưới gần kín hết: dùng khối 1 ô, luôn có khả năng đặt được cao nhất.
            if (!canPlaceAnywhere(grid, b)) {
                b = getBlockShape(0);
                b.color = 1 + (rand() % 6);
            }
        }

        tray[i] = b;
    }

    // Cập nhật streak khớp vừa khít để tăng khả năng xuất hiện liên tiếp ở các lượt sau.
    s_perfectFitStreak = usedPerfectFitThisRound ? s_perfectFitStreak + 1 : 0;

    return tray;
}

bool isGameOver(int grid[GRID_SIZE][GRID_SIZE], const std::array<Block, 3>& tray, const std::array<bool, 3>& used) {
    for (int i = 0; i < 3; i++) {
        if (used[i]) continue; // khối đã dùng thì bỏ qua

        // thử tất cả 64 vị trí trên lưới cho khối này
        for (int r = 0; r < GRID_SIZE; r++) {
            for (int c = 0; c < GRID_SIZE; c++) {
                if (canPlaceBlock(grid, tray[i], r, c)) {
                    return false; // vẫn còn ít nhất 1 khối đặt được -> chưa thua
                }
            }
        }
    }
    return true; // không khối nào trong khay đặt được ở bất kỳ đâu -> thua
}
