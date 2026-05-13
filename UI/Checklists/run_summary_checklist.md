# Run Summary UI Fidelity Checklist

Source geometry: `C:\UE\T66\UI\Geometry\run_summary_reference_geometry.md`

Reference notes:
- `Run Summary Screen.png` is the visual authority.
- The reference shows two rows of eight inventory slots; this overrides the older 2x7 text in the master plan as a visual-layout conflict.
- Run Summary has no top bar.

## Structure

- [ ] RunSummary.Root | exists=true
- [ ] RunSummary.Title | exists=true
- [ ] RunSummary.TopStats.CouponsPanel | exists=true
- [ ] RunSummary.TopStats.CouponsPanel.Icon | exists=true
- [ ] RunSummary.TopStats.CouponsPanel.Label | exists=true
- [ ] RunSummary.TopStats.CouponsPanel.Value | exists=true
- [ ] RunSummary.TopStats.AchievementsPanel | exists=true
- [ ] RunSummary.TopStats.SecretAchievementsPanel | exists=true
- [ ] RunSummary.EventLogButton | exists=true
- [ ] RunSummary.EventLogButton.Icon | exists=true
- [ ] RunSummary.EventLogButton.Label | exists=true
- [ ] RunSummary.Left.RunOutcomePanel | exists=true
- [ ] RunSummary.Left.RunOutcomePanel.Header | exists=true
- [ ] RunSummary.Left.RunOutcomePanel.StageRow | exists=true
- [ ] RunSummary.Left.RunOutcomePanel.ScoreRow | exists=true
- [ ] RunSummary.Left.RunOutcomePanel.TimeRow | exists=true
- [ ] RunSummary.Left.SkullProgressPanel | exists=true
- [ ] RunSummary.Left.SkullProgressPanel.Skull01 | exists=true
- [ ] RunSummary.Left.SkullProgressPanel.Skull05 | exists=true
- [ ] RunSummary.Left.RankPanel | exists=true
- [ ] RunSummary.Left.RankPanel.WeeklyHeader | exists=true
- [ ] RunSummary.Left.RankPanel.AllTimeHeader | exists=true
- [ ] RunSummary.Left.SeedLuckPanel | exists=true
- [ ] RunSummary.Actions.GoAgainButton | exists=true
- [ ] RunSummary.Actions.ContinueButton | exists=true
- [ ] RunSummary.Actions.MainMenuButton | exists=true
- [ ] RunSummary.Actions.SaveAndQuitButton | exists=true
- [ ] RunSummary.Middle.CharacterPreviewPanel | exists=true
- [ ] RunSummary.Middle.IdolsPanel | exists=true
- [ ] RunSummary.Middle.IdolsPanel.Idol01 | exists=true
- [ ] RunSummary.Middle.IdolsPanel.Idol04 | exists=true
- [ ] RunSummary.Middle.InventoryPanel | exists=true
- [ ] RunSummary.Middle.InventorySlotGrid | exists=true
- [ ] RunSummary.Middle.InventorySlot01 | exists=true
- [ ] RunSummary.Middle.InventorySlot16 | exists=true
- [ ] RunSummary.Right.StatTabs.StatsButton | exists=true
- [ ] RunSummary.Right.StatTabs.DamageDealtButton | exists=true
- [ ] RunSummary.Right.StatTabs.DamageReceivedButton | exists=true
- [ ] RunSummary.Right.StatsPanel | exists=true
- [ ] RunSummary.Right.StatsPanel.LevelRow | exists=true
- [ ] RunSummary.Right.StatsPanel.SpeedRow | exists=true
- [ ] RunSummary.Right.ProofPanel | exists=true
- [ ] RunSummary.Right.ProofUrlField | exists=true
- [ ] RunSummary.Right.CopyButton | exists=true
- [ ] RunSummary.Right.SubmitCheatingButton | exists=true

## Geometry

