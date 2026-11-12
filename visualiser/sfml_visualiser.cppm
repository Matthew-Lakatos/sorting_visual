// visualiser/sfml_visualiser.cppm
// ─────────────────────────────────────────────────────────────────────────────
// FIX: render_bars() previously used static_cast<double>(vals[i]) which does
//      not compile for non-arithmetic T (e.g. std::string).  Replaced with
//      rank-based heights: sort a copy, find lower_bound rank for each element,
//      and normalise rank/(N-1) to [0.01,1].  Works for any totally_ordered T.
//
// NEW: value_to_str<T>() exported — algorithm modules use this instead of
//      std::to_string() so HUD labels compile for strings and floats alike.
//
// NEW: HighlightRole::Gap added for shell sort's gap-pair highlighting.
// ─────────────────────────────────────────────────────────────────────────────

export module visualiser.sfml_visualiser;

#include <SFML/Graphics.hpp>

import <vector>;
import <string>;
import <algorithm>;
import <iterator>;
import <cstddef>;
import <cmath>;
import <sstream>;
import <type_traits>;
import <concepts>;

// ── value_to_str ──────────────────────────────────────────────────────────────

export template <typename T>
std::string value_to_str(const T& v) {
    if constexpr (std::is_floating_point_v<T>) {
        std::ostringstream oss;
        oss.precision(4);
        oss << v;
        return oss.str();
    } else if constexpr (std::is_arithmetic_v<T>) {
        return std::to_string(v);
    } else {
        return std::string(v);   // std::string → identity
    }
}

// ── highlight types ───────────────────────────────────────────────────────────

export enum class HighlightRole {
    None,
    Compare,   // amber
    Swap,      // red
    Sorted,    // green
    Pivot,     // magenta
    Bucket,    // cyan
    Gap,       // orange — shell sort gap-pair
};

export struct FrameHighlight {
    std::size_t   index = 0;
    HighlightRole role  = HighlightRole::None;
};

export template <typename T>
struct SortFrame {
    std::vector<T>              values;
    std::vector<FrameHighlight> highlights;
    std::string                 label;
};

// ── SortVisualiser ────────────────────────────────────────────────────────────

export class SortVisualiser {
public:
    static constexpr unsigned WINDOW_W    = 1280;
    static constexpr unsigned WINDOW_H    = 720;
    static constexpr float    DEFAULT_FPS = 30.f;
    static constexpr float    HUD_HEIGHT  = 88.f;

    template <typename T>
        requires std::totally_ordered<T>
    static void run(const std::vector<SortFrame<T>>& frames,
                    const std::string& algorithm_name,
                    float fps = DEFAULT_FPS)
    {
        if (frames.empty()) return;

        sf::RenderWindow window(
            sf::VideoMode(WINDOW_W, WINDOW_H),
            "Sorting Visualiser — " + algorithm_name,
            sf::Style::Titlebar | sf::Style::Close);
        window.setFramerateLimit(60);

        sf::Font font;
        bool font_loaded = false;
        for (const char* p : {
                "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
                "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
                "/usr/share/fonts/opentype/noto/NotoSans-Regular.ttf",
                "/System/Library/Fonts/Helvetica.ttc",
                "C:/Windows/Fonts/arial.ttf" })
            if (font.loadFromFile(p)) { font_loaded = true; break; }

        std::size_t frame_idx   = 0;
        bool        playing     = true;
        float       speed_fps   = fps;
        float       frame_timer = 0.f;
        sf::Clock   clock;

        while (window.isOpen()) {
            float dt = clock.restart().asSeconds();

            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) window.close();
                if (event.type == sf::Event::KeyPressed) {
                    switch (event.key.code) {
                        case sf::Keyboard::Space:
                            playing = !playing; break;
                        case sf::Keyboard::Right:
                            if (frame_idx + 1 < frames.size()) ++frame_idx; break;
                        case sf::Keyboard::Left:
                            if (frame_idx > 0) --frame_idx; break;
                        case sf::Keyboard::R:
                            frame_idx = 0; playing = true; break;
                        case sf::Keyboard::Up:
                            speed_fps = std::min(speed_fps * 1.5f, 240.f); break;
                        case sf::Keyboard::Down:
                            speed_fps = std::max(speed_fps / 1.5f, 1.f); break;
                        case sf::Keyboard::Escape:
                            window.close(); break;
                        default: break;
                    }
                }
            }

            if (playing && frame_idx + 1 < frames.size()) {
                frame_timer += dt;
                float dur = 1.f / speed_fps;
                while (frame_timer >= dur && frame_idx + 1 < frames.size()) {
                    frame_timer -= dur;
                    ++frame_idx;
                }
            }

            window.clear(sf::Color(18, 18, 28));
            render_bars(window, frames[frame_idx]);
            if (font_loaded)
                render_hud(window, font, frames[frame_idx], frame_idx,
                           frames.size(), algorithm_name, speed_fps, playing);
            window.display();
        }
    }

