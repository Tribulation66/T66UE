# Run Summary Reference Geometry

Reference image: `C:\UE\T66\UI\Screen References\Run Summary Screen.png`

Native resolution: `1672x941`

Normalization basis: values below are `(x, y, w, h)` normalized to the same 1920x1080 basis used by `T66.UI.DumpScreen`; because the source image is 16:9, this is equivalent to `(x/1672, y/941, w/1672, h/941)`.

Measurement method: visual inspection of the Run Summary V3 reference image at native resolution.

Reference-vs-spec note:
- The reference shows an 8-column by 2-row inventory grid, while the current master-plan text says 2x7. This is a visual-layout conflict, so the reference wins under Pablo's post-Challenges rule.

| Tag / Region | Ref BBox | Tolerance | Notes |
| --- | --- | --- | --- |
| RunSummary.Root | `(0.018, 0.017, 0.964, 0.925)` | `+/-0.012` | full owned layout |
| RunSummary.Title | `(0.025, 0.036, 0.166, 0.049)` | `+/-0.012` | RUN SUMMARY title |
| RunSummary.TopStats.CouponsPanel | `(0.283, 0.017, 0.135, 0.080)` | `+/-0.010` | CHAG COUPONS mini-stat |
| RunSummary.TopStats.AchievementsPanel | `(0.430, 0.017, 0.135, 0.080)` | `+/-0.010` | ACHIEVEMENTS mini-stat |
| RunSummary.TopStats.SecretAchievementsPanel | `(0.576, 0.017, 0.135, 0.080)` | `+/-0.010` | SECRET ACH. mini-stat |
| RunSummary.EventLogButton | `(0.723, 0.017, 0.256, 0.080)` | `+/-0.010` | selected top-right action |
| RunSummary.Left.RunOutcomePanel | `(0.018, 0.110, 0.286, 0.174)` | `+/-0.010` | RUN OUTCOME panel |
| RunSummary.Left.SkullProgressPanel | `(0.018, 0.303, 0.286, 0.085)` | `+/-0.010` | five skull squares |
| RunSummary.Left.RankPanel | `(0.018, 0.403, 0.286, 0.206)` | `+/-0.010` | weekly/all-time split panel |
| RunSummary.Left.SeedLuckPanel | `(0.018, 0.626, 0.286, 0.113)` | `+/-0.010` | seed luck panel |
| RunSummary.Actions.GoAgainButton | `(0.018, 0.755, 0.144, 0.082)` | `+/-0.010` | selected CTA |
| RunSummary.Actions.ContinueButton | `(0.169, 0.755, 0.135, 0.082)` | `+/-0.010` | default action |
| RunSummary.Actions.MainMenuButton | `(0.018, 0.849, 0.144, 0.085)` | `+/-0.010` | default action |
| RunSummary.Actions.SaveAndQuitButton | `(0.169, 0.849, 0.135, 0.085)` | `+/-0.010` | default action |
| RunSummary.Middle.CharacterPreviewPanel | `(0.319, 0.113, 0.338, 0.368)` | `+/-0.012` | preserved 3D character render |
| RunSummary.Middle.IdolsPanel | `(0.319, 0.498, 0.338, 0.176)` | `+/-0.010` | four idol icons |
| RunSummary.Middle.InventoryPanel | `(0.319, 0.687, 0.338, 0.255)` | `+/-0.010` | inventory and currency panel |
| RunSummary.Middle.InventorySlotGrid | `(0.327, 0.760, 0.322, 0.139)` | `+/-0.012` | two rows by eight columns in reference |
| RunSummary.Right.StatTabs.StatsButton | `(0.678, 0.124, 0.089, 0.049)` | `+/-0.010` | selected tab |
| RunSummary.Right.StatTabs.DamageDealtButton | `(0.774, 0.124, 0.098, 0.049)` | `+/-0.010` | default tab |
| RunSummary.Right.StatTabs.DamageReceivedButton | `(0.879, 0.124, 0.098, 0.049)` | `+/-0.010` | default tab |
| RunSummary.Right.StatsPanel | `(0.672, 0.189, 0.310, 0.483)` | `+/-0.010` | active stat table panel |
| RunSummary.Right.ProofPanel | `(0.672, 0.688, 0.311, 0.158)` | `+/-0.010` | proof URL panel |
| RunSummary.Right.ProofUrlField | `(0.681, 0.755, 0.241, 0.071)` | `+/-0.010` | read-only URL field |
| RunSummary.Right.CopyButton | `(0.931, 0.755, 0.043, 0.071)` | `+/-0.010` | copy action |
| RunSummary.Right.SubmitCheatingButton | `(0.672, 0.861, 0.311, 0.081)` | `+/-0.010` | report action |
