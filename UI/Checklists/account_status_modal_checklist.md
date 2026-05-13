# Account Status Modal Structural Preservation Checklist

Source inventory: `C:\UE\T66\UI\Geometry\account_status_modal_structural_inventory.md`

Baseline capture: `C:\UE\T66\Saved\Codex\UI\AccountStatusModal\baseline_suspension_capture.png`

Baseline dump: `C:\UE\T66\Saved\Codex\UI\AccountStatusModal\baseline_suspension_dump.json`

Capture state: `-Screen AccountStatus -T66AccountTab=Suspension -ExecCmds="t66.AccountStatus.Force 1"`

## Structure

- [ ] AccountStatusModal.Root | exists=true
- [ ] AccountStatusModal.SubTabs | exists=true
- [ ] AccountStatusModal.SubTabs.SuspensionButton | exists=true
- [ ] AccountStatusModal.SubTabs.OverviewButton | exists=true
- [ ] AccountStatusModal.SubTabs.HistoryButton | exists=true
- [ ] AccountStatusModal.InfoStrip | exists=true
- [ ] AccountStatusModal.ContentPanel | exists=true
- [ ] AccountStatusModal.Content | exists=true
- [ ] AccountStatusModal.SectionLabel | exists=true
- [ ] AccountStatusModal.Headline | exists=true
- [ ] AccountStatusModal.Description | exists=true
- [ ] AccountStatusModal.ReasonPanel | exists=true
- [ ] AccountStatusModal.ReasonLabel | exists=true
- [ ] AccountStatusModal.ReasonValue | exists=true
- [ ] AccountStatusModal.AppealStatus | exists=true
- [ ] AccountStatusModal.AppealButton | exists=true

## Geometry

- [ ] AccountStatusModal.Root | x=0.000 | 0.005
- [ ] AccountStatusModal.Root | y=0.000 | 0.005
- [ ] AccountStatusModal.Root | w=1.000 | 0.005
- [ ] AccountStatusModal.Root | h=1.000 | 0.005
- [ ] AccountStatusModal.SubTabs | x=0.058 | 0.020
- [ ] AccountStatusModal.SubTabs | y=0.142 | 0.020
- [ ] AccountStatusModal.SubTabs | w=0.885 | 0.020
- [ ] AccountStatusModal.SubTabs | h=0.096 | 0.020
- [ ] AccountStatusModal.SubTabs.SuspensionButton | x=0.058 | 0.020
- [ ] AccountStatusModal.SubTabs.SuspensionButton | y=0.142 | 0.020
- [ ] AccountStatusModal.SubTabs.SuspensionButton | w=0.285 | 0.020
- [ ] AccountStatusModal.SubTabs.SuspensionButton | h=0.096 | 0.020
- [ ] AccountStatusModal.SubTabs.OverviewButton | x=0.358 | 0.020
- [ ] AccountStatusModal.SubTabs.OverviewButton | y=0.142 | 0.020
- [ ] AccountStatusModal.SubTabs.OverviewButton | w=0.285 | 0.020
- [ ] AccountStatusModal.SubTabs.OverviewButton | h=0.096 | 0.020
- [ ] AccountStatusModal.SubTabs.HistoryButton | x=0.658 | 0.020
- [ ] AccountStatusModal.SubTabs.HistoryButton | y=0.142 | 0.020
- [ ] AccountStatusModal.SubTabs.HistoryButton | w=0.285 | 0.020
- [ ] AccountStatusModal.SubTabs.HistoryButton | h=0.096 | 0.020
- [ ] AccountStatusModal.InfoStrip | x=0.002 | 0.020
- [ ] AccountStatusModal.InfoStrip | y=0.252 | 0.020
- [ ] AccountStatusModal.InfoStrip | w=0.997 | 0.020
- [ ] AccountStatusModal.InfoStrip | h=0.072 | 0.020
- [ ] AccountStatusModal.ContentPanel | x=0.002 | 0.020
- [ ] AccountStatusModal.ContentPanel | y=0.337 | 0.020
- [ ] AccountStatusModal.ContentPanel | w=0.997 | 0.020
- [ ] AccountStatusModal.ContentPanel | h=0.640 | 0.020
- [ ] AccountStatusModal.Content | x=0.018 | 0.020
- [ ] AccountStatusModal.Content | y=0.369 | 0.020
- [ ] AccountStatusModal.Content | w=0.964 | 0.020
- [ ] AccountStatusModal.Content | h=0.579 | 0.020
- [ ] AccountStatusModal.SectionLabel | x=0.018 | 0.020
- [ ] AccountStatusModal.SectionLabel | y=0.369 | 0.020
- [ ] AccountStatusModal.SectionLabel | w=0.964 | 0.020
- [ ] AccountStatusModal.SectionLabel | h=0.031 | 0.020
- [ ] AccountStatusModal.Headline | x=0.018 | 0.020
- [ ] AccountStatusModal.Headline | y=0.413 | 0.020
- [ ] AccountStatusModal.Headline | w=0.964 | 0.020
- [ ] AccountStatusModal.Headline | h=0.038 | 0.020
- [ ] AccountStatusModal.Description | x=0.018 | 0.020
- [ ] AccountStatusModal.Description | y=0.459 | 0.020
- [ ] AccountStatusModal.Description | w=0.964 | 0.020
- [ ] AccountStatusModal.Description | h=0.019 | 0.020
- [ ] AccountStatusModal.ReasonPanel | x=0.018 | 0.020
- [ ] AccountStatusModal.ReasonPanel | y=0.491 | 0.020
- [ ] AccountStatusModal.ReasonPanel | w=0.964 | 0.020
- [ ] AccountStatusModal.ReasonPanel | h=0.099 | 0.020
- [ ] AccountStatusModal.ReasonLabel | x=0.033 | 0.020
- [ ] AccountStatusModal.ReasonLabel | y=0.520 | 0.020
- [ ] AccountStatusModal.ReasonLabel | w=0.934 | 0.020
- [ ] AccountStatusModal.ReasonLabel | h=0.017 | 0.020
- [ ] AccountStatusModal.ReasonValue | x=0.033 | 0.020
- [ ] AccountStatusModal.ReasonValue | y=0.545 | 0.020
- [ ] AccountStatusModal.ReasonValue | w=0.934 | 0.020
- [ ] AccountStatusModal.ReasonValue | h=0.019 | 0.020
- [ ] AccountStatusModal.AppealStatus | x=0.018 | 0.020
- [ ] AccountStatusModal.AppealStatus | y=0.603 | 0.020
- [ ] AccountStatusModal.AppealStatus | w=0.964 | 0.020
- [ ] AccountStatusModal.AppealStatus | h=0.017 | 0.020
- [ ] AccountStatusModal.AppealButton | x=0.018 | 0.020
- [ ] AccountStatusModal.AppealButton | y=0.636 | 0.020
- [ ] AccountStatusModal.AppealButton | w=0.964 | 0.020
- [ ] AccountStatusModal.AppealButton | h=0.043 | 0.020

