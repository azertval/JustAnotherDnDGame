// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/ProceduralFont.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <ios>
#include <sstream>
#include <utility>

#include <nlohmann/json.hpp>

namespace hmi {

namespace {

// Nombre de colonnes et de lignes de pixels d'un glyphe de base (le corps de la lettre).
constexpr int GLYPH_COLUMNS = 5;
constexpr int GLYPH_ROWS = 7;

// Dimensions d'une cellule de glyphe dans l'atlas : corps 5x7 + zone d'accents au-dessus + une
// ligne de cedille en dessous.
constexpr int CELL_WIDTH = 6;
constexpr int CELL_HEIGHT = 10;
constexpr int BODY_TOP = 2;  // ligne ou commence le corps du glyphe dans la cellule

// Nombre de cellules par ligne dans l'atlas genere.
constexpr int COLUMNS = 16;

// Premier et dernier code ASCII imprimable couverts par la table de base.
constexpr char32_t ASCII_FIRST = 0x20;  // espace
constexpr char32_t ASCII_LAST = 0x7E;   // ~

// Motif d'un glyphe : 7 lignes de 5 pixels (bit 4 = colonne de gauche).
using Pattern = std::array<std::uint8_t, GLYPH_ROWS>;

// Table des glyphes ASCII imprimables (0x20 a 0x7E), un motif 5x7 par caractere. Chaque octet code
// une ligne : les 5 bits de poids faible sont les colonnes, le bit 4 (valeur 16) etant la colonne
// de gauche. Police pixel art « from scratch », suffisante pour un repli de secours lisible.
constexpr std::array<Pattern, (ASCII_LAST - ASCII_FIRST) + 1> ASCII_FONT = {{
    {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000},  // ' '
    {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000, 0b00100},  // '!'
    {0b01010, 0b01010, 0b01010, 0b00000, 0b00000, 0b00000, 0b00000},  // '"'
    {0b01010, 0b11111, 0b01010, 0b01010, 0b11111, 0b01010, 0b00000},  // '#'
    {0b00100, 0b01111, 0b10100, 0b01110, 0b00101, 0b11110, 0b00100},  // '$'
    {0b11001, 0b11010, 0b00010, 0b00100, 0b01000, 0b01011, 0b10011},  // '%'
    {0b01100, 0b10010, 0b10100, 0b01000, 0b10101, 0b10010, 0b01101},  // '&'
    {0b00100, 0b00100, 0b01000, 0b00000, 0b00000, 0b00000, 0b00000},  // '\''
    {0b00010, 0b00100, 0b01000, 0b01000, 0b01000, 0b00100, 0b00010},  // '('
    {0b01000, 0b00100, 0b00010, 0b00010, 0b00010, 0b00100, 0b01000},  // ')'
    {0b00000, 0b00100, 0b10101, 0b01110, 0b10101, 0b00100, 0b00000},  // '*'
    {0b00000, 0b00100, 0b00100, 0b11111, 0b00100, 0b00100, 0b00000},  // '+'
    {0b00000, 0b00000, 0b00000, 0b00000, 0b00100, 0b00100, 0b01000},  // ','
    {0b00000, 0b00000, 0b00000, 0b01110, 0b00000, 0b00000, 0b00000},  // '-'
    {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00110, 0b00110},  // '.'
    {0b00001, 0b00010, 0b00010, 0b00100, 0b01000, 0b01000, 0b10000},  // '/'
    {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110},  // '0'
    {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110},  // '1'
    {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111},  // '2'
    {0b11111, 0b00010, 0b00100, 0b00010, 0b00001, 0b10001, 0b01110},  // '3'
    {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010},  // '4'
    {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110},  // '5'
    {0b00110, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110},  // '6'
    {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000},  // '7'
    {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110},  // '8'
    {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00010, 0b01100},  // '9'
    {0b00000, 0b00110, 0b00110, 0b00000, 0b00110, 0b00110, 0b00000},  // ':'
    {0b00000, 0b00110, 0b00110, 0b00000, 0b00110, 0b00100, 0b01000},  // ';'
    {0b00010, 0b00100, 0b01000, 0b10000, 0b01000, 0b00100, 0b00010},  // '<'
    {0b00000, 0b00000, 0b11111, 0b00000, 0b11111, 0b00000, 0b00000},  // '='
    {0b01000, 0b00100, 0b00010, 0b00001, 0b00010, 0b00100, 0b01000},  // '>'
    {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b00000, 0b00100},  // '?'
    {0b01110, 0b10001, 0b10111, 0b10101, 0b10111, 0b10000, 0b01110},  // '@'
    {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001},  // 'A'
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110},  // 'B'
    {0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110},  // 'C'
    {0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110},  // 'D'
    {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111},  // 'E'
    {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000},  // 'F'
    {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01111},  // 'G'
    {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001},  // 'H'
    {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b11111},  // 'I'
    {0b00111, 0b00010, 0b00010, 0b00010, 0b00010, 0b10010, 0b01100},  // 'J'
    {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001},  // 'K'
    {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111},  // 'L'
    {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001},  // 'M'
    {0b10001, 0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001},  // 'N'
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110},  // 'O'
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000},  // 'P'
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101},  // 'Q'
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001},  // 'R'
    {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110},  // 'S'
    {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100},  // 'T'
    {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110},  // 'U'
    {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100},  // 'V'
    {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b11011, 0b10001},  // 'W'
    {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001},  // 'X'
    {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100},  // 'Y'
    {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111},  // 'Z'
    {0b01110, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01110},  // '['
    {0b10000, 0b01000, 0b01000, 0b00100, 0b00010, 0b00010, 0b00001},  // '\\'
    {0b01110, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b01110},  // ']'
    {0b00100, 0b01010, 0b10001, 0b00000, 0b00000, 0b00000, 0b00000},  // '^'
    {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111},  // '_'
    {0b01000, 0b00100, 0b00010, 0b00000, 0b00000, 0b00000, 0b00000},  // '`'
    {0b00000, 0b00000, 0b01110, 0b00001, 0b01111, 0b10001, 0b01111},  // 'a'
    {0b10000, 0b10000, 0b10110, 0b11001, 0b10001, 0b10001, 0b11110},  // 'b'
    {0b00000, 0b00000, 0b01110, 0b10001, 0b10000, 0b10001, 0b01110},  // 'c'
    {0b00001, 0b00001, 0b01101, 0b10011, 0b10001, 0b10001, 0b01111},  // 'd'
    {0b00000, 0b00000, 0b01110, 0b10001, 0b11111, 0b10000, 0b01110},  // 'e'
    {0b00110, 0b01001, 0b01000, 0b11100, 0b01000, 0b01000, 0b01000},  // 'f'
    {0b00000, 0b00000, 0b01111, 0b10001, 0b01111, 0b00001, 0b01110},  // 'g'
    {0b10000, 0b10000, 0b10110, 0b11001, 0b10001, 0b10001, 0b10001},  // 'h'
    {0b00100, 0b00000, 0b01100, 0b00100, 0b00100, 0b00100, 0b01110},  // 'i'
    {0b00010, 0b00000, 0b00110, 0b00010, 0b00010, 0b10010, 0b01100},  // 'j'
    {0b10000, 0b10000, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010},  // 'k'
    {0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110},  // 'l'
    {0b00000, 0b00000, 0b11010, 0b10101, 0b10101, 0b10101, 0b10101},  // 'm'
    {0b00000, 0b00000, 0b10110, 0b11001, 0b10001, 0b10001, 0b10001},  // 'n'
    {0b00000, 0b00000, 0b01110, 0b10001, 0b10001, 0b10001, 0b01110},  // 'o'
    {0b00000, 0b00000, 0b11110, 0b10001, 0b11110, 0b10000, 0b10000},  // 'p'
    {0b00000, 0b00000, 0b01111, 0b10001, 0b01111, 0b00001, 0b00001},  // 'q'
    {0b00000, 0b00000, 0b10110, 0b11001, 0b10000, 0b10000, 0b10000},  // 'r'
    {0b00000, 0b00000, 0b01111, 0b10000, 0b01110, 0b00001, 0b11110},  // 's'
    {0b01000, 0b01000, 0b11100, 0b01000, 0b01000, 0b01001, 0b00110},  // 't'
    {0b00000, 0b00000, 0b10001, 0b10001, 0b10001, 0b10011, 0b01101},  // 'u'
    {0b00000, 0b00000, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100},  // 'v'
    {0b00000, 0b00000, 0b10001, 0b10001, 0b10101, 0b10101, 0b01010},  // 'w'
    {0b00000, 0b00000, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001},  // 'x'
    {0b00000, 0b00000, 0b10001, 0b10001, 0b01111, 0b00001, 0b01110},  // 'y'
    {0b00000, 0b00000, 0b11111, 0b00010, 0b00100, 0b01000, 0b11111},  // 'z'
    {0b00110, 0b00100, 0b00100, 0b01000, 0b00100, 0b00100, 0b00110},  // '{'
    {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100},  // '|'
    {0b01100, 0b00100, 0b00100, 0b00010, 0b00100, 0b00100, 0b01100},  // '}'
    {0b00000, 0b00000, 0b01000, 0b10101, 0b00010, 0b00000, 0b00000},  // '~'
}};

