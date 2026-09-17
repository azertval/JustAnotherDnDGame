// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QString>
#include <QWidget>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Core/Levels/LevelProperties.h"
#include "Core/Levels/MapEntity.h"
#include "Core/World/EntityKinds.h"
#include "HMI/Editor/EditorDiagnostics.h"

/**
 * @file HMI/Editor/EntityPanel.h
 * @brief Panneau « Entités » : famille à poser, liste des entités, propriétés de l'entité
 *        sélectionnée et avertissements (`LOT-11`).
 */

class QFormLayout;

namespace core {
class LevelDraft;
}

namespace Ui {
class EntityPanel;
}

namespace hmi {

class Localization;

/**
 * @brief Vue des entités du brouillon courant — remplace le panneau « Propriétés » retiré au
 *        `LOT-01`.
 *
 * Même patron que `hmi::LinkPanel` : le panneau reflète et demande, le viewport applique. Le
 * formulaire est **dérivé** de `core::knownEntityKinds` : une famille ou une propriété ajoutée à la
 * table y apparaît sans toucher au panneau, avec le contrôle que sa nature appelle (liste, texte,
 * entier, case à cocher). Une propriété que la table ne déclare pas est montrée telle quelle, sans
 * contrôle : elle est transportée, pas éditée (`EX-EDIT-011`).
 */
class EntityPanel : public QWidget {
    Q_OBJECT

public:
    explicit EntityPanel(QWidget* parent = nullptr);
    ~EntityPanel() override;

    /**
     * @brief Reconstruit liste, formulaire et avertissements.
     *
     * Le formulaire n'est refait que si l'entité sélectionnée, ses propriétés ou les choix proposés
     * ont changé : un coup de pinceau ailleurs sur la carte ne doit pas effacer un nom en cours de
     * saisie.
     */
    void refresh(const core::LevelDraft& draft, std::optional<std::size_t> selected,
                 const core::EntityReferenceContext& context,
                 const std::vector<EditorDiagnostic>& diagnostics);

    void retranslateUi(const Localization& loc);

    /// @return Le type que l'outil « Entité » pose, vide en simple sélection.
    [[nodiscard]] std::string kindToPlace() const;

signals:
    void kindToPlaceChanged(const QString& type);
    void entitySelected(std::optional<std::size_t> index);
    void propertyChanged(std::size_t index, const QString& key, const core::PropertyValue& value);
    void removeRequested(std::size_t index);

private:
    void rebuildKinds();
    void rebuildTable();
    void rebuildForm();
    /// Retire les lignes du formulaire ; leurs champs sont détruits au retour à la boucle.
    void clearForm();
    /**
     * @brief Ajoute au formulaire le champ d'une propriété déclarée.
     * @param index   Rang de l'entité dans la carte.
     * @param spec    Déclaration de la propriété.
     * @param value   Valeur portée par l'entité, ou la valeur par défaut.
     * @param choices Valeurs proposées, pour une propriété de choix.
     */
    void addPropertyRow(std::size_t index, const core::EntityPropertySpec& spec,
                        const core::PropertyValue& value, const std::vector<std::string>& choices);
    /**
     * @brief Ajoute au formulaire la liste déroulante d'une propriété de choix.
     * @param index   Rang de l'entité dans la carte.
     * @param spec    Déclaration de la propriété.
     * @param value   Valeur portée par l'entité, ou la valeur par défaut.
     * @param choices Valeurs proposées.
     */
    void addChoiceRow(std::size_t index, const core::EntityPropertySpec& spec,
                      const core::PropertyValue& value, const std::vector<std::string>& choices);
    void rebuildWarnings();
    [[nodiscard]] QString text(const char* key, const QString& fallback) const;
    [[nodiscard]] QString kindLabel(const std::string& type) const;
    [[nodiscard]] QString propertyLabel(const std::string& key) const;

    std::unique_ptr<Ui::EntityPanel> _ui;
    QFormLayout* _form;
    std::vector<core::MapEntity> _entities;
    std::optional<std::size_t> _selected;
    core::EntityReferenceContext _context;
    std::vector<EditorDiagnostic> _diagnostics;
    /// Ce que montre le formulaire, pour ne le refaire que s'il a changé.
    std::optional<std::size_t> _formIndex;
    std::optional<core::MapEntity> _formEntity;
    std::vector<std::vector<std::string>> _formChoices;
    bool _rebuilding = false;
    const Localization* _loc = nullptr;
};

}  // namespace hmi
