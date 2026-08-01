#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <optional>
#include <string>
#include <cstdint>

#include "Grid.h"
#include "Placement.h"
#include "Score.h"
#include "Spawn.h"
#include "Sound.h"
#include "Roast.h"

// ===================== CẤU HÌNH GIAO DIỆN =====================
const int CELL = 58;
// Mở rộng: dời lưới sang phải + nới rộng cửa sổ để có chỗ cho panel điểm bên trái
// (tab điểm/độ khó) và panel bên phải (điểm cao nhất/combo), như yêu cầu.
const int GRID_ORIGIN_X = 300;
const int GRID_ORIGIN_Y = 90;
const unsigned int WINDOW_W = 1040;
const unsigned int WINDOW_H = 760;

// Panel bên trái: tab ĐIỂM + ĐỘ KHÓ
const float LEFT_PANEL_X = 26.f;
const float LEFT_PANEL_W = 234.f;
// Panel bên phải: ĐIỂM CAO NHẤT + COMBO
const float RIGHT_PANEL_X = static_cast<float>(GRID_ORIGIN_X) + GRID_SIZE * CELL + 26.f;
const float RIGHT_PANEL_W = 234.f;

const std::string HIGHSCORE_FILE        = "highscore.txt";       // Classic (dự phòng tương thích)
const std::string HIGHSCORE_CLASSIC_FILE = "highscore_classic.txt";
const std::string HIGHSCORE_TIMEATTACK_FILE = "highscore_timeattack.txt";
const std::string HIGHSCORE_SURVIVAL_FILE = "highscore_survival.txt";
const std::string LEADERBOARD_FILE = "leaderboard.txt";

// ===================== CHẾ ĐỘ CHƠI =====================
enum class GameMode { CLASSIC, TIME_ATTACK, SURVIVAL };

