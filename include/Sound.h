#pragma once
#include <SFML/Audio.hpp>
#include <array>
#include <optional>

// ===================== MODULE ÂM THANH (SFX + NHẠC NỀN) =====================
// Cách hoạt động:
//  - Mọi hiệu ứng (đặt khối, đặt sai chỗ, xóa hàng/cột, "hô" combo kiểu
//    Nice/Great/Awesome/Perfect, game over) được TỰ TỔNG HỢP bằng sóng
//    sine ngay trong code (xem Sound.cpp), nên game có âm thanh chạy được
//    NGAY mà không cần bất kỳ file .wav/.ogg nào đi kèm.
//  - Nếu bạn có file âm thanh/giọng đọc thật (ví dụ giọng người nói
//    "Good/Great/Perfect", một bản nhạc Lo-fi/EDM .ogg), chỉ cần bỏ đúng
//    tên file vào thư mục liệt kê bên dưới - class này sẽ TỰ ĐỘNG ưu tiên
//    dùng file đó thay cho âm thanh tổng hợp, không cần sửa code:
//
//      assets/sfx/place.ogg          - tiếng đặt khối
//      assets/sfx/invalid.ogg        - tiếng đặt sai chỗ
//      assets/sfx/gameover.ogg       - tiếng khi thua
//      assets/sfx/clear1.ogg .. clear5.ogg     - tiếng xóa 1,2,3,4,5+ hàng/cột (5 mức riêng biệt)
//      assets/sfx/fullclear.ogg      - tiếng khi ăn SẠCH toàn bộ lưới (All Clear), phát
//                                       THÊM (không thay thế) tiếng clearN ở trên
//      assets/sfx/voice_nice.ogg     - "hô" khi combo thấp    (Nice!)
//      assets/sfx/voice_great.ogg    - "hô" khi combo vừa     (Great!)
//      assets/sfx/voice_awesome.ogg  - "hô" khi combo cao     (Awesome!)
//      assets/sfx/voice_perfect.ogg  - "hô" khi combo rất cao (Perfect!)
//      assets/sfx/voice_legendary.ogg- "hô" khi combo max     (Legendary!)
//      assets/music/bgm.ogg (hoặc .wav) - nhạc nền lặp khi chơi (Lofi/EDM nhẹ...)
//
// (định dạng .wav cũng dùng được cho các file sfx, không bắt buộc .ogg)

class SoundManager {
public:
    SoundManager();

    void playPlace();                                   // khi đặt khối xuống lưới thành công
    void playInvalid();                                  // khi thả khối vào vị trí không hợp lệ
    void playClear(int linesCleared, int comboStreak);   // khi xóa hàng/cột (kèm "hô" combo)
    void playFullClear();                                 // khi ăn SẠCH toàn bộ lưới (All Clear) - phát thêm, kèm playClear
    void playGameOver();                                 // khi game kết thúc

    void startMusic();                // bắt đầu / tiếp tục phát nhạc nền (tự lặp)
    void stopMusic();                 // dừng nhạc nền (vd khi về menu)
    void setMusicVolume(float v0to100);
    void setSfxVolume(float v0to100);

private:
    float sfxVolume = 75.f;
    float musicVolume = 32.f;

    // ---- buffer chứa dữ liệu âm thanh (tổng hợp hoặc nạp từ file) ----
    sf::SoundBuffer placeBuf, invalidBuf, gameOverBuf, fullClearBuf;
    std::array<sf::SoundBuffer, 5> clearBuf; // ứng với 1 / 2 / 3 / 4 / 5+ hàng-cột bị xóa cùng lúc
    std::array<sf::SoundBuffer, 5> voiceBuf; // ứng với 5 mức "hô": Nice / Great / Awesome / Perfect / Legendary
    sf::SoundBuffer fallbackMusicBuf;         // nhạc nền tổng hợp dự phòng (khi không có file thật)

    // sf::Sound không có constructor mặc định (bắt buộc phải có buffer ngay khi
    // tạo) nên dùng std::optional để có thể khởi tạo SAU khi buffer đã nạp xong.
    std::optional<sf::Sound> placeSnd, invalidSnd, gameOverSnd, fullClearSnd, fallbackMusicSnd;
    std::array<std::optional<sf::Sound>, 5> clearSnd;
    std::array<std::optional<sf::Sound>, 5> voiceSnd;

    sf::Music fileMusic;     // nhạc nền thật, dùng khi tìm thấy assets/music/bgm.*
    bool hasFileMusic = false;
};
