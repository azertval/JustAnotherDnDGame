// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/CharacterSheetPage.h"

#include <QFile>
#include <QFontMetrics>
#include <QImage>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <algorithm>
#include <fstream>
#include <sstream>

#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Localization/Localization.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace hmi {
namespace {

[[nodiscard]] QColor toQColor(DesignColor color) {
    return QColor(color.r, color.g, color.b, color.a);
}

/// Rend au trait la densite que la reduction lui a prise.
///
/// La planche est gravee a 2448 pixels de large et affichee autour de 650 : un filet d'un pixel
/// de large y devient un pixel a 25 % d'opacite, et la feuille entiere vire au beige delave. Ce
/// n'est pas une question de couleur -- l'encre est la bonne -- c'est la moyenne du filtrage qui
/// dilue la couverture.
///
/// La courbe ci-dessous redonne du corps aux valeurs FAIBLES sans toucher aux pleins : `1-(1-a)^3`
/// remonte 25 % a 58 %, laisse 100 % a 100 %, et ne cree pas de marche visible. Un simple facteur
/// aurait sature les pleins avant de rattraper les filets.
void renforcerEncre(QImage& image) {
    for (int ligne = 0; ligne < image.height(); ++ligne) {
        auto* const pixels = reinterpret_cast<QRgb*>(image.scanLine(ligne));
        for (int colonne = 0; colonne < image.width(); ++colonne) {
            const int alpha = qAlpha(pixels[colonne]);
            if (alpha == 0 || alpha == 255) {
                continue;
            }
            const double reste = 1.0 - (alpha / 255.0);
            const int renforce = static_cast<int>((1.0 - (reste * reste * reste)) * 255.0 + 0.5);
            // L'image est premultipliee : les composantes suivent l'alpha, sans quoi le trait
            // vire au noir pur au lieu de s'epaissir.
            const double facteur = alpha == 0 ? 0.0 : static_cast<double>(renforce) / alpha;
            const auto borne = [](double valeur) {
                return static_cast<int>(std::clamp(valeur, 0.0, 255.0));
            };
            pixels[colonne] = qRgba(borne(qRed(pixels[colonne]) * facteur),
                                    borne(qGreen(pixels[colonne]) * facteur),
                                    borne(qBlue(pixels[colonne]) * facteur), renforce);
        }
    }
}

/// @return Le contenu d'un fichier d'`Assets/UI/`, ou une chaîne vide s'il manque. Un asset absent
///         est un cas ATTENDU (`EX-NFR-040`) : la planche s'affiche alors nue, ce qui se voit,
///         plutôt que de faire tomber l'application.
[[nodiscard]] std::string readAsset(const char* file) {
    const std::filesystem::path chemin = executableDirectory() / "Assets" / "UI" / file;
    std::ifstream flux(chemin, std::ios::binary);
    if (!flux) {
        return {};
    }
    std::ostringstream tampon;
    tampon << flux.rdbuf();
    return tampon.str();
}

/// Le rôle, tel que la feuille de style le nomme en propriété dynamique. C'est le SEUL endroit où
/// un rôle de planche devient un mot de feuille de style, et il est exhaustif par construction :
/// un rôle ajouté à l'énumération ne compile plus tant qu'il n'est pas nommé ici.
[[nodiscard]] const char* styleRole(PlateRole role, bool isValue) {
    switch (role) {
        case PlateRole::Name:
            return isValue ? "plateName" : "plateCaption";
        case PlateRole::Band:
            return isValue ? "plateValue" : "plateCaption";
        case PlateRole::Dial:
            return isValue ? "plateDial" : "plateCaption";
        case PlateRole::Plaque:
            return isValue ? "plateValue" : "plateCaption";
        case PlateRole::Ability:
            return isValue ? "plateAbility" : "plateEngraved";
        case PlateRole::Score:
            return "plateScore";
        case PlateRole::Skill:
            return isValue ? "plateValue" : "plateSkill";
        case PlateRole::Save:
            return isValue ? "plateValue" : "plateEngraved";
        case PlateRole::Banner:
            return "plateBanner";
        case PlateRole::Prose:
            return isValue ? "plateProse" : "plateEngraved";
        case PlateRole::Table:
            return isValue ? "plateValue" : "plateSkill";
        case PlateRole::Portrait:
            return "plateValue";
    }
    return "plateValue";
}

/// La face de la charte que ce role emploie : le TITRAGE la ou il y a la place de le porter, le
/// CORPS partout ailleurs. Les deux viennent des jetons d'identite -- c'est la meme charte que la
/// feuille de style, appliquee ici parce que la TAILLE, elle, ne peut venir que du code.
[[nodiscard]] QFont policeDuRole(PlateRole role) {
    const bool titrage = role == PlateRole::Name || role == PlateRole::Banner ||
                         role == PlateRole::Dial || role == PlateRole::Ability;
    const std::string famille =
        titrage ? resolvedIdentityTitleFamily() : resolvedFontFamily(FontRole::Identity);
    QFont police(
        QString::fromStdString(famille.empty() ? resolvedFontFamily(FontRole::Identity) : famille));
    // La graisse suit le role, comme la feuille de style la posait : pleine pour ce qui est
    // grave dans un cartouche, normale pour le texte courant du livre.
    police.setBold(role != PlateRole::Skill && role != PlateRole::Prose &&
                   role != PlateRole::Table);
    return police;
}

/// Regle le corps du texte pour qu'il tienne dans son cartouche.
///
/// Le corps part de la HAUTEUR du rectangle grave : c'est elle qui donne a la feuille son rythme,
/// et un intitule doit garder la meme taille apparente quelle que soit celle de la fenetre. Mais
/// la hauteur seule ne suffit pas -- << Bonus de maitrise >> est trois fois plus long que
/// << Force >> pour le meme cartouche, et depassait de son cadre pour se lire par-dessus le trait
/// du livre. Le corps est donc reduit jusqu'a ce que le texte tienne en LARGEUR.
///
/// Reduit, et non elide : sur une feuille de personnage, un intitule coupe (<< Bonus de mait... >>)
/// est illisible, alors qu'un intitule un point plus petit se lit encore tres bien.
void ajusterTexte(QLabel& etiquette, const QRect& cadre, double hauteurGravee) {
    if (cadre.width() <= 0 || cadre.height() <= 0) {
        return;
    }
    QFont police = etiquette.font();
    const int depart = std::clamp(static_cast<int>(hauteurGravee * 0.66), 6, 96);
    police.setPixelSize(depart);
    const QString texte = etiquette.text();
    if (!texte.isEmpty()) {
        for (int corps = depart; corps > 5; --corps) {
            police.setPixelSize(corps);
            const QFontMetrics mesures(police);
            // Replie : c'est la HAUTEUR du bloc rendu qui doit tenir, la largeur etant imposee.
            bool tient = false;
            if (etiquette.wordWrap()) {
                // La hauteur du bloc replie, ET la largeur du PLUS LONG MOT.
                //
                // `boundingRect` sous contrainte de largeur rabote sa reponse a cette largeur :
                // un mot indivisible plus large que le cadre s'y declare donc a l'aise, et
                // << Competences >> debordait sur le bandeau voisin sans que rien ne le signale.
                // Un mot ne se coupant pas, c'est lui qui fixe la largeur minimale.
                const QRect rendu = mesures.boundingRect(QRect(0, 0, cadre.width(), 0),
                                                         Qt::TextWordWrap | Qt::AlignCenter, texte);
                int plusLong = 0;
                for (const QString& mot : texte.split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
                    plusLong = std::max(plusLong, mesures.horizontalAdvance(mot));
                }
                tient = plusLong <= cadre.width() && rendu.height() <= cadre.height();
            } else {
                tient = mesures.horizontalAdvance(texte) <= cadre.width();
            }
            if (tient) {
                break;
            }
        }
    }
    etiquette.setFont(police);
}

}  // namespace

