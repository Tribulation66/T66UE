# Leaderboard Layout List

Canvas: 1920x1080.

Fixed structure:
- Standalone framed leaderboard panel centered on a dark background.
- Back button at upper left inside the panel.
- Large centered title at the top.
- Small icon tab row at upper left inside the content panel.
- Centered Weekly and All Time tabs.
- Two dropdown filters beneath the tabs, then Score and Speed Run mode choices.
- Leaderboard list area below with name and score columns and a few visible rows.
- Wide empty space below current rows remains part of the table body.

Runtime-owned regions:
- Title, icon tab state, tab labels/states, dropdown values/options, mode labels/states, row ranks, names, avatars, scores, favorite state, and back label.

Visual restyle target:
- Preserve the standalone panel geometry and filter/list hierarchy.
- Restyle the outer frame, tabs, dropdowns, mode selectors, avatar sockets, and row separators in the main-menu purple neon metal style while keeping row data live.
