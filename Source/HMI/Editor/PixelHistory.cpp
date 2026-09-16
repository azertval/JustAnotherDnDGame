// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/PixelHistory.h"

#include <utility>

namespace hmi {

std::string_view pixelOperationTranslationKey(PixelOperationKind kind) noexcept {
    switch (kind) {
        case PixelOperationKind::Brush:
            return "pixel_editor.operation.brush";
        case PixelOperationKind::Eraser:
            return "pixel_editor.operation.eraser";
        case PixelOperationKind::Fill:
            return "pixel_editor.operation.fill";
        case PixelOperationKind::Move:
            return "pixel_editor.operation.move";
        case PixelOperationKind::FlipHorizontal:
            return "pixel_editor.operation.flip_horizontal";
        case PixelOperationKind::FlipVertical:
            return "pixel_editor.operation.flip_vertical";
        case PixelOperationKind::RotateClockwise:
            return "pixel_editor.operation.rotate_cw";
        case PixelOperationKind::RotateCounterClockwise:
            return "pixel_editor.operation.rotate_ccw";
        case PixelOperationKind::Paste:
            return "pixel_editor.operation.paste";
    }
    return "pixel_editor.operation.brush";  // inatteignable (switch exhaustif) ; /W4 l'exige.
}

PixelHistory::PixelHistory(std::size_t maxDepth) : _maxDepth(maxDepth) {}

void PixelHistory::push(PixelOperationKind kind, PixelRegion region,
                        std::vector<std::uint32_t> before, std::vector<std::uint32_t> after) {
    _applied.push_back(PixelHistoryEntry{
        .kind = kind, .region = region, .before = std::move(before), .after = std::move(after)});
    _undone.clear();
    if (_applied.size() > _maxDepth) {
        // Oublie l'entree la plus ancienne : l'etat courant du tampon (deja a jour) n'est jamais
        // affecte, seule la portee de l'annulation recule.
        _applied.erase(_applied.begin());
    }
}

bool PixelHistory::undo(DecodedImage& image) {
    if (_applied.empty()) {
        return false;
    }
    PixelHistoryEntry entry = std::move(_applied.back());
    _applied.pop_back();
    writeRegion(image, entry.region, entry.before);
    _undone.push_back(std::move(entry));
    return true;
}

bool PixelHistory::redo(DecodedImage& image) {
    if (_undone.empty()) {
        return false;
    }
    PixelHistoryEntry entry = std::move(_undone.back());
    _undone.pop_back();
    writeRegion(image, entry.region, entry.after);
    _applied.push_back(std::move(entry));
    return true;
}

bool PixelHistory::jumpTo(DecodedImage& image, std::size_t index) {
    if (index >= _applied.size()) {
        return false;
    }
    // Annulations successives jusqu'a l'entree visee : "en un seul appel" du point de vue de
    // l'appelant, qui n'a pas a boucler lui-meme sur undo().
    while (_applied.size() - 1 > index) {
        undo(image);
    }
    return true;
}

}  // namespace hmi
