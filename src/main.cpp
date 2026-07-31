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
