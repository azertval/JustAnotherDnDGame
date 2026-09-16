// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/PixelCanvas.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QRectF>
#include <QToolTip>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <cstring>

#include "HMI/Editor/PixelAutotilePreview.h"
#include "HMI/Editor/PixelPalette.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {

// Convertit des pixels RGBA decodes en QImage. Meme conversion que PalettePanel/
// AssetThumbnailView (LOT-42/LOT-43) : trop petite pour justifier une fonction partagee entre
// widgets independants.
[[nodiscard]] QImage toQImage(const DecodedImage& decoded) {
    QImage image(decoded.width, decoded.height, QImage::Format_RGBA8888);
    for (int y = 0; y < decoded.height; ++y) {
        const std::uint32_t* const row =
            decoded.pixels.data() + (static_cast<std::size_t>(y) * decoded.width);
        std::memcpy(image.scanLine(y), row, static_cast<std::size_t>(decoded.width) * 4);
    }
    return image;
}

[[nodiscard]] QColor toQColor(DesignColor color) noexcept {
    return {color.r, color.g, color.b};
}

// Cote d'une case du damier de transparence, en pixels LOGIQUES (espace ecran, TACHE-03 point
// d'attention) : fixe, independant du zoom de l'oeuvre -- sinon il zoome avec le dessin et devient
// illisible aux forts facteurs.
constexpr int CHECKER_LOGICAL_CELL = 6;

// Correspondance outil -> type d'operation d'historique (LOT-54 TACHE-02/TACHE-06). L'outil
// pipette ne mute jamais l'image (aucune region non vide n'est jamais poussee pour lui) : la
// valeur de repli n'est donc jamais observee pour lui. L'outil Selection ne mute l'image QUE
// lorsqu'il deplace une selection existante -- c'est alors toujours un deplacement de region.
[[nodiscard]] PixelOperationKind operationKindForTool(PixelTool tool) noexcept {
    switch (tool) {
        case PixelTool::Brush:
            return PixelOperationKind::Brush;
        case PixelTool::Eraser:
            return PixelOperationKind::Eraser;
        case PixelTool::Fill:
            return PixelOperationKind::Fill;
        case PixelTool::Eyedropper:
            return PixelOperationKind::Brush;
        case PixelTool::Selection:
            return PixelOperationKind::Move;
    }
    return PixelOperationKind::Brush;
}

}  // namespace

PixelCanvas::PixelCanvas(QWidget* parent) : QWidget(parent) {
    // Suivi de souris SANS bouton presse : necessaire pour que le pixel survole (barre d'etat,
    // TACHE-04) se mette a jour au simple deplacement, pas seulement pendant un geste de dessin.
    setMouseTracking(true);
    // Cible reelle du clavier (raccourcis d'outil) quand l'utilisateur clique dans le canevas --
    // c'est ce changement de focus que MainWindow observe pour reassigner le contexte d'edition
    // actif (TACHE-04, EX-IHM-062).
    setFocusPolicy(Qt::StrongFocus);
    // Croix, jamais la fleche par defaut : la fleche pointe depuis son coin haut-gauche, ce qui la
    // fait paraitre decalee de plusieurs pixels ecran par rapport a la case reellement ciblee des
    // que le zoom grossit chaque pixel image en plusieurs pixels ecran.
    setCursor(Qt::CrossCursor);
}

void PixelCanvas::setImage(DecodedImage image) {
    // Depart d'un document neuf (ouverture/creation, TACHE-05) : tout l'etat propre au document
    // precedent est reinitialise ensemble, pas seulement l'image -- un nom d'asset ou un historique
    // laisse derriere induirait la barre d'etat et l'annulation en erreur.
    _image = std::move(image);
    _assetName.clear();
    _dirty = false;
    _history = PixelHistory();
    _gestureActive = false;
    _lastGesturePixel.reset();
    _selection = PixelRegion{};
    _clipboard = PixelClipboard{};
    _selectionMoveActive = false;
    if (_hoveredPixel) {
        _hoveredPixel.reset();
        emit hoveredPixelChanged(_hoveredPixel);
    }
    updateGeometry();
    update();
    emit imageChanged();
    emit historyChanged();
}

