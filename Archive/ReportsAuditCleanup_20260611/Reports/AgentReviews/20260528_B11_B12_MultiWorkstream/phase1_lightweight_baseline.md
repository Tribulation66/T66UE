# Phase 1 Lightweight Baseline

Date: 2026-05-28T11:14:25.2367000-03:00

## Gate Result

- Accepted rows: 3/3
- MedianFPS: 192.801542357369
- MeanFPS: 193.70755406725
- StdevFPS: 5.80086307978863
- Max PerformanceSystem overhead: 1011.4 us
- Executable SHA256: 86EDE7D6F2533614D9E0525305230BC06CD468F1547642A47C6A2B5C1613C9F5
- Median >= 170: True
- Every row >= 160: True
- Stdev <= 20: True
- Every row fired/hit >= 10: True
- Zero non-zero exits / overhead rejects / HeroDeath: True

## Notes

- RouteBossOrGuardianObserved=1 in each row is expected for the placed guardian/non-director actor and is not a basic-mob rich-route leak.
- Row 3 observed git-lfs before capture and after capture, but the runner waited before starting the Unreal process and after exit. No row was accepted with a staged-exe hash change or PerformanceSystem overhead rejection.

## Rows

``csv
"Run","AvgFPS","PerfSystemOverheadMaxUs","ProjectileManagerFired","ProjectileManagerHitHero","HeroHP","PeakLightweight","PeakRich","RouteBossOrGuardianObserved","RouteRangedRoutedLightweightBasic","RichSpawns","LightweightSpawns","GitLfsPreCaptureObserved","GitLfsPostCaptureActive","ExeSHA256"
"1","199.9081132761505","837","45","45","19560","90","0","1","26","0","26","False","True","86EDE7D6F2533614D9E0525305230BC06CD468F1547642A47C6A2B5C1613C9F5"
"2","192.8015423573691","1011.4","26","26","19580","90","0","1","25","0","25","False","False","86EDE7D6F2533614D9E0525305230BC06CD468F1547642A47C6A2B5C1613C9F5"
"3","188.41300656822918","920.2","63","63","19400","90","0","1","22","0","22","True","True","86EDE7D6F2533614D9E0525305230BC06CD468F1547642A47C6A2B5C1613C9F5"
``