CharacterSheetPage::CharacterSheetPage(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("CharacterSheetPage"));
    setAttribute(Qt::WA_StyledBackground, true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    _layout = parseCharacterSheetPlate(readAsset(PLATE_TABLE_FILE));

    // Le MASQUE D'ENCRE de la planche : un PNG monochrome a alpha, rendu a trois fois la taille
    // de la maquette. Absent, c'est un cas ATTENDU (EX-NFR-040) : la fiche s'affiche sur son
    // parchemin nu, sans son trait -- ce qui se voit, plutot que de faire tomber l'application.
    static_cast<void>(_plate.load(
        QString::fromStdString((executableDirectory() / "Assets" / "UI" / PLATE_FILE).string())));

    // Le portrait du centre : un JETON de table virtuelle, deja rond et detoure (EX-IHM-076).
    static_cast<void>(_portrait.load(QString::fromStdString(
        (executableDirectory() / "Assets" / "UI" / PORTRAIT_FILE).string())));

    buildFields();
}

CharacterSheetPage::~CharacterSheetPage() = default;

void CharacterSheetPage::buildFields() {
    for (const PlateField& field : _layout.fields) {
        if (field.role == PlateRole::Portrait) {
            // Le portrait n'est pas une etiquette de texte : il se pose dans l'anneau vide que la
            // roue gravee laisse en son centre.
            _portraitLabel = new QLabel(this);
            _portraitLabel->setObjectName(QStringLiteral("platePortrait"));
            _portraitLabel->setAlignment(Qt::AlignCenter);
            _portraitLabel->setScaledContents(true);
            _portraitLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
            _portraitLabel->setVisible(!_portrait.isNull());
            continue;
        }
        const auto ajoute = [this, &field](const PlateRect& rect, bool isValue) {
            if (rect.isEmpty()) {
                return;
            }
            auto* const etiquette = new QLabel(this);
            etiquette->setProperty("rpgRole", QString::fromLatin1(styleRole(field.role, isValue)));
            etiquette->setAlignment(
                (field.align == PlateAlign::Left ? Qt::AlignLeft : Qt::AlignHCenter) |
                Qt::AlignVCenter);
            etiquette->setAttribute(Qt::WA_TransparentForMouseEvents, true);
            // Le texte ne DEBORDE jamais son rectangle : la planche est gravee, et un intitule qui
            // depasserait son cartouche se lirait par-dessus le trait du livre.
            etiquette->setTextInteractionFlags(Qt::NoTextInteraction);
            // Un bandeau de section porte deux mots (<< Jets de sauvegarde >>) la ou la gravure
            // en portait un : il se replie sur deux lignes plutot que de retrecir jusqu'a
            // l'illisible.
            etiquette->setWordWrap(field.role == PlateRole::Banner);
            etiquette->setText(isValue ? _emptyMark : QString{});
            _placed.push_back({.widget = etiquette, .field = &field, .isValue = isValue});
        };
        if (!field.labelKey.empty()) {
            if (field.role == PlateRole::Save) {
                // Les onglets de jets de sauvegarde portent leur abreviation TOURNEE d'un quart de
                // tour, comme la gravure la portait : l'onglet fait treize unites de large pour
                // vingt-six de haut, et rien d'horizontal n'y tient. Une etiquette Qt ne tourne
                // pas ; ces trois lettres sont donc peintes, et c'est le seul texte de la planche
                // qui le soit.
                _rotated.push_back(&field);
            } else {
                ajoute(field.label, false);
            }
        }
        ajoute(field.value, true);
    }
}

