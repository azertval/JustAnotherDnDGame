<#
.SYNOPSIS
    Configure, construit et teste JustAnotherDnDGame dans un environnement MSVC correct.

.DESCRIPTION
    Les générateurs Ninja et Makefiles n'établissent pas l'environnement MSVC : ils héritent des
    variables LIB / INCLUDE du terminal appelant. Une invite « Developer PowerShell » démarre en
    x86, ce qui fait échouer l'édition de liens d'une cible x64. Ce script supprime le problème en
    entrant lui-même dans l'environnement x64 avant d'appeler CMake — il fonctionne donc depuis
    n'importe quel terminal, y compris un PowerShell nu.

    L'installation de Visual Studio est localisée par vswhere.exe, présent à un emplacement fixe
    sur tout poste où Visual Studio est installé : aucun chemin propre à une machine n'est codé en
    dur.

.PARAMETER Preset
    Preset CMake à utiliser : « ninja » (défaut) ou « vs ».

.PARAMETER Test
    Exécuter CTest après la construction.

.PARAMETER Clean
    Supprimer le répertoire de build avant de configurer.

.PARAMETER QtPath
    Chemin d'une installation Qt à utiliser, si la détection automatique ne la trouve pas
    (ex. C:\Qt\6.8.1\msvc2022_64).

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts/build.ps1
    Configure et construit avec le preset ninja.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts/build.ps1 -Preset vs -Test -Clean
    Reconstruit de zéro avec le générateur Visual Studio, puis lance les tests.
#>
[CmdletBinding()]
param(
    # « ninja-release » sert au plugin Qt Designer (scripts/designer.ps1) : designer.exe est une
    # binaire Release, et une DLL Debug est rejetee EN SILENCE -- le plugin n'apparaitrait nulle
    # part, sans le moindre message.
    [ValidateSet('ninja', 'ninja-release', 'vs')]
    [string]$Preset = 'ninja',

    [switch]$Test,

    [switch]$Clean,

    [string]$QtPath
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repoRoot = Split-Path -Parent $PSScriptRoot

function Enter-X64Environment {
    # Déjà en x64 (invite développeur x64, ou script déjà appelé) : rien à faire.
    if ($env:VSCMD_ARG_TGT_ARCH -eq 'x64') {
        Write-Host 'Environnement MSVC x64 déjà actif.' -ForegroundColor DarkGray
        return
    }

    $installerDir = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer'
    $vswhere = Join-Path $installerDir 'vswhere.exe'
    if (-not (Test-Path $vswhere)) {
        throw "vswhere.exe introuvable ($vswhere). Visual Studio (ou les Build Tools) doit être installé."
    }

    # Le module DevShell invoque vswhere sans chemin absolu : sans cet ajout au PATH, il émet une
    # erreur « commande non reconnue » avant de se rabattre sur un autre mécanisme.
    if ($env:PATH -notlike "*$installerDir*") {
        $env:PATH = "$installerDir;$env:PATH"
    }

    $vsPath = & $vswhere -latest -prerelease -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath
    if (-not $vsPath) {
        throw 'Aucune installation Visual Studio avec les outils C++ x64 trouvée (composant Microsoft.VisualStudio.Component.VC.Tools.x86.x64).'
    }

    $devShell = Join-Path $vsPath 'Common7\Tools\Microsoft.VisualStudio.DevShell.dll'
    if (-not (Test-Path $devShell)) {
        throw "Module DevShell introuvable ($devShell)."
    }

    Import-Module $devShell
    # -SkipAutomaticLocation : conserve le répertoire courant, que le script gère lui-même.
    Enter-VsDevShell -VsInstallPath $vsPath `
        -DevCmdArguments '-arch=amd64 -host_arch=amd64' `
        -SkipAutomaticLocation | Out-Null

    if ($env:VSCMD_ARG_TGT_ARCH -ne 'x64') {
        throw "L'environnement de développement n'a pas basculé en x64 (VSCMD_ARG_TGT_ARCH=$env:VSCMD_ARG_TGT_ARCH)."
    }
    Write-Host "Environnement MSVC x64 établi ($vsPath)." -ForegroundColor DarkGray
}

function Invoke-Step {
    param([string]$Label, [scriptblock]$Action)

    Write-Host "==> $Label" -ForegroundColor Cyan
    & $Action
    if ($LASTEXITCODE -ne 0) {
        throw "$Label : échec (code $LASTEXITCODE)."
    }
}

Enter-X64Environment

if ($QtPath) {
    if (-not (Test-Path $QtPath)) {
        throw "Chemin Qt inexistant : $QtPath"
    }
    # Repris par find_package(Qt6) ; complète la détection automatique de Source/HMI/CMakeLists.txt.
    $env:CMAKE_PREFIX_PATH = $QtPath
}

$buildDir = Join-Path $repoRoot "build\$Preset"
if ($Clean -and (Test-Path $buildDir)) {
    Invoke-Step "Nettoyage de $buildDir" { Remove-Item -Recurse -Force $buildDir; $global:LASTEXITCODE = 0 }
}

Push-Location $repoRoot
try {
    Invoke-Step 'Configuration (CMake)' { cmake --preset $Preset }
    Invoke-Step 'Construction'          { cmake --build --preset $Preset }
    if ($Test) {
        Invoke-Step 'Tests (CTest)'     { ctest --preset $Preset }
    }
}
finally {
    Pop-Location
}

# Emplacement de l'exécutable : le générateur Visual Studio est multi-configuration (sous-dossier
# par configuration), Ninja ne l'est pas.
$exe = if ($Preset -eq 'vs') { "$buildDir\bin\Debug\JustAnotherDnDGame.exe" } else { "$buildDir\bin\JustAnotherDnDGame.exe" }
if (Test-Path $exe) {
    Write-Host "`nExécutable : $exe" -ForegroundColor Green
}
else {
    Write-Warning "Aucun exécutable produit ($exe). Qt6 est probablement introuvable : relancer avec -QtPath <chemin>."
}
