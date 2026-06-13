# Changelog

## 2.0.0

This update adds profile-driven custom skill groups, a new MCM group editor, and several fixes to make runtime settings apply more reliably.

### New Features

- Added custom skill groups for each profile.
- Added a new Groups MCM page for assigning skills, renaming groups, importing groups from another profile, and applying group changes.
- Added support for dynamic group names and up to 18 group slots.
- Added separate controls for grouped character XP and multiplier-based settings.

### Changed

- Character XP grouping is now loaded from the selected profile instead of being fixed.
- Group edits are staged until Apply is pressed; leaving the page without applying discards them.
- The old `Enabled` option is now `Grouping enabled` for clarity.
- `Multipliers enabled` now controls level thresholds, flat rank XP, character XP multipliers, and skill XP multipliers separately from grouping.
- Level threshold changes now recalculate the current player threshold immediately.

### Fixed

- Fixed profile settings not always applying on fresh saves or game load.
- Fixed grouping sometimes staying active after being disabled.
- Fixed custom group sizes not always being reflected by skill XP division.
- Fixed Archery and Speech group gating.
- Fixed level threshold recalculation after toggling multiplier settings.
- Fixed several MCM profile import, apply, and staging issues.