QSize CharacterSheetPage::sizeHint() const {
    const int echelle = std::max(1, identityScale());
    return {static_cast<int>(_layout.pageWidth) * echelle / 2,
            static_cast<int>(_layout.pageHeight) * echelle / 2};
}

QSize CharacterSheetPage::minimumSizeHint() const {
    // Au-dessous de cette largeur, le lettrage de la planche cesse d'etre lisible : les dix-huit
    // lignes de competences font moins de sept pixels de haut.
    return {static_cast<int>(_layout.pageWidth) / 2, static_cast<int>(_layout.pageHeight) / 2};
}

bool CharacterSheetPage::hasHeightForWidth() const {
    return true;
}

int CharacterSheetPage::heightForWidth(int width) const {
    return static_cast<int>(static_cast<double>(width) * _layout.pageHeight / _layout.pageWidth);
}

QRectF CharacterSheetPage::plateRect() const {
    const double disponibleX = width();
    const double disponibleY = height();
    if (disponibleX <= 0.0 || disponibleY <= 0.0 || !_layout.isValid()) {
        return {};
    }
    // Le plus petit des deux rapports : la planche entre entierement, et garde ses proportions.
    const double facteur =
        std::min(disponibleX / _layout.pageWidth, disponibleY / _layout.pageHeight);
    const double largeur = _layout.pageWidth * facteur;
    const double hauteur = _layout.pageHeight * facteur;
    return {(disponibleX - largeur) / 2.0, (disponibleY - hauteur) / 2.0, largeur, hauteur};
}