// ===================== NGÔN NGỮ (Choose A Language) =====================
// Đã tách sang UIStrings.h/.cpp (module riêng của người phụ trách UI) —
// enum Language, hàm U8(), struct UiStrings, makeEnglishStrings(),
// makeVietnameseStrings() đều nằm trong đó, include lại ở đây.
#include "UIStrings.h"
// Chuyển màu HSV -> RGB (h: 0-360, s/v: 0-1) để sinh màu ngẫu nhiên nhưng vẫn tươi, rực
static sf::Color hsvToColor(float h, float s, float v) {
    float c = v * s;
    float x = c * (1 - std::fabs(std::fmod(h / 60.f, 2.f) - 1));
    float m = v - c;
    float r, g, b;
    if (h < 60)       { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else              { r = c; g = 0; b = x; }
    return sf::Color(
        static_cast<std::uint8_t>((r + m) * 255),
        static_cast<std::uint8_t>((g + m) * 255),
        static_cast<std::uint8_t>((b + m) * 255));
}

// Bảng màu 6 khối - được random lại (RANDOMIZED_PALETTE) mỗi lần chạy game trong main(),
// nên mỗi lần mở game màu khối sẽ khác nhau, không cố định theo 1 bộ màu duy nhất.
std::array<sf::Color, 6> RANDOMIZED_PALETTE = {
    sf::Color(231, 76, 60), sf::Color(52, 152, 219), sf::Color(46, 204, 113),
    sf::Color(241, 196, 15), sf::Color(155, 89, 182), sf::Color(230, 126, 34)
};

// Sinh lại 6 màu ngẫu nhiên, chia đều quanh vòng tròn màu (HSV) từ 1 điểm bắt đầu ngẫu nhiên
// để 6 màu luôn khác biệt rõ ràng nhưng đổi mới mỗi lần chạy game.
static void randomizePalette() {
    float startHue = static_cast<float>(rand() % 360);
    for (int i = 0; i < 6; i++) {
        float hue = std::fmod(startHue + i * (360.f / 6.f) + (rand() % 20 - 10), 360.f);
        if (hue < 0) hue += 360.f;
        float sat = 0.65f + (rand() % 20) / 100.f;   // 0.65 - 0.85
        float val = 0.85f + (rand() % 15) / 100.f;   // 0.85 - 1.00
        RANDOMIZED_PALETTE[i] = hsvToColor(hue, sat, val);
    }
}

// Mở rộng: vẽ nền dạng gradient dọc (từ màu top ở đỉnh cửa sổ tới màu bottom ở đáy)
// thay cho nền 1 màu phẳng cũ, để giao diện có chiều sâu hơn.
static void drawBackgroundGradient(sf::RenderWindow& window, sf::Color top, sf::Color bottom,
                                    unsigned int w, unsigned int h) {
    const int BANDS = 48;
    float bandH = static_cast<float>(h) / BANDS;
    for (int i = 0; i < BANDS; i++) {
        float t = static_cast<float>(i) / (BANDS - 1);
        sf::Color c(
            static_cast<std::uint8_t>(top.r + (bottom.r - top.r) * t),
            static_cast<std::uint8_t>(top.g + (bottom.g - top.g) * t),
            static_cast<std::uint8_t>(top.b + (bottom.b - top.b) * t));
        sf::RectangleShape band(sf::Vector2f(static_cast<float>(w), bandH + 1.f));
        band.setPosition(sf::Vector2f(0.f, i * bandH));
        band.setFillColor(c);
        window.draw(band);
    }
}

// Mở rộng: màu nền (gradient top/bottom) theo mốc độ khó - đi từ màu NHẠT (Yên bình,// siêu dễ) tới màu ĐẬM (Bậc thầy, đỉnh cao), đổi tông rõ rệt giữa từng mốc để người
// chơi dễ nhận ra mình đang ở cấp độ nào chỉ qua màu nền.
//   Yên bình -> Dễ -> Bình thường: tông xanh dương nhạt dần đậm lên.
//   Khó -> Cực đoan: chuyển sang tông xanh-tím rất tối, gần đen.
//   Điên rồ: tông đỏ thẫm/máu tối, cảnh báo nguy hiểm.
//   Bậc thầy: đen tuyền pha ánh vàng kim, tượng trưng cho đỉnh cao.
struct BgGradient { sf::Color top; sf::Color bottom; };
static BgGradient backgroundForTier(int tier) {
    switch (tier) {
        case 0: return { sf::Color(235, 244, 255), sf::Color(210, 226, 250) }; // Yên bình
        case 1: return { sf::Color(178, 214, 250), sf::Color(140, 184, 236) }; // Dễ
        case 2: return { sf::Color(94, 132, 205),  sf::Color(64, 92, 168)   }; // Bình thường
        case 3: return { sf::Color(46, 48, 92),    sf::Color(26, 26, 56)   }; // Khó
        case 4: return { sf::Color(16, 12, 28),    sf::Color(4, 3, 10)    }; // Cực đoan
        case 5: return { sf::Color(64, 8, 14),     sf::Color(20, 2, 4)    }; // Điên rồ (đỏ thẫm)
        default: return { sf::Color(30, 24, 4),    sf::Color(2, 2, 2)    }; // Bậc thầy (đen-vàng kim)
    }
}

sf::Color colorForId(int colorId) {
    if (colorId >= 1 && colorId <= 6) return RANDOMIZED_PALETTE[colorId - 1];
    return sf::Color(149, 165, 166);
}

// Làm khối sáng/tối hơn theo hệ số factor (>1 sáng hơn, <1 tối hơn), dùng cho hiệu ứng gloss 3D
static sf::Color shade(sf::Color c, float factor) {
    auto clampCh = [](float v) { return static_cast<std::uint8_t>(std::max(0.f, std::min(255.f, v))); };
    return sf::Color(clampCh(c.r * factor), clampCh(c.g * factor), clampCh(c.b * factor), c.a);
}

// vẽ 1 ô có hiệu ứng "gạch bóng" 3D: nền màu + viền tối + dải sáng gloss góc trên +
// viền đáy tối tạo bóng đổ. Dùng chung cho khối trong khay VÀ ô đã đặt trên lưới,
// để 2 chỗ nhìn đồng nhất (không bị chỗ thì bóng đẹp, chỗ thì phẳng lì).
static void drawGlossyCell(sf::RenderWindow& window, float px, float py, float w, sf::Color base) {
    sf::Color outline = shade(base, 0.55f);

    sf::RectangleShape rect(sf::Vector2f(w, w));
    rect.setPosition(sf::Vector2f(px, py));
    rect.setFillColor(base);
    rect.setOutlineColor(outline);
    rect.setOutlineThickness(-2.f);
    window.draw(rect);

    // Dải sáng gloss ở góc trên-trái để tạo cảm giác khối bóng, có chiều sâu
    sf::RectangleShape highlight(sf::Vector2f(w - 8.f, w * 0.32f));
    highlight.setPosition(sf::Vector2f(px + 4.f, py + 3.f));
    sf::Color hl = shade(base, 1.55f);
    hl.a = 130;
    highlight.setFillColor(hl);
    window.draw(highlight);

    // Viền đáy tối hơn để tạo bóng đổ nhẹ
    sf::RectangleShape shadowEdge(sf::Vector2f(w, w * 0.14f));
    shadowEdge.setPosition(sf::Vector2f(px, py + w - w * 0.14f));sf::Color sh = shade(base, 0.65f);
    sh.a = 150;
    shadowEdge.setFillColor(sh);
    window.draw(shadowEdge);
}

// vẽ 1 khối tại pixel (x, y) với kích thước ô "size"
void drawBlock(sf::RenderWindow& window, const Block& b, float x, float y, float size, float gap) {
    for (auto& cell : b.cells) {
        float px = x + cell.second * size;
        float py = y + cell.first * size;
        float w = size - gap;

        sf::Color base;
        sf::Color outline;
        if (b.isSpecial && b.id == BOMB_ID) {
            base = sf::Color(44, 44, 44);
            outline = sf::Color(231, 76, 60);
        } else if (b.isSpecial && b.id == WILDCARD_ID) {
            base = sf::Color(236, 240, 241);
            outline = sf::Color(155, 89, 182);
        } else if (b.isSpecial && b.id == SUPERBOMB_ID) {
            // Mở rộng (cực hiếm): khối Đại Bác - tông đỏ cam rực để phân biệt rõ với Bomb thường.
            base = sf::Color(120, 22, 18);
            outline = sf::Color(255, 165, 0);
        } else {
            base = colorForId(b.color);
            drawGlossyCell(window, px, py, w, base);
            continue;
        }

        // Nền khối + viền tối tạo chiều sâu (kiểu khối "gạch bóng") — dùng cho khối đặc biệt
        sf::RectangleShape rect(sf::Vector2f(w, w));
        rect.setPosition(sf::Vector2f(px, py));
        rect.setFillColor(base);
        rect.setOutlineColor(outline);
        rect.setOutlineThickness(-2.f);
        window.draw(rect);
    }
}

// Vẽ 1 hình trái tim đơn giản (2 hình tròn + 1 tam giác) để làm khung điểm số nổi bật
static void drawHeart(sf::RenderWindow& window, float centerX, float centerY, float radius, sf::Color color) {
    sf::CircleShape circleL(radius);
    circleL.setOrigin(sf::Vector2f(radius, radius));
    circleL.setPosition(sf::Vector2f(centerX - radius * 0.9f, centerY - radius * 0.55f));
    circleL.setFillColor(color);
    window.draw(circleL);

    sf::CircleShape circleR(radius);
    circleR.setOrigin(sf::Vector2f(radius, radius));
    circleR.setPosition(sf::Vector2f(centerX + radius * 0.9f, centerY - radius * 0.55f));
    circleR.setFillColor(color);
    window.draw(circleR);

    sf::ConvexShape tri;
    tri.setPointCount(3);
    tri.setPoint(0, sf::Vector2f(centerX - radius * 1.75f, centerY - radius * 0.15f));
    tri.setPoint(1, sf::Vector2f(centerX + radius * 1.75f, centerY - radius * 0.15f));
    tri.setPoint(2, sf::Vector2f(centerX, centerY + radius * 1.95f));
    tri.setFillColor(color);
    window.draw(tri);
}

// ===================== MỞ RỘNG: PHÂN TÍCH LƯỚI CHO "ROBOT MỎ HỖN" =====================
// Các hàm dưới đây không thuộc Module 1-4 gốc, chỉ phục vụ riêng việc phát hiện
// "nước đi tệ" để nuôi RoastManager (xem Roast.h/.cpp) - tách riêng để không
// đụng vào code gốc của 4 thành viên.// Đếm số vùng trống liên thông (4 hướng) trên lưới + kích thước vùng trống lớn nhất.
// Dùng để phát hiện tình huống người chơi đặt khối làm "vỡ vụn" không gian trống
// đang có (đi vào lòng đất) thay vì giữ nó liền mạch, dễ dùng cho các khối sau.
static void analyzeEmptyRegions(int grid[GRID_SIZE][GRID_SIZE], int& outRegionCount, int& outLargestRegion) {
    bool visited[GRID_SIZE][GRID_SIZE];
    for (int r = 0; r < GRID_SIZE; r++)
        for (int c = 0; c < GRID_SIZE; c++)
            visited[r][c] = false;

    outRegionCount = 0;
    outLargestRegion = 0;
    static const int dr[4] = {-1, 1, 0, 0};
    static const int dc[4] = {0, 0, -1, 1};

    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            if (grid[r][c] != 0 || visited[r][c]) continue;

            int size = 0;
            std::vector<std::pair<int, int>> stack;
            stack.push_back({r, c});
            visited[r][c] = true;
            while (!stack.empty()) {
                auto [cr, cc] = stack.back();
                stack.pop_back();
                size++;
                for (int d = 0; d < 4; d++) {
                    int nr = cr + dr[d], nc = cc + dc[d];
                    if (nr >= 0 && nr < GRID_SIZE && nc >= 0 && nc < GRID_SIZE &&
                        grid[nr][nc] == 0 && !visited[nr][nc]) {
                        visited[nr][nc] = true;
                        stack.push_back({nr, nc});
                    }
                }
            }
            outRegionCount++;
            outLargestRegion = std::max(outLargestRegion, size);
        }
    }
}

// Danh sách hàng/cột "gần đầy" (chỉ còn đúng 1 ô trống) tại thời điểm gọi.
// Nếu 1 lượt đặt khối trôi qua mà những hàng/cột này KHÔNG được ăn, coi như
// người chơi vừa bỏ lỡ 1 cơ hội ăn điểm rõ ràng.
struct NearCompleteLine { bool isRow; int index; };
static std::vector<NearCompleteLine> findNearCompleteLines(int grid[GRID_SIZE][GRID_SIZE]) {
    std::vector<NearCompleteLine> result;
    for (int r = 0; r < GRID_SIZE; r++) {
        int emptyCount = 0;
        for (int c = 0; c < GRID_SIZE; c++) if (grid[r][c] == 0) emptyCount++;
        if (emptyCount == 1) result.push_back({true, r});
    }
    for (int c = 0; c < GRID_SIZE; c++) {
        int emptyCount = 0;
        for (int r = 0; r < GRID_SIZE; r++) if (grid[r][c] == 0) emptyCount++;
        if (emptyCount == 1) result.push_back({false, c});
    }
    return result;
}

struct TraySlot {
    Block block;
    bool used;
    sf::Vector2f basePos; // vị trí gốc trong khay để vẽ khi không kéo
};
