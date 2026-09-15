// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Runtime/CreditsModel.h"

#include <QFile>
#include <QStringList>
#include <QVariantMap>

#include "HMI/HmiLog.h"
#include "HMI/Presentation/CreditsCatalog.h"

namespace hmi {
namespace {

constexpr const char* CREDITS_RESOURCE = ":/jadg/credits/credits.json";

}  // namespace

CreditsModel::CreditsModel(QObject* parent) : QObject(parent) {
    reload();
}

void CreditsModel::setColumn(int column) {
    if (_column == column) {
        return;
    }
    _column = column;
    reload();
}

void CreditsModel::setLanguage(const QString& language) {
    if (_language == language) {
        return;
    }
    _language = language;
    reload();
}

void CreditsModel::reload() {
    QFile file{QString::fromLatin1(CREDITS_RESOURCE)};
    if (!file.open(QIODevice::ReadOnly)) {
        HMI_LOG_ERROR("Credits : ressource introuvable (" + std::string(CREDITS_RESOURCE) + ").");
        _sections.clear();
        emit sectionsChanged();
        return;
    }
    const QByteArray json = file.readAll();
    const CreditsResult result =
        readCredits(std::string_view(json.constData(), json.size()), _language.toStdString());
    if (!result.ok()) {
        HMI_LOG_ERROR("Credits : " + result.error);
    }

    _sections.clear();
    for (const CreditSection& section : result.sections) {
        if (section.column != _column) {
            continue;
        }
        QVariantList lines;
        for (const CreditLine& line : section.lines) {
            QStringList names;
            for (const std::string& name : line.names) {
                names.append(QString::fromStdString(name));
            }
            lines.append(QVariantMap{{QStringLiteral("role"), QString::fromStdString(line.role)},
                                     {QStringLiteral("names"), names.join(QLatin1Char('\n'))}});
        }
        const QString id = QString::fromStdString(section.id);
        _sections.append(QVariantMap{
            {QStringLiteral("sectionId"), id},
            {QStringLiteral("title"), QString::fromStdString(section.title)},
            {QStringLiteral("iconKey"), QStringLiteral("ui/icon/credits-section/") + id},
            {QStringLiteral("lines"), lines}});
    }
    emit sectionsChanged();
}

}  // namespace hmi
