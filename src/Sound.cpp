#include "Sound.h"
#include <cmath>
#include <cstdint>
#include <vector>
#include <initializer_list>
#include <algorithm>
#include <filesystem>

namespace {

constexpr unsigned SAMPLE_RATE = 44100;
constexpr float PI2 = 6.28318530718f;

// Sinh 1 nốt sóng sine với bao (envelope) attack nhanh + decay theo hàm mũ,
// tạo cảm giác "tưng" giống tiếng chuông/tiếng khối game thay vì tiếng bíp thô.
std::vector<std::int16_t> synthNote(float freqHz, float durationSec,
                                     float peakVol = 0.5f, float attackSec = 0.005f,
                                     float decaySharpness = 0.9f) {
    std::size_t n = static_cast<std::size_t>(durationSec * SAMPLE_RATE);
    std::vector<std::int16_t> out(n);
    for (std::size_t i = 0; i < n; i++) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float env;
        if (t < attackSec) {
            env = t / attackSec;
        } else {
            float rel = (t - attackSec) / std::max(0.001f, durationSec - attackSec);
            env = std::pow(1.f - std::min(rel, 1.f), 0.4f + decaySharpness * 2.6f);
        }
        float sample = std::sin(PI2 * freqHz * t) * peakVol * env;
        sample = std::max(-1.f, std::min(1.f, sample));
        out[i] = static_cast<std::int16_t>(sample * 32000.f);
    }
    return out;
}

// Nối nhiều đoạn mẫu lại theo thời gian (phát tuần tự từng nốt), có thể chèn khoảng lặng nhỏ giữa các nốt.
std::vector<std::int16_t> concatSamples(std::initializer_list<std::vector<std::int16_t>> parts, float gapSec = 0.0f) {
    std::vector<std::int16_t> gap(static_cast<std::size_t>(gapSec * SAMPLE_RATE), 0);
    std::vector<std::int16_t> out;
    bool first = true;
    for (auto& p : parts) {
        if (!first) out.insert(out.end(), gap.begin(), gap.end());
        out.insert(out.end(), p.begin(), p.end());
        first = false;
    }
    return out;
}

// Trộn (mix) nhiều đoạn mẫu phát CÙNG lúc, ví dụ để tạo hợp âm từ các nốt đơn.
std::vector<std::int16_t> mixSamples(std::initializer_list<std::vector<std::int16_t>> parts) {
    std::size_t maxLen = 0;
    for (auto& p : parts) maxLen = std::max(maxLen, p.size());
    std::vector<float> acc(maxLen, 0.f);
    for (auto& p : parts) {
        for (std::size_t i = 0; i < p.size(); i++) acc[i] += static_cast<float>(p[i]);
    }
    std::vector<std::int16_t> out(maxLen);
    for (std::size_t i = 0; i < maxLen; i++) {
        out[i] = static_cast<std::int16_t>(std::max(-32000.f, std::min(32000.f, acc[i])));
    }
    return out;
}

void loadSamplesMono(sf::SoundBuffer& buf, const std::vector<std::int16_t>& samples) {
    bool ok = buf.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE, {sf::SoundChannel::Mono});
    (void)ok; // dữ liệu tự tổng hợp trong code nên luôn hợp lệ, bỏ qua giá trị trả về
}

// Nếu tồn tại file thật ở đường dẫn chỉ định thì nạp đè lên buffer đã tổng hợp sẵn.
void preferFileIfExists(sf::SoundBuffer& buf, const std::string& path) {
    if (std::filesystem::exists(path)) {
        sf::SoundBuffer fileBuf;
        if (fileBuf.loadFromFile(path)) buf = fileBuf;
    }
}