void PixelCanvas::undo() {
    if (_history.undo(_image)) {
        _dirty = true;
        update();
        emit imageChanged();
        emit historyChanged();
    }
}

void PixelCanvas::redo() {
    if (_history.redo(_image)) {
        _dirty = true;
        update();
        emit imageChanged();
        emit historyChanged();
    }
}

void PixelCanvas::jumpHistoryTo(std::size_t index) {
    if (_history.jumpTo(_image, index)) {
        _dirty = true;
        update();
        emit imageChanged();
        emit historyChanged();
    }
}

void PixelCanvas::setCurrentColor(std::uint32_t color) {
    if (_currentColor == color) {
        return;  // evite un signal redondant quand la source n'a rien change (ex. pipette sur la
                 // couleur deja active).
    }
    _currentColor = color;
    emit currentColorChanged(_currentColor);
}

void PixelCanvas::zoomIn() {
    _view = pixelCanvasZoomIn(_view);
    updateGeometry();
    update();
}

void PixelCanvas::zoomOut() {
    _view = pixelCanvasZoomOut(_view);
    updateGeometry();
    update();
}

std::optional<std::pair<int, int>> PixelCanvas::imagePixelAt(const QPointF& widgetPosition) const {
    return screenToImagePixel(_view, _image.width, _image.height, widgetPosition.x(),
                              widgetPosition.y());
}

QSize PixelCanvas::sizeHint() const {
    const double scale = pixelCanvasScale(_view);
    return {std::max(1, static_cast<int>(std::lround(_image.width * scale))),
            std::max(1, static_cast<int>(std::lround(_image.height * scale)))};
}

void PixelCanvas::applyToolAtPoint(int x, int y) {
    PixelRegion touched;
    switch (_activeTool) {
        case PixelTool::Brush:
            touched = setPixel(_image, x, y, effectivePaintColor());
            break;
        case PixelTool::Eraser:
            touched = erasePixel(_image, x, y);
            break;
        case PixelTool::Fill:
            touched = floodFill(_image, x, y, effectivePaintColor());
            break;
        case PixelTool::Eyedropper:
            // Ne mute jamais l'image : prelevement seul, aucune entree d'historique. Passe par
            // setCurrentColor (pas une affectation directe) pour que le temoin de couleur de la
            // barre d'outils se resynchronise, comme les autres sources de changement.
            if (const std::optional<std::uint32_t> picked = pickColor(_image, x, y)) {
                setCurrentColor(*picked);
            }
            return;
        case PixelTool::Selection: {
            // Premier point du geste : decide s'il deplace la selection existante (clic dedans) ou
            // en definit une nouvelle (clic dehors) -- ne mute jamais l'image a ce point precis
            // (un deplacement nul serait un no-op couteux et polluerait l'historique).
            const bool insideSelection = !_selection.empty() && x >= _selection.minX &&
                                         x <= _selection.maxX && y >= _selection.minY &&
                                         y <= _selection.maxY;
            _selectionMoveActive = insideSelection;
            if (!insideSelection) {
                _selectionDragAnchor = std::make_pair(x, y);
                _selection = PixelRegion{.minX = x, .minY = y, .maxX = x, .maxY = y};
                update();
            }
            return;
        }
    }
    _gestureRegion = unionPixelRegion(_gestureRegion, touched);
}

void PixelCanvas::continueToolTo(int x, int y) {
    const auto [lastX, lastY] = _lastGesturePixel.value_or(std::make_pair(x, y));
    PixelRegion touched;
    switch (_activeTool) {
        case PixelTool::Brush:
            touched = drawLine(_image, lastX, lastY, x, y, effectivePaintColor());
            break;
        case PixelTool::Eraser:
            touched = eraseLine(_image, lastX, lastY, x, y);
            break;
        case PixelTool::Fill:
        case PixelTool::Eyedropper:
            // Geste ponctuel : le pot de peinture et la pipette ignorent le glisser.
            return;
        case PixelTool::Selection:
            if (_selectionMoveActive) {
                const int dx = x - lastX;
                const int dy = y - lastY;
                touched = moveRegion(_image, _selection, dx, dy);
                _selection = PixelRegion{.minX = _selection.minX + dx,
                                         .minY = _selection.minY + dy,
                                         .maxX = _selection.maxX + dx,
                                         .maxY = _selection.maxY + dy};
            } else {
                _selection = PixelRegion{.minX = std::min(_selectionDragAnchor.first, x),
                                         .minY = std::min(_selectionDragAnchor.second, y),
                                         .maxX = std::max(_selectionDragAnchor.first, x),
                                         .maxY = std::max(_selectionDragAnchor.second, y)};
                update();
                return;  // definir une selection ne mute jamais l'image : aucune region touchee.
            }
            break;
    }
    _gestureRegion = unionPixelRegion(_gestureRegion, touched);
}