// Signe diacritique appliqué a une lettre de base pour former une lettre accentuee.
enum class Accent : std::uint8_t { ACUTE, GRAVE, CIRCUMFLEX, DIAERESIS, CEDILLA };

// Motif d'un accent superieur (2 lignes de 5 pixels), place au-dessus du corps.
using TopAccent = std::array<std::uint8_t, 2>;

TopAccent topAccentPattern(Accent accent) noexcept {
    switch (accent) {
        case Accent::ACUTE:
            return {0b00010, 0b00100};
        case Accent::GRAVE:
            return {0b01000, 0b00100};
        case Accent::CIRCUMFLEX:
            return {0b00100, 0b01010};
        case Accent::DIAERESIS:
            return {0b01010, 0b00000};
        case Accent::CEDILLA:
            break;
    }
    return {0b00000, 0b00000};
}

// Une lettre accentuee : son point de code, la lettre ASCII de base et le diacritique.
struct AccentedGlyph {
    char32_t codePoint;
    char base;
    Accent accent;
};

// Lettres accentuees francaises couvertes, composees a partir des lettres de base. Les points de
// code sont ecrits en valeur numerique (pas en litteral `U'e'`) pour rester independants de
// l'encodage du fichier source.
constexpr std::array<AccentedGlyph, 21> ACCENTED_GLYPHS = {{
    {.codePoint = 0x00E0, .base = 'a', .accent = Accent::GRAVE},       // a
    {.codePoint = 0x00E2, .base = 'a', .accent = Accent::CIRCUMFLEX},  // a
    {.codePoint = 0x00E4, .base = 'a', .accent = Accent::DIAERESIS},   // a
    {.codePoint = 0x00E7, .base = 'c', .accent = Accent::CEDILLA},     // c
    {.codePoint = 0x00E9, .base = 'e', .accent = Accent::ACUTE},       // e
    {.codePoint = 0x00E8, .base = 'e', .accent = Accent::GRAVE},       // e
    {.codePoint = 0x00EA, .base = 'e', .accent = Accent::CIRCUMFLEX},  // e
    {.codePoint = 0x00EB, .base = 'e', .accent = Accent::DIAERESIS},   // e
    {.codePoint = 0x00EE, .base = 'i', .accent = Accent::CIRCUMFLEX},  // i
    {.codePoint = 0x00EF, .base = 'i', .accent = Accent::DIAERESIS},   // i
    {.codePoint = 0x00F4, .base = 'o', .accent = Accent::CIRCUMFLEX},  // o
    {.codePoint = 0x00F6, .base = 'o', .accent = Accent::DIAERESIS},   // o
    {.codePoint = 0x00F9, .base = 'u', .accent = Accent::GRAVE},       // u
    {.codePoint = 0x00FB, .base = 'u', .accent = Accent::CIRCUMFLEX},  // u
    {.codePoint = 0x00FC, .base = 'u', .accent = Accent::DIAERESIS},   // u
    {.codePoint = 0x00C0, .base = 'A', .accent = Accent::GRAVE},       // A
    {.codePoint = 0x00C2, .base = 'A', .accent = Accent::CIRCUMFLEX},  // A
    {.codePoint = 0x00C7, .base = 'C', .accent = Accent::CEDILLA},     // C
    {.codePoint = 0x00C9, .base = 'E', .accent = Accent::ACUTE},       // E
    {.codePoint = 0x00C8, .base = 'E', .accent = Accent::GRAVE},       // E
    {.codePoint = 0x00CA, .base = 'E', .accent = Accent::CIRCUMFLEX},  // E
}};

