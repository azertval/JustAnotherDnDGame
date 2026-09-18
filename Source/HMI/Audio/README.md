# HMI/Audio/

Lecture audio. Enveloppe fine autour de Qt Multimedia (`QSoundEffect`) — aucune dépendance dans
`Core` (`EX-ARCH-012`).

- `AudioEngine` : ouverture du périphérique de sortie, préchargement, volume global, repli
  « muet » si aucun périphérique n'est disponible (`EX-REN-047`, `EX-NFR-040`).
- `AudioLog.h` : macros de journalisation de la catégorie « Audio ».

Aucun son n'est encore livré : seul le volume de l'option Audio règle le moteur.
Réf. specs : `EX-REN-047`, `EX-REN-048`.