void PixelCanvas::beginGesture(int x, int y) {
    _gestureActive = true;
    _gestureBeforeSnapshot = _image;
    _gestureRegion = PixelRegion{};
    applyToolAtPoint(x, y);
    _lastGesturePixel = std::make_pair(x, y);
    update();
}

void PixelCanvas::endGesture() {
    if (_gestureActive && !_gestureRegion.empty()) {
        // Un geste complet (appui, glisser, relachement) produit UNE seule entree d'historique,
        // jamais une par deplacement de souris (TACHE-02/TACHE-03) : sinon annuler deviendrait
        // inutilisable et l'apercu live (TACHE-08) invaliderait le cache a chaque pixel.
        const std::vector<std::uint32_t> before =
            readRegion(_gestureBeforeSnapshot, _gestureRegion);
        const std::vector<std::uint32_t> after = readRegion(_image, _gestureRegion);
        _history.push(operationKindForTool(_activeTool), _gestureRegion, before, after);
        _dirty = true;
        emit imageChanged();
        emit historyChanged();
    }
    _gestureActive = false;
    _lastGesturePixel.reset();
}

std::uint32_t PixelCanvas::effectivePaintColor() const noexcept {
    if (!_paletteConstrained) {
        return _currentColor;
    }
    return nearestPaletteColor(_currentColor, _paletteColors);
}

PixelRegion PixelCanvas::effectiveRegion() const {
    if (!_selection.empty()) {
        return _selection;
    }
    if (_image.width <= 0 || _image.height <= 0) {
        return {};
    }
    return PixelRegion{.minX = 0, .minY = 0, .maxX = _image.width - 1, .maxY = _image.height - 1};
}

void PixelCanvas::commitRegionMutation(PixelOperationKind kind, const DecodedImage& beforeSnapshot,
                                       const PixelRegion& touched) {
    if (touched.empty()) {
        return;
    }
    const std::vector<std::uint32_t> before = readRegion(beforeSnapshot, touched);
    const std::vector<std::uint32_t> after = readRegion(_image, touched);
    _history.push(kind, touched, before, after);
    _dirty = true;
    update();
    emit imageChanged();
    emit historyChanged();
}

void PixelCanvas::applyRegionOperation(PixelOperationKind kind,
                                       PixelRegion (*operation)(DecodedImage&,
                                                                const PixelRegion&)) {
    const PixelRegion region = effectiveRegion();
    if (region.empty()) {
        return;
    }
    const DecodedImage beforeSnapshot = _image;
    const PixelRegion touched = operation(_image, region);
    commitRegionMutation(kind, beforeSnapshot, touched);
}

void PixelCanvas::applyFlipHorizontal() {
    applyRegionOperation(PixelOperationKind::FlipHorizontal, &flipHorizontal);
}

void PixelCanvas::applyFlipVertical() {
    applyRegionOperation(PixelOperationKind::FlipVertical, &flipVertical);
}

void PixelCanvas::applyRotateClockwise() {
    applyRegionOperation(PixelOperationKind::RotateClockwise, &rotateClockwise);
}

void PixelCanvas::applyRotateCounterClockwise() {
    applyRegionOperation(PixelOperationKind::RotateCounterClockwise, &rotateCounterClockwise);
}

void PixelCanvas::copy() {
    const PixelRegion region = effectiveRegion();
    _clipboard = copyRegion(_image, region);
}

