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

// Văn bản tiếng Việt CÓ DẤU (ví dụ Disclaimer) phải nạp qua U8() để SFML
// hiểu đúng là UTF-8 (font DejaVu Sans Bold đi kèm có đủ ký tự tiếng Việt).
static sf::String U8(const std::string& utf8) {
    return sf::String::fromUtf8(utf8.begin(), utf8.end());
}

    // Mở rộng: căn đều cả 5 mốc theo cùng khuôn "Tên
// ===================== NGÔN NGỮ (Choose A Language) =====================
enum class Language { ENGLISH, VIETNAMESE };

struct UiStrings {
    sf::String menuTitle;
    sf::String btnClassic, btnTimeAttack, btnSurvival; // 3 nút chế độ chơi mới
    sf::String btnHowTo, btnQuit;
    sf::String highScoreLabel;
    sf::String howtoTitle;
    // Mở rộng: tách màn hình Hướng dẫn thành các khối riêng (Cơ bản / Độ khó /
    // Đấu thời gian / Sinh tồn) để vẽ dạng 2 cột có khung màu, thay cho 1 khối
    // văn bản dài như trước.
    sf::String howtoBasicTitle, howtoBasicBody;
    sf::String howtoDifficultyTitle;
    // Mở rộng: tách bảng độ khó thành từng cột riêng (khoảng điểm / tên mức /
    // mô tả) để vẽ dạng bảng canh cột thật bằng tọa độ x cố định - dùng dấu
    // cách để canh trong 1 chuỗi văn bản KHÔNG đều nhau vì font không phải
    // monospace, nên phải tách cột như vậy mới thẳng hàng chính xác.
    sf::String howtoDifficultyIntro;
    std::array<sf::String, 7> howtoDifficultyRanges;
    std::array<sf::String, 7> howtoDifficultyDescriptors;
    sf::String howtoTimeAttackTitle, howtoTimeAttackBody;
    sf::String howtoSurvivalTitle, howtoSurvivalBody;
    sf::String howtoHint;
    sf::String hudPrefixScore, hudPrefixHigh, hudPrefixCombo;
    sf::String hudComboBlocksPrefix, hudComboBlocksSuffix; // "còn bao nhiêu khối trước khi mất combo"
    sf::String gameOverTitle;
    sf::String finalScorePrefix, finalScoreHighSuffixOpen; // "Score: X   (High: Y)"
    sf::String retryHint;
    // Time Attack
    sf::String timeAttackLabel; // nhãn đồng hồ đếm ngược
    sf::String survivalObstacleLabel; // nhãn ô chướng ngại
    // Mở rộng: nhãn thanh thời gian đã chơi, chỉ hiện ở chế độ Classic - đếm LÊN
    // (khác với đồng hồ đếm NGƯỢC của Time Attack) để người chơi biết mình đã
    // chơi ván Classic hiện tại bao lâu.
    sf::String classicPlayTimeLabel;

    // Survival - Pressure System
    sf::String pressureLabel; // nhãn mặc định (Level 1 - SAFE)
    sf::String pressureWarnRising;   // Level 2 (26-50) WARNING
    sf::String pressureWarnHigh;     // Level 3 (51-75) DANGER
    sf::String pressureWarnCritical; // Level 4 (76-99) CRITICAL
    sf::String pressureOverloadLine1, pressureOverloadLine2; // Pressure = 100

    // Mở rộng: nhãn cho panel bên trái (Điểm/Độ khó) và bên phải (Cao nhất/Combo).
    sf::String panelScoreLabel, panelDifficultyLabel;
    sf::String panelHighScoreLabel, panelComboLabel;
    std::array<sf::String, 7> difficultyTierNames; // 0=Yên bình .. 4=Cực đoan .. 6=Bậc thầy

    // Mở rộng: bảng Settings (bật/tắt âm thanh đặt khối, bật/tắt nhạc nền) và
    // 2 nút bấm "Trang chủ" / "Chơi lại" trên màn hình Game Over.
    sf::String settingsTitle;
    sf::String labelSound, labelMusic;
    sf::String stateOn, stateOff;
    sf::String btnHome, btnReplay;
};

