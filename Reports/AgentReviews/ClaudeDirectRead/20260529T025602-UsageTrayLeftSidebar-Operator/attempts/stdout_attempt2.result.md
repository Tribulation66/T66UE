# Operator Packet — AI Usage Widget → Codex Left Sidebar (below Chats)

Operator=Claude (read-only). Validator/Integrator=Codex. This is a proposal for Codex to integrate, not a greenlight.

## Findings (current state)

- `MainWindow.xaml`: window `Width="340" Height="208"`; 3 stacked rows at `Height="54"` each; label/value `FontSize` are 26–28; badges `MinWidth="104"`, `Padding="12,2"`; outer `Border Padding="12,10"`.
- `MainWindow.xaml.cs` → `PositionWithinCodexWindow` (lines 329–344): widget is pinned **bottom-right** of the Codex window: `targetLeft = rightBottom.X - Width - 40`, `targetTop = rightBottom.Y - Height - 24`, then clamped with `leftTop.X + 16` / `leftTop.Y + 88`.
- The only geometry the placement code consumes from `CodexWindowState` is `WindowRect` and `WorkArea`. **Nothing in the inspected code exposes a sidebar width.** "Exactly match the sidebar width" therefore needs either a tuned constant or a new field on the tracker (Codex's call — outside the 2 inspected files).

## 1. Proposed XAML size/font changes (`MainWindow.xaml`)

Window header (line 5–6):
```xml
Width="248"
Height="150"
```
(`248` = placeholder sidebar width; see Risks. `150` derived from row math below.)

Row definitions (lines 22–24) — shrink 54 → 34:
```xml
<RowDefinition Height="34" />
<RowDefinition Height="34" />
<RowDefinition Height="34" />
```

Inter-row margins — change each `Margin="0,0,0,8"` → `Margin="0,0,0,6"` (rows 0 and 1, lines 27 and 51).

Font sizes — reduce across the five `TextBlock`s:
- `Operator:` label (line 35) `26` → `14`
- `OperatorText` (line 42) `28` → `16`
- `Claude` label (line 59) `28` → `15`
- `ClaudeText` (line 73) `28` → `16`
- `Codex` label (line 91) `28` → `15`
- `CodexText` (line 105) `28` → `16`

Badge tightening so values fit the narrower sidebar:
- `ClaudePercentBadge` (line 67) `MinWidth="104"` → `MinWidth="56"`
- `CodexPercentBadge` (line 98) `MinWidth="104"` → `MinWidth="56"`
- both badge `Padding="12,2"` → `Padding="8,2"` (lines 66, 97)

These are tunable; Codex should eyeball against the live sidebar and nudge.

## 2. Proposed placement changes (`MainWindow.xaml.cs`, `PositionWithinCodexWindow`)

Replace the bottom-right targeting (lines 339–343) with left-edge, below-Chats anchoring:

```csharp
// Match sidebar width exactly (see Risks — sidebar width not exposed by tracker).
Width = CodexSidebarWidthDip;

// Flush to the left edge of the Codex window (sidebar origin).
Left = leftTop.X;

// "Below Chats": pin to the bottom of the sidebar column, just above the window bottom.
Top = Math.Min(rightBottom.Y - Height - 16, leftTop.Y + ChatsHeaderOffsetDip);
```

Add tunable constants (near lines 355–358):
```csharp
private const double CodexSidebarWidthDip = 248;   // exact sidebar width; tune to live UI
private const double ChatsHeaderOffsetDip = 360;    // distance from window top to below the Chats list
```

Notes:
- Keep the existing `transform` / `leftTop` / `rightBottom` DPI math (lines 331–337) unchanged — reuse it.
- Setting `Width` here overrides the XAML literal; XAML `Width` becomes the design-time default only.
- If "below Chats" should mean *directly under the Chats header* rather than *bottom of the column*, use `Top = leftTop.Y + ChatsHeaderOffsetDip;` alone. Which one is intended is the open question (see below).

## 3. Verification commands (Codex runs)

```powershell
dotnet build C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray
dotnet run --project C:\Users\DoPra\Tools\AIUsageTray\src\T66.AIUsageTray
```
Then: open Codex, confirm the widget renders flush against the left sidebar, width equal to the sidebar, sits below Chats, and text is legible without clipping (`TextTrimming="CharacterEllipsis"` already guards the operator name). Use tray menu **"Reposition for Codex"** to force a re-place after resizing the Codex window, and confirm it re-anchors left, not right.

## 4. Risks / caveats

- **Exact sidebar width is not derivable from the inspected code.** `CodexWindowState` only surfaces `WindowRect`/`WorkArea`. "Exactly" the sidebar width requires either (a) a hand-tuned `CodexSidebarWidthDip` constant, or (b) a new sidebar-width field on `CodexWindowTrackerService` — the latter is outside the 2 inspected files and is Codex's integration decision.
- **"Below Chats" Y-offset is a guess** (`ChatsHeaderOffsetDip`). It won't track Codex layout/zoom/DPI changes automatically; the bottom-anchored variant is more robust. Needs the user to confirm intent.
- Flush `Left = leftTop.X` removes the prior `+16` inset; if the widget should hug the sidebar's inner padding, add a small inset.
- Narrower width + smaller badges: re-confirm 3-digit values like `100%` still fit `MinWidth="56"`.
- Font/row numbers are first-pass; expect one tuning round against the live UI.

## 5. Token routing

This run's manifest will supply the Claude token count for routing/attribution.

---

**Open question for the user before Codex integrates:** does "below Chats" mean pinned to the **bottom of the sidebar column** (robust, recommended) or **immediately under the Chats header** (fixed offset, fragile)? Codex needs this to pick the `Top` formula above.