QRectF CharacterSheetPage::toWidget(const PlateRect& rect) const {
    const QRectF planche = plateRect();
    if (planche.isEmpty()) {
        return {};
    }
    const double facteur = planche.width() / _layout.pageWidth;
    return {planche.x() + rect.x * facteur, planche.y() + rect.y * facteur, rect.width * facteur,
            rect.height * facteur};
}

void CharacterSheetPage::layoutPlate() {
    const QRectF planche = plateRect();
    if (planche.isEmpty()) {
        return;
    }
    const double facteur = planche.width() / _layout.pageWidth;

    for (const Placed& place : _placed) {
        const PlateRect& source = place.isValue ? place.field->value : place.field->label;
        const QRect cadre = toWidget(source).toRect();
        place.widget->setGeometry(cadre);
        ajusterTexte(*place.widget, cadre, source.height * facteur);
    }

    if (_portraitLabel != nullptr) {
        for (const PlateField& field : _layout.fields) {
            if (field.role == PlateRole::Portrait) {
                const QRect cadre = toWidget(field.value).toRect();
                _portraitLabel->setGeometry(cadre);
                if (!_portrait.isNull() && cadre.width() > 0) {
                    _portraitLabel->setPixmap(_portrait.scaled(cadre.size(), Qt::KeepAspectRatio,
                                                               Qt::SmoothTransformation));
                }
                break;
            }
        }
    }
}

void CharacterSheetPage::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    _plateCache = QPixmap();  // la gravure sera retracee a la nouvelle taille, une seule fois
    layoutPlate();
}

