User request:

Ok so what we are going to do, is change the number of rooms for floor 2 and 3, to a fixed 10 for each. No change to floor 1 and 4. Then lets change a tile, to be 1000 units to make it simpler. And change the room size to a fixed (5000 - 10000) x (5000 - 1000) meaning not every room, is a perfect square. This is for room 2 and 3 only no change to size of boss or start floor, however we are going to globally change the tile size 1000 units, so its fine if floor 1 and 4 get adjusted a bit to accomodate the new unit but the spirit of the exercise is to keep those two mostly the same size wise. With the big change being floors 2 and 3. Then we are going to have every room have 1- 2 traps, every floor has a trap with some having 2 and every room having one interactable or npc, respecting our npc rules of 1 vendor per floor for floors 2 and 3 and others.

Task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: implement the requested tower tuning changes for floors 2 and 3: fixed 10 combat rooms each, 1000-unit tiles, combat room sizes interpreted as 5000-10000 by 5000-10000 units with independent width/height rolls, room-level trap/content rules, and floor/NPC constraints while preserving floor 1 and floor 4 as much as the global tile-size change allows.
Stop condition: changes are implemented in config/code/docs as needed, validated by Claude, and verified with the strongest feasible build/staged checks or clearly reported if a gate cannot run.

Assumption:

The typed request `(5000 - 10000) x (5000 - 1000)` is interpreted as `(5000 - 10000) x (5000 - 10000)`, with width and height rolled independently, because a max of 1000 on the second axis conflicts with the stated range and with the non-square-room intent.
