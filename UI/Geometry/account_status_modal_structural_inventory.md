# Account Status Modal Structural Inventory

Source baseline: `C:\UE\T66\Saved\Codex\UI\AccountStatusModal\baseline_suspension_capture.png`

Source dump: `C:\UE\T66\Saved\Codex\UI\AccountStatusModal\baseline_suspension_dump.json`

Normalized basis: 1920x1080.

Target state: `AccountStatus` with `-T66AccountTab=Suspension` and `t66.AccountStatus.Force 1`.

## Visible Structural Regions

| Element | Baseline source | Text | Role | x | y | w | h | Notes |
| --- | --- | --- | --- | ---: | ---: | ---: | ---: | --- |
| AccountStatusModal.Root | viewport |  | Root | 0.0000 | 0.0000 | 1.0000 | 1.0000 | Full-screen forced suspension surface. |
| AccountStatusModal.SubTabs | widget 13 | SUSPENSION / OVERVIEW / HISTORY | ToggleGroup.AccountStatusModalTabs | 0.0575 | 0.1424 | 0.8850 | 0.0960 | Three-tab account row below the verified top bar. |
| AccountStatusModal.SubTabs.SuspensionButton | widget 15 | SUSPENSION | Button | 0.0575 | 0.1424 | 0.2850 | 0.0960 | Selected tab in forced suspension state. |
| AccountStatusModal.SubTabs.OverviewButton | widget 24 | OVERVIEW | Button | 0.3575 | 0.1424 | 0.2850 | 0.0960 | Navigates to Overview tab. |
| AccountStatusModal.SubTabs.HistoryButton | widget 33 | HISTORY | Button | 0.6575 | 0.1424 | 0.2850 | 0.0960 | Navigates to History tab. |
| AccountStatusModal.InfoStrip | widget 42 | Review moderation status, appeal state, and flagged run details. | Label panel | 0.0015 | 0.2517 | 0.9970 | 0.0720 | Red informational strip. |
| AccountStatusModal.ContentPanel | widget 47 |  | Panel | 0.0015 | 0.3371 | 0.9970 | 0.6400 | Main suspension content container. |
| AccountStatusModal.Content | widget 48 |  | Content cluster | 0.0180 | 0.3691 | 0.9640 | 0.5787 | Inner content area. |
| AccountStatusModal.SectionLabel | widget 49 | SUSPENSION | Label | 0.0180 | 0.3691 | 0.9640 | 0.0306 | Upper section label. |
| AccountStatusModal.Headline | widget 50 | ACCOUNT SUSPENDED | Label | 0.0180 | 0.4130 | 0.9640 | 0.0380 | Red headline. |
| AccountStatusModal.Description | widget 51 | This account cannot submit leaderboard scores while the suspension is active. | Label | 0.0180 | 0.4589 | 0.9640 | 0.0185 | Body copy. |
| AccountStatusModal.ReasonPanel | widget 52 |  | Panel | 0.0180 | 0.4908 | 0.9640 | 0.0992 | Reason box. |
| AccountStatusModal.ReasonLabel | widget 54 | REASON | Label | 0.0330 | 0.5201 | 0.9340 | 0.0167 | Reason label. |
| AccountStatusModal.ReasonValue | widget 55 | Too Lucky | Label | 0.0330 | 0.5448 | 0.9340 | 0.0185 | Live reason value; baseline forced state is "Too Lucky". |
| AccountStatusModal.AppealStatus | widget 56 | Appeal: Not Submitted | Label | 0.0180 | 0.6033 | 0.9640 | 0.0167 | Live appeal status. |
| AccountStatusModal.AppealButton | widget 60 | APPEAL | Button | 0.0180 | 0.6360 | 0.9640 | 0.0427 | Opens the appeal editor when available. |

## Structural Rules

- Preserve each visible region within +/- 0.02 normalized x/y/w/h.
- Preserve the verified `top_bar` dump section without modifying `UT66FrontendTopBarWidget`.
- Replace legacy reference chrome with `FT66FlatStyle` helpers in the Suspension reachable path.
- Keep the Suspension/Overview/History row as a separate toggle group from the top bar category row.
- Treat hidden appeal-editor controls as interaction state, not baseline-visible structure.