private:
    static sf::Color bar_colour(HighlightRole role) {
        switch (role) {
            case HighlightRole::Compare: return { 255, 180,  40 };
            case HighlightRole::Swap:    return { 230,  60,  60 };
            case HighlightRole::Sorted:  return {  80, 220, 100 };
            case HighlightRole::Pivot:   return { 200,  80, 220 };
            case HighlightRole::Bucket:  return {  60, 200, 220 };
            case HighlightRole::Gap:     return { 255, 120,  40 };
            default:                     return {  80, 140, 200 };
        }
    }

    // ── rank-based bar renderer ───────────────────────────────────────────────
    // Bar height = lower_bound rank in sorted copy / (N-1).
    // Correct for any totally_ordered T — strings sort lexicographically.
    template <typename T>
        requires std::totally_ordered<T>
    static void render_bars(sf::RenderWindow& window, const SortFrame<T>& frame)
    {
        const auto& vals = frame.values;
        if (vals.empty()) return;

        const float  chart_h = static_cast<float>(WINDOW_H) - HUD_HEIGHT - 10.f;
        const float  chart_w = static_cast<float>(WINDOW_W) - 20.f;
        const float  bar_w   = chart_w / static_cast<float>(vals.size());
        const double n       = static_cast<double>(vals.size());

        std::vector<T> sorted_vals = vals;
        std::sort(sorted_vals.begin(), sorted_vals.end());

        std::vector<HighlightRole> roles(vals.size(), HighlightRole::None);
        for (const auto& h : frame.highlights)
            if (h.index < vals.size())
                roles[h.index] = h.role;

        for (std::size_t i = 0; i < vals.size(); ++i) {
            auto it    = std::lower_bound(sorted_vals.begin(), sorted_vals.end(), vals[i]);
            double rk  = static_cast<double>(std::distance(sorted_vals.begin(), it));
            double ratio = (n > 1.0) ? rk / (n - 1.0) : 1.0;
            ratio = std::max(ratio, 0.01);   // min-visible bar for smallest element

            float bar_h = static_cast<float>(ratio) * chart_h;
            float x     = 10.f + static_cast<float>(i) * bar_w;
            float y     = 10.f + chart_h - bar_h;

            sf::RectangleShape bar({ std::max(1.f, bar_w - 1.f), bar_h });
            bar.setPosition(x, y);
            bar.setFillColor(bar_colour(roles[i]));
            bar.setOutlineThickness(0.5f);
            bar.setOutlineColor({ 255, 255, 255, 30 });
            window.draw(bar);
        }
    }

    // ── HUD overlay ───────────────────────────────────────────────────────────
    template <typename T>
    static void render_hud(sf::RenderWindow& window, const sf::Font& font,
                            const SortFrame<T>& frame,
                            std::size_t frame_idx, std::size_t total_frames,
                            const std::string& algo_name,
                            float speed_fps, bool playing)
    {
        const float y_base = static_cast<float>(WINDOW_H) - HUD_HEIGHT;

        sf::RectangleShape bg({ static_cast<float>(WINDOW_W), HUD_HEIGHT });
        bg.setPosition(0.f, y_base);
        bg.setFillColor({ 10, 10, 20, 220 });
        window.draw(bg);

        // Progress bar
        float prog = total_frames > 1
            ? static_cast<float>(frame_idx) / static_cast<float>(total_frames - 1)
            : 1.f;
        sf::RectangleShape pb({ static_cast<float>(WINDOW_W), 4.f });
        pb.setPosition(0.f, y_base);
        pb.setFillColor({ 60, 60, 80 });
        window.draw(pb);
        sf::RectangleShape pf({ static_cast<float>(WINDOW_W) * prog, 4.f });
        pf.setPosition(0.f, y_base);
        pf.setFillColor({ 80, 140, 200 });
        window.draw(pf);

        auto txt = [&](const std::string& s, float x, float y,
                        unsigned sz = 14, sf::Color col = { 220, 220, 220 }) {
            sf::Text t;
            t.setFont(font);
            t.setString(s);
            t.setCharacterSize(sz);
            t.setFillColor(col);
            t.setPosition(x, y);
            window.draw(t);
        };

        txt(algo_name, 12.f, y_base + 8.f, 17, { 255, 255, 100 });
        txt("Frame " + std::to_string(frame_idx + 1) + " / "
            + std::to_string(total_frames), 12.f, y_base + 32.f, 12);

        if (!frame.label.empty())
            txt(frame.label, 255.f, y_base + 8.f, 13, { 200, 200, 210 });

        float rx = static_cast<float>(WINDOW_W) - 300.f;
        txt("Speed: " + std::to_string(static_cast<int>(speed_fps)) + " fps",
            rx, y_base + 8.f, 13);
        txt(playing ? "[PLAYING]" : "[PAUSED]", rx, y_base + 26.f, 13,
            playing ? sf::Color{ 80, 220, 100 } : sf::Color{ 220, 100, 80 });

        txt("SPACE=pause  \u2190\u2192=step  R=restart  \u2191\u2193=speed  ESC=quit",
            static_cast<float>(WINDOW_W) / 2.f - 185.f, y_base + 64.f, 11, { 150, 150, 180 });

        // Colour legend
        struct L { sf::Color c; const char* lbl; };
        const L legend[] = {
            {{ 80,140,200}, "Default"},
            {{255,180, 40}, "Compare"},
            {{230, 60, 60}, "Swap"   },
            {{ 80,220,100}, "Sorted" },
            {{200, 80,220}, "Pivot"  },
            {{ 60,200,220}, "Bucket" },
            {{255,120, 40}, "Gap"    },
        };
        float lx = 12.f, ly = y_base + 60.f;
        for (const auto& l : legend) {
            sf::RectangleShape sw({ 11.f, 11.f });
            sw.setPosition(lx, ly);
            sw.setFillColor(l.c);
            window.draw(sw);
            txt(l.lbl, lx + 14.f, ly - 1.f, 11, { 200, 200, 200 });
            lx += 74.f;
        }
    }
};
