# Mở rộng: "Robot mỏ hỗn" (Roast Bot)

## File mới
- `src/Roast.h`, `src/Roast.cpp` — module quản lý cooldown (20s) + xác suất
  chửi (40%) + 4 bộ câu thoại (đặt khối tệ, bỏ lỡ combo, game over điểm thấp,
  nghĩ lâu vẫn ra nước tệ).
- `assets/robot.png` — avatar robot, cắt ra từ ảnh bạn gửi (chỉ lấy phần robot,
  bỏ khung thoại mẫu vì mình sẽ tự vẽ khung thoại động chứa câu chửi thật).

## File đã sửa
- `src/main.cpp`:
  - `#include "Roast.h"`.
  - 2 hàm phân tích lưới mới (không đụng tới Module 1-4 gốc):
    `analyzeEmptyRegions()` (đếm vùng trống liên thông + vùng lớn nhất) và
    `findNearCompleteLines()` (hàng/cột chỉ còn đúng 1 ô trống).
  - `RoastManager roastManager;`, `sf::Texture robotTexture` (nạp
    `assets/robot.png`), `sf::Clock thinkClock` + `pendingThinkTime` để đo thời
    gian suy nghĩ giữa 2 lần đặt khối.
  - Trong sự kiện thả chuột (đặt khối): chụp trạng thái lưới TRƯỚC khi đặt, so
    sánh SAU khi đặt để gọi `roastManager.tryTrigger(...)` đúng loại tình huống.
  - Khi Game Over với điểm < 1500: gọi `tryTrigger(GameOverLowScore)`.
  - Vẽ avatar + bong bóng thoại ở góc dưới-trái màn hình chơi (giữa 2 tab
    ĐIỂM/ĐỘ KHÓ và khay khối), chỉ hiện khi bot đang "nói".
- `CMakeLists.txt`: thêm `src/Roast.cpp` vào danh sách file build.

## Thêm vào project Visual Studio 2022 (nếu bạn không dùng CMake)
1. Chuột phải **Header Files** → Add → Existing Item... → chọn `src/Roast.h`.
2. Chuột phải **Source Files** → Add → Existing Item... → chọn `src/Roast.cpp`.
3. Copy `assets/robot.png` vào đúng thư mục `assets/` đang dùng cho `font.ttf`
   (thư mục này phải nằm cạnh file .exe khi chạy, y như hướng dẫn cũ trong
   README.md).
4. Build lại (Ctrl+Shift+B) — không cần thêm thư viện gì mới, chỉ dùng
   `sf::Texture`/`sf::Sprite` đã có sẵn trong SFML::Graphics.

## Vị trí bong bóng thoại
Mình đặt avatar + bong bóng ở góc dưới-trái màn hình chơi (khoảng trống giữa
2 tab ĐIỂM/ĐỘ KHÓ và khay 3 khối bên dưới) — vì ảnh bạn gửi không thấy vòng
khoanh rõ ràng nên mình chọn vị trí không đè lên bất kỳ UI nào khác. Nếu bạn
muốn đặt chỗ khác, chỉ cần sửa 2 dòng:
```cpp
const sf::Vector2f robotPos(20.f, 330.f);
```
trong khối code `if (roastManager.isActive()) { ... }` ở cuối phần vẽ màn
hình PLAY (`main.cpp`).

## Các thông số có thể chỉnh
Trong `Roast.h`:
```cpp
static constexpr float COOLDOWN_SECONDS = 20.f; // giãn cách giữa 2 câu chửi
static constexpr float DISPLAY_SECONDS = 5.5f;  // câu hiện trong bao lâu
static constexpr float TRIGGER_CHANCE = 0.40f;  // xác suất chửi khi đủ điều kiện
```
Trong `main.cpp`:
```cpp
const int LOW_SCORE_ROAST_THRESHOLD = 1500; // ngưỡng "điểm thấp" khi Game Over
bool wasLongThink = pendingThinkTime > 10.f; // ngưỡng "nghĩ lâu" (giây)
```

## Logic phát hiện "nước đi tệ" (tóm tắt)
1. **Đặt khối tệ / "đi vào lòng đất"**: khối ≥4 ô, không ăn được hàng/cột nào,
   và khiến số vùng trống liên thông tăng lên HOẶC vùng trống lớn nhất co lại
   còn dưới 60% so với trước khi đặt.
