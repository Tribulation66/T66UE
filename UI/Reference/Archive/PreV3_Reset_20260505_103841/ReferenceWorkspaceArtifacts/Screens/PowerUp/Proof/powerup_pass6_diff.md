# PowerUp pass 6 packaged diff

Reference:
- C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\PowerUp.png

Packaged proof:
- C:\UE\T66\UI\Reference\Screens\PowerUp\Proof\powerup_pass6_1672x941.png

Difference list after pass 6:
- live-data: Runtime top bar keeps live coupon value `10`, live portrait, live enabled button states, and runtime navigation labels rather than reference placeholder `--`/placeholder portrait state.
- live-data: Runtime PowerUp cards keep live stat names, diploma rank names, costs, coupon icons, and diploma art; the reference uses placeholder item labels and empty wells.
- layout: The top navigation bar in the packaged runtime is taller than the reference bar, so the PowerUp title is partially occluded by the top bar at the same 1672x941 capture size.
- asset: Top navigation chrome is visibly different from the reference and is not owned by C:\UE\T66\Source\T66\UI\Screens\T66PowerUpScreen.cpp.
- asset: The generated PowerUp-owned card/button/info chrome is closer to the reference than pass 3, but tiny dark-green chroma-key remnants remain at a few outer-frame/card edges.

Blocker:
- The remaining unapproved visible differences require either editing shared/top-bar UI chrome outside the requested target file or another built-in imagegen pass specifically for cleaner alpha/no green fringe. The prompt limits source changes to C:\UE\T66\Source\T66\UI\Screens\T66PowerUpScreen.cpp and target-owned assets only.

Verification commands:
- Built focused target compile: & 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66 Win64 Development 'C:\UE\T66\T66.uproject' -WaitMutex -FromMsBuild -SingleFile='C:\UE\T66\Source\T66\UI\Screens\T66PowerUpScreen.cpp'
- Staged packaged output: $env:_CL_='/wd4458'; & 'C:\UE\T66\Scripts\StageStandaloneBuild.ps1' -ClientConfig Development -SkipCook
- Captured packaged proof: & 'C:\UE\T66\Scripts\CaptureT66UIScreen.ps1' -Screen PowerUp -Output 'C:\UE\T66\UI\Reference\Screens\PowerUp\Proof\powerup_pass6_1672x941.png' -ResX 1672 -ResY 941 -DelaySeconds 4 -TimeoutSeconds 60
