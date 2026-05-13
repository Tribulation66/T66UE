# Challenges UI Fidelity Checklist

Source geometry: `C:\UE\T66\UI\Geometry\challenges_reference_geometry.md`

Reference notes: `Challanges.png` is the visual authority per Pablo's conflict resolution. The status notification panel is removed. BACK and CONFIRM are top-corner buttons. The visible card row has four cards. A four-dot pagination indicator is retained as a functional affordance below the card row.

## Structure

- [ ] Challenges.Root | exists=true
- [ ] Challenges.TopRow.BackButton | exists=true
- [ ] Challenges.TopRow.BackButton.Icon | exists=true
- [ ] Challenges.TopRow.BackButton.Label | exists=true
- [ ] Challenges.Title | exists=true
- [ ] Challenges.TopRow.ConfirmButton | exists=true
- [ ] Challenges.TopRow.ConfirmButton.Label | exists=true
- [ ] Challenges.TopRow.ConfirmButton.Icon | exists=true
- [ ] Challenges.Tabs.OfficialButton | exists=true
- [ ] Challenges.Tabs.OfficialButton.Icon | exists=true
- [ ] Challenges.Tabs.OfficialButton.Label | exists=true
- [ ] Challenges.Tabs.OfficialButton.InfoIcon | exists=true
- [ ] Challenges.Tabs.CommunityButton | exists=true
- [ ] Challenges.Tabs.CommunityButton.Icon | exists=true
- [ ] Challenges.Tabs.CommunityButton.Label | exists=true
- [ ] Challenges.Tabs.CommunityButton.InfoIcon | exists=true
- [ ] Challenges.Tabs.CreateButton | exists=true
- [ ] Challenges.Tabs.CreateButton.Icon | exists=true
- [ ] Challenges.Tabs.CreateButton.Label | exists=true
- [ ] Challenges.Tabs.CreateButton.InfoIcon | exists=true
- [ ] Challenges.LeftPanel | exists=true
- [ ] Challenges.LeftPanel.Card01 | exists=true
- [ ] Challenges.LeftPanel.Card02 | exists=true
- [ ] Challenges.LeftPanel.Card03 | exists=true
- [ ] Challenges.LeftPanel.Card04 | exists=true
- [ ] Challenges.Pagination | exists=true
- [ ] Challenges.Pagination.Dot01 | exists=true
- [ ] Challenges.Pagination.Dot02 | exists=true
- [ ] Challenges.Pagination.Dot03 | exists=true
- [ ] Challenges.Pagination.Dot04 | exists=true
- [ ] Challenges.RightPanel | exists=true
- [ ] Challenges.RightPanel.Title | exists=true
- [ ] Challenges.RightPanel.OriginLabel | exists=true
- [ ] Challenges.RightPanel.DescriptionHeader | exists=true
- [ ] Challenges.RightPanel.DescriptionText | exists=true
- [ ] Challenges.RightPanel.SkullRatingHeader | exists=true
- [ ] Challenges.RightPanel.SkullRatingIcon | exists=true
- [ ] Challenges.RightPanel.RulesHeader | exists=true
- [ ] Challenges.RightPanel.Rule01 | exists=true
- [ ] Challenges.RightPanel.Rule01.Bullet | exists=true
- [ ] Challenges.RightPanel.Rule02 | exists=true
- [ ] Challenges.RightPanel.Rule02.Bullet | exists=true
- [ ] Challenges.RightPanel.RewardHeader | exists=true
- [ ] Challenges.RightPanel.Reward | exists=true
- [ ] Challenges.RightPanel.Reward.Icon | exists=true
- [ ] Challenges.RightPanel.Reward.Label | exists=true

## Geometry