2. **Bỏ lỡ combo**: trước lượt đặt có ít nhất 1 hàng/cột chỉ còn đúng 1 ô
   trống, nhưng lượt đặt đó không ăn được hàng/cột đó.
3. **Game Over điểm thấp**: thua với điểm < 1500.
4. **Nghĩ lâu vẫn tệ**: giống tình huống (1) nhưng thời gian giữa lúc bắt đầu
   kéo khối và lần đặt trước đó > 10 giây.

Mỗi tình huống trên chỉ THỰC SỰ kích hoạt câu chửi nếu cooldown toàn cục 20s
đã hết VÀ roll trúng 40%.

## Sửa lỗi: chữ trong bong bóng thoại bị tràn/lệch ra ngoài khung
**Nguyên nhân thật sự:** hàm `wrapUtf8Text()` đo bề rộng từng dòng bằng một
`sf::Text measurer` ở kiểu chữ THƯỜNG (regular), nhưng dòng chữ khi vẽ ra màn
hình lại dùng `t.setStyle(sf::Text::Bold)`. Chữ Bold trong SFML luôn rộng hơn
chữ thường một chút (vài px mỗi ký tự), nên nhiều dòng được coi là "vừa khít"
lúc đo lại hoá ra rộng hơn khung bong bóng lúc vẽ thật -> chữ bị tràn/lệch ra
ngoài viền, nhất là với câu dài hoặc cỡ chữ lớn.

**Cách sửa** (trong `main.cpp`):
1. `measurer.setStyle(sf::Text::Bold);` ngay sau khi tạo `measurer` trong
   `wrapUtf8Text()`, để đo đúng bằng kiểu chữ sẽ render (Bold), không còn lệch
   giữa lúc đo và lúc vẽ.
2. Trừ thêm 4px an toàn vào bề rộng tối đa khi gọi `wrapUtf8Text(...)` cho
   bong bóng roast, để bù sai số làm tròn pixel, đảm bảo chữ luôn nằm gọn
   trong khung kể cả ở trường hợp biên.

Sau khi sửa, mọi dòng chữ trong bong bóng đều nằm đúng trong viền khung và
canh giữa đều 2 bên, không còn hiện tượng lệch ra ngoài như trước.

## Sửa tiếp: chữ tự bẻ dòng bị đẩy tràn xuống dưới màn hình + câu "màu nền đổi
## theo từng mốc" dính vào bảng độ khó
Sau lần sửa word-wrap ở trên, chữ không còn tràn ra 2 bên nữa, nhưng vì bẻ
dòng nhiều hơn (đặc biệt 2 khung ĐẤU THỜI GIAN / SINH TỒN) nên khối chữ trở
nên CAO hơn bản cũ và bị đẩy tràn xuống dưới, đè lên cả dòng hướng dẫn
"Nhấn phím bất kỳ..." ở cuối màn hình.

**Cách sửa** (`main.cpp`):
1. Giảm cỡ chữ nội dung 3 khối "CƠ BẢN" / "ĐẤU THỜI GIAN" / "SINH TỒN" từ 15
   xuống 13 (cả lúc tạo `sf::Text` lẫn lúc gọi `wrapHowtoBody(...)`), vừa giúp
   chữ gọn đẹp hơn, vừa giảm số dòng bị bẻ (chữ nhỏ hơn thì mỗi dòng chứa được
   nhiều từ hơn) lẫn chiều cao mỗi dòng, nên khối chữ thấp hơn hẳn và không
   còn tràn xuống dưới màn hình nữa.
2. Câu giới thiệu độ khó ("Độ khó tăng dần theo điểm số, màu nền cũng đổi
   theo từng mốc:") dài khoảng 550px trong khi cột trái chỉ rộng ~476px, nên
   trước đây bị tràn/dính sát vào bảng độ khó ngay bên dưới. Giờ câu này cũng
   được đưa qua `wrapHowtoBody(...)` để tự xuống dòng, không còn dính vào
   khung/bảng nữa.
3. Đồng thời đổi cách tính khoảng cách xuống bảng độ khó từ
   `getGlobalBounds()` (chỉ đo đúng phần mực thật, không đúng cho nhiều dòng)
   sang `textBlockHeight()` (đo theo line-height thật của font) để khoảng
   cách luôn chuẩn dù câu chiếm 1 hay 2 dòng.
