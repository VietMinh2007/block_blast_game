#include "Score.h"
#include <fstream>
#include <sstream>
#include <algorithm>

ClearResult checkFullLines(int grid[GRID_SIZE][GRID_SIZE]) {
    ClearResult result;

    // Việc 1: checkFullRows
    // Survival: ô Rock Block (OBSTACLE_COLOR) được tính là ô "đã lấp đầy" khi xét
    // hàng/cột đầy - Rock Block không cản việc hoàn thành hàng/cột, nó chỉ không
    // thể bị người chơi đặt đè lên hay xóa trực tiếp (xem Placement.cpp). Rock Block
    // chỉ biến mất khi hàng/cột chứa nó được hoàn thành và bị xóa (clearLines() gán
    // toàn bộ ô trong hàng/cột đó, kể cả ô đá, về 0).
    for (int r = 0; r < GRID_SIZE; r++) {
        bool full = true;
        for (int c = 0; c < GRID_SIZE; c++) {
            if (grid[r][c] == 0) { full = false; break; }
        }
        if (full) result.rows.push_back(r);
    }

    // Việc 2: checkFullCols
    for (int c = 0; c < GRID_SIZE; c++) {
        bool full = true;
        for (int r = 0; r < GRID_SIZE; r++) {
            if (grid[r][c] == 0) { full = false; break; }
        }
        if (full) result.cols.push_back(c);
    }

    return result;
}

void clearLines(int grid[GRID_SIZE][GRID_SIZE], const ClearResult& result) {
    for (int r : result.rows)
        for (int c = 0; c < GRID_SIZE; c++)
            grid[r][c] = 0;

    for (int c : result.cols)
        for (int r = 0; r < GRID_SIZE; r++)
            grid[r][c] = 0;
}

int computeScoreForMove(int cellsPlaced, int linesCleared, int& streak, int& missStreak,
                         bool isExplosiveBlock) {
    // Việc 4: mỗi ô đặt được +1 điểm, mỗi hàng/cột xóa được +10 điểm
    int score = cellsPlaced * 1;
    int lineScore = linesCleared * 10;

    // Việc 5 (mở rộng): yêu cầu ăn N hàng/cột CÙNG 1 lượt phải đúng bằng gấp N lần
    // điểm của việc ăn 1 hàng/cột (tuyến tính, không nhân thêm hệ số escalate như
    // bản cũ). Vì lineScore phía trên đã là linesCleared * 10 (tự nó đã tuyến tính
    // theo số hàng/cột), nên KHÔNG nhân thêm hệ số nào nữa - giữ multiplier = 1.
    double multiplier = 1.0;

    lineScore = static_cast<int>(lineScore * multiplier);

    // Việc 6: streak - cấp độ combo giữa CÁC lượt đặt khối liên tiếp.
    // Chỉ mất combo khi đặt QUÁ COMBO_GRACE_BLOCKS (3) khối liên tiếp mà không ăn được
    // hàng/cột nào (cho người chơi 3 khối "ân hạn" trước khi bị reset combo).
    if (linesCleared > 0) {
        streak++;
        missStreak = 0; // vừa ăn combo -> reset lại số khối ân hạn

        // Combo càng cao thì điểm thưởng càng tăng mạnh (tăng theo cấp số cộng dồn):
        // streak 1 -> +5, 2 -> +15, 3 -> +30, 4 -> +50, 5 -> +75, ...
        int comboBonus = (streak * (streak + 1) / 2) * 5;
        lineScore += comboBonus;
    } else if (!isExplosiveBlock) {
        // Khối NỔ (Bom/Đại Bác) không ăn dòng vẫn KHÔNG bị tính là "bỏ lỡ" - giữ nguyên
        // missStreak, không tiêu tốn khối ân hạn nào cả (xem giải thích ở Score.h).
        missStreak++;
        if (missStreak > COMBO_GRACE_BLOCKS) {
            streak = 0;
            missStreak = 0;
        }
    }

    return score + lineScore;
}

int fullClearBonus(int currentScore) {
    // Thưởng 10% tổng điểm hiện có khi dọn sạch toàn bộ lưới trong 1 lượt ăn dòng.
    return currentScore / 10;
}

int difficultyLevel(int score) {
    // Cứ mỗi 10.000 điểm tăng thêm 1 cấp độ khó, tăng dần tới vô tận.
    return score / 10000;
}

int difficultyTier(int score) {
    if (score < 2500) return 0;        // Yên bình
    if (score < 10000) return 1;       // Dễ
    if (score < 50000) return 2;       // Bình thường
    if (score < 250000) return 3;      // Khó
    if (score < 1000000) return 4;     // Cực đoan
    if (score < 2000000) return 5;     // Điên rồ
    return 6;                          // Bậc thầy
}

int loadHighScore(const std::string& path) {
    std::ifstream fin(path);
    int hs = 0;
    if (fin) fin >> hs;
    return hs;
}

void saveHighScore(const std::string& path, int score) {
    std::ofstream fout(path);
    if (fout) fout << score;
}

std::vector<Achievement> checkAchievements(int totalLinesCleared, int maxCombo, int score) {
    std::vector<Achievement> list;
    list.push_back({"Combo x5", maxCombo >= 5});
    list.push_back({"Xoa tong 50 hang/cot", totalLinesCleared >= 50});
    list.push_back({"Dat 1000 diem", score >= 1000});
    return list;
}

void updateLeaderboard(const std::string& path, const std::string& playerName, int score) {
    struct Entry { std::string name; int score; };
    std::vector<Entry> entries;

    std::ifstream fin(path);
    std::string name;
    int sc;
    while (fin >> name >> sc) entries.push_back({name, sc});

    entries.push_back({playerName, score});
    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
        return a.score > b.score;
    });
    if (entries.size() > 5) entries.resize(5);

    std::ofstream fout(path);
    for (auto& e : entries) fout << e.name << " " << e.score << "\n";
}