- [ ] RunSummary.Title | x=0.025 | 0.014
- [ ] RunSummary.Title | y=0.036 | 0.014
- [ ] RunSummary.TopStats.CouponsPanel | x=0.283 | 0.012
- [ ] RunSummary.TopStats.CouponsPanel | y=0.017 | 0.012
- [ ] RunSummary.TopStats.AchievementsPanel | x=0.430 | 0.012
- [ ] RunSummary.TopStats.SecretAchievementsPanel | x=0.576 | 0.012
- [ ] RunSummary.EventLogButton | x=0.723 | 0.012
- [ ] RunSummary.Left.RunOutcomePanel | x=0.018 | 0.012
- [ ] RunSummary.Left.RunOutcomePanel | y=0.110 | 0.012
- [ ] RunSummary.Left.SkullProgressPanel | y=0.303 | 0.012
- [ ] RunSummary.Left.RankPanel | y=0.403 | 0.012
- [ ] RunSummary.Left.SeedLuckPanel | y=0.626 | 0.012
- [ ] RunSummary.Actions.GoAgainButton | y=0.755 | 0.012
- [ ] RunSummary.Actions.MainMenuButton | y=0.849 | 0.012
- [ ] RunSummary.Middle.CharacterPreviewPanel | x=0.319 | 0.014
- [ ] RunSummary.Middle.CharacterPreviewPanel | y=0.113 | 0.014
- [ ] RunSummary.Middle.IdolsPanel | y=0.498 | 0.012
- [ ] RunSummary.Middle.InventoryPanel | y=0.687 | 0.012
- [ ] RunSummary.Right.StatTabs.StatsButton | x=0.678 | 0.012
- [ ] RunSummary.Right.StatTabs.StatsButton | y=0.124 | 0.012
- [ ] RunSummary.Right.StatsPanel | x=0.672 | 0.012
- [ ] RunSummary.Right.StatsPanel | y=0.189 | 0.012
- [ ] RunSummary.Right.ProofPanel | y=0.688 | 0.012
- [ ] RunSummary.Right.SubmitCheatingButton | y=0.861 | 0.012

## Colors

- [ ] RunSummary.EventLogButton | button_state=Selected
- [ ] RunSummary.Actions.GoAgainButton | button_state=Selected
- [ ] RunSummary.Actions.ContinueButton | button_state=Default
- [ ] RunSummary.Actions.MainMenuButton | button_state=Default
- [ ] RunSummary.Actions.SaveAndQuitButton | button_state=Default
- [ ] RunSummary.Right.StatTabs.StatsButton | button_state=Selected
- [ ] RunSummary.Right.StatTabs.DamageDealtButton | button_state=Default
- [ ] RunSummary.Right.StatTabs.DamageReceivedButton | button_state=Default
- [ ] RunSummary.Left.RunOutcomePanel | border_color=DefaultBorder
- [ ] RunSummary.Right.StatsPanel | border_color=DefaultBorder
- [ ] RunSummary.Right.ProofPanel | border_color=DefaultBorder
- [ ] RunSummary.Right.SubmitCheatingButton | button_state=Default

## Content