// Assemble une couleur RVBA (octets) en un pixel R8G8B8A8_UNORM (ordre memoire R,G,B,A).
std::uint32_t pack(std::uint8_t red, std::uint8_t green, std::uint8_t blue,
                   std::uint8_t alpha) noexcept {
    return static_cast<std::uint32_t>(red) | (static_cast<std::uint32_t>(green) << 8) |
           (static_cast<std::uint32_t>(blue) << 16) | (static_cast<std::uint32_t>(alpha) << 24);
}

constexpr std::uint32_t GLYPH_PIXEL = 0xFFFFFFFFU;  // blanc opaque, colorable par teinte

}  // namespace

// Metriques effectives d'un point de code, avec substitution.
const GlyphMetrics* FontMetrics::glyph(char32_t codePoint) const {
    auto found = glyphs.find(codePoint);
    if (found != glyphs.end()) {
        return &found->second;
    }
    if (codePoint == replacementCodePoint) {
        return nullptr;  // deja le remplacement, et absent : rien a substituer de plus
    }
    found = glyphs.find(replacementCodePoint);
    return found == glyphs.end() ? nullptr : &found->second;
}

// Decode le point de code UTF-8 suivant d'une chaine.
char32_t nextUtf8CodePoint(std::string_view text, std::size_t& index) noexcept {
    const auto lead = static_cast<unsigned char>(text[index]);
    if (lead < 0x80) {
        index += 1;
        return lead;
    }

    int continuationBytes = 0;
    char32_t codePoint = 0;
    if ((lead & 0xE0) == 0xC0) {
        continuationBytes = 1;
        codePoint = lead & 0x1FU;
    } else if ((lead & 0xF0) == 0xE0) {
        continuationBytes = 2;
        codePoint = lead & 0x0FU;
    } else if ((lead & 0xF8) == 0xF0) {
        continuationBytes = 3;
        codePoint = lead & 0x07U;
    } else {
        index += 1;  // octet de tete invalide : on avance d'un octet
        return 0xFFFD;
    }

    for (int byte = 1; byte <= continuationBytes; ++byte) {
        if (index + static_cast<std::size_t>(byte) >= text.size()) {
            index = text.size();
            return 0xFFFD;
        }
        const auto continuation =
            static_cast<unsigned char>(text[index + static_cast<std::size_t>(byte)]);
        if ((continuation & 0xC0) != 0x80) {
            index += 1;  // sequence tronquee : on avance d'un octet et on signale
            return 0xFFFD;
        }
        codePoint = (codePoint << 6) | (continuation & 0x3FU);
    }
    index += static_cast<std::size_t>(continuationBytes) + 1;
    return codePoint;
}

