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

// ===================== MÀN HÌNH DISCLAIMER =====================
// Một "từ" có màu riêng, dùng để dựng đoạn văn có phần tô màu giống ảnh
// mẫu, rồi tự động xuống dòng (word-wrap) theo chiều rộng cửa sổ.
struct ColoredWord {
    sf::String text;
    sf::Color color;
};
using Paragraph = std::vector<ColoredWord>;

static void appendWords(Paragraph& p, const std::string& utf8Text, sf::Color color) {
    std::istringstream iss(utf8Text);
    std::string token;
    while (iss >> token) {
        p.push_back({U8(token), color});
    }
}

// Một từ đã được layout sẵn: nội dung + vị trí (x, y) tuyệt đối trên màn hình.
struct LaidOutWord {
    sf::String text;
    sf::Color color;
    float x, y;
};

// Word-wrap toàn bộ danh sách đoạn văn thành các từ đã có toạ độ, trả về
// vị trí Y ngay sau đoạn văn cuối cùng.
static float layoutParagraphs(const sf::Font& font, const std::vector<Paragraph>& paragraphs,
                               float originX, float originY, float maxWidth,
                               unsigned int charSize, float lineHeight, float paragraphGap,
                               std::vector<LaidOutWord>& out) {
    float curY = originY;
    sf::Text measurer(font, "", charSize);
    measurer.setString(" .");
    float dotAndSpaceW = measurer.getLocalBounds().size.x;
    measurer.setString(".");
    float spaceW = dotAndSpaceW - measurer.getLocalBounds().size.x;

    for (const auto& para : paragraphs) {
        float curX = originX;
        for (const auto& w : para) {
            measurer.setString(w.text);
            float wWidth = measurer.getLocalBounds().size.x;
            if (curX > originX && curX + wWidth > originX + maxWidth) {
                curX = originX;
                curY += lineHeight;
            }
            out.push_back({w.text, w.color, curX, curY});
            curX += wWidth + spaceW;
        }
        curY += lineHeight + paragraphGap;
    }
    return curY;
}

// Lõi dùng chung: bẻ 1 chuỗi sf::String (đã tách sẵn theo khoảng trắng) thành
// nhiều dòng không vượt quá maxWidth pixel, đo đúng theo style (bold hay
// không) sẽ dùng để vẽ, để tránh lệch giữa lúc đo và lúc vẽ thật.
static std::vector<sf::String> wrapWordsToWidth(const sf::Font& font, const sf::String& full,
                                                 unsigned int charSize, float maxWidth, bool bold) {
    std::vector<sf::String> words;
    sf::String word;
    for (std::size_t i = 0; i < full.getSize(); i++) {
        char32_t ch = full[i];
        if (ch == static_cast<std::uint32_t>(' ')) {
            if (!word.isEmpty()) { words.push_back(word); word.clear(); }
        } else {
            word += static_cast<char32_t>(ch);
        }
    }
    if (!word.isEmpty()) words.push_back(word);

    std::vector<sf::String> lines;
    sf::String currentLine;
    sf::Text measurer(font, "", charSize);
    if (bold) measurer.setStyle(sf::Text::Bold);
    for (auto& w : words) {
        sf::String candidate = currentLine.isEmpty() ? w : (currentLine + sf::String(" ") + w);
        measurer.setString(candidate);
        float width = measurer.getLocalBounds().size.x;
        if (width > maxWidth && !currentLine.isEmpty()) {
            lines.push_back(currentLine);
            currentLine = w;
        } else {
            currentLine = candidate;
        }
    }
    if (!currentLine.isEmpty()) lines.push_back(currentLine);
    return lines;
}

// Mở rộng: word-wrap 1 câu UTF-8 đơn giản (tách theo khoảng trắng) thành nhiều dòng
// sao cho mỗi dòng không vượt quá maxWidth pixel, dùng để hiển thị câu roast bên
// trong bong bóng thoại của "robot mỏ hỗn".

    // Word-wrap cho chuỗi UTF-8 (tiếng Việt có dấu) — đảm bảo không cắt byte UTF-8
// và đo đúng chiều rộng theo style Bold để tránh tràn khung.
// Word-wrap cho chuỗi UTF‑8 (tiếng Việt có dấu) — tương thích SFML 3
static std::vector<sf::String> wrapUtf8Text(const sf::Font& font,
    const std::string& utf8Text,
    unsigned int charSize,
    float maxWidth)
{
    // Chuyển toàn bộ sang UTF‑32 để tránh cắt byte UTF‑8
    sf::String full = sf::String::fromUtf8(utf8Text.begin(), utf8Text.end());

    std::vector<sf::String> words;
    sf::String word;
    for (std::size_t i = 0; i < full.getSize(); i++) {
        if (full[i] == U' ') {
            if (!word.isEmpty()) { words.push_back(word); word.clear(); }
        }
        else {
            word += full[i];
        }
    }
    if (!word.isEmpty()) words.push_back(word);

    std::vector<sf::String> lines;
    sf::String currentLine;
    sf::Text measurer(font, "", charSize);
    measurer.setStyle(sf::Text::Bold);

    // đo đúng style Bold như khi vẽ thật

    for (auto& w : words) {
        sf::String candidate = currentLine.isEmpty() ? w : (currentLine + U' ' + w);
        measurer.setString(candidate);

        // SFML 3: getLocalBounds() trả về sf::FloatRect với hàm .size.x thay cho .width
        float width = measurer.getLocalBounds().size.x;

        if (width > maxWidth && !currentLine.isEmpty()) {
            lines.push_back(currentLine);
            currentLine = w;
        }
        else {
            currentLine = candidate;
        }
    }
    if (!currentLine.isEmpty()) lines.push_back(currentLine);
    return lines;
}

// Mở rộng: SỬA LỖI "hướng dẫn chơi bị lệch/tràn khung" — trước đây nội dung
// HOW TO PLAY (đặc biệt bản tiếng Việt) được viết sẵn với dấu xuống dòng "\n"
// đặt thủ công theo cảm tính, không tính đúng bề rộng pixel thật của font.
// Vì tiếng Việt có dấu thường rộng hơn tiếng Anh, nhiều dòng bị vượt quá bề
// rộng khung (đặc biệt 2 khung TIME ATTACK / SURVIVAL bên phải), khiến chữ
// tràn ra ngoài viền khung. Hàm này giữ nguyên các dấu xuống dòng CHỦ ĐỘNG có
// sẵn (ranh giới giữa các gạch đầu dòng) nhưng tự động bẻ thêm dòng cho bất kỳ
// đoạn nào bị dài hơn maxWidth, đo đúng theo pixel thật của font đang dùng,
// nên chữ luôn nằm gọn trong khung dù nội dung/ngôn ngữ dài ngắn khác nhau.
static sf::String wrapHowtoBody(const sf::Font& font, const sf::String& body,
                                 unsigned int charSize, float maxWidth) {
    std::vector<sf::String> paragraphs;
    sf::String cur;
    for (std::size_t i = 0; i < body.getSize(); i++) {
        char32_t ch = body[i];
        if (ch == static_cast<std::uint32_t>('\n')) {
            paragraphs.push_back(cur);
            cur.clear();
        } else {
            cur += ch;
        }
    }
    paragraphs.push_back(cur);

    sf::String result;
    bool first = true;
    for (auto& p : paragraphs) {
        // Đoạn rỗng (dòng trống chủ động) vẫn giữ nguyên làm 1 dòng trống.
        std::vector<sf::String> wrapped = p.isEmpty()
            ? std::vector<sf::String>{ sf::String() }
            : wrapWordsToWidth(font, p, charSize, maxWidth, /*bold=*/false);
        for (auto& ln : wrapped) {
            if (!first) result += sf::String("\n");
            result += ln;
            first = false;
        }
    }
    return result;
}

// ---------- Fix lỗi Fullscreen/Resize ----------
// Trước đây window được tạo với size cố định 1040x760, toàn bộ code vẽ/UI dùng
// thẳng WINDOW_W/WINDOW_H, và tọa độ chuột lấy trực tiếp từ event->position
// (pixel thật) rồi coi luôn là tọa độ "logic". Khi người dùng phóng to / bật
// fullscreen (Windows maximize hoặc double-click titlebar), SFML KHÔNG tự cập
// nhật lại view theo kích thước cửa sổ mới nếu không lắng nghe sự kiện Resized
// -> nội dung bị vẽ kéo dãn (viewport mặc định vẫn là 0..1 nhưng size cửa sổ đã
// đổi), đồng thời tọa độ chuột (pixel thật, không còn khớp khung 1040x760 logic)
// lệch khỏi vị trí nút/khối hiển thị -> click/kéo/thả trật chỗ. Đây là gốc rễ
// của cả 2 triệu chứng người dùng mô tả.
//
// Cách fix: giữ 1 view logic cố định WINDOW_W x WINDOW_H, dùng letterbox (viền
// đen 2 bên) để giữ tỉ lệ khi cửa sổ đổi size, và luôn quy đổi tọa độ chuột qua
// mapPixelToCoords(..., view hiện tại) thay vì dùng thẳng event->position.

// Cập nhật viewport của view theo kích thước cửa sổ mới, giữ nguyên tỉ lệ khung
// hình gốc (kỹ thuật "letterboxing" tiêu chuẩn của SFML).
void updateLetterboxView(sf::View& view, unsigned int windowW, unsigned int windowH) {
    float windowRatio = static_cast<float>(windowW) / static_cast<float>(windowH);
    float viewRatio = view.getSize().x / view.getSize().y;
    float sizeX = 1.f, sizeY = 1.f, posX = 0.f, posY = 0.f;
    bool horizontalSpacing = true;
    if (windowRatio < viewRatio) horizontalSpacing = false;

    if (horizontalSpacing) {
        sizeX = viewRatio / windowRatio;
        posX = (1.f - sizeX) / 2.f;
    } else {
        sizeY = windowRatio / viewRatio;
        posY = (1.f - sizeY) / 2.f;
    }
    view.setViewport(sf::FloatRect({posX, posY}, {sizeX, sizeY}));
}

