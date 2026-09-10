// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Presentation/ParchmentOrnaments.h"

#include <algorithm>

namespace hmi {

namespace {

/// Envergure d'une aile, en fraction de la HAUTEUR du bandeau. Relevee sur la planche : l'aile y
/// couvre 50 points de large pour une plaque haute de 31, soit une fois et demie. En dessous, elle
/// se lit comme une touffe accrochee au bout de la plaque plutot que comme une aile deployee.
constexpr float WING_SPAN_RATIO = 1.6f;

/// Largeur minimale laissee a la plaque, en fraction de la largeur totale. En dessous, les ailes
/// cedent : voir l'en-tete.
constexpr float PLATE_MINIMUM_SHARE = 0.34f;

/// Profondeur du chevron d'un bout de plaque, en fraction de la hauteur.
constexpr float CHEVRON_RATIO = 0.30f;

/// Retrait de la face eclairee de la plaque, en fraction de la hauteur.
constexpr float PLATE_FACE_INSET = 0.16f;

[[nodiscard]] OrnamentPoint point(float x, float y) {
    return OrnamentPoint{x, y};
}

/// Une aile gauche, tracee dans un rectangle @p span x @p height dont l'emplanture est a DROITE.
///
/// Deux bords, et ils ne se ressemblent pas : le bord d'ATTAQUE file haut et presque droit vers la
/// pointe exterieure, le bord de FUITE redescend en trois doigts profondement echancres. C'est
/// cette dissymetrie qui fait lire une aile ; deux bords echancres donnent une feuille, deux bords
/// lisses donnent une bannerole.
///
/// Les echancrures montent a un sixieme de la hauteur. Moins profondes, elles se referment des que
/// le bandeau descend sous la quarantaine de pixels et l'aile redevient un pave dore -- ce qui ne
/// se voit qu'a la petite taille, c'est-a-dire jamais pendant qu'on la regle.
[[nodiscard]] std::vector<OrnamentPoint> wingOutline(float span, float height) {
    return {
        point(span, height * 0.10f),          // emplanture haute, derriere la plaque
        point(span * 0.72f, height * 0.05f),  // bord d'attaque : haut, et presque droit
        point(span * 0.28f, height * 0.13f),
        point(span * 0.02f, height * 0.33f),  // pointe exterieure
        point(span * 0.21f, height * 0.47f),  // bord de fuite : trois doigts
        point(span * 0.06f, height * 0.63f),
        point(span * 0.35f, height * 0.62f),
        point(span * 0.23f, height * 0.83f),
        point(span * 0.57f, height * 0.76f),
        point(span * 0.50f, height * 0.97f),
        point(span, height * 0.88f),  // emplanture basse
    };
}

/// Miroir d'une aile gauche autour de l'axe vertical d'un bandeau de @p width de large.
[[nodiscard]] std::vector<OrnamentPoint> mirrored(const std::vector<OrnamentPoint>& points,
                                                  float width) {
    std::vector<OrnamentPoint> flipped;
    flipped.reserve(points.size());
    for (const OrnamentPoint& p : points) {
        flipped.push_back(point(width - p.x, p.y));
    }
    return flipped;
}

/// Hexagone allonge a bouts en chevron, entre @p left et @p right, retrait de @p inset.
[[nodiscard]] std::vector<OrnamentPoint> plateOutline(float left, float right, float height,
                                                      float chevron, float inset) {
    const float top = inset;
    const float bottom = height - inset;
    const float middle = height * 0.5f;
    return {
        point(left + inset, middle),  point(left + chevron, top),     point(right - chevron, top),
        point(right - inset, middle), point(right - chevron, bottom), point(left + chevron, bottom),
    };
}

}  // namespace

int bannerWingSpan(int height) {
    return height <= 0 ? 0 : static_cast<int>(static_cast<float>(height) * WING_SPAN_RATIO);
}

std::vector<OrnamentShape> cabochonShapes(int size) {
    if (size < ORNAMENT_MINIMUM_SIZE) {
        return {};
    }
    const float s = static_cast<float>(size);

    // Le serti : un octogone plutot qu'un carre. C'est ce qui distingue une pierre SERTIE d'un
    // aplat dore -- les angles coupes sont les griffes qui la tiennent, et ils se lisent meme a la
    // plus petite taille, quand les facettes internes ne se lisent plus.
    const float cut = s * 0.28f;
    const std::vector<OrnamentPoint> bezel = {
        point(cut, 0.0f),  point(s - cut, 0.0f), point(s, cut),        point(s, s - cut),
        point(s - cut, s), point(cut, s),        point(0.0f, s - cut), point(0.0f, cut),
    };

    // La gemme : un losange inscrit. Deux facettes, et deux seulement -- une claire au-dessus de
    // la diagonale, une profonde en dessous. Trois facettes ou plus ne se distinguent plus des
    // que le cabochon descend sous la vingtaine de pixels, et un degrade ne serait plus un role.
    const float inset = s * 0.24f;
    const float half = s * 0.5f;
    const OrnamentPoint north = point(half, inset);
    const OrnamentPoint east = point(s - inset, half);
    const OrnamentPoint south = point(half, s - inset);
    const OrnamentPoint west = point(inset, half);

    return {
        {bezel, OrnamentRole::Ornament, true},
        {bezel, OrnamentRole::Edge, false},
        {{north, east, south, west}, OrnamentRole::GemShadow, true},
        // La facette claire couvre la moitie NORD-OUEST : la lumiere de cette charte vient d'en
        // haut a gauche pour les gemmes, et d'elle seule -- l'encadrement, lui, est plat (LOT-66).
        {{north, east, west}, OrnamentRole::Gem, true},
        // L'eclat, enfin : un petit losange contre le sommet nord. Pose en DERNIER, il recouvre la
        // facette claire, ce qui est exactement ce qu'un reflet fait.
        {{north, point(half + (s * 0.10f), inset + (s * 0.10f)), point(half, inset + (s * 0.18f)),
          point(half - (s * 0.10f), inset + (s * 0.10f))},
         OrnamentRole::OrnamentLight,
         true},
    };
}

std::vector<OrnamentShape> titleBannerShapes(int width, int height) {
    if (width <= 0 || height < ORNAMENT_MINIMUM_SIZE) {
        return {};
    }
    const float w = static_cast<float>(width);
    const float h = static_cast<float>(height);

    // L'envergure suit la HAUTEUR, jamais la largeur : c'est tout l'enjeu du trace. Elle cede
    // ensuite, et seule, si la plaque n'a plus sa part minimale -- reduire la plaque a la place
    // laisserait deux ailes autour de rien.
    const float wanted = static_cast<float>(bannerWingSpan(height));
    const float available = (w * (1.0f - PLATE_MINIMUM_SHARE)) * 0.5f;
    const float span = std::max(0.0f, std::min(wanted, available));

    const float chevron = h * CHEVRON_RATIO;
    const float inset = h * PLATE_FACE_INSET;
    const float left = span;
    const float right = w - span;

    std::vector<OrnamentShape> shapes;
    shapes.reserve(5);

    // Les ailes d'abord : la plaque se pose dessus, et c'est ce recouvrement qui fait lire
    // l'emplanture comme passant DERRIERE la plaque plutot que collee a son bord.
    if (span >= static_cast<float>(ORNAMENT_MINIMUM_SIZE)) {
        const std::vector<OrnamentPoint> wing = wingOutline(span, h);
        shapes.push_back({wing, OrnamentRole::Ornament, true});
        shapes.push_back({mirrored(wing, w), OrnamentRole::Ornament, true});
    }

    shapes.push_back({plateOutline(left, right, h, chevron, 0.0f), OrnamentRole::GemShadow, true});
    shapes.push_back({plateOutline(left, right, h, chevron, inset), OrnamentRole::Gem, true});
    // Le filet d'or, en dernier : il borde la plaque ET recouvre l'emplanture des ailes, ce qui
    // ferme la silhouette. Trace, pas rempli -- un contour rempli serait la plaque elle-meme.
    shapes.push_back({plateOutline(left, right, h, chevron, 0.0f), OrnamentRole::Ornament, false});
    return shapes;
}

}  // namespace hmi