// ----- Vòng lặp nhạc nền tổng hợp dự phòng: hợp âm Am7 nhẹ nhàng, biên độ
// nhấp nhô chậm (LFO) như nhạc lofi/ambient nền, fade-in/out 2 đầu để lặp mượt. -----
std::vector<std::int16_t> makeAmbientLoop(float durationSec = 6.f) {
    std::size_t n = static_cast<std::size_t>(durationSec * SAMPLE_RATE);
    std::array<float, 4> freqs = {220.00f, 261.63f, 329.63f, 392.00f}; // A3-C4-E4-G4 (Am7)
    std::vector<std::int16_t> out(n, 0);
    for (std::size_t i = 0; i < n; i++) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float sample = 0.f;
        for (std::size_t k = 0; k < freqs.size(); k++) {
            float lfo = 0.5f + 0.5f * std::sin(PI2 * (0.06f + k * 0.013f) * t + k * 1.7f);
            sample += std::sin(PI2 * freqs[k] * t) * (0.09f + 0.05f * lfo);
        }
        float fadeLen = 0.4f;
        float fade = 1.f;
        if (t < fadeLen) fade = t / fadeLen;
        else if (t > durationSec - fadeLen) fade = (durationSec - t) / fadeLen;
        sample *= fade;
        sample = std::max(-1.f, std::min(1.f, sample));
        out[i] = static_cast<std::int16_t>(sample * 21000.f);
    }
    return out;
}

} // namespace