// Quy đổi tọa độ pixel thật (event->position) sang tọa độ logic 1040x760 dùng
// xuyên suốt code vẽ UI/grid. Dùng hàm này ở MỌI nơi xử lý chuột thay vì đọc
// thẳng mp->position / mm->position, nếu không click sẽ lệch khi cửa sổ resize
// hoặc fullscreen.
sf::Vector2f toGameCoords(const sf::RenderWindow& window, sf::Vector2i pixel) {
    return window.mapPixelToCoords(pixel, window.getView());
}

int main() {
    srand(static_cast<unsigned int>(time(0))); // Việc 2 - Module 4: seed random
    randomizePalette(); // sinh ngẫu nhiên bảng màu 6 khối cho lần chơi này

    sf::RenderWindow window(sf::VideoMode({WINDOW_W, WINDOW_H}), "Block Blast - UTC2 Core-5");
    window.setFramerateLimit(60);

    // Icon ứng dụng (taskbar + thanh tiêu đề cửa sổ). File .exe trên Windows dùng
    // icon riêng ở assets/icon.ico được nhúng qua app.rc (xem CMakeLists.txt).
    sf::Image appIcon;

if (appIcon.loadFromFile("assets/icon.png")) {
        window.setIcon(appIcon);
    } else {
        std::cerr << "Khong tim thay icon assets/icon.png\n";
    }

    // View logic cố định WINDOW_W x WINDOW_H; mọi tọa độ vẽ (kể cả chuột sau khi
    // quy đổi qua toGameCoords) đều nằm trong hệ tọa độ này bất kể cửa sổ to nhỏ.
    sf::View gameView(sf::FloatRect({0.f, 0.f}, {static_cast<float>(WINDOW_W), static_cast<float>(WINDOW_H)}));
    window.setView(gameView);

    sf::Font font;
    if (!font.openFromFile("assets/font.ttf")) {
        std::cerr << "Khong tim thay font assets/font.ttf\n";
        return 1;
    }

    // ---------- Mở rộng: "Robot mỏ hỗn" - chửi người chơi theo thời gian thực ----------
    sf::Texture robotTexture;
    bool robotTextureLoaded = robotTexture.loadFromFile("assets/robot.png");
    RoastManager roastManager;
    roastManager.setLanguage(false); // Mặc định khởi tạo theo Language::ENGLISH
    // Đo "thời gian suy nghĩ" giữa 2 lần đặt khối liên tiếp, để phát hiện tình huống
    // nghĩ rất lâu (AFK/cân nhắc kỹ) nhưng vẫn ra 1 nước đi tệ.
    sf::Clock thinkClock;
    float pendingThinkTime = 0.f;

    // ---------- Âm thanh (SFX tổng hợp bằng sóng sine + nhạc nền), mở rộng từ bản MoreShapes ----------
    SoundManager soundManager;
    bool musicMuted = false;
    bool sfxMuted = false; // Mở rộng: bật/tắt riêng SFX (đặt/xóa khối...) qua bảng Settings
    soundManager.startMusic(); // phát nhạc nền ngay từ màn hình đầu tiên, tự lặp xuyên suốt game

    // Mở rộng: bảng Settings (biểu tượng bánh răng) - bật/tắt Sound & BGM bất cứ lúc nào
    // ở màn hình Menu hoặc Play, hoạt động như 1 overlay đè lên trên màn hình hiện tại.
    bool settingsOpen = false;

   // ---------- Trạng thái game ----------
    int grid[GRID_SIZE][GRID_SIZE];
    resetGrid(grid); // Việc 4 - Module 1

    int score = 0;
    int streak = 0;
    int missStreak = 0; // số khối liên tiếp không ăn dòng nào kể từ lần ăn combo gần nhất
    int totalLinesCleared = 0;
    int maxCombo = 0;
    bool gameOver = false;

    // ----- Game Mode và High Score riêng biệt -----
    GameMode currentMode = GameMode::CLASSIC;
    int highScoreClassic   = loadHighScore(HIGHSCORE_CLASSIC_FILE);
    int highScoreTimeAttack= loadHighScore(HIGHSCORE_TIMEATTACK_FILE);
    int highScoreSurvival  = loadHighScore(HIGHSCORE_SURVIVAL_FILE);
    // Tham chiếu thuận tiện vào high score của mode hiện tại
    auto getHighScore = [&]() -> int& {
        if (currentMode == GameMode::TIME_ATTACK) return highScoreTimeAttack;
        if (currentMode == GameMode::SURVIVAL)    return highScoreSurvival;
        return highScoreClassic;
    };
    auto getHighScoreFile = [&]() -> const std::string& {
        if (currentMode == GameMode::TIME_ATTACK) return HIGHSCORE_TIMEATTACK_FILE;
        if (currentMode == GameMode::SURVIVAL)    return HIGHSCORE_SURVIVAL_FILE;
        return HIGHSCORE_CLASSIC_FILE;
    };
    int highScore = highScoreClassic; // giữ tương thích với code cũ

    // ----- Time Attack -----
    float timeAttackRemaining = 180.f; // giây
    const float TIME_ATTACK_INITIAL = 180.f;

    // ----- Classic: đồng hồ đếm thời gian đã chơi (đếm LÊN từ 0, không giới hạn) -----
    // Reset về 0 mỗi khi bắt đầu ván Classic mới (chọn Classic ở Menu, bấm R, hoặc bấm
    // nút "Chơi lại" sau Game Over) - xem các nơi restart bên dưới.
    sf::Clock classicPlayClock;

    // ----- Survival: ô chướng ngại (mã màu -2 trên lưới) -----
    const int OBSTACLE_COLOR = -2; // giá trị ô chướng ngại trên lưới
    int totalBlocksPlaced = 0;    // tổng số khối đã đặt (dùng để trigger sinh ô mới mỗi 15 khối)
    int lastObstacleAt = 0;       // giá trị totalBlocksPlaced khi lần cuối sinh ô chướng ngại

    // Sinh ngẫu nhiên count ô chướng ngại vào các ô trống của lưới
    auto spawnObstacles = [&](int count) {
        std::vector<std::pair<int,int>> empty;
        for (int r = 0; r < GRID_SIZE; r++)
            for (int c = 0; c < GRID_SIZE; c++)
                if (grid[r][c] == 0) empty.push_back({r, c});
        // trộn ngẫu nhiên danh sách ô trống
        for (int i = (int)empty.size()-1; i > 0; i--) {
            int j = rand() % (i + 1);
            std::swap(empty[i], empty[j]);
        }
        int toPlace = std::min(count, (int)empty.size());
        for (int k = 0; k < toPlace; k++)
            grid[empty[k].first][empty[k].second] = OBSTACLE_COLOR;
    };

    // ----- Survival: Pressure System -----
    // Pressure đại diện cho mức độ nguy hiểm (0-100). Đạt 100 -> Game Over ngay lập tức.
    int pressure = 0;                 // giá trị Pressure thực (số nguyên, kẹp [0,100])
    float pressureDisplay = 0.f;      // giá trị hiển thị, nội suy mượt về "pressure" mỗi khung hình
    float pressureShakeTimer = 0.f;   // đếm ngược hiệu ứng rung màn hình / phủ đỏ khi Overload
    bool pressureGameOver = false;    // true nếu Game Over là do Pressure đạt 100 (không phải hết chỗ đặt)

    // Cấp độ Pressure: 1 (0-25) SAFE, 2 (26-50) WARNING, 3 (51-75) DANGER, 4 (76-99) CRITICAL
    auto pressureLevel = [](int p) -> int {
        if (p <= 25) return 1;
        if (p <= 50) return 2;
        if (p <= 75) return 3;
        return 4;
    };

    // Đếm số Rock Block đang tồn tại trên bàn cờ
    auto countRocks = [&]() -> int {
        int n = 0;
        for (int r = 0; r < GRID_SIZE; r++)
            for (int c = 0; c < GRID_SIZE; c++)
                if (grid[r][c] == OBSTACLE_COLOR) n++;
        return n;
    };

    // Số Rock Block cần sinh trong 1 đợt Spawn = spawn cơ bản theo Pressure Level
    // hiện tại + đá cộng thêm theo số Rock Block đang tồn tại (trước khi sinh đợt này).
    auto computeRockSpawnCount = [&](int existingRocks) -> int {
        int lvl = pressureLevel(pressure);
        int base = (lvl == 1) ? 2 : (lvl == 2) ? 3 : (lvl == 3) ? 4 : 5;
        int extra = (existingRocks <= 4) ? 1 : (existingRocks <= 9) ? 2 : (existingRocks <= 14) ? 3 : 4;
        return base + extra;
    };

    // Cộng/trừ Pressure, luôn kẹp trong [0,100]. Trả về true nếu Pressure VỪA đạt 100
    // ở lần gọi này (dùng để kích hoạt Game Over "PRESSURE OVERLOAD").
    auto addPressure = [&](int delta) -> bool {
        bool wasMax = (pressure >= 100);
        pressure += delta;
        if (pressure < 0) pressure = 0;
        if (pressure >= 100) { pressure = 100; return !wasMax; }
        return false;
    };

    // Mở rộng: mốc độ khó (10k, 20k, 30k, ... điểm) đã đạt được gần nhất, để phát
    // hiện đúng thời điểm "lên cấp" và thông báo cho người chơi (xem difficultyLevel trong Score.h).
    int lastDifficultyLevel = difficultyLevel(score);

    std::array<TraySlot, 3> tray;
    auto refillTray = [&]() {
        // Module 4 - thuật toán mới xét đến trạng thái lưới VÀ điểm số hiện tại (độ khó).
        // classicHardMode chỉ = true khi đang chơi Classic, để KHÔNG ảnh hưởng Time
        // Attack/Survival (yêu cầu: chỉ tăng độ khó cho chế độ Classic).
        auto blocks = spawnNewBlocks(grid, score, currentMode == GameMode::CLASSIC);
        for (int i = 0; i < 3; i++) {
            tray[i].block = blocks[i];
            tray[i].used = false;
        }
    };
    refillTray();
    // ===================== HIỆU ỨNG POPUP "+ĐIỂM" BAY LÊN =====================
    struct ScorePopup {
        float x, y;
        float age = 0.f;
        int amount = 0;
        bool showAmount = true; // false: chỉ hiện nhãn chữ (vd thông báo lên cấp độ khó)
        sf::String label; // "" nếu chỉ hiện số điểm, không có nhãn combo
    };
    std::vector<ScorePopup> scorePopups;
    const float POPUP_LIFETIME = 1.1f;

    // Mở rộng: đồng hồ tích lũy dùng để tạo hiệu ứng "nhấp nháy" (pulse) cho điểm/combo
    // khi lưới đã bị chiếm >= 1/2, giúp người chơi chú ý hơn lúc bàn cờ sắp chật.
    float uiPulseTime = 0.f;

    // Mở rộng: màu nền hiện đang hiển thị, được nội suy dần (lerp) mỗi khung hình về
    // phía màu mục tiêu của mốc độ khó hiện tại - để nền đổi màu MƯỢT thay vì giật cục
    // ngay khi vừa qua ngưỡng điểm.
    BgGradient currentBg = backgroundForTier(0);

    sf::Clock frameClock;

    enum class Screen { SPLASH, DISCLAIMER, LANGUAGE, MENU, HOWTO, PLAY };
    Screen screen = Screen::SPLASH;
// ---------- Màn hình SPLASH (giới thiệu logo lúc mở game) ----------
    // Mở rộng: hiện logo "BLOCK BLAST" (mẫu Bùng Nổ) ngay khi khởi động, kiểu
    // splash screen của Garena/Liên Quân Mobile - logo phóng to bung ra kèm
    // hiệu ứng "nổ" các khối màu, giữ vài giây rồi tự mờ dần chuyển sang màn
    // chọn ngôn ngữ. Người chơi có thể bấm chuột/phím bất kỳ để bỏ qua ngay.
    // Mở rộng: dùng đúng artwork "Mẫu 1 - Bùng Nổ" (cụm khối 3D nổ tung + chữ
    // "BLOCK BLAST" đã được thiết kế sẵn trong assets/splash_logo.png, nền đen
    // đã được key thành trong suốt) thay vì chỉ dùng icon app + chữ vẽ tay,
    // để splash giống hệt ảnh mẫu, đúng tinh thần splash Garena/Liên Quân Mobile.
    sf::Texture splashLogoTexture;
    bool splashLogoLoaded = splashLogoTexture.loadFromFile("assets/splash_logo.png");
    sf::Sprite splashLogoSprite(splashLogoTexture);
    if (splashLogoLoaded) {
        sf::FloatRect lb = splashLogoSprite.getLocalBounds();
        splashLogoSprite.setOrigin(sf::Vector2f(lb.position.x + lb.size.x / 2.f,
                                                  lb.position.y + lb.size.y / 2.f));
        // Artwork gốc rộng ~675px -> co lại vừa khung splash (~460px chiều ngang)
        float targetW = 460.f;
        float s = targetW / std::max(1.f, lb.size.x);
        splashLogoSprite.setScale(sf::Vector2f(s, s));
    }
    sf::Clock splashClock;
    const float SPLASH_LOGO_BASE_SCALE = splashLogoLoaded
        ? splashLogoSprite.getScale().x : 1.f;
    const float SPLASH_TOTAL_DURATION = 2.8f; // tổng thời lượng splash (giây) trước khi tự chuyển màn

    // Các khối màu văng ra khi logo "nổ", lấy màu từ chính bảng màu khối trong game
    // để đồng bộ cảm giác với gameplay (mẫu 1 - Bùng nổ).
    struct SplashParticle { float angle, speed, size; sf::Color color; };
    std::vector<SplashParticle> splashParticles;
    for (int i = 0; i < 20; i++) {
        SplashParticle p;
        p.angle = static_cast<float>(rand() % 360) * 3.14159265f / 180.f;
        p.speed = 70.f + static_cast<float>(rand() % 110);
        p.size = 10.f + static_cast<float>(rand() % 16);
        p.color = RANDOMIZED_PALETTE[rand() % 6];
        splashParticles.push_back(p);
    }

    // ---------- Ngôn ngữ giao diện ----------
    Language selectedLanguage = Language::ENGLISH; // mặc định giống ảnh mẫu
    UiStrings uiEn = makeEnglishStrings();
    UiStrings uiVi = makeVietnameseStrings();
    auto UI = [&]() -> const UiStrings& {
        return selectedLanguage == Language::ENGLISH ? uiEn : uiVi;
    };

    // Hàm tạo popup "+điểm" bay lên - đặt SAU khi selectedLanguage đã khai báo
    // vì cần chọn nhãn combo theo đúng ngôn ngữ đang chọn.
    auto spawnScorePopup = [&](int amount, int linesCleared, int comboStreak) {
        sf::String label;
        if (linesCleared >= 4 || comboStreak >= 4) {
            label = selectedLanguage == Language::VIETNAMESE ? U8("Tuyệt đỉnh!") : sf::String("Amazing!");
        } else if (linesCleared == 3) {
            label = selectedLanguage == Language::VIETNAMESE ? U8("Xuất sắc!") : sf::String("Awesome!");
        } else if (linesCleared == 2) {
            label = selectedLanguage == Language::VIETNAMESE ? U8("Tuyệt vời!") : sf::String("Great!");
        } else if (linesCleared == 1) {
            label = selectedLanguage == Language::VIETNAMESE ? U8("Tốt lắm!") : sf::String("Nice!");
        }
        ScorePopup p;
        p.amount = amount;
        p.label = label;
        // Fix lỗi: các popup sinh ra cùng lúc (vd combo + bonus dọn sạch lưới) trước đây luôn
        // vẽ đè lên đúng 1 điểm, gây rối mắt. Nay xếp chồng lên nhau theo thứ tự + rung nhẹ trục X.
        int stackIndex = static_cast<int>(scorePopups.size());
        p.x = GRID_ORIGIN_X + (GRID_SIZE * CELL) / 2.f + static_cast<float>(rand() % 30 - 15);
        p.y = GRID_ORIGIN_Y + (GRID_SIZE * CELL) / 2.f - stackIndex * 34.f;
        scorePopups.push_back(p);
    };

    // Popup phụ dùng cho các thông báo mở rộng: thưởng dọn sạch lưới (All Clear), lên cấp độ
    // khó, và thưởng đặt trúng "vùng giải đố" ở trung tâm lưới khi lưới gần đầy.
    auto spawnBonusPopup = [&](int amount, const sf::String& label, bool showAmount) {
        ScorePopup p;
        p.amount = amount;
        p.label = label;
        p.showAmount = showAmount;
        int stackIndex = static_cast<int>(scorePopups.size());
        
        p.x = GRID_ORIGIN_X + (GRID_SIZE * CELL) / 2.f + static_cast<float>(rand() % 30 - 15);
        p.y = GRID_ORIGIN_Y + (GRID_SIZE * CELL) / 2.f - stackIndex * 34.f;
        scorePopups.push_back(p);
    };
     // ---------- trạng thái kéo-thả (Module 2) ----------
    bool dragging = false;
    int dragIndex = -1;
    sf::Vector2f dragPos;
    int hoverRow = -1, hoverCol = -1;
    bool hoverValid = false;

    // Mở rộng (từ bản MoreShapes): các ô thuộc hàng/cột SẼ bị xóa nếu thả khối
    // ngay tại vị trí đang hover - dùng để vẽ hiệu ứng nhấp nháy cảnh báo bên dưới.
    std::vector<std::pair<int, int>> previewClearCells;
    float previewPulseTime = 0.f;

    // Mở rộng: thu nhỏ khay khối để vừa với chiều rộng lưới (464px, do lưới nay nằm
    // giữa 2 panel trái/phải thay vì chiếm hết bề ngang cửa sổ như trước).
    const float traySlotW = 145.f;
    const float traySlotH = 130.f;
    const float trayGap = 8.f;
    const float trayTotalW = traySlotW * 3.f + trayGap * 2.f;
    const float trayY = GRID_ORIGIN_Y + GRID_SIZE * CELL + 40.f;
    const float trayStartX = GRID_ORIGIN_X + (GRID_SIZE * CELL - trayTotalW) / 2.f;

    auto traySlotPos = [&](int i) {
        return sf::Vector2f(trayStartX + i * (traySlotW + trayGap), trayY);
    };

    for (int i = 0; i < 3; i++) tray[i].basePos = traySlotPos(i);

    sf::Text title(font, "BLOCK BLAST", 46);
    title.setFillColor(sf::Color::White);

    sf::Text hudText(font, "", 24);
    hudText.setFillColor(sf::Color::White);

    sf::Text howtoTitleText(font, "", 30);
    howtoTitleText.setFillColor(sf::Color::White);
    howtoTitleText.setPosition(sf::Vector2f(60, 40));

    sf::Text howtoHintText(font, "", 18);
    howtoHintText.setFillColor(sf::Color(189, 195, 199));

    // Mở rộng: màn hình Hướng dẫn dạng 2 cột - cột trái là "CƠ BẢN" + "ĐỘ KHÓ",
    // cột phải là 2 khung màu "ĐẤU THỜI GIAN" (xanh dương) và "SINH TỒN" (xanh lá).
    sf::Text howtoBasicTitleText(font, "", 20);
    howtoBasicTitleText.setFillColor(sf::Color(255, 110, 110));
    howtoBasicTitleText.setStyle(sf::Text::Bold);
    sf::Text howtoBasicBodyText(font, "", 13);
    howtoBasicBodyText.setFillColor(sf::Color::White);
    howtoBasicBodyText.setLineSpacing(1.45f);

    sf::Text howtoDifficultyTitleText(font, "", 20);
    howtoDifficultyTitleText.setFillColor(sf::Color(255, 205, 80));
    howtoDifficultyTitleText.setStyle(sf::Text::Bold);
    sf::Text howtoDifficultyIntroText(font, "", 15);
    howtoDifficultyIntroText.setFillColor(sf::Color::White);
    // Mở rộng: 3 cột của bảng độ khó (khoảng điểm / tên mức / mô tả) được vẽ
    // bằng các sf::Text riêng đặt ở tọa độ x CỐ ĐỊNH cho từng cột, để luôn
    // thẳng hàng thật sự - không phụ thuộc vào việc font có phải monospace
    // hay không (khác với cách canh bằng dấu cách trong 1 chuỗi trước đây).
    std::array<sf::Text, 7> howtoDiffRangeText = {
        sf::Text(font, "", 15), sf::Text(font, "", 15), sf::Text(font, "", 15),
        sf::Text(font, "", 15), sf::Text(font, "", 15), sf::Text(font, "", 15),
        sf::Text(font, "", 15)
    };
    std::array<sf::Text, 7> howtoDiffNameText = {
        sf::Text(font, "", 15), sf::Text(font, "", 15), sf::Text(font, "", 15),
        sf::Text(font, "", 15), sf::Text(font, "", 15), sf::Text(font, "", 15),
        sf::Text(font, "", 15)
    };
    std::array<sf::Text, 7> howtoDiffDescText = {
        sf::Text(font, "", 15), sf::Text(font, "", 15), sf::Text(font, "", 15),
        sf::Text(font, "", 15), sf::Text(font, "", 15), sf::Text(font, "", 15),
        sf::Text(font, "", 15)
    };
    for (auto& t : howtoDiffRangeText) t.setFillColor(sf::Color(210, 220, 235));
    for (auto& t : howtoDiffNameText)  { t.setFillColor(sf::Color::White); t.setStyle(sf::Text::Bold); }
    for (auto& t : howtoDiffDescText)  t.setFillColor(sf::Color(160, 200, 255));

    sf::Text howtoTaTitleText(font, "", 20);
    howtoTaTitleText.setFillColor(sf::Color(120, 200, 255));
    howtoTaTitleText.setStyle(sf::Text::Bold);
    sf::Text howtoTaBodyText(font, "", 13);
    howtoTaBodyText.setFillColor(sf::Color::White);
    howtoTaBodyText.setLineSpacing(1.45f);

    sf::Text howtoSurvTitleText(font, "", 20);
    howtoSurvTitleText.setFillColor(sf::Color(140, 230, 150));
    howtoSurvTitleText.setStyle(sf::Text::Bold);
    sf::Text howtoSurvBodyText(font, "", 13);
    howtoSurvBodyText.setFillColor(sf::Color::White);
    howtoSurvBodyText.setLineSpacing(1.45f);

    sf::Text hsText(font, "", 20);
    hsText.setFillColor(sf::Color(241, 196, 15));

    sf::Text overText(font, "", 48);
    overText.setFillColor(sf::Color(231, 76, 60));

    sf::Text finalScoreText(font, "", 24);
    finalScoreText.setFillColor(sf::Color::White);

    sf::Text retryText(font, "", 20);
    retryText.setFillColor(sf::Color(189, 195, 199));

// ===================== NỘI DUNG MÀN HÌNH DISCLAIMER =====================
    // Disclaimer đổi theo ngôn ngữ đã chọn ở màn hình trước:
    //   - Tiếng Việt -> chữ có dấu (U8()).
    //   - English    -> chữ tiếng Anh thường.
    // Riêng 2 nút lựa chọn bên dưới LUÔN là tiếng Việt KHÔNG DẤU
    // ("Toi dong tinh." / "Toi khong dong tinh.") theo đúng yêu cầu, bất kể
    // ngôn ngữ đang chọn là gì.
    const sf::Color DC_WHITE(230, 230, 230);
    const sf::Color DC_HILITE(241, 196, 15); // vàng nhấn, cùng tông với "Diem cao nhat"

    // Bản tiếng Việt (có dấu) - viết lại "chất" bài tập lớn môn CTDL & Giải thuật :)
    auto buildDisclaimerParasVi = [&]() {
        std::vector<Paragraph> paras;

        Paragraph p0;
        appendWords(p0, "Bằng việc chơi Block Blast phiên bản hắc hóa này, bạn sẽ trở thành", DC_WHITE);
        appendWords(p0, "Đại sứ kiên nhẫn của làng xếp khối Việt Nam,", DC_HILITE);
        appendWords(p0, "đại diện cho bài tập lớn môn Cấu trúc dữ liệu & Giải thuật, khoa CNTT, UTC2. Đây là một vinh dự lớn lao và cũng là một trọng trách nặng nề đối với quỹ thời gian ôn bài của bạn, nên bạn phải ghi nhớ những điều sau:", DC_WHITE);
        paras.push_back(p0);

        Paragraph p1;
        appendWords(p1, "1. Game có chứa", DC_WHITE);
        appendWords(p1, "một số nội dung gây nghiện cao,", DC_HILITE);
        appendWords(p1, "có thể gây ám ảnh với các khối hình vuông rơi lơ lửng trong đầu mỗi khi bạn nhắm mắt ôn bài.", DC_WHITE);
        paras.push_back(p1);

        Paragraph p2;
        appendWords(p2, "2. Lối chơi được", DC_WHITE);
        appendWords(p2, "truyền cảm hứng", DC_HILITE);
        appendWords(p2, "từ bài toán sắp xếp và tối ưu không gian lưu trữ thực tế, nhưng đã được kịch tính hóa hoàn toàn để thử thách giới hạn chịu đựng của não bộ.", DC_WHITE);
        paras.push_back(p2);

        Paragraph p3;
        appendWords(p3, "3. Game", DC_WHITE);
        appendWords(p3, "không nhằm đến, không cổ vũ, và không miệt thị", DC_HILITE);
        appendWords(p3, "bất kỳ sinh viên, giảng viên hay tổ chức nào của môn Cấu trúc dữ liệu & Giải thuật tại UTC2. Bất kỳ sự tương đồng nào với deadline ngoài đời thực đều chỉ là ngẫu nhiên.", DC_WHITE);
        paras.push_back(p3);

        Paragraph p4;
        appendWords(p4, "4. Mọi khối đặt xuống đều có ý nghĩa. Tôi không khuyến khích trải nghiệm game này vào ban đêm khi bạn còn dang dở commit trên GitHub hay chưa cài đặt xong giải thuật cho bài tập lớn. Tôi sẽ", DC_WHITE);
        appendWords(p4, "không chịu trách nhiệm", DC_HILITE);
        appendWords(p4, "cho bất kỳ vấn đề phát sinh nào về deadline của bạn sau khi chơi.", DC_WHITE);
        paras.push_back(p4);

        Paragraph p5;
        appendWords(p5, "5. Sản phẩm này không phải là một công cụ minh họa giải thuật, và chắc chắn không có tác dụng thay thế cho việc ôn tập Sắp xếp, Tìm kiếm, Cây hay Đồ thị cho môn Cấu trúc dữ liệu & Giải thuật.", DC_WHITE);
        paras.push_back(p5);

        Paragraph p6;
        appendWords(p6, "Chúc bạn có một trải nghiệm khó quên và không bị \"Stack Overflow\" quá sớm!", DC_WHITE);
        paras.push_back(p6);

        return paras;
    };

    // Bản tiếng Anh - dịch và giữ tinh thần hài hước tương tự
    auto buildDisclaimerParasEn = [&]() {
        std::vector<Paragraph> paras;

        Paragraph p0;
        appendWords(p0, "By playing this darker version of Block Blast, you will become", DC_WHITE);
        appendWords(p0, "the Patient Ambassador of the Vietnamese block-stacking village,", DC_HILITE);
        appendWords(p0, "representing the Data Structures & Algorithms coursework, CS Department, UTC2. This is both a great honor and a heavy responsibility for your remaining study time, so you must remember the following:", DC_WHITE);
        paras.push_back(p0);

        Paragraph p1;
        appendWords(p1, "1. This game contains", DC_WHITE);
        appendWords(p1, "some highly addictive content,", DC_HILITE);
        appendWords(p1, "which may haunt you with floating square blocks every time you close your eyes to study.", DC_WHITE);
        paras.push_back(p1);

        Paragraph p2;
        appendWords(p2, "2. The gameplay is", DC_WHITE);
        appendWords(p2, "inspired", DC_HILITE);
        appendWords(p2, "by real-world storage arrangement and optimization problems, but has been fully dramatized to test the limits of your brain's endurance.", DC_WHITE);
        paras.push_back(p2);

        Paragraph p3;
        appendWords(p3, "3. This game", DC_WHITE);
        appendWords(p3, "does not target, endorse, or mock", DC_HILITE);
        appendWords(p3, "any student, lecturer, or organization of the Data Structures & Algorithms course at UTC2. Any resemblance to real-life deadlines is purely coincidental.", DC_WHITE);
        paras.push_back(p3);

        Paragraph p4;
        appendWords(p4, "4. Every block placed down has meaning. I do not recommend playing this game at night while you still have unfinished commits on GitHub or an algorithm implementation left to do for your assignment. I will", DC_WHITE);
        appendWords(p4, "not be held responsible", DC_HILITE);
        appendWords(p4, "for any issues regarding your deadline after playing.", DC_WHITE);
        paras.push_back(p4);

        Paragraph p5;
        appendWords(p5, "5. This product is not an algorithm visualization tool, and it definitely does not replace reviewing Sorting, Searching, Trees, or Graphs for your Data Structures & Algorithms course.", DC_WHITE);
        paras.push_back(p5);

        Paragraph p6;
        appendWords(p6, "Have an unforgettable experience, and try not to hit \"Stack Overflow\" too soon!", DC_WHITE);
        paras.push_back(p6);

        return paras;
    };

    const float dcMarginX = 50.f;
    const float dcMaxWidth = WINDOW_W - 2 * dcMarginX;
    const unsigned int dcCharSize = 16; // giảm cỡ chữ để đoạn văn dài hơn vẫn vừa khung
    std::vector<LaidOutWord> disclaimerLayout;

    sf::Text disclaimerTitle(font, "DISCLAIMER", 34);
    disclaimerTitle.setFillColor(sf::Color::White);

    sf::Text agreeBtn(font, "[ I agree. ]", 20);
    agreeBtn.setFillColor(sf::Color::White);
    sf::Text disagreeBtn(font, "[ I disagree. ]", 20);
    disagreeBtn.setFillColor(sf::Color::White);

    // Dựng lại layout disclaimer theo ngôn ngữ hiện đang được chọn, đồng thời
    // tự tính lại vị trí 2 nút Đồng ý / Không đồng ý dựa theo độ dài THỰC TẾ
    // của đoạn văn, để dù bản dịch dài ngắn khác nhau cũng không bao giờ bị
    // đè lên nhau. Được gọi lại mỗi khi người chơi bấm OK ở màn hình ngôn ngữ.
    auto rebuildDisclaimerLayout = [&](Language lang) {
        std::vector<Paragraph> disclaimerParas =
            (lang == Language::VIETNAMESE) ? buildDisclaimerParasVi() : buildDisclaimerParasEn();
        disclaimerLayout.clear();
        float contentEndY = layoutParagraphs(font, disclaimerParas, dcMarginX, 100.f, dcMaxWidth,
                                              dcCharSize, dcCharSize * 1.4f, 8.f, disclaimerLayout);

        // Nút luôn cách đoạn văn ít nhất 25px, nhưng không thấp hơn 640 (giữ bố
        // cục đẹp khi văn bản ngắn) và không vượt quá đáy cửa sổ.
        float btnY = std::max(640.f, contentEndY + 25.f);
        btnY = std::min(btnY, WINDOW_H - 55.f);
        if (lang == Language::VIETNAMESE) {
            agreeBtn.setString(U8("[ Toi dong tinh ]"));
            disagreeBtn.setString(U8("[ Toi khong dong tinh ]"));
            disclaimerTitle.setString(U8("LƯU Ý TRƯỚC KHI CHƠI"));
        } else {
            agreeBtn.setString("[ I agree. ]");
            disagreeBtn.setString("[ I disagree. ]");
            disclaimerTitle.setString("DISCLAIMER");
        }

        disclaimerTitle.setPosition(sf::Vector2f(WINDOW_W / 2.f - disclaimerTitle.getGlobalBounds().size.x / 2.f, 30.f));

        // Nút bên trái (Đồng ý) sẽ đẩy lùi sang trái một đoạn bằng chính độ rộng của nó
        // Nút bên phải (Không đồng ý) sẽ tiến sang phải
        agreeBtn.setPosition(sf::Vector2f(WINDOW_W / 2.f - agreeBtn.getGlobalBounds().size.x - 30.f, btnY));
        disagreeBtn.setPosition(sf::Vector2f(WINDOW_W / 2.f + 30.f, btnY));
    };
    // Chỉ dựng layout đoạn văn (nút và title sẽ được dựng bên trong hàm)
    rebuildDisclaimerLayout(selectedLanguage);


    // ===================== MÀN HÌNH CHỌN NGÔN NGỮ =====================
    sf::Text langTitle(font, "Choose A Language", 30);
    langTitle.setFillColor(sf::Color::White);
    langTitle.setPosition(sf::Vector2f(WINDOW_W / 2.f - langTitle.getGlobalBounds().size.x / 2.f, 150.f));

    sf::Text langSubtitle(font, "You can change anytime later", 16);
    langSubtitle.setFillColor(sf::Color(160, 160, 160));
    langSubtitle.setPosition(sf::Vector2f(WINDOW_W / 2.f - langSubtitle.getGlobalBounds().size.x / 2.f, 190.f));

    const sf::Vector2f selectBoxPos(WINDOW_W / 2.f - 300.f, 250.f);
    const sf::Vector2f selectBoxSize(600.f, 50.f);

    const sf::Vector2f optionRowSize(600.f, 45.f);
    const sf::Vector2f optionsOrigin(selectBoxPos.x, selectBoxPos.y + selectBoxSize.y + 10.f);

    sf::FloatRect englishRow(optionsOrigin, optionRowSize);
    sf::FloatRect vietnameseRow(sf::Vector2f(optionsOrigin.x, optionsOrigin.y + optionRowSize.y), optionRowSize);

    sf::FloatRect langOkBtn(sf::Vector2f(WINDOW_W / 2.f - 80.f, optionsOrigin.y + 2 * optionRowSize.y + 40.f), sf::Vector2f(160.f, 50.f));

    // ===================== Mở rộng: bảng SETTINGS (biểu tượng bánh răng) =====================
    // Biểu tượng bánh răng ở góc trên-phải, hiện ở Menu và trong lúc chơi (khi chưa Game Over).
    const sf::FloatRect gearBtn(sf::Vector2f(WINDOW_W - 64.f, 18.f), sf::Vector2f(44.f, 44.f));

    // Bảng Settings (overlay đè lên trên màn hình hiện tại), gồm 2 công tắc Sound & BGM,
    // và (mở rộng) 2 nút to "Trang chủ" / "Chơi lại" ngay trong bảng, giống thiết kế tham khảo.
    const sf::Vector2f settingsPanelSize(420.f, 400.f);
    const sf::Vector2f settingsPanelPos(WINDOW_W / 2.f - settingsPanelSize.x / 2.f, WINDOW_H / 2.f - settingsPanelSize.y / 2.f);
    const sf::FloatRect settingsCloseBtn(sf::Vector2f(settingsPanelPos.x + settingsPanelSize.x - 46.f, settingsPanelPos.y + 10.f), sf::Vector2f(36.f, 36.f));
    const sf::Vector2f switchSize(76.f, 36.f);
    const sf::FloatRect soundSwitchBtn(sf::Vector2f(settingsPanelPos.x + 70.f, settingsPanelPos.y + 130.f), switchSize);
    const sf::FloatRect musicSwitchBtn(sf::Vector2f(settingsPanelPos.x + settingsPanelSize.x - 70.f - switchSize.x, settingsPanelPos.y + 130.f), switchSize);

    // Mở rộng: 2 nút to bên trong bảng Settings - "Trang chủ" (về Menu) và "Chơi lại"
    // (reset ván hiện tại), dùng được ở mọi lúc mở Settings (Menu lẫn đang chơi).
    const sf::Vector2f settingsBtnSize(settingsPanelSize.x - 80.f, 56.f);
    const sf::FloatRect settingsHomeBtn(sf::Vector2f(settingsPanelPos.x + 40.f, settingsPanelPos.y + 220.f), settingsBtnSize);
    const sf::FloatRect settingsReplayBtn(sf::Vector2f(settingsPanelPos.x + 40.f, settingsPanelPos.y + 220.f + settingsBtnSize.y + 16.f), settingsBtnSize);

    // Mở rộng: 2 nút "Trang chủ" / "Chơi lại" hiển thị trên màn hình Game Over.
    // Đẩy 2 nút lên trên một chút (380.f) để bù lại phần chữ hướng dẫn phím tắt vừa bị xóa.
    const sf::Vector2f gameOverBtnSize(180.f, 56.f);
    const sf::FloatRect homeBtn(sf::Vector2f(WINDOW_W / 2.f - gameOverBtnSize.x / 2.f, 380.f), gameOverBtnSize);
    const sf::FloatRect replayBtn(sf::Vector2f(WINDOW_W / 2.f - gameOverBtnSize.x / 2.f, 380.f + gameOverBtnSize.y + 16.f), gameOverBtnSize);

    while (window.isOpen()) {
        float dt = frameClock.restart().asSeconds();
        uiPulseTime += dt; // mở rộng: đồng hồ nhấp nháy cho hiệu ứng nổi bật điểm/combo
        previewPulseTime += dt; // dùng để tạo hiệu ứng nhấp nháy cho preview "sắp bị xóa"
        roastManager.update(dt); // mở rộng: cập nhật cooldown/thời gian hiển thị của "robot mỏ hỗn"

        // Splash screen tự động kết thúc sau SPLASH_TOTAL_DURATION giây (nếu người chơi
        // không bấm bỏ qua trước đó), rồi chuyển tiếp sang màn hình chọn ngôn ngữ.
        if (screen == Screen::SPLASH && splashClock.getElapsedTime().asSeconds() >= SPLASH_TOTAL_DURATION) {
            screen = Screen::LANGUAGE;
        }

        // ----- Survival: nội suy mượt thanh Pressure về giá trị thực mỗi khung hình -----
        // (yêu cầu: Pressure tăng/giảm dù chỉ 1 điểm cũng phải thấy thanh di chuyển ngay,
        // không chờ tới mốc lớn - nên ta lerp "pressureDisplay" về "pressure" mỗi frame).
        float pressureDiff = static_cast<float>(pressure) - pressureDisplay;
        pressureDisplay += pressureDiff * std::min(1.f, dt * 8.f);
        if (std::abs(pressureDiff) < 0.05f) pressureDisplay = static_cast<float>(pressure);
        if (pressureShakeTimer > 0.f) pressureShakeTimer = std::max(0.f, pressureShakeTimer - dt);

        // Cập nhật tuổi các popup "+điểm" đang bay, xóa popup đã hết hạn
        for (auto& p : scorePopups) p.age += dt;
        scorePopups.erase(
            std::remove_if(scorePopups.begin(), scorePopups.end(),
                            [&](const ScorePopup& p) { return p.age >= POPUP_LIFETIME; }),
            scorePopups.end());

        // ----- Time Attack: đếm ngược thời gian -----
        if (screen == Screen::PLAY && !gameOver && currentMode == GameMode::TIME_ATTACK) {
            timeAttackRemaining -= dt;
            if (timeAttackRemaining <= 0.f) {
                timeAttackRemaining = 0.f; // Đảm bảo đồng hồ dừng chính xác ở 00:00
                gameOver = true;
                soundManager.playGameOver();
                if (score > getHighScore()) {
                    getHighScore() = score;
                    highScore = score;
                    saveHighScore(getHighScoreFile(), score);
                }
                updateLeaderboard(LEADERBOARD_FILE, "Player", score);

                // Thêm Roastbot mắng mỏ nếu điểm quá thấp khi hết giờ
                const int LOW_SCORE_ROAST_THRESHOLD = 1500;
                if (score < LOW_SCORE_ROAST_THRESHOLD) {
                    roastManager.tryTrigger(RoastTrigger::GameOverLowScore);
                }
            }
        }

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            // Fix lỗi bung/kéo dãn hình khi bật fullscreen (maximize) hoặc resize cửa sổ:
            // cập nhật lại viewport (letterbox) của gameView theo size mới của cửa sổ,
            // thay vì để SFML giữ nguyên viewport 0..1 cũ (gây kéo dãn ảnh) và lệch tọa
            // độ chuột so với những gì hiển thị trên màn hình.
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                updateLetterboxView(gameView, resized->size.x, resized->size.y);
                window.setView(gameView);
            }

            // Phím M: bật/tắt nhạc nền, hoạt động ở mọi màn hình (mở rộng từ bản MoreShapes)
            if (const auto* kpMute = event->getIf<sf::Event::KeyPressed>()) {
                if (kpMute->code == sf::Keyboard::Key::M) {
                    musicMuted = !musicMuted;
                    soundManager.setMusicVolume(musicMuted ? 0.f : 32.f);
                }
            }
// ---------------- MÀN HÌNH SPLASH (logo lúc mở game) ----------------
            // Bấm chuột hoặc phím bất kỳ đều bỏ qua splash ngay lập tức, giống
            // hành vi "tap to skip" của các splash screen game mobile.
            if (screen == Screen::SPLASH) {
                if (event->is<sf::Event::MouseButtonPressed>() || event->is<sf::Event::KeyPressed>()) {
                    screen = Screen::LANGUAGE;
                }
                continue;
            }

            // ---------------- BẢNG SETTINGS (overlay) ----------------
            // Mở rộng: khi bảng Settings đang mở, xử lý riêng các nút bấm của nó (đóng bảng,
            // công tắc Sound, công tắc BGM) rồi bỏ qua (continue), KHÔNG cho sự kiện này lan
            // xuống xử lý bình thường của màn hình bên dưới.
            if (settingsOpen) {
                if (const auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mp->button == sf::Mouse::Button::Left) {
                        sf::Vector2f m = toGameCoords(window, mp->position);
                        if (settingsCloseBtn.contains(m)) {
                            settingsOpen = false;
                        } else if (soundSwitchBtn.contains(m)) {
                            sfxMuted = !sfxMuted;
                            soundManager.setSfxVolume(sfxMuted ? 0.f : 75.f);
                        } else if (musicSwitchBtn.contains(m)) {
                            musicMuted = !musicMuted;
                            soundManager.setMusicVolume(musicMuted ? 0.f : 32.f);
                        } else if (settingsHomeBtn.contains(m)) {
                            // Về trang chủ: đóng Settings và quay lại Menu (không reset điểm,
                            // giống hệt nút Home ở màn Game Over - lượt chơi mới sẽ reset khi bấm Start).
                            settingsOpen = false;
                            screen = Screen::MENU;
                        } else if (settingsReplayBtn.contains(m)) {
                            // Chơi lại: reset toàn bộ ván hiện tại và bắt đầu chơi ngay
                            settingsOpen = false;
                            resetGrid(grid);
                            score = 0; streak = 0; missStreak = 0; totalLinesCleared = 0; maxCombo = 0;
                            lastDifficultyLevel = 0;
                            totalBlocksPlaced = 0; lastObstacleAt = 0;
                            pressure = 0; pressureDisplay = 0.f; pressureGameOver = false; pressureShakeTimer = 0.f;
                            timeAttackRemaining = TIME_ATTACK_INITIAL;
                            thinkClock.restart();
                            if (currentMode == GameMode::SURVIVAL) spawnObstacles(2);
                            refillTray();
                            gameOver = false;
                            screen = Screen::PLAY;
                        }
                    }
                }
                if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                    if (kp->code == sf::Keyboard::Key::Escape) settingsOpen = false;
                }
                continue;
            }

            // Bấm vào biểu tượng bánh răng (góc trên-phải) để mở bảng Settings - hoạt động ở
            // Menu và trong lúc chơi (ẩn/đóng khi đang ở màn hình Game Over, vì màn đó đã có
            // sẵn 2 nút Trang chủ/Chơi lại riêng).
            if (screen == Screen::MENU || (screen == Screen::PLAY && !gameOver)) {
                if (const auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mp->button == sf::Mouse::Button::Left) {
                        sf::Vector2f m = toGameCoords(window, mp->position);
                        if (gearBtn.contains(m)) {
                            settingsOpen = true;
                            continue;
                        }
                    }
                }
            }

            // ---------------- MÀN HÌNH DISCLAIMER ----------------
            if (screen == Screen::DISCLAIMER) {
                if (const auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mp->button == sf::Mouse::Button::Left) {
                        sf::Vector2f m = toGameCoords(window, mp->position);
                        if (agreeBtn.getGlobalBounds().contains(m)) {
                            screen = Screen::MENU;
                        } else if (disagreeBtn.getGlobalBounds().contains(m)) {
                            window.close();
                        }
                    }
                }
            }
            // ---------------- MÀN HÌNH CHỌN NGÔN NGỮ ----------------
            else if (screen == Screen::LANGUAGE) {
                if (const auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mp->button == sf::Mouse::Button::Left) {
                        sf::Vector2f m = toGameCoords(window, mp->position);
                        if (englishRow.contains(m)) {
                            selectedLanguage = Language::ENGLISH;
                            roastManager.setLanguage(false);
                        } else if (vietnameseRow.contains(m)) {
                            selectedLanguage = Language::VIETNAMESE;
                            roastManager.setLanguage(true);
                        } else if (langOkBtn.contains(m)) {
                            rebuildDisclaimerLayout(selectedLanguage);
                            screen = Screen::DISCLAIMER;
                        }
                    }
                }
            }
            // ---------------- MÀN HÌNH MENU ----------------
            else if (screen == Screen::MENU) {
                if (const auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                    sf::Vector2f m = toGameCoords(window, mp->position);
                    const float btnW = 340, btnH = 60, btnGap = 80; // Mở rộng nút từ 240 lên 340 để chứa đủ chữ
                    const float btnX = WINDOW_W / 2.f - btnW / 2.f;
                    sf::FloatRect classicBtn  (sf::Vector2f(btnX, 300),            sf::Vector2f(btnW, btnH));
                    sf::FloatRect taBtn       (sf::Vector2f(btnX, 300 + btnGap),   sf::Vector2f(btnW, btnH));
                    sf::FloatRect survivalBtn (sf::Vector2f(btnX, 300 + btnGap*2), sf::Vector2f(btnW, btnH));
                    sf::FloatRect howtoBtn    (sf::Vector2f(btnX, 300 + btnGap*3), sf::Vector2f(btnW, btnH));
                    sf::FloatRect quitBtn     (sf::Vector2f(btnX, 300 + btnGap*4), sf::Vector2f(btnW, btnH));

                    auto startGameMode = [&](GameMode mode) {
                        currentMode = mode;
                        highScore = getHighScore();
                        resetGrid(grid);
                        score = 0; streak = 0; missStreak = 0; totalLinesCleared = 0; maxCombo = 0;
                        lastDifficultyLevel = 0;
                        totalBlocksPlaced = 0; lastObstacleAt = 0;
                            pressure = 0; pressureDisplay = 0.f; pressureGameOver = false; pressureShakeTimer = 0.f;
                        timeAttackRemaining = TIME_ATTACK_INITIAL;
                        thinkClock.restart();
                        classicPlayClock.restart();
                        if (mode == GameMode::SURVIVAL) spawnObstacles(2);
                        refillTray();
                        gameOver = false;
                        screen = Screen::PLAY;
                    };

                    if (classicBtn.contains(m)) {
                        startGameMode(GameMode::CLASSIC);
                    } else if (taBtn.contains(m)) {
                        startGameMode(GameMode::TIME_ATTACK);
                    } else if (survivalBtn.contains(m)) {
                        startGameMode(GameMode::SURVIVAL);
                    } else if (howtoBtn.contains(m)) {
                        screen = Screen::HOWTO;
                    } else if (quitBtn.contains(m)) {
                        window.close();
                    }
                }
            }
            // ---------------- MÀN HÌNH HƯỚNG DẪN ----------------
            else if (screen == Screen::HOWTO) {
                if (event->is<sf::Event::MouseButtonPressed>() || event->is<sf::Event::KeyPressed>()) {
                    screen = Screen::MENU;
                }
            }
           // ---------------- MÀN HÌNH CHƠI GAME ----------------
            else if (screen == Screen::PLAY) {
                if (gameOver) {
                    if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                        if (kp->code == sf::Keyboard::Key::R) {
                            resetGrid(grid);
                            score = 0; streak = 0; missStreak = 0; totalLinesCleared = 0; maxCombo = 0;
                            lastDifficultyLevel = 0;
                            totalBlocksPlaced = 0; lastObstacleAt = 0;
                            pressure = 0; pressureDisplay = 0.f; pressureGameOver = false; pressureShakeTimer = 0.f;
                            timeAttackRemaining = TIME_ATTACK_INITIAL;
                            thinkClock.restart();
                            classicPlayClock.restart();
                            if (currentMode == GameMode::SURVIVAL) spawnObstacles(2);
                            refillTray();
                            gameOver = false;
                        }
                        if (kp->code == sf::Keyboard::Key::Escape) {
                            screen = Screen::MENU;
                        }
                    }
                    // Mở rộng: 2 nút bấm "Trang chủ" / "Chơi lại" trên màn hình Game Over,
                    // làm y hệt phím tắt ESC / R ở trên nhưng bằng chuột.
                    if (const auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                        if (mp->button == sf::Mouse::Button::Left) {
                            sf::Vector2f m = toGameCoords(window, mp->position);
                            if (homeBtn.contains(m)) {
                                screen = Screen::MENU;
                            } else if (replayBtn.contains(m)) {
                                resetGrid(grid);
                                score = 0; streak = 0; missStreak = 0; totalLinesCleared = 0; maxCombo = 0;
                                lastDifficultyLevel = 0;
                                totalBlocksPlaced = 0; lastObstacleAt = 0;
                            pressure = 0; pressureDisplay = 0.f; pressureGameOver = false; pressureShakeTimer = 0.f;
                                timeAttackRemaining = TIME_ATTACK_INITIAL;
                                thinkClock.restart();
                                classicPlayClock.restart();
                                if (currentMode == GameMode::SURVIVAL) spawnObstacles(2);
                                refillTray();
                                gameOver = false;
                            }
                        }
                    }
                    continue;
                }

                // Module 2 - Việc: nhận lựa chọn khối bằng chuột + bắt đầu kéo
                if (const auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mp->button == sf::Mouse::Button::Left) {
                        sf::Vector2f m = toGameCoords(window, mp->position);
                        for (int i = 0; i < 3; i++) {
                            if (tray[i].used) continue;
                            sf::FloatRect area(tray[i].basePos, sf::Vector2f(traySlotW, traySlotH));
                            if (area.contains(m)) {
                                dragging = true;
                                dragIndex = i;
                                dragPos = m;
                                previewClearCells.clear();
                                // Mở rộng: ghi lại thời gian đã "suy nghĩ" trước khi bắt đầu kéo khối này
                                pendingThinkTime = thinkClock.getElapsedTime().asSeconds();
                                
                                auto box = blockBoundingBox(tray[dragIndex].block);
                                int bw = box.second, bh = box.first;
                                float gx = dragPos.x - GRID_ORIGIN_X - (bw * CELL) / 2.f;
                                float gy = dragPos.y - GRID_ORIGIN_Y - (bh * CELL) / 2.f;
                                hoverCol = static_cast<int>(std::round(gx / CELL));
                                hoverRow = static_cast<int>(std::round(gy / CELL));
                                hoverValid = canPlaceBlock(grid, tray[dragIndex].block, hoverRow, hoverCol);
                                
                                break;
                            }
                        }
                    }
                }

                if (const auto* mm = event->getIf<sf::Event::MouseMoved>()) {
                    if (dragging) {
                        dragPos = toGameCoords(window, mm->position);

                        auto box = blockBoundingBox(tray[dragIndex].block);
                        int bw = box.second, bh = box.first;

                        float gx = dragPos.x - GRID_ORIGIN_X - (bw * CELL) / 2.f;
                        float gy = dragPos.y - GRID_ORIGIN_Y - (bh * CELL) / 2.f;

                        hoverCol = static_cast<int>(std::round(gx / CELL));
                        hoverRow = static_cast<int>(std::round(gy / CELL));

                        hoverValid = canPlaceBlock(grid, tray[dragIndex].block, hoverRow, hoverCol);

                        // Mở rộng (từ bản MoreShapes): mô phỏng thử đặt khối trên 1 bản sao lưới
                        // để biết trước những hàng/cột nào sẽ đầy và bị xóa, phục vụ hiệu ứng
                        // nhấp nháy cảnh báo bên dưới. Wildcard đè lên ô đã có khối khác nên
                        // vẫn mô phỏng đúng bằng cách ghi màu khối lên bản sao lưới.
                        previewClearCells.clear();
                        if (hoverValid) {
                            int temp[GRID_SIZE][GRID_SIZE];
                            for (int i = 0; i < GRID_SIZE; i++)
                                for (int j = 0; j < GRID_SIZE; j++)
                                    temp[i][j] = grid[i][j];
                            for (auto& cell : tray[dragIndex].block.cells) {
                                int r = hoverRow + cell.first, c = hoverCol + cell.second;
                                if (r >= 0 && r < GRID_SIZE && c >= 0 && c < GRID_SIZE)
                                    temp[r][c] = tray[dragIndex].block.color;
                            }
                            ClearResult cr = checkFullLines(temp);
                            for (int r : cr.rows)
                                for (int c = 0; c < GRID_SIZE; c++) previewClearCells.push_back({r, c});
                            for (int c : cr.cols)
                                for (int r = 0; r < GRID_SIZE; r++) previewClearCells.push_back({r, c});
                        }
                    }
                }

                // Việc 4-5 Module 2: kiểm tra hợp lệ & đặt khối khi thả chuột
                if (const auto* mr = event->getIf<sf::Event::MouseButtonReleased>()) {
                    if (mr->button == sf::Mouse::Button::Left && dragging) {
                        if (hoverValid && canPlaceBlock(grid, tray[dragIndex].block, hoverRow, hoverCol)) {
                            // Mở rộng: ghi nhận độ lấp lưới TRƯỚC khi đặt, dùng để thưởng "vùng giải
                            // đố" trung tâm khi người chơi đặt chuẩn lúc lưới đã rất chật (>= 2/3).
                            int occupiedBefore = 0;
                            for (int rr = 0; rr < GRID_SIZE; rr++)
                                for (int cc = 0; cc < GRID_SIZE; cc++)
                                    if (grid[rr][cc] != 0) occupiedBefore++;
                            bool wasPuzzleZone = occupiedBefore >= (GRID_SIZE * GRID_SIZE * 2) / 3;

                            // Mở rộng: chụp lại trạng thái lưới TRƯỚC khi đặt, dùng riêng cho
                            // "robot mỏ hỗn" (Roast.h/.cpp) để so sánh trước/sau nước đi này.
                            int regionsBeforeMove, largestBeforeMove;
                            analyzeEmptyRegions(grid, regionsBeforeMove, largestBeforeMove);
                            std::vector<NearCompleteLine> nearCompleteBeforeMove = findNearCompleteLines(grid);
                            bool placedSpecialBlock = tray[dragIndex].block.isSpecial;

                            int rocksBeforePlace = 0;
                            if (currentMode == GameMode::SURVIVAL) {
                                rocksBeforePlace = countRocks();
                            }

                            int bonus = 0;
                            placeBlock(grid, tray[dragIndex].block, hoverRow, hoverCol, bonus); // Module 2
                            soundManager.playPlace(); // hiệu ứng âm thanh khi đặt khối (mở rộng từ bản MoreShapes)

                            int cellsPlaced = countCells(tray[dragIndex].block); // Module 1

                            // Thưởng đặt trúng vùng trung tâm 2x2 khi đang trong "chế độ giải đố"
                            if (wasPuzzleZone) {
                                int centerHits = 0;
                                for (auto& cellOff : tray[dragIndex].block.cells) {
                                    int r = hoverRow + cellOff.first;
                                    int c = hoverCol + cellOff.second;
                                    if (r >= 3 && r <= 4 && c >= 3 && c <= 4) centerHits++;
                                }
                                if (centerHits > 0) {
                                    int centerBonus = centerHits * 25;
                                    score += centerBonus;
                                    sf::String centerLabel = selectedLanguage == Language::VIETNAMESE
                                        ? U8("Trúng tâm giải đố!") : sf::String("Puzzle center!");
                                    spawnBonusPopup(centerBonus, centerLabel, true);
                                }
                            }

                            // Module 3: kiểm tra & xóa hàng/cột đầy
                            ClearResult cr = checkFullLines(grid);
                            int linesCleared = static_cast<int>(cr.rows.size() + cr.cols.size());

                            // Survival: đếm số Rock Block SẼ bị phá bởi lượt này TRƯỚC khi
                            // clearLines() xóa mất dữ liệu (dùng cho Pressure System bên dưới).
                            int rocksDestroyedThisMove = 0;
                            if (currentMode == GameMode::SURVIVAL) {
                                rocksDestroyedThisMove = rocksBeforePlace - countRocks();
                                if (linesCleared > 0) {
                                    bool destroyedMark[GRID_SIZE][GRID_SIZE] = {};
                                    for (int r : cr.rows)
                                        for (int c = 0; c < GRID_SIZE; c++)
                                            if (grid[r][c] == OBSTACLE_COLOR) destroyedMark[r][c] = true;
                                    for (int c : cr.cols)
                                        for (int r = 0; r < GRID_SIZE; r++)
                                            if (grid[r][c] == OBSTACLE_COLOR) destroyedMark[r][c] = true;
                                    for (int i = 0; i < GRID_SIZE; i++)
                                        for (int j = 0; j < GRID_SIZE; j++)
                                            if (destroyedMark[i][j]) rocksDestroyedThisMove++;
                                }
                            }

                            clearLines(grid, cr);
                            totalLinesCleared += linesCleared;

                            // Yêu cầu: khối NỔ (Bom/Đại Bác) không tính vào "khối cho phép bỏ lỡ"
                            // dù lượt đặt đó có ăn được hàng/cột hay không.
                            bool placedExplosiveBlock = tray[dragIndex].block.isSpecial
                                && (tray[dragIndex].block.id == BOMB_ID || tray[dragIndex].block.id == SUPERBOMB_ID);
                            int gained = computeScoreForMove(cellsPlaced, linesCleared, streak, missStreak,
                                                              placedExplosiveBlock);
                            score += gained + bonus;
                            maxCombo = std::max(maxCombo, streak);

                            if (linesCleared > 0) {
                                spawnScorePopup(gained + bonus, linesCleared, streak);
                                soundManager.playClear(linesCleared, streak);

                                // ----- Time Attack: cộng thêm thời gian khi xóa hàng/cột -----
                                if (currentMode == GameMode::TIME_ATTACK) {
                                    float bonus_time = 0.f;
                                    if      (linesCleared >= 4) bonus_time = 5.f;
                                    else if (linesCleared == 3) bonus_time = 3.f;
                                    else if (linesCleared == 2) bonus_time = 2.f;
                                    else                         bonus_time = 1.f;
                                    timeAttackRemaining += bonus_time;
                                    // Hiện popup thông báo +giây
                                    sf::String timeLabel = sf::String("+") + sf::String(std::to_string((int)bonus_time))
                                        + (selectedLanguage == Language::VIETNAMESE ? U8("s") : sf::String("s"));
                                    spawnBonusPopup(0, timeLabel, false);
                                }
                            }

                            // Mở rộng: thưởng "dọn sạch lưới" (All Clear)
                            if (linesCleared > 0 && isGridEmpty(grid)) {
                                int clearBonus = fullClearBonus(score);
                                score += clearBonus;
                                sf::String clearLabel = selectedLanguage == Language::VIETNAMESE
                                    ? U8("Dọn sạch lưới!") : sf::String("ALL CLEAR!");
                                spawnBonusPopup(clearBonus, clearLabel, true);
                                soundManager.playFullClear(); // âm thanh riêng khi ăn sạch toàn bộ lưới
                            }

                            // ----- Survival: cập nhật Pressure System -----
                            if (currentMode == GameMode::SURVIVAL) {
                                // Pressure tăng khi đặt Block (+2), giảm khi Clear hàng/cột (-4/hàng-cột,
                                // thêm -5 nếu ăn từ 2 hàng/cột trở lên cùng lượt) và khi phá Rock Block (-6/viên).
                                int delta = 2;
                                if (linesCleared > 0) {
                                    delta -= 4 * linesCleared;
                                    if (linesCleared >= 2) delta -= 5; // Multi Clear Bonus
                                }
                                delta -= 6 * rocksDestroyedThisMove;

                                if (addPressure(delta)) {
                                    gameOver = true;
                                    pressureGameOver = true;
                                    pressureShakeTimer = 0.5f;
                                    soundManager.playGameOver();
                                    if (score > getHighScore()) {
                                        getHighScore() = score;
                                        highScore = score;
                                        saveHighScore(getHighScoreFile(), score);
                                    }
                                    updateLeaderboard(LEADERBOARD_FILE, "Player", score);
                                }
                            }

                            // ----- Survival: đếm khối đã đặt và sinh Rock Block mới theo chu kỳ -----
                            if (currentMode == GameMode::SURVIVAL && !gameOver) {
                                totalBlocksPlaced++;
                                if (totalBlocksPlaced - lastObstacleAt >= 15) {
                                    lastObstacleAt = totalBlocksPlaced;
                                    // Số Rock Block sinh ra = spawn cơ bản theo Pressure Level hiện tại
                                    // + đá cộng thêm theo số Rock Block đang tồn tại (TRƯỚC đợt spawn này).
                                    int existingBefore = countRocks();
                                    int spawnCount = computeRockSpawnCount(existingBefore);
                                    spawnObstacles(spawnCount);

                                    // Đến chu kỳ Spawn: +5 Pressure. Sau khi Spawn hoàn tất, mỗi Rock Block
                                    // còn tồn tại trên bàn (đếm trước đợt spawn này): +1 Pressure mỗi viên.
                                    if (addPressure(5 + existingBefore)) {
                                        gameOver = true;
                                        pressureGameOver = true;
                                        pressureShakeTimer = 0.5f;
                                        soundManager.playGameOver();
                                        if (score > getHighScore()) {
                                            getHighScore() = score;
                                            highScore = score;
                                            saveHighScore(getHighScoreFile(), score);
                                        }
                                        updateLeaderboard(LEADERBOARD_FILE, "Player", score);
                                    }
                                }
                            }

                            // Mở rộng: thông báo lên mốc độ khó mới
                            int newDifficultyLevel = difficultyLevel(score);
                            if (newDifficultyLevel > lastDifficultyLevel) {
                                lastDifficultyLevel = newDifficultyLevel;
                                sf::String lvlLabel = selectedLanguage == Language::VIETNAMESE
                                    ? U8("Tăng độ khó! Cấp ") + sf::String(std::to_string(newDifficultyLevel))
                                    : sf::String("Difficulty up! Lv ") + sf::String(std::to_string(newDifficultyLevel));
                                spawnBonusPopup(0, lvlLabel, false);
                            }

// ---------- Mở rộng: "Robot mỏ hỗn" - phát hiện nước đi tệ ----------
                            // 1) Đặt khối làm vỡ vụn / bóp nghẹt không gian trống đang có, mà
                            //    không ăn được hàng/cột nào để bù lại - "đi vào lòng đất".
                            if (!placedSpecialBlock && linesCleared == 0 && cellsPlaced >= 4) {
                                int regionsAfterMove, largestAfterMove;
                                analyzeEmptyRegions(grid, regionsAfterMove, largestAfterMove);
                                bool fragmentedBadly =
                                    (regionsAfterMove > regionsBeforeMove) ||
                                    (largestBeforeMove > 0 && largestAfterMove < largestBeforeMove * 0.6);
                                if (fragmentedBadly) {
                                    bool wasLongThink = pendingThinkTime > 10.f;
                                    roastManager.tryTrigger(wasLongThink ? RoastTrigger::AfkBadMove
                                                                          : RoastTrigger::BadPlacement);
                                }
                            }
                            // 2) Có hàng/cột đã gần đầy (chỉ còn 1 ô trống) từ TRƯỚC lượt này,
                            //    nhưng lượt đặt vừa rồi lại không ăn được hàng/cột đó.
                            if (!nearCompleteBeforeMove.empty()) {
                                bool missedSomeLine = false;
                                for (auto& line : nearCompleteBeforeMove) {
                                    bool wasCleared = false;
                                    if (line.isRow) {
                                        for (int rr : cr.rows) if (rr == line.index) { wasCleared = true; break; }
                                    } else {
                                        for (int cc : cr.cols) if (cc == line.index) { wasCleared = true; break; }
                                    }
                                    if (!wasCleared) { missedSomeLine = true; break; }
                                }
                                if (missedSomeLine) {
                                    roastManager.tryTrigger(RoastTrigger::MissedClear);
                                }
                            }

                            tray[dragIndex].used = true;

                            // Module 4: khi khay hết thì sinh khối mới
                            bool allUsed = tray[0].used && tray[1].used && tray[2].used;
                            if (allUsed) refillTray();

                            // Module 4 - Việc 3: kiểm tra thua
                            std::array<Block,3> curBlocks = {tray[0].block, tray[1].block, tray[2].block};
                            std::array<bool,3> curUsed = {tray[0].used, tray[1].used, tray[2].used};
if (!gameOver && isGameOver(grid, curBlocks, curUsed)) {
                                gameOver = true;
                                soundManager.playGameOver();
                                // Cập nhật high score riêng của mode đang chơi
                                if (score > getHighScore()) {
                                    getHighScore() = score;
                                    highScore = score;
                                    saveHighScore(getHighScoreFile(), score);
                                }
                                updateLeaderboard(LEADERBOARD_FILE, "Player", score);
                                const int LOW_SCORE_ROAST_THRESHOLD = 1500;
                                if (score < LOW_SCORE_ROAST_THRESHOLD) {
                                    roastManager.tryTrigger(RoastTrigger::GameOverLowScore);
                                }
                            }
                        } else {
                            // Thả khối vào vị trí không hợp lệ -> tiếng "bíp" báo sai chỗ (mở rộng từ bản MoreShapes)
                            soundManager.playInvalid();
                        }
                        dragging = false;
                        dragIndex = -1;
                        hoverRow = hoverCol = -1;
                        previewClearCells.clear();
                        thinkClock.restart(); // mở rộng: bắt đầu đo lại "thời gian nghĩ" cho lượt tiếp theo
                    }
                }

                if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                    // Phím H: gợi ý (Hint System - Module 2 mở rộng)
                    if (kp->code == sf::Keyboard::Key::H) {
                        for (int i = 0; i < 3; i++) {
                            if (tray[i].used) continue;
                            int hr, hc;
                            if (giveHint(grid, tray[i].block, hr, hc)) {
                                std::cout << "Goi y: dat khoi " << i << " tai (" << hr << "," << hc << ")\n";
                            }
                        }
                    }
                    if (kp->code == sf::Keyboard::Key::Escape) {
                        screen = Screen::MENU;
                    }
                }
            }
}