void PixelCanvas::paste() {
    if (_clipboard.empty()) {
        return;
    }
    const int x = _hoveredPixel ? _hoveredPixel->first : 0;
    const int y = _hoveredPixel ? _hoveredPixel->second : 0;
    const DecodedImage beforeSnapshot = _image;
    const PixelRegion touched = pasteClipboard(_image, _clipboard, x, y);
    if (touched.empty()) {
        return;
    }
    _selection = touched;  // le contenu colle devient la selection : ajustable immediatement.
    commitRegionMutation(PixelOperationKind::Paste, beforeSnapshot, touched);
}

void PixelCanvas::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        _panning = true;
        _panStartWidgetPos = event->position();
        _panStartView = _view;
        return;
    }
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }
    const std::optional<std::pair<int, int>> pixel = imagePixelAt(event->position());
    if (!pixel) {
        return;
    }
    beginGesture(pixel->first, pixel->second);
}

void PixelCanvas::mouseMoveEvent(QMouseEvent* event) {
    const std::optional<std::pair<int, int>> pixel = imagePixelAt(event->position());
    if (pixel != _hoveredPixel) {
        _hoveredPixel = pixel;
        emit hoveredPixelChanged(_hoveredPixel);
        update();  // repeint le contour de vise (pixel survole) meme hors geste/pan.
    }

    // Mode planche a raccords (TACHE-08) : infobulle nommant la configuration de la case survolee
    // -- "quelle case represente quoi" est ce que l'auteur doit savoir sans quitter le canevas des
    // yeux.
    const std::optional<AutotileCell> hoveredCell =
        pixel ? bitmaskCellAtPixel(_image, TextureAtlas::TILE_SIZE, pixel->first, pixel->second)
              : std::nullopt;
    if (hoveredCell && _loc != nullptr) {
        const auto mask = static_cast<std::uint8_t>((hoveredCell->row * AUTOTILE_SHEET_SIDE) +
                                                    hoveredCell->column);
        const std::string label(autotileConfigurationLabelKey(mask));
        QToolTip::showText(event->globalPosition().toPoint(),
                           QString::fromStdString(_loc->text(label)), this);
    } else {
        QToolTip::hideText();
    }

    if (_panning) {
        const QPointF delta = event->position() - _panStartWidgetPos;
        _view.panX =
            _panStartView.panX - static_cast<int>(std::lround(delta.x() / pixelCanvasScale(_view)));
        _view.panY =
            _panStartView.panY - static_cast<int>(std::lround(delta.y() / pixelCanvasScale(_view)));
        update();
        return;
    }
    if (!_gestureActive || !pixel) {
        return;  // hors image (geste en cours) : ignore, le trait reprend au prochain point dedans.
    }
    continueToolTo(pixel->first, pixel->second);
    _lastGesturePixel = pixel;
    update();
}

void PixelCanvas::leaveEvent(QEvent* /*event*/) {
    if (_hoveredPixel) {
        _hoveredPixel.reset();
        emit hoveredPixelChanged(_hoveredPixel);
        update();  // efface le contour de vise laisse par le dernier pixel survole.
    }
}

void PixelCanvas::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        _panning = false;
        return;
    }
    if (event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }
    endGesture();
    update();
}

void PixelCanvas::wheelEvent(QWheelEvent* event) {
    const int notches = event->angleDelta().y() / 120;  // 120 = un cran de molette (Qt/Win32).
    for (int i = 0; i < notches; ++i) {
        zoomIn();
    }
    for (int i = 0; i > notches; --i) {
        zoomOut();
    }
}

void PixelCanvas::setUnderlay(DecodedImage image, float opacity) {
    _underlay = std::move(image);
    _underlayOpacity = std::clamp(opacity, 0.0F, 1.0F);
    update();
}

void PixelCanvas::setOverlay(DecodedImage image, float opacity) {
    _overlay = std::move(image);
    _overlayOpacity = std::clamp(opacity, 0.0F, 1.0F);
    update();
}

void PixelCanvas::setReferenceGridStep(int step) {
    _referenceGridStep = std::max(0, step);
    update();
}

