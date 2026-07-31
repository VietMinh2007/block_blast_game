#pragma once
#include <string>
#include <vector>

// ===================== MODULE MỞ RỘNG: "ROBOT MỎ HỖN" =====================
// Một con bot chửi/troll người chơi theo thời gian thực, dựa trên các tình
// huống chơi tệ trong game. Toàn bộ logic KHÔNG phụ thuộc SFML (chỉ xử lý
// chuỗi + xác suất + thời gian), việc vẽ giao diện (avatar + bong bóng thoại)
// được làm riêng trong main.cpp để module này tái sử dụng được ở bất kỳ nơi nào.
//
// Quy tắc hoạt động:
//   - Có 4 loại tình huống kích hoạt (RoastTrigger).
//   - Cooldown TOÀN CỤC 20 giây: sau khi vừa "chửi" 1 câu, phải đợi đủ 20s
//     mới có thể chửi câu tiếp theo (dù tình huống kích hoạt xảy ra dồn dập).
//   - Khi cooldown đã hết VÀ có tình huống kích hoạt xảy ra, chỉ có 40% xác
//     suất bot thực sự lên tiếng (60% còn lại bot im lặng, dù tình huống có xảy ra).
//   - Mỗi câu hiển thị trên màn hình trong một khoảng thời gian ngắn rồi tự ẩn.

enum class RoastTrigger {
    BadPlacement,      // 1. Đặt khối vào chỗ "đi vào lòng đất" (tự bóp không gian)
    MissedClear,        // 2. Bỏ lỡ cơ hội ăn hàng/cột dù đang có sẵn
    GameOverLowScore,    // 3. Game Over với điểm số quá thấp
    AfkBadMove           // 4. Nghĩ rất lâu nhưng vẫn ra nước đi tệ
};

class RoastManager {
public:
    RoastManager();

    // Thay đổi ngôn ngữ của Robot (tiếng Anh / tiếng Việt)
    void setLanguage(bool isVietnamese);

    // Gọi mỗi khung hình (frame) với thời gian trôi qua (giây) để cập nhật
    // đồng hồ cooldown và thời gian hiển thị câu nói hiện tại.
    void update(float dt);

    // Thử kích hoạt 1 câu chửi cho loại `trigger`. Có cơ chế:
    //   - Nếu cooldown CHƯA hết -> bỏ qua, không làm gì (return false).
    //   - Nếu cooldown đã hết -> roll xác suất 40%, nếu trúng thì chọn ngẫu
    //     nhiên 1 câu trong danh sách tương ứng, hiển thị lên và reset cooldown.
    // Trả về true nếu vừa thực sự kích hoạt được 1 câu nói mới.
    bool tryTrigger(RoastTrigger trigger);

    // true nếu bot đang hiển thị 1 câu nói trên màn hình (bong bóng thoại còn hiện).
    bool isActive() const { return displayTimer > 0.f; }

    // Nội dung câu nói hiện tại (chuỗi UTF-8 thô, chưa qua sf::String::fromUtf8).
    const std::string& getCurrentLine() const { return currentLine; }

    // Số giây còn lại trước khi được phép chửi câu tiếp theo (dùng để debug/HUD nếu cần).
    float getCooldownRemaining() const { return cooldownTimer; }

private:
    float cooldownTimer = 0.f;  // giây còn lại trước khi được chửi câu tiếp theo
    float displayTimer = 0.f;   // giây còn lại đang hiển thị câu nói hiện tại
    std::string currentLine;

    static constexpr float COOLDOWN_SECONDS = 20.f; // "khoảng 20s mới chửi 1 câu"
    static constexpr float DISPLAY_SECONDS = 8.5f;  // thời gian bong bóng thoại hiển thị mỗi câu
    static constexpr float TRIGGER_CHANCE = 0.40f;  // tỉ lệ chửi khi kích phát & hết cooldown

    std::vector<std::string> badPlacementLines;
    std::vector<std::string> missedClearLines;
    std::vector<std::string> gameOverLines;
    std::vector<std::string> afkBadMoveLines;

    const std::vector<std::string>& linesFor(RoastTrigger t) const;
};
