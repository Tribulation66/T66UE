from pathlib import Path
from PIL import Image, ImageEnhance
root = Path(r'C:/UE/T66')
src = root/'UI/Reference/Screens/Achievements/Steam/Working/Pass_02/Candidates/AchievementsSteam_text_free_sheet_pass02_source.png'
runtime = root/'SourceAssets/UI/Reference/Screens/Achievements/Steam'
for sub in ['Panels','Controls','Slots','Buttons/Pill','Buttons/SquareIcon','ScreenArt']:
    (runtime/sub).mkdir(parents=True, exist_ok=True)
img = Image.open(src).convert('RGBA')
# chroma key flat green to alpha, preserving generated UI pixels.
pix = img.load()
w,h=img.size
for y in range(h):
    for x in range(w):
        r,g,b,a=pix[x,y]
        if g > 180 and r < 80 and b < 80:
            pix[x,y]=(0,0,0,0)

def crop(name, box):
    out = img.crop(box)
    # trim transparent border but keep 2 px padding
    bbox = out.getbbox()
    if bbox:
        l,t,r,b = bbox
        pad=2
        l=max(0,l-pad); t=max(0,t-pad); r=min(out.width,r+pad); b=min(out.height,b+pad)
        out=out.crop((l,t,r,b))
    out.save(runtime/name)

crop(Path('Panels/achievements_panels_reference_progress_panel_v2.png'), (38, 45, 1348, 285))
crop(Path('Panels/achievements_panels_reference_row_shell_v2.png'), (26, 334, 1358, 486))
crop(Path('Slots/achievements_slots_reference_square_slot_frame_normal.png'), (56, 539, 199, 678))
crop(Path('Buttons/SquareIcon/normal.png'), (246, 541, 377, 677))
# Button states from generated selected/inactive plates.
crop(Path('Buttons/Pill/selected.png'), (418, 572, 641, 648))
crop(Path('Buttons/Pill/normal.png'), (905, 577, 1125, 646))
crop(Path('Controls/achievements_controls_progress_track_v2.png'), (43, 741, 1191, 799))
crop(Path('Controls/achievements_controls_progress_fill_v2.png'), (43, 839, 894, 895))
crop(Path('Controls/achievements_controls_scrollbar_track_v2.png'), (1414, 472, 1490, 961))
crop(Path('Controls/achievements_controls_scrollbar_thumb_v2.png'), (1305, 725, 1366, 873))
# Duplicate generated plates for states that are visually equivalent in static proof.
for state in ['hover','pressed','disabled']:
    base = Image.open(runtime/'Buttons/Pill/normal.png').convert('RGBA')
    if state == 'hover':
        base = ImageEnhance.Brightness(base).enhance(1.08)
    elif state == 'pressed':
        base = ImageEnhance.Brightness(base).enhance(0.90)
    elif state == 'disabled':
        base = ImageEnhance.Brightness(base).enhance(0.65)
    base.save(runtime/f'Buttons/Pill/{state}.png')
for state in ['hover','pressed','disabled','selected']:
    base = Image.open(runtime/'Buttons/SquareIcon/normal.png').convert('RGBA')
    base.save(runtime/f'Buttons/SquareIcon/{state}.png')
print('wrote runtime assets')