// Largeur et hauteur qu'occuperait text avec metrics, a l'echelle scale.
TextExtent measureText(const FontMetrics& metrics, std::string_view text, float scale) noexcept {
    TextExtent extent;
    extent.height = static_cast<float>(metrics.lineHeight) * scale;
    if (text.empty()) {
        return extent;
    }

    float lineWidth = 0.0F;
    float maxWidth = 0.0F;
    int lineCount = 1;
    std::size_t index = 0;
    while (index < text.size()) {
        const char32_t codePoint = nextUtf8CodePoint(text, index);
        if (codePoint == U'\n') {
            maxWidth = std::max(maxWidth, lineWidth);
            lineWidth = 0.0F;
            ++lineCount;
            continue;
        }
        if (const GlyphMetrics* glyph = metrics.glyph(codePoint)) {
            lineWidth += static_cast<float>(glyph->advance) * scale;
        }
    }
    maxWidth = std::max(maxWidth, lineWidth);

    extent.width = maxWidth;
    extent.height = static_cast<float>(metrics.lineHeight) * static_cast<float>(lineCount) * scale;
    return extent;
}

namespace {

// Allume le pixel (x, y) de l'atlas de largeur textureWidth.
void litGlyphPixel(std::vector<std::uint32_t>& pixels, int textureWidth, int x, int y) {
    pixels[(static_cast<std::size_t>(y) * static_cast<std::size_t>(textureWidth)) +
           static_cast<std::size_t>(x)] = GLYPH_PIXEL;
}

// Dessine le corps 5x7 d'une lettre dans la cellule d'indice donne.
void blitBody(std::vector<std::uint32_t>& pixels, int textureWidth, int cellIndex,
              const Pattern& pattern) {
    const int originX = (cellIndex % COLUMNS) * CELL_WIDTH;
    const int originY = (cellIndex / COLUMNS) * CELL_HEIGHT;
    for (int row = 0; row < GLYPH_ROWS; ++row) {
        for (int column = 0; column < GLYPH_COLUMNS; ++column) {
            const bool lit = (pattern[static_cast<std::size_t>(row)] &
                              (1U << (GLYPH_COLUMNS - 1 - column))) != 0;
            if (lit) {
                litGlyphPixel(pixels, textureWidth, originX + column, originY + BODY_TOP + row);
            }
        }
    }
}

// Ajoute un accent superieur (2 lignes) au-dessus du corps, dans la meme cellule.
void blitTopAccent(std::vector<std::uint32_t>& pixels, int textureWidth, int cellIndex,
                   const TopAccent& accent) {
    const int originX = (cellIndex % COLUMNS) * CELL_WIDTH;
    const int originY = (cellIndex / COLUMNS) * CELL_HEIGHT;
    for (int row = 0; std::cmp_less(row, accent.size()); ++row) {
        for (int column = 0; column < GLYPH_COLUMNS; ++column) {
            const bool lit =
                (accent[static_cast<std::size_t>(row)] & (1U << (GLYPH_COLUMNS - 1 - column))) != 0;
            if (lit) {
                litGlyphPixel(pixels, textureWidth, originX + column, originY + row);
            }
        }
    }
}

// Ajoute une cedille sous le corps (derniere ligne de la cellule).
void blitCedilla(std::vector<std::uint32_t>& pixels, int textureWidth, int cellIndex) {
    const int originX = (cellIndex % COLUMNS) * CELL_WIDTH;
    const int originY = (cellIndex / COLUMNS) * CELL_HEIGHT;
    constexpr std::uint8_t CEDILLA = 0b01100;
    for (int column = 0; column < GLYPH_COLUMNS; ++column) {
        const bool lit = (CEDILLA & (1U << (GLYPH_COLUMNS - 1 - column))) != 0;
        if (lit) {
            litGlyphPixel(pixels, textureWidth, originX + column, originY + CELL_HEIGHT - 1);
        }
    }
}

}  // namespace