SoundManager::SoundManager() {
    // ---------- Tiếng "cộp" khi đặt khối: 1 tiếng thump trầm + 1 tiếng click cao trộn lại ----------
    {
        auto thump = synthNote(170.f, 0.09f, 0.55f, 0.002f, 0.7f);
        auto click = synthNote(1500.f, 0.03f, 0.30f, 0.001f, 1.1f);
        loadSamplesMono(placeBuf, mixSamples({thump, click}));
    }

    // ---------- Tiếng "bíp bíp" trầm khi đặt sai vị trí ----------
    {
        auto b1 = synthNote(146.f, 0.07f, 0.42f, 0.004f, 0.6f);
        auto b2 = synthNote(123.f, 0.09f, 0.42f, 0.004f, 0.8f);
        loadSamplesMono(invalidBuf, concatSamples({b1, b2}, 0.02f));
    }

    // ---------- Tiếng khi thua: 3 nốt đi xuống, trầm buồn ----------
    {
        auto d1 = synthNote(392.00f, 0.20f, 0.42f, 0.004f, 0.7f); // G4
        auto d2 = synthNote(329.63f, 0.20f, 0.42f, 0.004f, 0.7f); // E4
        auto d3 = synthNote(261.63f, 0.38f, 0.42f, 0.004f, 1.0f); // C4
        loadSamplesMono(gameOverBuf, concatSamples({d1, d2, d3}, 0.02f));
    }

    // ---------- Tiếng chuông khi xóa 1 / 2 / 3 / 4 / 5+ hàng-cột cùng lúc ----------
    // Càng xóa nhiều cùng lúc thì hợp âm càng dài & càng nhiều nốt (cảm giác "khủng" dần).
    const float C6 = 1046.50f, E6 = 1318.51f, G6 = 1567.98f, C7 = 2093.00f, E7 = 2637.02f;
    loadSamplesMono(clearBuf[0], concatSamples({ synthNote(C6, 0.16f, 0.5f, 0.004f, 0.9f) }));
    loadSamplesMono(clearBuf[1], concatSamples({
        synthNote(C6, 0.11f, 0.5f, 0.004f, 0.9f), synthNote(E6, 0.18f, 0.5f, 0.004f, 0.9f) }, 0.01f));
    loadSamplesMono(clearBuf[2], concatSamples({
        synthNote(C6, 0.09f, 0.5f, 0.004f, 0.9f), synthNote(E6, 0.11f, 0.5f, 0.004f, 0.9f),
        synthNote(G6, 0.20f, 0.5f, 0.004f, 0.9f) }, 0.01f));
    loadSamplesMono(clearBuf[3], concatSamples({
        synthNote(C6, 0.08f, 0.5f, 0.004f, 0.9f), synthNote(E6, 0.09f, 0.5f, 0.004f, 0.9f),
        synthNote(G6, 0.10f, 0.5f, 0.004f, 0.9f), synthNote(C7, 0.30f, 0.5f, 0.004f, 0.9f) }, 0.008f));
    // 5+ hàng/cột cùng lúc: hợp âm đầy đủ nhất, thêm 1 nốt cao vút (E7) ở cuối tạo
    // cảm giác "khủng" hơn hẳn 4 mức trước, khuyến khích người chơi dồn combo lớn.
    loadSamplesMono(clearBuf[4], concatSamples({
        synthNote(C6, 0.07f, 0.5f, 0.004f, 0.9f), synthNote(E6, 0.08f, 0.5f, 0.004f, 0.9f),
        synthNote(G6, 0.09f, 0.5f, 0.004f, 0.9f), synthNote(C7, 0.11f, 0.5f, 0.004f, 0.9f),
        synthNote(E7, 0.38f, 0.55f, 0.004f, 0.85f) }, 0.007f));

    // ---------- 4 mức "hô" combo (thay cho giọng đọc thật): Nice / Great / Awesome / Perfect ----------
    // Đây là các hợp âm/chuỗi nốt tổng hợp đóng vai trò "tiếng hô" - vì không thể tạo giọng nói
    // người thật bằng cách tổng hợp toán học; xem chú thích trong Sound.h để thay bằng file giọng thật.
    loadSamplesMono(voiceBuf[0], mixSamples({
        synthNote(880.00f, 0.20f, 0.40f, 0.005f, 0.8f),
        synthNote(1318.51f, 0.20f, 0.30f, 0.005f, 0.8f) })); // "Nice!"
    loadSamplesMono(voiceBuf[1], mixSamples({
        synthNote(1046.50f, 0.26f, 0.36f, 0.005f, 0.75f),
        synthNote(1318.51f, 0.26f, 0.30f, 0.005f, 0.75f),
        synthNote(1567.98f, 0.26f, 0.26f, 0.005f, 0.75f) })); // "Great!"
    loadSamplesMono(voiceBuf[2], mixSamples({
        synthNote(1046.50f, 0.32f, 0.32f, 0.005f, 0.7f),
        synthNote(1318.51f, 0.32f, 0.28f, 0.005f, 0.7f),
        synthNote(1567.98f, 0.32f, 0.24f, 0.005f, 0.7f),
        synthNote(2093.00f, 0.32f, 0.20f, 0.005f, 0.7f) })); // "Awesome!"
    {
        auto arp = concatSamples({
            synthNote(1318.51f, 0.08f, 0.42f, 0.003f, 1.0f),
            synthNote(1567.98f, 0.08f, 0.42f, 0.003f, 1.0f),
            synthNote(2093.00f, 0.08f, 0.42f, 0.003f, 1.0f),
            synthNote(2637.02f, 0.12f, 0.42f, 0.003f, 1.0f) });
        auto shimmer = mixSamples({
            synthNote(1318.51f, 0.45f, 0.20f, 0.05f, 0.65f),
            synthNote(1567.98f, 0.45f, 0.18f, 0.05f, 0.65f),
            synthNote(2093.00f, 0.45f, 0.16f, 0.05f, 0.65f),
            synthNote(2637.02f, 0.45f, 0.14f, 0.05f, 0.65f) });
        loadSamplesMono(voiceBuf[3], concatSamples({arp, shimmer})); // "Perfect!"
    }
    // Mức "hô" thứ 5 (cao nhất): "Legendary!" - arpeggio dài hơn, leo cao hơn Perfect!,
    // dùng cho lúc xóa 5+ hàng/cột cùng lúc hoặc chuỗi combo cực dài.
    {
        auto arp = concatSamples({
            synthNote(1046.50f, 0.06f, 0.40f, 0.003f, 1.0f),
            synthNote(1318.51f, 0.06f, 0.40f, 0.003f, 1.0f),
            synthNote(1567.98f, 0.06f, 0.40f, 0.003f, 1.0f),
            synthNote(2093.00f, 0.06f, 0.42f, 0.003f, 1.0f),
            synthNote(2637.02f, 0.06f, 0.44f, 0.003f, 1.0f),
            synthNote(3135.96f, 0.16f, 0.46f, 0.003f, 0.95f) });
        auto shimmer = mixSamples({
            synthNote(1567.98f, 0.55f, 0.20f, 0.05f, 0.6f),
            synthNote(2093.00f, 0.55f, 0.19f, 0.05f, 0.6f),
            synthNote(2637.02f, 0.55f, 0.17f, 0.05f, 0.6f),
            synthNote(3135.96f, 0.55f, 0.15f, 0.05f, 0.6f) });
        loadSamplesMono(voiceBuf[4], concatSamples({arp, shimmer})); // "Legendary!"
    }

    // ---------- Tiếng khi ăn SẠCH toàn bộ lưới (All Clear) ----------
    // Phát THÊM (không thay thế) tiếng clearN ở trên: 1 chuỗi hợp âm quét từ trầm
    // lên cao rồi bung "lấp lánh" dài hơi, tạo cảm giác thành tựu lớn, khác hẳn
    // các tiếng xóa dòng thông thường.
    {
        auto sweep = concatSamples({
            synthNote(523.25f, 0.05f, 0.40f, 0.002f, 1.0f),
            synthNote(659.25f, 0.05f, 0.40f, 0.002f, 1.0f),
            synthNote(783.99f, 0.05f, 0.42f, 0.002f, 1.0f),
            synthNote(1046.50f, 0.05f, 0.44f, 0.002f, 1.0f),
            synthNote(1318.51f, 0.05f, 0.46f, 0.002f, 1.0f),
            synthNote(1567.98f, 0.05f, 0.48f, 0.002f, 1.0f),
            synthNote(2093.00f, 0.22f, 0.52f, 0.002f, 0.9f) });
        auto glitter = mixSamples({
            synthNote(2093.00f, 0.7f, 0.22f, 0.08f, 0.55f),
            synthNote(2637.02f, 0.7f, 0.20f, 0.08f, 0.55f),
            synthNote(3135.96f, 0.7f, 0.18f, 0.08f, 0.55f),
            synthNote(4186.01f, 0.7f, 0.14f, 0.08f, 0.55f) });
        loadSamplesMono(fullClearBuf, concatSamples({sweep, glitter}));
    }

    // ---------- Nếu người dùng có sẵn file âm thanh/giọng đọc thật thì ưu tiên dùng ----------
    preferFileIfExists(placeBuf, "assets/sfx/place.ogg");
    preferFileIfExists(placeBuf, "assets/sfx/place.wav");
    preferFileIfExists(invalidBuf, "assets/sfx/invalid.ogg");
    preferFileIfExists(invalidBuf, "assets/sfx/invalid.wav");
    preferFileIfExists(gameOverBuf, "assets/sfx/gameover.ogg");
    preferFileIfExists(gameOverBuf, "assets/sfx/gameover.wav");
    preferFileIfExists(fullClearBuf, "assets/sfx/fullclear.ogg");
    preferFileIfExists(fullClearBuf, "assets/sfx/fullclear.wav");
    const char* clearFiles[5] = {"clear1", "clear2", "clear3", "clear4", "clear5"};
    const char* voiceFiles[5] = {"voice_nice", "voice_great", "voice_awesome", "voice_perfect", "voice_legendary"};
    for (int i = 0; i < 5; i++) {
        preferFileIfExists(clearBuf[i], std::string("assets/sfx/") + clearFiles[i] + ".ogg");
        preferFileIfExists(clearBuf[i], std::string("assets/sfx/") + clearFiles[i] + ".wav");
        preferFileIfExists(voiceBuf[i], std::string("assets/sfx/") + voiceFiles[i] + ".ogg");
        preferFileIfExists(voiceBuf[i], std::string("assets/sfx/") + voiceFiles[i] + ".wav");
    }

    // ---------- Tạo các sf::Sound SAU khi buffer đã có dữ liệu đầy đủ ----------
    placeSnd.emplace(placeBuf);
    invalidSnd.emplace(invalidBuf);
    gameOverSnd.emplace(gameOverBuf);
    fullClearSnd.emplace(fullClearBuf);
    for (int i = 0; i < 5; i++) {
        clearSnd[i].emplace(clearBuf[i]);
        voiceSnd[i].emplace(voiceBuf[i]);
    }

    // ---------- Nhạc nền: ưu tiên file thật (Lofi/EDM nhẹ/nhạc vui...), nếu không có thì
    // dùng vòng lặp ambient tổng hợp làm nhạc nền dự phòng để game vẫn có nhạc ngay. ----------
    if (fileMusic.openFromFile("assets/music/bgm.ogg") || fileMusic.openFromFile("assets/music/bgm.wav")) {
        hasFileMusic = true;
        fileMusic.setLooping(true);
    } else {
        loadSamplesMono(fallbackMusicBuf, makeAmbientLoop());
        fallbackMusicSnd.emplace(fallbackMusicBuf);
        fallbackMusicSnd->setLooping(true);
    }

    setSfxVolume(sfxVolume);
    setMusicVolume(musicVolume);
}

