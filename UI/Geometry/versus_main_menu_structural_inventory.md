# VersusMainMenu Structural Inventory

Generated from baseline dump: Saved/Codex/UI/VersusMainMenu/baseline_dump.json
Migrated tag surface from: Saved/Codex/UI/VersusMainMenu/pass_01_dump.json

## Baseline Text Elements

| Text | x | y | w | h |
|---|---:|---:|---:|---:|
| VERSUS | 0.000 | 0.000 | 1.000 | 1.000 |
| VERSUS | 0.025 | 0.061 | 0.949 | 0.877 |
| VERSUS | 0.426 | 0.061 | 0.148 | 0.112 |
| 1v1 arcade gauntlet for competing against friends | 0.340 | 0.184 | 0.320 | 0.035 |
| FORMAT | 0.025 | 0.254 | 0.949 | 0.685 |
| FORMAT | 0.025 | 0.254 | 0.410 | 0.685 |
| FORMAT | 0.088 | 0.342 | 0.302 | 0.533 |
| FORMAT | 0.088 | 0.342 | 0.302 | 0.045 |
| Rounds | 0.088 | 0.411 | 0.302 | 0.088 |
| Rounds | 0.088 | 0.411 | 0.302 | 0.029 |
| Both players roll through the same arcade cabinet challenge, then compare score, time, or survival result. | 0.088 | 0.445 | 0.302 | 0.054 |
| Arcade pool | 0.088 | 0.518 | 0.302 | 0.088 |
| Arcade pool | 0.088 | 0.518 | 0.302 | 0.029 |
| Whack-a-Mole and the other in-world arcade games become the competitive playlist. | 0.088 | 0.552 | 0.302 | 0.054 |
| Party | 0.088 | 0.624 | 0.302 | 0.088 |
| Party | 0.088 | 0.624 | 0.302 | 0.029 |
| The first wiring uses the existing duo lobby/session path so invite flow can be layered in next. | 0.088 | 0.658 | 0.302 | 0.054 |
| SETUP | 0.452 | 0.254 | 0.522 | 0.685 |
| SETUP | 0.515 | 0.342 | 0.414 | 0.533 |
| SETUP | 0.515 | 0.342 | 0.414 | 0.045 |
| HOST 1V1 | 0.515 | 0.417 | 0.414 | 0.077 |
| HOST 1V1 | 0.529 | 0.430 | 0.387 | 0.051 |
| JOIN FRIEND | 0.515 | 0.515 | 0.414 | 0.077 |
| JOIN FRIEND | 0.529 | 0.529 | 0.387 | 0.051 |
| LOCAL PRACTICE SHELL | 0.515 | 0.614 | 0.414 | 0.077 |
| LOCAL PRACTICE SHELL | 0.529 | 0.627 | 0.387 | 0.051 |
| Choose a 1v1 setup path. Arcade game selection comes next. | 0.515 | 0.723 | 0.414 | 0.029 |
| BACK TO MINIGAMES | 0.735 | 0.797 | 0.195 | 0.077 |
| BACK TO MINIGAMES | 0.748 | 0.811 | 0.168 | 0.051 |

## Migrated Tagged Elements

| Tag | Role | State | Text | x | y | w | h | Click |
|---|---|---|---|---:|---:|---:|---:|---|
| Versus.Panel.Format | Panel | Selected | FORMAT | 0.025 | 0.254 | 0.410 | 0.685 | False |
| Versus.Panel.Setup | Panel | Default | SETUP | 0.452 | 0.254 | 0.522 | 0.685 | False |
| Versus.Button.HOST1V1 | Button | Selected | HOST 1V1 | 0.517 | 0.419 | 0.411 | 0.077 | True |
| Versus.Button.JOINFRIEND | Button | Default | JOIN FRIEND | 0.517 | 0.518 | 0.411 | 0.077 | True |
| Versus.Button.LOCALPRACTICESHELL | Button | Default | LOCAL PRACTICE SHELL | 0.517 | 0.617 | 0.411 | 0.077 | True |
| Versus.Button.BACKTOMINIGAMES | Button | Default | BACK TO MINIGAMES | 0.733 | 0.795 | 0.195 | 0.077 | True |

## Notes

- Versus has one no-reference screen class and preserves the baseline two-panel layout while replacing reference border/button chrome with FT66FlatStyle surfaces.
