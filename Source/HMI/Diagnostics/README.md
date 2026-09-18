# HMI/Diagnostics/

**Dossier vide.** Ce que l'application expose du journal de la session **à l'utilisateur** vit
désormais dans `HMI/Runtime/` : `hmi::OptionsModel::saveLogs` enregistre sur disque les messages de
la session (`core::MemoryLogSink`), derrière le bouton **« Enregistrer les journaux »** de l'écran
Options. C'est ce qui permet à un joueur non-développeur de joindre un fichier exploitable à un
rapport de défaut, sans lancer l'application depuis un terminal. La journalisation elle-même
(niveaux, puits, format) est définie par `Core/Diagnostics/`.

Réf. specs : `EX-NFR-040` ; guide
[`guide-journalisation`](../../../Documentation/Guide/guide-journalisation.md).