static UiStrings makeEnglishStrings() {
    UiStrings s;
    s.menuTitle = "BLOCK BLAST";
    s.btnClassic    = sf::String("Classic");
    s.btnTimeAttack = sf::String("Time Attack");
    s.btnSurvival   = sf::String("Survival");
    s.btnHowTo = sf::String("How To Play");
    s.btnQuit = "Quit";
    s.highScoreLabel = "High score: ";
    s.howtoTitle = "HOW TO PLAY";
    s.howtoBasicTitle = "| BASICS";
    s.howtoBasicBody =
        "- Drag a block from the tray below onto the 8x8 grid.\n"
        "- Fill a full row or column to clear it and score points.\n"
        "- Clearing several rows/columns at once gives a combo bonus.\n"
        "- Dark block (red outline) = BOMB block: blasts a 3x3 area.\n"
        "- White block (purple outline) = WILDCARD block: can be placed\n"
        "  on any cell, even one already occupied.\n"
        "- Press H to print a placement hint to the console.\n"
        "- Press M anytime to mute/unmute the background music.\n"
        "- The game ends when no block in the tray fits on the grid.";
    s.howtoDifficultyTitle = "| DIFFICULTY";
    // Mở rộng: căn đều 5 mốc độ khó theo cùng 1 khuôn "Tên (Mức)" để nhìn
    // khoa học, thay vì chỉ có mốc đầu/cuối mới có phần mô tả trong ngoặc.
    s.howtoDifficultyIntro = "Difficulty rises with your score; background color shifts too:";
    s.howtoDifficultyRanges = {
        sf::String("0 - 2,500"), sf::String("2,500 - 10,000"), sf::String("10,000 - 50,000"),
        sf::String("50,000 - 250,000"), sf::String("250,000 - 1,000,000"),
        sf::String("1,000,000 - 2,000,000"), sf::String("2,000,000+")
    };
    s.howtoDifficultyDescriptors = {
        sf::String("(Very easy)"), sf::String("(Easy)"), sf::String("(Medium)"),
        sf::String("(Hard)"), sf::String("(Very hard)"),
        sf::String("(Insane)"), sf::String("(Master)")
    };
    s.howtoTimeAttackTitle = "TIME ATTACK";
    s.howtoTimeAttackBody =
        "Race the clock and score as high as you can!\n"
        "- You start with 3 minutes.\n"
        "- Place blocks to complete rows/columns and score.\n"
        "- Each row/column cleared adds bonus time.\n"
        "- Clearing more lines at once grants more bonus time.\n"
        "- Ends when time runs out or no block fits anymore.";
    s.howtoSurvivalTitle = "SURVIVAL";
    s.howtoSurvivalBody =
        "Control the board and keep Pressure safe to survive!\n"
        "- You start with 2 Rock Blocks on the board.\n"
        "- Every 15 blocks placed, more Rock Blocks appear.\n"
        "- A Rock Block clears only when its row/column is\n"
        "  completed - the more Rocks left standing, the\n"
        "  faster Pressure rises.\n"
        "- Clear rows/columns and destroy Rocks to lower\n"
        "  Pressure. If Pressure hits 100, it's game over.\n"
        "- Also ends when there's no space left for a block.\n"
        "Tip: don't just chase points - clear Rocks often and\n"
        "go for multi-line clears to keep Pressure low.";
    s.pressureLabel = "PRESSURE";
    s.pressureWarnRising = U8("\u26A0 PRESSURE RISING");
    s.pressureWarnHigh = U8("\u26A0 HIGH PRESSURE");
    s.pressureWarnCritical = U8("\u2620 CRITICAL PRESSURE");
    s.pressureOverloadLine1 = "PRESSURE OVERLOAD";
    s.pressureOverloadLine2 = "You Couldn't Survive...";
    s.howtoHint = "(Press any key or click to go back)";
    s.hudPrefixScore = "Score: ";
    s.hudPrefixHigh = "   High: ";
    s.hudPrefixCombo = "   Combo: ";
    s.hudComboBlocksPrefix = "Combo grace: ";
    s.hudComboBlocksSuffix = " blocks";
    s.gameOverTitle = "GAME OVER";
    s.finalScorePrefix = "Final score: ";
    s.finalScoreHighSuffixOpen = "   (High: ";
    s.retryHint = "Press R to play again   |   ESC for Menu";
    s.timeAttackLabel = "TIME";
    s.classicPlayTimeLabel = "PLAYED";
    s.survivalObstacleLabel = "ROCKS";

    s.panelScoreLabel = "SCORE";
    s.panelDifficultyLabel = "DIFFICULTY";
    s.panelHighScoreLabel = "HIGH SCORE";
    s.panelComboLabel = "COMBO";
    s.difficultyTierNames = {
        sf::String("Calm"), sf::String("Easy"), sf::String("Normal"),
        sf::String("Hard"), sf::String("Extreme"), sf::String("Insane"), sf::String("Master")
    };

    s.settingsTitle = "Settings";
    s.labelSound = "Sound";
    s.labelMusic = "BGM";
    s.stateOn = "ON";
    s.stateOff = "OFF";
    s.btnHome = "Home";
    s.btnReplay = "Replay";
    return s;
}