## Colors

- [ ] AccountStatusModal.SubTabs.SuspensionButton | button_state=Selected
- [ ] AccountStatusModal.SubTabs.OverviewButton | button_state=Default
- [ ] AccountStatusModal.SubTabs.HistoryButton | button_state=Default
- [ ] AccountStatusModal.InfoStrip | button_state=Selected
- [ ] AccountStatusModal.ContentPanel | button_state=Default
- [ ] AccountStatusModal.ReasonPanel | button_state=Default
- [ ] AccountStatusModal.Headline | text_color=SelectedText
- [ ] AccountStatusModal.AppealButton | button_state=Selected

## Content

- [ ] AccountStatusModal.SectionLabel | text=SUSPENSION
- [ ] AccountStatusModal.SectionLabel | is_label=true
- [ ] AccountStatusModal.Headline | text=ACCOUNT SUSPENDED
- [ ] AccountStatusModal.Headline | is_label=true
- [ ] AccountStatusModal.Description | text=This account cannot submit leaderboard scores while the suspension is active.
- [ ] AccountStatusModal.Description | is_label=true
- [ ] AccountStatusModal.ReasonLabel | text=REASON
- [ ] AccountStatusModal.ReasonLabel | is_label=true
- [ ] AccountStatusModal.ReasonValue | text=Too Lucky
- [ ] AccountStatusModal.ReasonValue | is_label=true
- [ ] AccountStatusModal.AppealStatus | text=Appeal: Not Submitted
- [ ] AccountStatusModal.AppealStatus | is_label=true
- [ ] AccountStatusModal.AppealButton | text=APPEAL

## Interactivity

- [ ] AccountStatusModal.SubTabs.SuspensionButton | has_click_handler=true
- [ ] AccountStatusModal.SubTabs.SuspensionButton | hover_capable=true
- [ ] AccountStatusModal.SubTabs.SuspensionButton | toggle_group=AccountStatusModalTabs
- [ ] AccountStatusModal.SubTabs.OverviewButton | has_click_handler=true
- [ ] AccountStatusModal.SubTabs.OverviewButton | hover_capable=true
- [ ] AccountStatusModal.SubTabs.OverviewButton | toggle_group=AccountStatusModalTabs
- [ ] AccountStatusModal.SubTabs.HistoryButton | has_click_handler=true
- [ ] AccountStatusModal.SubTabs.HistoryButton | hover_capable=true
- [ ] AccountStatusModal.SubTabs.HistoryButton | toggle_group=AccountStatusModalTabs
- [ ] AccountStatusModal.AppealButton | has_click_handler=true
- [ ] AccountStatusModal.AppealButton | hover_capable=true