// Genere, en memoire, une police bitmap minimale et deterministe.
ProceduralFont buildProceduralFont() {
    const int asciiCount = static_cast<int>(ASCII_FONT.size());
    const int glyphCount = asciiCount + static_cast<int>(ACCENTED_GLYPHS.size());
    const int rows = (glyphCount + COLUMNS - 1) / COLUMNS;

    ProceduralFont result;
    result.image.width = COLUMNS * CELL_WIDTH;
    result.image.height = rows * CELL_HEIGHT;
    result.image.pixels.assign(static_cast<std::size_t>(result.image.width) *
                                   static_cast<std::size_t>(result.image.height),
                               pack(0, 0, 0, 0));
    result.metrics.lineHeight = CELL_HEIGHT;
    result.metrics.replacementCodePoint = U'?';

    auto& pixels = result.image.pixels;
    const int textureWidth = result.image.width;

    const auto registerGlyph = [&](char32_t codePoint, int cellIndex) {
        GlyphMetrics metrics;
        metrics.x = (cellIndex % COLUMNS) * CELL_WIDTH;
        metrics.y = (cellIndex / COLUMNS) * CELL_HEIGHT;
        metrics.width = CELL_WIDTH;
        metrics.height = CELL_HEIGHT;
        metrics.advance = CELL_WIDTH;  // chasse fixe
        result.metrics.glyphs[codePoint] = metrics;
    };

    for (int glyph = 0; glyph < asciiCount; ++glyph) {
        blitBody(pixels, textureWidth, glyph, ASCII_FONT[static_cast<std::size_t>(glyph)]);
        registerGlyph(ASCII_FIRST + static_cast<char32_t>(glyph), glyph);
    }

    for (int accented = 0; std::cmp_less(accented, ACCENTED_GLYPHS.size()); ++accented) {
        const AccentedGlyph& entry = ACCENTED_GLYPHS[static_cast<std::size_t>(accented)];
        const int cellIndex = asciiCount + accented;
        blitBody(pixels, textureWidth, cellIndex,
                 ASCII_FONT[static_cast<std::size_t>(entry.base - ASCII_FIRST)]);
        if (entry.accent == Accent::CEDILLA) {
            blitCedilla(pixels, textureWidth, cellIndex);
        } else {
            blitTopAccent(pixels, textureWidth, cellIndex, topAccentPattern(entry.accent));
        }
        registerGlyph(entry.codePoint, cellIndex);
    }

    // Alias : espace insecable et guillemets typographiques rediriges vers un glyphe couvert,
    // plutot qu'un trou silencieux (EX-NFR-040).
    result.metrics.glyphs[0x00A0] = result.metrics.glyphs[U' '];
    result.metrics.glyphs[0x2018] = result.metrics.glyphs[U'\''];
    result.metrics.glyphs[0x2019] = result.metrics.glyphs[U'\''];
    result.metrics.glyphs[0x2013] = result.metrics.glyphs[U'-'];
    result.metrics.glyphs[0x2014] = result.metrics.glyphs[U'-'];

    return result;
}