- [ ] RunSummary.Title | text=RUN SUMMARY
- [ ] RunSummary.Title | is_label=true
- [ ] RunSummary.TopStats.CouponsPanel.Label | text=CHAG COUPONS
- [ ] RunSummary.TopStats.CouponsPanel.Value | text=0
- [ ] RunSummary.TopStats.CouponsPanel.Value | is_label=true
- [ ] RunSummary.TopStats.AchievementsPanel.Label | text=ACHIEVEMENTS
- [ ] RunSummary.TopStats.SecretAchievementsPanel.Label | text=SECRET ACH.
- [ ] RunSummary.EventLogButton.Label | text=EVENT LOG
- [ ] RunSummary.Left.RunOutcomePanel.Header | text=RUN OUTCOME
- [ ] RunSummary.Left.RunOutcomePanel.StageRow.Label | text=Stage Reached
- [ ] RunSummary.Left.RunOutcomePanel.StageRow.Value | text=1
- [ ] RunSummary.Left.RunOutcomePanel.ScoreRow.Label | text=Score
- [ ] RunSummary.Left.RunOutcomePanel.ScoreRow.Value | text=0
- [ ] RunSummary.Left.RunOutcomePanel.TimeRow.Label | text=Time
- [ ] RunSummary.Left.RunOutcomePanel.TimeRow.Value | text=00:00
- [ ] RunSummary.Left.RankPanel.WeeklyHeader | text=WEEKLY RANK
- [ ] RunSummary.Left.RankPanel.AllTimeHeader | text=ALL TIME RANK
- [ ] RunSummary.Left.SeedLuckPanel.Header | text=SEED LUCK
- [ ] RunSummary.Left.SeedLuckPanel.Value | text=65 / 100 (Fortunate)
- [ ] RunSummary.Actions.GoAgainButton.Label | text=GO AGAIN!
- [ ] RunSummary.Actions.ContinueButton.Label | text=CONTINUE
- [ ] RunSummary.Actions.MainMenuButton.Label | text=MAIN MENU
- [ ] RunSummary.Actions.SaveAndQuitButton.Label | text=SAVE AND QUIT
- [ ] RunSummary.Middle.IdolsPanel.Header | text=IDOLS
- [ ] RunSummary.Middle.InventoryPanel.Header | text=INVENTORY
- [ ] RunSummary.Middle.InventoryPanel.GoldValue | text=1,275
- [ ] RunSummary.Middle.InventoryPanel.DebtValue | text=320
- [ ] RunSummary.Middle.InventoryPanel.NetWorthValue | text=955
- [ ] RunSummary.Right.StatTabs.StatsButton.Label | text=STATS
- [ ] RunSummary.Right.StatTabs.DamageDealtButton.Label | text=DAMAGE DEALT
- [ ] RunSummary.Right.StatTabs.DamageReceivedButton.Label | text=DAMAGE RECEIVED
- [ ] RunSummary.Right.StatsPanel.LevelRow | text=LEVEL: 1
- [ ] RunSummary.Right.StatsPanel.DamageRow | text=Damage: 1
- [ ] RunSummary.Right.StatsPanel.SpeedRow | text=Speed: 1
- [ ] RunSummary.Right.ProofPanel.Header | text=PROOF OF RUN
- [ ] RunSummary.Right.ProofUrlField | text=youtube.com/watch?v=run-proof-001
- [ ] RunSummary.Right.SubmitCheatingButton.Label | text=SUBMIT SUSPICION OF CHEATING

## Interactivity

- [ ] RunSummary.EventLogButton | has_click_handler=true
- [ ] RunSummary.EventLogButton | hover_capable=true
- [ ] RunSummary.Actions.GoAgainButton | has_click_handler=true
- [ ] RunSummary.Actions.GoAgainButton | hover_capable=true
- [ ] RunSummary.Actions.ContinueButton | has_click_handler=true
- [ ] RunSummary.Actions.ContinueButton | hover_capable=true
- [ ] RunSummary.Actions.MainMenuButton | has_click_handler=true
- [ ] RunSummary.Actions.MainMenuButton | hover_capable=true
- [ ] RunSummary.Actions.SaveAndQuitButton | has_click_handler=true
- [ ] RunSummary.Actions.SaveAndQuitButton | hover_capable=true
- [ ] RunSummary.Right.StatTabs.StatsButton | has_click_handler=true
- [ ] RunSummary.Right.StatTabs.StatsButton | hover_capable=true
- [ ] RunSummary.Right.StatTabs.StatsButton | toggle_group=RunSummaryStatTabs
- [ ] RunSummary.Right.StatTabs.DamageDealtButton | has_click_handler=true
- [ ] RunSummary.Right.StatTabs.DamageDealtButton | hover_capable=true
- [ ] RunSummary.Right.StatTabs.DamageDealtButton | toggle_group=RunSummaryStatTabs
- [ ] RunSummary.Right.StatTabs.DamageReceivedButton | has_click_handler=true
- [ ] RunSummary.Right.StatTabs.DamageReceivedButton | hover_capable=true
- [ ] RunSummary.Right.StatTabs.DamageReceivedButton | toggle_group=RunSummaryStatTabs
- [ ] RunSummary.Right.CopyButton | has_click_handler=true
- [ ] RunSummary.Right.CopyButton | hover_capable=true
- [ ] RunSummary.Right.SubmitCheatingButton | has_click_handler=true
- [ ] RunSummary.Right.SubmitCheatingButton | hover_capable=true
- [ ] RunSummary.TopStats.CouponsPanel.Value | is_label=true
- [ ] RunSummary.Left.SeedLuckPanel.Value | is_label=true
- [ ] RunSummary.Right.ProofUrlField | is_label=true
