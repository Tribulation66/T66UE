# DeckMap Structural Inventory

Generated from baseline dump: Saved/Codex/UI/DeckMap/baseline_dump.json
Migrated tag surface from: Saved/Codex/UI/DeckMap/pass_01_dump.json

## Baseline Text Elements

| Text | x | y | w | h |
|---|---:|---:|---:|---:|
| CHOOSE A ROOM BELOW | 0.000 | 0.000 | 1.000 | 1.000 |
| CHOOSE A ROOM BELOW | 0.090 | 0.099 | 0.820 | 0.808 |
| CHOOSE A ROOM BELOW | 0.091 | 0.100 | 0.818 | 0.805 |
| CHOOSE A ROOM BELOW | 0.091 | 0.101 | 0.817 | 0.803 |
| CHOOSE A ROOM BELOW | 0.109 | 0.133 | 0.781 | 0.739 |
| CHOOSE A ROOM BELOW | 0.109 | 0.133 | 0.781 | 0.054 |
| Guard Room | 0.109 | 0.219 | 0.781 | 0.552 |
| Guard Room | 0.109 | 0.219 | 0.781 | 0.104 |
| Guard Room | 0.110 | 0.220 | 0.780 | 0.102 |
| Guard Room | 0.111 | 0.222 | 0.778 | 0.099 |
| Guard Room | 0.122 | 0.240 | 0.757 | 0.062 |
| Guard Room | 0.122 | 0.240 | 0.757 | 0.031 |
| Floor 1 \| Combat | 0.122 | 0.279 | 0.757 | 0.023 |
| Cultist Fork | 0.109 | 0.339 | 0.781 | 0.104 |
| Cultist Fork | 0.110 | 0.341 | 0.780 | 0.102 |
| Cultist Fork | 0.111 | 0.342 | 0.778 | 0.099 |
| Cultist Fork | 0.122 | 0.361 | 0.757 | 0.062 |
| Cultist Fork | 0.122 | 0.361 | 0.757 | 0.031 |
| Floor 1 \| Combat | 0.122 | 0.399 | 0.757 | 0.023 |

## Migrated Tagged Elements

| Tag | Role | State | Text | x | y | w | h | Click | Toggle Group |
|---|---|---|---|---:|---:|---:|---:|---|---|
| Deck.Choice.GUARDROOM | ToggleButton | Default | Guard Room | 0.109 | 0.219 | 0.781 | 0.104 | True | DeckChoiceSelection |
| Deck.Choice.CULTISTFORK | ToggleButton | Default | Cultist Fork | 0.109 | 0.339 | 0.781 | 0.104 | True | DeckChoiceSelection |
| Deck.Button.MENU | Button | Default | MENU | 0.109 | 0.803 | 0.781 | 0.069 | True |  |

## Notes

- Deck map is reached with -T66DeckStartGameplay and preserves the room-choice structure.
- Structural target: preserve the baseline layout and visible content while replacing reachable legacy chrome with FT66FlatStyle surfaces.
- Other Deck internal view modes share the same migrated helper surface but do not have a direct capture alias; the existing automation path validates the map branch.