namespace {

// Noms des champs du format de metriques (voir HMI/Graphics/README.md), nommes plutot que
// repetes en litteraux -- meme discipline que hmi::AnimationCatalog.
constexpr const char* FIELD_VERSION = "version";
constexpr const char* FIELD_LINE_HEIGHT = "lineHeight";
constexpr const char* FIELD_REPLACEMENT = "replacement";
constexpr const char* FIELD_GLYPHS = "glyphs";
constexpr const char* FIELD_CHAR = "char";
constexpr const char* FIELD_X = "x";
constexpr const char* FIELD_Y = "y";
constexpr const char* FIELD_WIDTH = "width";
constexpr const char* FIELD_HEIGHT = "height";
constexpr const char* FIELD_ADVANCE = "advance";

[[nodiscard]] FontMetricsResult metricsFailure(std::string message, FontMetricsError code) {
    return FontMetricsResult{
        .metrics = std::nullopt, .error = std::move(message), .errorCode = code};
}

// Decode une chaine JSON attendue comme UN SEUL point de code UTF-8 (champ "char"/"replacement").
[[nodiscard]] std::optional<char32_t> singleCodePoint(const std::string& utf8) {
    if (utf8.empty()) {
        return std::nullopt;
    }
    std::size_t index = 0;
    const char32_t codePoint = nextUtf8CodePoint(utf8, index);
    if (index != utf8.size()) {
        return std::nullopt;  // plus d'un point de code : champ invalide
    }
    return codePoint;
}

// Lit la position et les dimensions d'un glyphe (champ « char » deja valide). Rend l'echec, ou
// std::nullopt apres avoir rempli @p out.
[[nodiscard]] std::optional<FontMetricsResult> parseGlyphGeometry(const nlohmann::json& glyphJson,
                                                                  GlyphMetrics& out) {
    const bool hasIntegerFields =
        glyphJson.contains(FIELD_X) && glyphJson[FIELD_X].is_number_integer() &&
        glyphJson.contains(FIELD_Y) && glyphJson[FIELD_Y].is_number_integer() &&
        glyphJson.contains(FIELD_WIDTH) && glyphJson[FIELD_WIDTH].is_number_integer() &&
        glyphJson.contains(FIELD_HEIGHT) && glyphJson[FIELD_HEIGHT].is_number_integer() &&
        glyphJson.contains(FIELD_ADVANCE) && glyphJson[FIELD_ADVANCE].is_number_integer();
    if (!hasIntegerFields) {
        return metricsFailure(
            "Le glyphe « " + glyphJson[FIELD_CHAR].get<std::string>() +
                " » n'a pas tous les champs « x/y/width/height/advance » exploitables "
                "(entiers).",
            FontMetricsError::MalformedStructure);
    }

    GlyphMetrics glyph;
    glyph.x = glyphJson[FIELD_X].get<int>();
    glyph.y = glyphJson[FIELD_Y].get<int>();
    glyph.width = glyphJson[FIELD_WIDTH].get<int>();
    glyph.height = glyphJson[FIELD_HEIGHT].get<int>();
    glyph.advance = glyphJson[FIELD_ADVANCE].get<int>();
    if (glyph.x < 0 || glyph.y < 0 || glyph.width <= 0 || glyph.height <= 0 || glyph.advance < 0) {
        return metricsFailure("Le glyphe « " + glyphJson[FIELD_CHAR].get<std::string>() +
                                  " » a des dimensions ou une position invalides.",
                              FontMetricsError::MalformedStructure);
    }
    out = glyph;
    return std::nullopt;
}

}  // namespace

