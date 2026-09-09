# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

<#
.SYNOPSIS
    Ouvre Qt Designer avec le plugin du dépôt, pour éditer les .ui d'identité (LOT-85, EX-IHM-006).

.DESCRIPTION
    Sans plugin, Designer ne connaît des widgets promus que leur classe de base : il dessine des
    rectangles gris là où le jeu peint un parchemin, et une maquette qu'on ne voit pas ne s'édite
    pas. Ce script bâtit le plugin, puis lance Designer en le lui désignant.

    Deux points que ce script existe pour tenir, et qui échouent SILENCIEUSEMENT si on lance
    Designer à la main :

    - Le plugin est bâti en **Release**. `designer.exe` est une binaire Release, et Qt rejette une
      DLL Debug sans le moindre message : le plugin n'apparaîtrait nulle part, et rien ne dirait
      pourquoi.
    - `QT_PLUGIN_PATH` désigne le dossier de BUILD, jamais l'installation Qt. Copier la DLL dans
      D:\Qt\...\plugins\designer\ marcherait une fois, puis laisserait une copie périmée que la
      reconstruction suivante ne toucherait pas.

.PARAMETER Path
    Fichier .ui à ouvrir. Par défaut, Designer s'ouvre sur le sélecteur de formulaire.

.PARAMETER SkipBuild
    Ne pas reconstruire le plugin : lance Designer sur la DLL déjà produite.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts/designer.ps1 -Path Source/Elements/UI/RpgCharacterSheetPlate.ui

.NOTES
    « powershell » et non « pwsh » : PowerShell 7 n'est pas installe sur le poste, et l'invoquer
    donne « command not found » sans dire lequel des deux manque.
#>
[CmdletBinding()]
param(
    [string]$Path,
    [switch]$SkipBuild
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot 'build\ninja-release'

if (-not $SkipBuild) {
    Write-Host '==> Construction du plugin (Release)' -ForegroundColor Cyan
    # Delegue a build.ps1 : c'est lui qui etablit l'environnement MSVC x64, et le dupliquer ici
    # les aurait laisses diverger.
    & (Join-Path $PSScriptRoot 'build.ps1') -Preset 'ninja-release'
    if ($LASTEXITCODE -ne 0) {
        throw "Construction du plugin : échec (code $LASTEXITCODE)."
    }
}

# Le theme que Designer appliquera. Deux etapes, et l'ordre compte : l'outil C++ resout le gabarit
# (lui seul sait le faire -- la substitution vit dans le code du jeu), puis le script Python en
# depose la tranche de chaque ecran dans son .ui. Les rejouer a chaque lancement est ce qui empeche
# d'ouvrir Designer sur une planche peinte avec les couleurs d'avant-hier.
if (-not $SkipBuild) {
    $identityQss = Join-Path $buildDir 'bin\IdentityQss.exe'
    if (-not (Test-Path $identityQss)) {
        throw "IdentityQss introuvable ($identityQss)."
    }
    Write-Host '==> Theme resolu pour Designer' -ForegroundColor Cyan
    & $identityQss $repoRoot
    if ($LASTEXITCODE -ne 0) {
        throw "Resolution du theme : echec (code $LASTEXITCODE)."
    }
    & python (Join-Path $PSScriptRoot 'sync_ui_theme.py')
    if ($LASTEXITCODE -ne 0) {
        throw "Injection du theme dans les .ui : echec (code $LASTEXITCODE)."
    }
}

$pluginDir = Join-Path $buildDir 'qtplugins'
$pluginDll = Join-Path $pluginDir 'designer\JadgDesignerPlugin.dll'
if (-not (Test-Path $pluginDll)) {
    throw @"
Plugin introuvable ($pluginDll).
Le composant « Qt Designer » (Qt6 UiPlugin) est probablement absent de l'installation Qt : la
cible se desactive alors d'elle-meme, avec un message a la configuration CMake.
"@
}

# L'installation Qt est celle que CMake a retenue, et non une supposition : la lire dans le cache
# garantit que Designer et le plugin viennent du MEME Qt. Deux versions voisines suffiraient a ce
# que le plugin soit rejete.
$cache = Join-Path $buildDir 'CMakeCache.txt'
$qtDirLine = Select-String -Path $cache -Pattern '^Qt6_DIR:PATH=(.+)$' | Select-Object -First 1
if (-not $qtDirLine) {
    throw "Qt6_DIR absent de $cache : la configuration CMake n'a pas trouvé Qt."
}
# <prefixe>/lib/cmake/Qt6 -> <prefixe>
$qtPrefix = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $qtDirLine.Matches[0].Groups[1].Value))
$designer = Join-Path $qtPrefix 'bin\designer.exe'
if (-not (Test-Path $designer)) {
    throw @"
designer.exe introuvable ($designer).
Ajouter le composant « Qt Designer » a l'installation Qt via l'installateur (Maintenance Tool).
"@
}

$env:QT_PLUGIN_PATH = $pluginDir
Write-Host "Plugin  : $pluginDll" -ForegroundColor DarkGray
Write-Host "Designer: $designer" -ForegroundColor DarkGray

$arguments = @()
if ($Path) {
    if (-not (Test-Path $Path)) {
        throw "Fichier introuvable : $Path"
    }
    $arguments += (Resolve-Path $Path).Path
}

Write-Host '==> Qt Designer' -ForegroundColor Cyan
& $designer @arguments
