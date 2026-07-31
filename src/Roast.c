#include "Roast.h"
#include <random>

namespace {
std::mt19937& roastRng() {
    static std::mt19937 engine(std::random_device{}());
    return engine;
}
double rollUnit01() {
    static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(roastRng());
}
} // namespace

RoastManager::RoastManager() {
    setLanguage(true); // Default to Vietnamese
}

void RoastManager::setLanguage(bool isVietnamese) {
    if (isVietnamese) {
        badPlacementLines = {
            u8"Bạn đang chơi Block Blast hay đang tập tô màu kín màn hình vậy?",
            u8"Không gian thì rộng mà tầm nhìn của bạn thì hơi hẹp nhỉ.",
            u8"Thiên tài toán học cũng phải sang chấn tâm lý với cách bạn tối ưu không gian.",
            u8"Khối này sinh ra không phải để nằm ở đây, và có vẻ bạn cũng không sinh ra để chơi game này.",
            u8"Nước đi này AI của tôi tính 14 triệu kết quả cũng không dám nghĩ tới. Tuyệt vời!",
            u8"Bạn xếp hình hay xây tường rào chống giặc thế? Chỗ trống to đùng bên kia để cúng à?",
            u8"Màn hình của bạn bị lỗi cảm ứng à? Chứ người bình thường ai lại đặt ở đấy."
        };
        missedClearLines = {
            u8"Cái hàng ngang kia nó vẫy tay chào bạn nãy giờ kìa, nhưng chắc ping mắt bạn hơi cao.",
            u8"Combo dâng tận miệng rồi mà bạn vẫn kiên quyết nhổ đi. Sống thanh cao thế?",
            u8"Tôi đã chuẩn bị sẵn hiệu ứng nổ combo siêu đẹp rồi, cảm ơn bạn đã cất nó vào kho.",
            u8"Game cho bạn cơ hội sửa sai, nhưng bạn chọn cách làm nó sai thêm.",
            u8"Nhìn cách bạn lướt qua cơ hội ghi điểm kìa, y hệt cách crush lướt qua đời bạn vậy."
        };
        gameOverLines = {
            u8"Điểm của bạn còn chưa kịp làm nóng cái bảng xếp hạng nữa.",
            u8"Kỷ lục mới! Chưa thấy ai có thể thua nhanh, gọn và dứt khoát đến thế.",
            u8"Màn hình Game Over này chắc bạn quen lắm rồi nhỉ? Cứ tự nhiên như ở nhà nhé.",
            u8"Chơi thêm ván nữa đi, để chứng minh ván này bạn thua không phải do xui, mà là do thực lực.",
            u8"Nút 'Chơi Lại' được thiết kế riêng cho những người kiên trì nhưng thiếu kỹ năng như bạn đấy.",
            u8"Điểm số này mà đem đi khoe chắc cả họ tự hào lắm."
        };
        afkBadMoveLines = {
            u8"Load não mất nửa ngày chỉ để đặt một khối vào chỗ vô dụng nhất bản đồ. Tốc độ xử lý đáng nể!",
            u8"Tôi tưởng bạn đang ngâm cứu siêu combo 5 dòng, hóa ra bạn đang tính xem làm sao để game over nhanh nhất.",
            u8"CPU của bạn chạy 100% nãy giờ chỉ để đưa ra quyết định này thôi sao?",
            u8"Bạn nghĩ lâu như vậy là để chờ tôi ngủ gật rồi lén ăn gian đúng không? Đáng tiếc là tôi không biết ngủ."
        };
    } else {
        badPlacementLines = {
            u8"Are you playing Block Blast or trying to color the whole screen?",
            u8"So much space, yet your vision is so narrow.",
            u8"Even a math genius would get PTSD from your spatial optimization.",
            u8"This block wasn't born to be here, and maybe you weren't born to play this game.",
            u8"My AI analyzed 14 million outcomes and couldn't predict this move. Brilliant!",
            u8"Are you playing Tetris or building a wall? What's that huge gap for?",
            u8"Is your touchscreen broken? Why would anyone place it there."
        };
        missedClearLines = {
            u8"That row has been waving at you, but I guess your ping is too high.",
            u8"A combo was served on a silver platter, but you refused it. So noble?",
            u8"I prepared a beautiful combo explosion effect, thanks for keeping it in the warehouse.",
            u8"The game gave you a chance to fix your mistake, but you chose to make it worse.",
            u8"Look at how you swipe past scoring opportunities, just like your crush swiped past you."
        };
        gameOverLines = {
            u8"Your score hasn't even warmed up the leaderboard yet.",
            u8"New record! I've never seen anyone lose so quickly and decisively.",
            u8"You must be very familiar with this Game Over screen by now. Make yourself at home.",
            u8"Play one more time, just to prove you lost because of skill, not bad luck.",
            u8"The 'Replay' button was designed specifically for persistent but unskilled players like you.",
            u8"Bring this score home and your whole family will be so proud."
        };
        afkBadMoveLines = {
            u8"Loading your brain for half a day just to put a block in the most useless spot. Amazing processing speed!",
            u8"I thought you were studying a 5-line super combo, turns out you were figuring out how to game over the fastest.",
            u8"Your CPU was at 100% all this time just to make THIS decision?",
            u8"Did you think so long hoping I'd fall asleep so you could cheat? Too bad I don't sleep."
        };
    }
}

const std::vector<std::string>& RoastManager::linesFor(RoastTrigger t) const {
    switch (t) {
        case RoastTrigger::BadPlacement:     return badPlacementLines;
        case RoastTrigger::MissedClear:      return missedClearLines;
        case RoastTrigger::GameOverLowScore: return gameOverLines;
        case RoastTrigger::AfkBadMove:       return afkBadMoveLines;
    }
    return badPlacementLines;
}

void RoastManager::update(float dt) {
    if (cooldownTimer > 0.f) cooldownTimer -= dt;
    if (displayTimer > 0.f) displayTimer -= dt;
}

bool RoastManager::tryTrigger(RoastTrigger trigger) {
    // Cooldown toàn cục chưa hết -> chưa được chửi câu tiếp theo.
    if (cooldownTimer > 0.f) return false;

    // Đã hết cooldown, nhưng chỉ có TRIGGER_CHANCE (40%) cơ hội bot lên tiếng.
    if (rollUnit01() >= TRIGGER_CHANCE) return false;

    const auto& pool = linesFor(trigger);
    if (pool.empty()) return false;

    std::uniform_int_distribution<size_t> pick(0, pool.size() - 1);
    currentLine = pool[pick(roastRng())];

    displayTimer = DISPLAY_SECONDS;
    cooldownTimer = COOLDOWN_SECONDS;
    return true;
}
