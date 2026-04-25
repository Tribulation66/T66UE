# Screen Pack Template

Copy this checklist into `UI\screens\<screen_slug>\layout\layout_list.md` before starting a new screen.

## Identity

- Screen slug:
- Runtime screen key:
- Source files:
- Authoring baseline: `1920x1080`
- Runtime validation aspects: `16:9`, `16:10`, `21:9`, smaller/windowed

## Image-Generation Inputs

- Main-menu anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Current target screenshot:
- Layout list path:
- Raw imagegen output:
- Normalized reference command: `python C:\UE\T66\Scripts\InvokeDeterministicResample.py <raw_image.png> reference\<screen_slug>_reference_1920x1080.png --target-width 1920 --target-height 1080`

## Layout List

- Region:
- Region:
- Region:

## Live Runtime Content

- Live text:
- Live values:
- Live images/icons/avatars:
- Live 3D/media/preview regions:

## Element And State Checklist

- Element:  | family:  | ownership:  | required states:  | status:
- Element:  | family:  | ownership:  | required states:  | status:
- Element:  | family:  | ownership:  | required states:  | status:

## Reference Gate

- Generated target reference path:
- Reference preserves target layout: `yes/no`
- Reference matches main-menu visual style: `yes/no`
- Approved for sprite/component generation: `yes/no`

## Packaged Review

- Latest packaged capture:
- Aspect validation captures:
- Latest diff/review notes:
- Current verdict: `blocked / close with blockers / exact enough`