// Lit des metriques de police depuis une chaine JSON.
FontMetricsResult loadFontMetricsFromString(std::string_view json) {
    // accept() puis parse(..., allow_exceptions=false) : aucune exception ne doit franchir cette
    // frontiere (EX-NFR-040), meme patron que hmi::AnimationCatalog::loadFromString.
    if (!nlohmann::json::accept(json)) {
        return metricsFailure("JSON malforme.", FontMetricsError::ParseError);
    }
    const nlohmann::json root = nlohmann::json::parse(json, nullptr, false);
    if (!root.is_object()) {
        return metricsFailure("La racine du document n'est pas un objet.",
                              FontMetricsError::ParseError);
    }

    int version = FONT_METRICS_FORMAT_VERSION;
    if (root.contains(FIELD_VERSION)) {
        if (!root[FIELD_VERSION].is_number_integer()) {
            return metricsFailure("Le champ « version » n'est pas un entier.",
                                  FontMetricsError::MalformedStructure);
        }
        version = root[FIELD_VERSION].get<int>();
    }
    if (version > FONT_METRICS_FORMAT_VERSION) {
        return metricsFailure("Version de format " + std::to_string(version) +
                                  " non geree (cette version du jeu lit jusqu'a " +
                                  std::to_string(FONT_METRICS_FORMAT_VERSION) + ").",
                              FontMetricsError::UnsupportedVersion);
    }

    if (!root.contains(FIELD_LINE_HEIGHT) || !root[FIELD_LINE_HEIGHT].is_number_integer() ||
        root[FIELD_LINE_HEIGHT].get<int>() <= 0) {
        return metricsFailure("Le champ « lineHeight » est absent ou n'est pas un entier positif.",
                              FontMetricsError::MalformedStructure);
    }

    FontMetrics metrics;
    metrics.lineHeight = root[FIELD_LINE_HEIGHT].get<int>();
    metrics.replacementCodePoint = U'?';
    if (root.contains(FIELD_REPLACEMENT)) {
        if (!root[FIELD_REPLACEMENT].is_string()) {
            return metricsFailure("Le champ « replacement » n'est pas une chaine.",
                                  FontMetricsError::MalformedStructure);
        }
        const std::optional<char32_t> replacement =
            singleCodePoint(root[FIELD_REPLACEMENT].get<std::string>());
        if (!replacement) {
            return metricsFailure("Le champ « replacement » doit contenir exactement un caractere.",
                                  FontMetricsError::MalformedStructure);
        }
        metrics.replacementCodePoint = *replacement;
    }

    if (!root.contains(FIELD_GLYPHS) || !root[FIELD_GLYPHS].is_array() ||
        root[FIELD_GLYPHS].empty()) {
        return metricsFailure("Le champ « glyphs » est absent, n'est pas un tableau, ou est vide.",
                              FontMetricsError::MalformedStructure);
    }

    for (const nlohmann::json& glyphJson : root[FIELD_GLYPHS]) {
        if (!glyphJson.is_object() || !glyphJson.contains(FIELD_CHAR) ||
            !glyphJson[FIELD_CHAR].is_string()) {
            return metricsFailure("Un glyphe n'a pas de champ « char » exploitable.",
                                  FontMetricsError::MalformedStructure);
        }
        const std::optional<char32_t> codePoint =
            singleCodePoint(glyphJson[FIELD_CHAR].get<std::string>());
        if (!codePoint) {
            return metricsFailure(
                "Le champ « char » d'un glyphe doit contenir exactement un caractere.",
                FontMetricsError::MalformedStructure);
        }

        GlyphMetrics glyph;
        if (std::optional<FontMetricsResult> failure = parseGlyphGeometry(glyphJson, glyph)) {
            return std::move(*failure);
        }
        metrics.glyphs[*codePoint] = glyph;
    }

    return FontMetricsResult{
        .metrics = std::move(metrics), .error = {}, .errorCode = FontMetricsError::None};
}