- [ ] Challenges.TopRow.BackButton | x=0.012 | 0.010
- [ ] Challenges.TopRow.BackButton | y=0.021 | 0.010
- [ ] Challenges.Title | x=0.374 | 0.014
- [ ] Challenges.Title | y=0.033 | 0.014
- [ ] Challenges.TopRow.ConfirmButton | x=0.846 | 0.010
- [ ] Challenges.TopRow.ConfirmButton | y=0.021 | 0.010
- [ ] Challenges.Tabs.OfficialButton | x=0.149 | 0.010
- [ ] Challenges.Tabs.OfficialButton | y=0.129 | 0.010
- [ ] Challenges.Tabs.CommunityButton | x=0.376 | 0.010
- [ ] Challenges.Tabs.CreateButton | x=0.615 | 0.010
- [ ] Challenges.LeftPanel | x=0.027 | 0.010
- [ ] Challenges.LeftPanel | y=0.230 | 0.010
- [ ] Challenges.LeftPanel.Card01 | x=0.038 | 0.010
- [ ] Challenges.LeftPanel.Card01 | y=0.252 | 0.010
- [ ] Challenges.LeftPanel.Card02 | y=0.418 | 0.012
- [ ] Challenges.LeftPanel.Card03 | y=0.586 | 0.012
- [ ] Challenges.LeftPanel.Card04 | y=0.753 | 0.012
- [ ] Challenges.Pagination | y=0.912 | 0.018
- [ ] Challenges.RightPanel | x=0.507 | 0.010
- [ ] Challenges.RightPanel | y=0.231 | 0.010
- [ ] Challenges.RightPanel.Title | x=0.533 | 0.014
- [ ] Challenges.RightPanel.DescriptionText | y=0.397 | 0.014
- [ ] Challenges.RightPanel.SkullRatingIcon | x=0.718 | 0.018
- [ ] Challenges.RightPanel.RulesHeader | y=0.615 | 0.014
- [ ] Challenges.RightPanel.Reward | y=0.864 | 0.014

## Colors

- [ ] Challenges.TopRow.BackButton | button_state=Default
- [ ] Challenges.TopRow.ConfirmButton | button_state=Selected
- [ ] Challenges.Tabs.OfficialButton | button_state=Selected
- [ ] Challenges.Tabs.CommunityButton | button_state=Default
- [ ] Challenges.Tabs.CreateButton | button_state=Default
- [ ] Challenges.LeftPanel | border_color=DefaultBorder
- [ ] Challenges.LeftPanel.Card01 | button_state=Selected
- [ ] Challenges.LeftPanel.Card02 | button_state=Default
- [ ] Challenges.LeftPanel.Card03 | button_state=Default
- [ ] Challenges.LeftPanel.Card04 | button_state=Default
- [ ] Challenges.Pagination.Dot01 | button_state=Selected
- [ ] Challenges.Pagination.Dot02 | button_state=Default
- [ ] Challenges.Pagination.Dot03 | button_state=Default
- [ ] Challenges.Pagination.Dot04 | button_state=Default
- [ ] Challenges.RightPanel | border_color=SelectedBorder

## Content

- [ ] Challenges.TopRow.BackButton.Label | text=BACK
- [ ] Challenges.TopRow.BackButton.Label | is_label=true
- [ ] Challenges.Title | text=CHALLENGES
- [ ] Challenges.Title | is_label=true
- [ ] Challenges.TopRow.ConfirmButton.Label | text=CONFIRM
- [ ] Challenges.TopRow.ConfirmButton.Label | is_label=true
- [ ] Challenges.Tabs.OfficialButton.Label | text=OFFICIAL
- [ ] Challenges.Tabs.CommunityButton.Label | text=COMMUNITY
- [ ] Challenges.Tabs.CreateButton.Label | text=CREATE CHALLENGE
- [ ] Challenges.LeftPanel.Card01.Title | text=GLASS ROUTE
- [ ] Challenges.LeftPanel.Card01.Reward | text=40 CHAD COUPONS
- [ ] Challenges.LeftPanel.Card01.Author | text=TRIBULATION 66
- [ ] Challenges.LeftPanel.Card02.Title | text=PRESSURE RUN
- [ ] Challenges.LeftPanel.Card02.Reward | text=30 CHAD COUPONS
- [ ] Challenges.LeftPanel.Card03.Title | text=LAST STAND
- [ ] Challenges.LeftPanel.Card03.Reward | text=60 CHAD COUPONS
- [ ] Challenges.LeftPanel.Card04.Title | text=APOCALYPSE PROTOCOL
- [ ] Challenges.LeftPanel.Card04.Reward | text=80 CHAD COUPONS
- [ ] Challenges.RightPanel.Title | text=GLASS ROUTE
- [ ] Challenges.RightPanel.OriginLabel | text=Official
- [ ] Challenges.RightPanel.DescriptionHeader | text=DESCRIPTION
- [ ] Challenges.RightPanel.DescriptionText | text=Clear the run without taking a single hit.
- [ ] Challenges.RightPanel.SkullRatingHeader | text=SKULL RATING
- [ ] Challenges.RightPanel.RulesHeader | text=RULES AND REQUIREMENTS
- [ ] Challenges.RightPanel.Rule01 | text=Challenge only completes on a full clear.
- [ ] Challenges.RightPanel.Rule02 | text=Take no damage for the run.
- [ ] Challenges.RightPanel.RewardHeader | text=REWARD
- [ ] Challenges.RightPanel.Reward.Label | text=40 CHAD COUPONS

