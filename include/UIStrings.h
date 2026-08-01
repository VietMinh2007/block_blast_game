#pragma once
// ===================== NGON NGU (Choose A Language) =====================
// Nguoi phu trach: D - UI/Da ngon ngu
// Module nay tach rieng toan bo chuoi giao dien tieng Anh/Viet ra khoi
// main.cpp de nguoi phu trach UI co the sua noi dung/them ngon ngu ma
// khong dung cham file cua nguoi khac.
#include <SFML/Graphics.hpp>
#include <array>
#include <string>

enum class Language { ENGLISH, VIETNAMESE };

// Văn bản tiếng Việt CÓ DẤU (ví dụ Disclaimer) phải nạp qua U8() để SFML
// hiểu đúng là UTF-8 (font DejaVu Sans Bold đi kèm có đủ ký tự tiếng Việt).
static sf::String U8(const std::string& utf8) {
    return sf::String::fromUtf8(utf8.begin(), utf8.end());
}

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

UiStrings makeEnglishStrings();
UiStrings makeVietnameseStrings();
