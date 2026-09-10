// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Presentation/KeyHintText.h"

#include <algorithm>

namespace hmi {

namespace {

/// Echappe ce qui casserait le fragment. Les libelles viennent du catalogue de traduction, pas
/// d'une saisie utilisateur, mais un `&` dans une traduction suffirait a produire une entite
/// tronquee -- et le defaut serait invisible jusqu'a ce qu'un traducteur en ecrive un.
[[nodiscard]] std::string escaped(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (const char c : text) {
        switch (c) {
            case '&':
                out += "&amp;";
                break;
            case '<':
                out += "&lt;";
                break;
            case '>':
                out += "&gt;";
                break;
            default:
                out += c;
                break;
        }
    }
    return out;
}

}  // namespace

std::string keyHintText(const std::vector<KeyHint>& hints, const DesignTokens& tokens, int scale) {
    if (hints.empty()) {
        return {};
    }
    const int factor = std::max(scale, 1);
    const IdentityBaseScale& base = identityBaseScale();
    const std::string padding = std::to_string(std::max(1, base.spaceSmall * factor / 2));
    const std::string gap = std::to_string(base.spaceExtraLarge * factor);
    const std::string capSize = std::to_string(base.caption * factor);

    const std::string capBackground = toCssColor(tokens.color.surfaceAlt);
    const std::string capBorder = toCssColor(tokens.color.frameEdge);
    const std::string capText = toCssColor(tokens.color.text);
    const std::string actionText = toCssColor(tokens.color.textMuted);

    std::string html;
    for (std::size_t i = 0; i < hints.size(); ++i) {
        if (i > 0) {
            // Espacement entre rappels : une marge sur le capuchon, pas des espaces insecables --
            // ceux-ci ne suivraient pas le facteur d'agrandissement.
            html += "<span style=\"margin-left:" + gap + "px;\"> </span>";
        }
        html += "<span style=\"background-color:" + capBackground + "; color:" + capText +
                "; border:1px solid " + capBorder + "; padding:" + padding + "px " + padding +
                "px; font-size:" + capSize + "px;\">" + escaped(hints[i].key) + "</span>";
        html += "<span style=\"color:" + actionText + "; font-size:" + capSize + "px;\"> " +
                escaped(hints[i].action) + "</span>";
    }
    return html;
}

}  // namespace hmi
