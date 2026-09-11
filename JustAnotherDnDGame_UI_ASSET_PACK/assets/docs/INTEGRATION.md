# Integration Qt/QML

Resource prefix: qrc:/MercenaryRpgUiKit/

The SVG layer is text-free and purely visual. Keep game data and calculations
in your C++ model or regular QML. For Qt Design Studio `.ui.qml`, avoid arbitrary
JavaScript functions and expose computed values as properties.

Typical image:
Image { source: "qrc:/MercenaryRpgUiKit/svg/inventory_inventory/longsword.svg" }

The QML components in this kit are intentionally reusable primitives, not a full
screen, so they can be composed into the existing project's character/inventory pages.