void CharacterSheetPage::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);
    const QRectF planche = plateRect();
    if (planche.isEmpty()) {
        return;
    }
    const ColorTokens& couleur = identityTokens().color;
    QPainter peintre(this);

    // Le champ de la feuille : le parchemin sur lequel la gravure est imprimee.
    peintre.fillRect(planche, toQColor(couleur.surface));

    if (!_plate.isNull()) {
        if (_plateCache.size() != planche.size().toSize()) {
            // La gravure est un MASQUE d'encre : un seul chemin noir, sans couleur propre. Elle est
            // rendue une fois par taille, puis teintee au jeton d'encre -- c'est ce qui la fait
            // suivre le theme au lieu d'imposer le noir du fichier.
            //
            // Le rendu passe par une QImage au format EXPLICITE, et non par un QPixmap : le format
            // d'un QPixmap depend de la plateforme, et un pixmap sans canal alpha rend la teinture
            // par `CompositionMode_SourceIn` inoperante -- la planche s'affichait alors
            // entierement vide, ce qui ne ressemblait a aucune erreur.
            QImage encre(planche.size().toSize(), QImage::Format_ARGB32_Premultiplied);
            encre.fill(Qt::transparent);
            {
                QPainter gravure(&encre);
                gravure.setRenderHint(QPainter::SmoothPixmapTransform, true);
                gravure.drawPixmap(QRectF(QPointF(0, 0), planche.size()), _plate,
                                   QRectF(_plate.rect()));
                gravure.setCompositionMode(QPainter::CompositionMode_SourceIn);
                gravure.fillRect(encre.rect(), toQColor(couleur.frameEdge));
            }
            renforcerEncre(encre);
            { QPainter gravure(&encre); }
            _plateCache = QPixmap::fromImage(encre);
        }
        peintre.drawPixmap(planche.topLeft(), _plateCache);
    }

    // Les abreviations des onglets de sauvegarde, tournees d'un quart de tour.
    if (!_rotated.empty()) {
        const int echelle = std::max(1, static_cast<int>(planche.width() / _layout.pageWidth));
        static_cast<void>(echelle);
        QFont police = policeDuRole(PlateRole::Save);
        peintre.setPen(toQColor(couleur.text));
        for (const PlateField* const champ : _rotated) {
            const QRectF cadre = toWidget(champ->label);
            if (cadre.isEmpty()) {
                continue;
            }
            const QString texte = _rotatedText.value(QString::fromStdString(champ->id));
            if (texte.isEmpty()) {
                continue;
            }
            // Le corps se regle sur la LARGEUR de l'onglet, qui devient la hauteur des lettres
            // une fois tourne.
            int corps = std::max(5, static_cast<int>(cadre.width() * 0.74));
            for (; corps > 5; --corps) {
                police.setPixelSize(corps);
                if (QFontMetrics(police).horizontalAdvance(texte) <= cadre.height()) {
                    break;
                }
            }
            police.setPixelSize(corps);
            peintre.save();
            peintre.translate(cadre.center());
            peintre.rotate(-90.0);
            peintre.setFont(police);
            peintre.drawText(
                QRectF(-cadre.height() / 2.0, -cadre.width() / 2.0, cadre.height(), cadre.width()),
                Qt::AlignCenter, texte);
            peintre.restore();
        }
    }

    // Les quelques cartouches dont le lettrage etait EVIDE dans son propre fond : la decoupe
    // vectorielle les aurait bouches au lieu de les nettoyer, leur fond plat est donc simplement
    // repeint ici, sous l'intitule traduit. Invisible, puisqu'il est plat.
    for (const PlateField& field : _layout.fields) {
        for (const PlateRect& rectangle : field.cover) {
            peintre.fillRect(toWidget(rectangle), toQColor(couleur.surface));
        }
    }
}

void CharacterSheetPage::retranslateUi(const Localization& loc) {
    for (const Placed& place : _placed) {
        if (!place.isValue && !place.field->labelKey.empty()) {
            place.widget->setText(QString::fromStdString(loc.text(place.field->labelKey)));
        }
    }
    layoutPlate();  // le corps depend du TEXTE : il se recalcule quand la langue change
    for (const PlateField* const champ : _rotated) {
        _rotatedText.insert(QString::fromStdString(champ->id),
                            QString::fromStdString(loc.text(champ->labelKey)));
    }
    update();
    // Les valeurs sont reposees : un changement de langue ne doit pas les effacer.
    applyValues(_values, _emptyMark.toStdString());
}

void CharacterSheetPage::applyValues(const std::map<std::string, std::string>& values,
                                     const std::string& emptyMark) {
    _values = values;
    _emptyMark = QString::fromStdString(emptyMark);
    for (const Placed& place : _placed) {
        if (!place.isValue) {
            continue;
        }
        // Identifiant vide : ce champ existe sur la planche et rien ne l'alimente encore. Il garde
        // son tiret, qui dit « on ne sait pas » la ou un zero affirmerait « rien ».
        const auto trouve =
            place.field->valueId.empty() ? values.end() : values.find(place.field->valueId);
        place.widget->setText(trouve == values.end() ? _emptyMark
                                                     : QString::fromStdString(trouve->second));
    }
    layoutPlate();  // une valeur plus longue que la precedente doit se reduire pour tenir
}

}  // namespace hmi
