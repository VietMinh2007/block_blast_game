#include "UIStrings.h"

UiStrings makeEnglishStrings() {
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

UiStrings makeVietnameseStrings() {
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
    s.howtoDifficultyTitle = U8("| ĐỘ KHÓ");
    // Mở rộng: căn đều cả 5 mốc theo cùng khuôn "Tên (Mức độ)" thay vì chỉ
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

