Result: OK

## Summary
The Codex draft covers all 20 idols in a clean Black/Red/Yellow/White grid, preserves every element-color constraint, and answers the user's question about the Claude validation process accurately (matches the Operator/Validator split and the prior Fire-only OK). It is deliverable. Two rarity-floor inconsistencies and one invented count should be fixed before sending; both are Codex-fixable, so the result stays OK.

## Suggested Answer Patch
- **Electricity AOE → Black:** change "Three purple lightning strikes drop from above" to a single strike (e.g. "One purple bolt drops from above inside the radius"). Black is the rarity floor; starting at three breaks the Black→White progression and collides with Red ("Three strikes plus a faint ground flash").
- **Fire Bounce row:** drop the hardcoded "Four." The prompt never specifies a count of four for Fire Bounce; let count/density scale with rarity like every other Bounce idol (Black = 1–2 cinders snap, … White = dense fast ember web). If four is a real gameplay target count, say so explicitly rather than asserting it as a visual given.

## Issues To Fix
- Electricity AOE Black uses "Three" strikes — violates the "Black = single/minimal" rarity grammar the draft itself relies on, and overlaps Red. Reduce to one.
- Fire Bounce hardcodes "Four" cinders across all four tiers — an unsupported assumption and the one place rarity does not read through density/count. Make it scale or label it a gameplay-driven count.
- Minor: only Wind AOE White restates "damage radius < movement circle." The user gave that as a standing constraint for Wind AOE; a one-line note that it holds across all Wind AOE tiers would prevent misreading.

## Question For User
None required. Planning-only and self-contained.

## Evidence Or Verification Gaps
- Roster (20 idols, 5×4) is confirmed against `Content/Data/Idols.csv` in both the prompt evidence and the independent answer; draft is consistent.
- The "four cinders" and "three strikes" counts have no evidence basis in the prompt — they are the only fact-like claims in the draft and both are the flagged issues.

## Notes
- Process note is correct and directly addresses the user's "did Claude give the OK?" question; keep it.
- The draft's closing note about gameplay-vs-visual count is good and actually pre-flags the Fire Bounce / Electricity AOE ambiguity — resolving the two fixes above makes it consistent with that note.