## Interactivity

- [ ] Challenges.TopRow.BackButton | has_click_handler=true
- [ ] Challenges.TopRow.BackButton | hover_capable=true
- [ ] Challenges.TopRow.ConfirmButton | has_click_handler=true
- [ ] Challenges.TopRow.ConfirmButton | hover_capable=true
- [ ] Challenges.Tabs.OfficialButton | has_click_handler=true
- [ ] Challenges.Tabs.OfficialButton | hover_capable=true
- [ ] Challenges.Tabs.OfficialButton | toggle_group=ChallengeTabs
- [ ] Challenges.Tabs.CommunityButton | has_click_handler=true
- [ ] Challenges.Tabs.CommunityButton | hover_capable=true
- [ ] Challenges.Tabs.CommunityButton | toggle_group=ChallengeTabs
- [ ] Challenges.Tabs.CreateButton | has_click_handler=true
- [ ] Challenges.Tabs.CreateButton | hover_capable=true
- [ ] Challenges.Tabs.CreateButton | toggle_group=ChallengeTabs
- [ ] Challenges.LeftPanel.Card01 | has_click_handler=true
- [ ] Challenges.LeftPanel.Card01 | hover_capable=true
- [ ] Challenges.LeftPanel.Card01 | toggle_group=ChallengeSelection
- [ ] Challenges.LeftPanel.Card02 | has_click_handler=true
- [ ] Challenges.LeftPanel.Card02 | hover_capable=true
- [ ] Challenges.LeftPanel.Card02 | toggle_group=ChallengeSelection
- [ ] Challenges.LeftPanel.Card03 | has_click_handler=true
- [ ] Challenges.LeftPanel.Card03 | hover_capable=true
- [ ] Challenges.LeftPanel.Card03 | toggle_group=ChallengeSelection
- [ ] Challenges.LeftPanel.Card04 | has_click_handler=true
- [ ] Challenges.LeftPanel.Card04 | hover_capable=true
- [ ] Challenges.LeftPanel.Card04 | toggle_group=ChallengeSelection
- [ ] Challenges.Pagination.Dot01 | has_click_handler=true
- [ ] Challenges.Pagination.Dot01 | hover_capable=true
- [ ] Challenges.Pagination.Dot02 | has_click_handler=true
- [ ] Challenges.Pagination.Dot02 | hover_capable=true
- [ ] Challenges.Pagination.Dot03 | has_click_handler=true
- [ ] Challenges.Pagination.Dot03 | hover_capable=true
- [ ] Challenges.Pagination.Dot04 | has_click_handler=true
- [ ] Challenges.Pagination.Dot04 | hover_capable=true
- [ ] Challenges.RightPanel.OriginLabel | is_label=true
- [ ] Challenges.RightPanel.DescriptionText | is_label=true
- [ ] Challenges.RightPanel.Rule01 | is_label=true
- [ ] Challenges.RightPanel.Rule02 | is_label=true