void SoundManager::playPlace() { if (placeSnd) placeSnd->play(); }
void SoundManager::playInvalid() { if (invalidSnd) invalidSnd->play(); }

void SoundManager::playClear(int linesCleared, int comboStreak) {
    if (linesCleared <= 0) return;
    int tier = std::min(std::max(linesCleared, 1), 5) - 1; // 0..4 (1/2/3/4/5+ hàng-cột)
    if (clearSnd[tier]) clearSnd[tier]->play();

    // Mức "hô" combo dựa trên số hàng/cột xóa cùng lúc VÀ chuỗi combo liên tiếp,
    // lấy giá trị lớn hơn để combo dài cũng được "hô" to dần theo thời gian chơi.
    int voiceTier = std::min(std::max(std::max(linesCleared, comboStreak), 1), 5) - 1;
    if (voiceSnd[voiceTier]) voiceSnd[voiceTier]->play();
}

void SoundManager::playFullClear() { if (fullClearSnd) fullClearSnd->play(); }

void SoundManager::playGameOver() { if (gameOverSnd) gameOverSnd->play(); }

void SoundManager::startMusic() {
    if (hasFileMusic) {
        if (fileMusic.getStatus() != sf::Music::Status::Playing) fileMusic.play();
    } else if (fallbackMusicSnd) {
        if (fallbackMusicSnd->getStatus() != sf::Sound::Status::Playing) fallbackMusicSnd->play();
    }
}

void SoundManager::stopMusic() {
    if (hasFileMusic) fileMusic.stop();
    else if (fallbackMusicSnd) fallbackMusicSnd->stop();
}

void SoundManager::setMusicVolume(float v) {
    musicVolume = v;
    if (hasFileMusic) fileMusic.setVolume(v);
    else if (fallbackMusicSnd) fallbackMusicSnd->setVolume(v);
}

void SoundManager::setSfxVolume(float v) {
    sfxVolume = v;
    if (placeSnd) placeSnd->setVolume(v);
    if (invalidSnd) invalidSnd->setVolume(v);
    if (gameOverSnd) gameOverSnd->setVolume(v);
    if (fullClearSnd) fullClearSnd->setVolume(v);
    for (auto& s : clearSnd) if (s) s->setVolume(v);
    for (auto& s : voiceSnd) if (s) s->setVolume(v * 0.9f);
}