static UiStrings makeVietnameseStrings() {
    UiStrings s;
    s.menuTitle = "BLOCK BLAST";
    s.btnClassic    = U8("Classic");
    s.btnTimeAttack = U8("Time Attack");
    s.btnSurvival   = U8("Survival");
    s.btnHowTo = U8("Hướng dẫn");
    s.btnQuit = U8("Tho\u00e1t");
    s.highScoreLabel = U8("\u0110i\u1ec3m cao nh\u1ea5t: ");
    s.howtoTitle = U8("HƯỚNG DẪN CHƠI");
    s.howtoBasicTitle = U8("| CƠ BẢN");
    s.howtoBasicBody = U8(
        "- Kéo một khối từ khay ở dưới lên lưới 8x8.\n"
        "- Lấp đầy 1 hàng hoặc 1 cột để được xóa và cộng điểm.\n"
        "- Xóa nhiều hàng/cột cùng lúc sẽ được nhận combo.\n"
        "- Khối màu tối (viền đỏ) = khối BOM: phá nổ vùng 3x3.\n"
        "- Khối trắng (viền tím) = khối WILDCARD: đặt được vào\n"
        "  bất kỳ ô nào, kể cả ô đã có khối khác.\n"
        "- Nhấn H để xem gợi ý vị trí đặt (in ra console).\n"
        "- Nhấn M bất cứ lúc nào để tắt/bật nhạc nền.\n"
        "- Game kết thúc khi không còn khối nào đặt vừa vào lưới.");
    s.howtoDifficultyTitle = U8("| ĐỘ KHÓ"); (Mức độ)" thay vì chỉ
    // mốc đầu/cuối mới có chú thích trong ngoặc như bản cũ, cho khoa học hơn.
    s.howtoDifficultyIntro = U8("Độ khó tăng dần theo điểm số, màu nền cũng đổi theo từng mốc:");
    s.howtoDifficultyRanges = {
        U8("0 - 2.500"), U8("2.500 - 10.000"), U8("10.000 - 50.000"),
        U8("50.000 - 250.000"), U8("250.000 - 1.000.000"),
        U8("1.000.000 - 2.000.000"), U8("2.000.000+")
    };
    s.howtoDifficultyDescriptors = {
        U8("(Rất dễ)"), U8("(Dễ)"), U8("(Trung bình)"),
        U8("(Khó)"), U8("(Rất khó)"),
        U8("(Siêu khó)"), U8("(Đỉnh cao)")
    };
    s.howtoTimeAttackTitle = U8("ĐẤU THỜI GIAN");
    s.howtoTimeAttackBody = U8(
        "Chạy đua với thời gian, đạt điểm cao nhất có thể!\n"
        "- Bạn bắt đầu với 3 phút.\n"
        "- Đặt khối để hoàn thành hàng/cột và ghi điểm.\n"
        "- Mỗi lần xóa hàng/cột sẽ được cộng thêm thời gian.\n"
        "- Xóa càng nhiều hàng/cột trong 1 lượt, thời gian\n"
        "  thưởng càng nhiều.\n"
        "- Kết thúc khi hết giờ hoặc không còn chỗ đặt khối.");
    s.howtoSurvivalTitle = U8("SINH TỒN");
    s.howtoSurvivalBody = U8(
        "Kiểm soát bàn cờ và giữ Pressure an toàn để sinh tồn!\n"
        "- Bắt đầu với 2 Rock Block trên bàn cờ.\n"
        "- Sau mỗi 15 khối được đặt, thêm Rock Block xuất hiện.\n"
        "- Rock Block chỉ mất khi hàng/cột chứa nó được hoàn\n"
        "  thành - càng nhiều Rock Block tồn tại, Pressure càng\n"
        "  tăng nhanh.\n"
        "- Xóa hàng/cột và phá Rock Block để giảm Pressure.\n"
        "  Pressure đạt 100 sẽ kết thúc trận ngay lập tức.\n"
        "- Ngoài ra, kết thúc khi không còn chỗ đặt khối.\n"
        "Mẹo: đừng chỉ chăm ghi điểm - hãy dọn Rock Block\n"
        "thường xuyên và ăn nhiều hàng/cột cùng lúc để giữ\n"
        "Pressure ở mức thấp.");
    s.pressureLabel = U8("PRESSURE");
    s.pressureWarnRising = U8("\u26A0 PRESSURE RISING");
    s.pressureWarnHigh = U8("\u26A0 HIGH PRESSURE");
    s.pressureWarnCritical = U8("\u2620 CRITICAL PRESSURE");
    s.pressureOverloadLine1 = U8("PRESSURE OVERLOAD");
    s.pressureOverloadLine2 = U8("You Couldn't Survive...");
    s.howtoHint = U8("(Nhấn phím bất kỳ hoặc click để quay lại)");
    s.hudPrefixScore = U8("Điểm: ");
    s.hudPrefixHigh = U8("   Cao nhất: ");
    s.hudPrefixCombo = U8("   Combo: ");
    s.hudComboBlocksPrefix = U8("Cho phép lỡ: ");
    s.hudComboBlocksSuffix = U8(" khối");
    s.gameOverTitle = U8("GAME OVER");
    s.finalScorePrefix = U8("Điểm cuối cùng: ");
    s.finalScoreHighSuffixOpen = U8("   (Cao nhất: ");
    s.retryHint = U8("Nhấn R để chơi lại   |   ESC để về Menu");
    s.timeAttackLabel = U8("THỜI GIAN");
    s.classicPlayTimeLabel = U8("ĐÃ CHƠI");
    s.survivalObstacleLabel = U8("ĐÁ");

    s.panelScoreLabel = U8("ĐIỂM");
    s.panelDifficultyLabel = U8("ĐỘ KHÓ");
    s.panelHighScoreLabel = U8("ĐIỂM CAO NHẤT");
    s.panelComboLabel = U8("COMBO");
    s.difficultyTierNames = {
        U8("Yên bình"), U8("Dễ"), U8("Bình thường"),
        U8("Khó"), U8("Cực đoan"), U8("Điên rồ"), U8("Bậc thầy")
    };

    s.settingsTitle = U8("Cài đặt");
    s.labelSound = U8("Âm thanh");
    s.labelMusic = U8("Nhạc nền");
    s.stateOn = U8("BẬT");
    s.stateOff = U8("TẮT");
    s.btnHome = U8("Trang chủ");
    s.btnReplay = U8("Chơi lại");
    return s;
}

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

// Mở rộng: màu nền (gradient top/bottom) theo mốc độ khó - đi từ màu NHẠT (Yên bình,
// siêu dễ) tới màu ĐẬM (Bậc thầy, đỉnh cao), đổi tông rõ rệt giữa từng mốc để người
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
    shadowEdge.setPosition(sf::Vector2f(px, py + w - w * 0.14f));
    sf::Color sh = shade(base, 0.65f);
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
// đụng vào code gốc của 4 thành viên.
// Đếm số vùng trống liên thông (4 hướng) trên lưới + kích thước vùng trống lớn nhất.