// Lit des metriques de police depuis un fichier.
FontMetricsResult loadFontMetricsFromFile(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) {
        return metricsFailure("Fichier introuvable ou illisible : " + path.string(),
                              FontMetricsError::FileNotFound);
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    return loadFontMetricsFromString(contents.str());
}

// Valide la coherence entre des metriques et les dimensions decodees du PNG associe.
AssetValidation validateFontMetricsAgainstTexture(const FontMetrics& metrics,
                                                  const std::string& fileName, int textureWidth,
                                                  int textureHeight) {
    if (metrics.lineHeight <= 0) {
        return AssetValidation{
            .valid = false,
            .message = "Police " + fileName + " refusee : hauteur de ligne non positive."};
    }
    for (const auto& [codePoint, glyph] : metrics.glyphs) {
        const bool withinBounds = glyph.x >= 0 && glyph.y >= 0 && glyph.width > 0 &&
                                  glyph.height > 0 && glyph.x + glyph.width <= textureWidth &&
                                  glyph.y + glyph.height <= textureHeight;
        if (!withinBounds) {
            std::ostringstream hex;
            hex << std::hex << static_cast<std::uint32_t>(codePoint);
            return AssetValidation{.valid = false,
                                   .message = "Police " + fileName + " refusee : le glyphe U+" +
                                              hex.str() +
                                              " reference une region hors des bornes de l'image (" +
                                              std::to_string(textureWidth) + "x" +
                                              std::to_string(textureHeight) + " px)."};
        }
    }
    return AssetValidation{.valid = true, .message = std::string{}};
}

}  // namespace hmi