QPixmap PixelCanvas::renderPixmap() const {
    const qreal scale = devicePixelRatioF();
    const PixelCanvasRealSize real = pixelCanvasRealSize(_image.width, _image.height, _view, scale);
    QPixmap pixmap(std::max(1, real.width), std::max(1, real.height));

    // Surface de peinture (fond + damier) : portee INVARIANTE des jetons (epic.md, decision de
    // cadrage) -- ne suit jamais le theme clair/sombre du chassis d'edition qui l'entoure.
    const QColor checkerA = toQColor(identityTokens().color.surface);
    const QColor checkerB = toQColor(identityTokens().color.surfaceAlt);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.fillRect(0, 0, real.width, real.height, checkerA);

    const int cell = std::max(1, static_cast<int>(std::lround(CHECKER_LOGICAL_CELL * scale)));
    for (int y = 0; y < real.height; y += cell) {
        const bool rowOffset = ((y / cell) % 2) != 0;
        for (int x = rowOffset ? 0 : cell; x < real.width; x += 2 * cell) {
            painter.fillRect(x, y, cell, cell, checkerB);
        }
    }

    // Repere SOUS le contenu (mode creation, LOT-69 TACHE-07) : dessine ici, entre le damier et
    // l'oeuvre. Il n'entre jamais dans _image -- ni historique, ni copier.
    if (_underlay.width > 0 && _underlay.height > 0) {
        painter.setOpacity(static_cast<qreal>(_underlayOpacity));
        painter.drawImage(QRect(0, 0, real.width, real.height), toQImage(_underlay));
        painter.setOpacity(1.0);
    }

    // Oeuvre : plus proche voisin, jamais interpolee (EX-ARCH-022) -- SmoothPixmapTransform
    // desactive ci-dessus rend drawImage() rapide (nearest-neighbor) plutot que lisse.
    const QImage art = toQImage(_image);
    painter.drawImage(QRect(0, 0, real.width, real.height), art);

    // Repere PAR-DESSUS le contenu, memes regles.
    if (_overlay.width > 0 && _overlay.height > 0) {
        painter.setOpacity(static_cast<qreal>(_overlayOpacity));
        painter.drawImage(QRect(0, 0, real.width, real.height), toQImage(_overlay));
        painter.setOpacity(1.0);
    }

    // Grille de TUILES (LOT-69 TACHE-07), distincte de la grille de pixels ci-dessous : sur un plan
    // pictural, c'est elle qui permet de viser une case du niveau.
    if (_referenceGridStep > 0) {
        painter.setPen(toQColor(identityTokens().color.accent));
        const double stepReal =
            static_cast<double>(_referenceGridStep) * pixelCanvasScale(_view) * scale;
        if (stepReal >= 2.0) {
            for (double x = 0.0; x <= real.width; x += stepReal) {
                const int screenX = static_cast<int>(std::lround(x));
                painter.drawLine(screenX, 0, screenX, real.height);
            }
            for (double y = 0.0; y <= real.height; y += stepReal) {
                const int screenY = static_cast<int>(std::lround(y));
                painter.drawLine(0, screenY, real.width, screenY);
            }
        }
    }

    // Grille de pixels, seulement au-dela du seuil de lisibilite (TACHE-03).
    if (pixelCanvasGridVisible(_view)) {
        painter.setPen(toQColor(identityTokens().color.border));
        const double zoomReal = pixelCanvasScale(_view) * scale;
        for (int column = 0; column <= _image.width; ++column) {
            const int screenX = static_cast<int>(std::lround(column * zoomReal));
            painter.drawLine(screenX, 0, screenX, real.height);
        }
        for (int row = 0; row <= _image.height; ++row) {
            const int screenY = static_cast<int>(std::lround(row * zoomReal));
            painter.drawLine(0, screenY, real.width, screenY);
        }
    }

    // Pixel survole : contour de vise, pas seulement le curseur systeme -- a fort zoom, la pointe
    // d'une fleche/croix reste large de plusieurs pixels ecran a cote de la case qu'elle designe
    // reellement, ce qui donne l'impression trompeuse que le clic "tombe a cote". Tait pendant un
    // geste (pinceau en cours) pour ne pas geler sur le dernier point survole avant le glisser.
    if (_hoveredPixel && !_gestureActive) {
        const double zoomReal = pixelCanvasScale(_view) * scale;
        const double rectX = (_hoveredPixel->first - _view.panX) * zoomReal;
        const double rectY = (_hoveredPixel->second - _view.panY) * zoomReal;
        painter.setPen(QPen(toQColor(identityTokens().color.accent), std::max(1.0, scale)));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(QRectF(rectX, rectY, zoomReal, zoomReal));
    }

    // Region selectionnee (TACHE-06) : reste visible pendant qu'on la deplace, tracee depuis les
    // jetons (portee invariante, comme le reste de la surface de peinture) sans masquer les pixels
    // du bord (contour seul, jamais rempli).
    if (!_selection.empty()) {
        const double zoomReal = pixelCanvasScale(_view) * scale;
        const double rectX = (_selection.minX - _view.panX) * zoomReal;
        const double rectY = (_selection.minY - _view.panY) * zoomReal;
        const double rectW = _selection.width() * zoomReal;
        const double rectH = _selection.height() * zoomReal;
        painter.setPen(QPen(toQColor(identityTokens().color.accent), std::max(1.0, scale)));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(QRectF(rectX, rectY, rectW, rectH));
    }

    // Mode planche a raccords (TACHE-08) : reperes des seize cases (grille de case, plus epaisse
    // que la grille de pixel) et apercu d'assemblage 3x3 -- les reperes disent a quoi sert chaque
    // case, l'assemblage dit si le resultat tient (epic.md).
    if (isBitmask16Candidate(_image, TextureAtlas::TILE_SIZE)) {
        const double zoomReal = pixelCanvasScale(_view) * scale;
        painter.setPen(QPen(toQColor(identityTokens().color.accent), std::max(2.0, 2.0 * scale)));
        for (int column = 0; column <= AUTOTILE_SHEET_SIDE; ++column) {
            const int screenX =
                static_cast<int>(std::lround(column * TextureAtlas::TILE_SIZE * zoomReal));
            painter.drawLine(screenX, 0, screenX, real.height);
        }
        for (int row = 0; row <= AUTOTILE_SHEET_SIDE; ++row) {
            const int screenY =
                static_cast<int>(std::lround(row * TextureAtlas::TILE_SIZE * zoomReal));
            painter.drawLine(0, screenY, real.width, screenY);
        }

        // Apercu d'assemblage, en incrustation dans le coin bas-droit -- agrandi pour rester
        // lisible independamment du zoom du canevas.
        const DecodedImage assembly = buildAutotileAssemblyPreview(_image, TextureAtlas::TILE_SIZE);
        constexpr int ASSEMBLY_DISPLAY_SIZE = 96;  // pixels reels, cote du carre affiche.
        const QImage assemblyImage = toQImage(assembly);
        const int insetMargin = std::max(4, static_cast<int>(std::lround(4 * scale)));
        const int insetX = real.width - ASSEMBLY_DISPLAY_SIZE - insetMargin;
        const int insetY = real.height - ASSEMBLY_DISPLAY_SIZE - insetMargin;
        if (insetX >= 0 && insetY >= 0) {
            painter.fillRect(
                QRect(insetX - (insetMargin / 2), insetY - (insetMargin / 2),
                      ASSEMBLY_DISPLAY_SIZE + insetMargin, ASSEMBLY_DISPLAY_SIZE + insetMargin),
                toQColor(identityTokens().color.surface));
            painter.drawImage(QRect(insetX, insetY, ASSEMBLY_DISPLAY_SIZE, ASSEMBLY_DISPLAY_SIZE),
                              assemblyImage);
            painter.setPen(QPen(toQColor(identityTokens().color.border), std::max(1.0, scale)));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(QRect(insetX, insetY, ASSEMBLY_DISPLAY_SIZE, ASSEMBLY_DISPLAY_SIZE));
        }
    }

    pixmap.setDevicePixelRatio(scale);
    return pixmap;
}

void PixelCanvas::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.drawPixmap(0, 0, renderPixmap());
}

}  // namespace hmi
