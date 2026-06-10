# Copyright Tribulation 66. All Rights Reserved.
#
# MotionRig telemetry analyzer (MOTION_RIG.md section 4 — the feel rubric).
# Consumes a telemetry CSV written by UT66MotionRigScenario and scores the
# measurable rubric axes plus the anti-jank invariants. Targets marked (p)
# in MOTION_RIG.md are provisional until the user's taste pass.
#
#   python Scripts/MotionRig/AnalyzeTelemetry.py <telemetry.csv> [--json out.json]

import argparse
import csv
import json
import math
import sys


def load_rows(path):
    rows = []
    with open(path, newline="") as f:
        for row in csv.DictReader(f):
            parsed = {"state": row["state"]}
            for key, value in row.items():
                if key == "state":
                    continue
                try:
                    parsed[key] = float(value)
                except ValueError:
                    parsed[key] = 0.0
            rows.append(parsed)
    return rows


def spans_of_state(rows, state):
    spans = []
    start = None
    for i, r in enumerate(rows):
        if r["state"] == state and start is None:
            start = i
        elif r["state"] != state and start is not None:
            spans.append((start, i))
            start = None
    if start is not None:
        spans.append((start, len(rows)))
    return spans


def planar_speed(r):
    return math.hypot(r["bean_vx"], r["bean_vy"])


def analyze(rows):
    result = {}
    notes = []

    # ---- invariants -------------------------------------------------------
    explosion = any(
        math.dist((r["pelvis_x"], r["pelvis_y"], r["pelvis_z"]),
                  (r["bean_x"], r["bean_y"], r["bean_z"])) > 1000.0
        for r in rows)
    floor_penetration = sum(1 for r in rows if r["pelvis_z"] < (r["bean_z"] - 220.0))
    # jitter: alternating-sign large deltas on a relaxed signal during Idle
    jitter_hits = 0
    idle_spans = spans_of_state(rows, "Idle")
    for a, b in idle_spans:
        seg = rows[a:b]
        deltas = [seg[i + 1]["hand_r_z"] - seg[i]["hand_r_z"] for i in range(len(seg) - 1)]
        for i in range(len(deltas) - 1):
            if abs(deltas[i]) > 1.5 and abs(deltas[i + 1]) > 1.5 and (deltas[i] * deltas[i + 1] < 0):
                jitter_hits += 1
    result["invariants"] = {
        "no_explosion": not explosion,
        "floor_penetration_frames": floor_penetration,
        "idle_jitter_events": jitter_hits,
        "pass": (not explosion) and floor_penetration == 0 and jitter_hits < 6,
    }

    # ---- axis 1: cadence / foot slide -------------------------------------
    walk_samples = [r for r in rows if r["state"] == "Walk" and r["grounded"] > 0.5 and planar_speed(r) > 150.0]
    if walk_samples:
        ratios = [min(r["foot_l_speed"], r["foot_r_speed"]) / max(planar_speed(r), 1.0) for r in walk_samples]
        slide = sum(ratios) / len(ratios)
        result["cadence_foot_slide"] = {
            "stance_foot_speed_ratio": round(slide, 3),
            "target": "< 0.45 (provisional; 0 = perfect plant, 1 = full skate)",
            "pass": slide < 0.45,
            "samples": len(walk_samples),
        }
    else:
        notes.append("no walk samples for cadence axis")

    # ---- axis 2: acceleration lean ----------------------------------------
    walk_spans = spans_of_state(rows, "Walk")
    lean = None
    if walk_spans:
        a, b = walk_spans[0]
        accel_window = rows[a:min(b, a + 50)]  # ~first 0.8s of first walk
        if accel_window:
            lean = max(abs(r["bean_pitch"]) for r in accel_window)
            result["accel_lean"] = {
                "peak_abs_bean_pitch_deg": round(lean, 2),
                "target": "4..25 deg (provisional)",
                "pass": 4.0 <= lean <= 25.0,
            }
    if lean is None:
        notes.append("no walk span for lean axis")

    # ---- axis 4: jump shape -----------------------------------------------
    jump_spans = spans_of_state(rows, "Jump")
    jumps = []
    for a, b in jump_spans:
        seg = rows[a:b]
        if len(seg) < 5:
            continue
        t0 = seg[0]["t"]
        apex_t = None
        for i in range(1, len(seg)):
            if seg[i - 1]["bean_vz"] > 0 >= seg[i]["bean_vz"]:
                apex_t = seg[i]["t"] - t0
                break
        landed = [r for r in seg if r["grounded"] > 0.5 and (r["t"] - t0) > 0.15]
        absorb = None
        if landed:
            land_t = landed[0]["t"]
            post = [r for r in seg if land_t <= r["t"] <= land_t + 0.5]
            if post:
                base = seg[0]["pelvis_z"] - seg[0]["bean_z"]
                absorb = max(0.0, base - min(r["pelvis_z"] - r["bean_z"] for r in post))
        jumps.append({"time_to_apex_s": apex_t, "landing_absorb_cm": absorb})
    if jumps:
        apexes = [j["time_to_apex_s"] for j in jumps if j["time_to_apex_s"]]
        result["jump_shape"] = {
            "jumps": jumps,
            "mean_time_to_apex_s": round(sum(apexes) / len(apexes), 3) if apexes else None,
            "target": "apex 0.30..0.60s; absorb 4..30cm (provisional)",
            "pass": bool(apexes) and all(0.30 <= t <= 0.60 for t in apexes),
        }

    # ---- axis 5: dive signature -------------------------------------------
    dive_spans = spans_of_state(rows, "Dive")
    if dive_spans:
        a, b = dive_spans[0]
        seg = rows[a:b]
        launch = None
        for r in seg[:20]:
            sp = planar_speed(r)
            if sp > 250.0:
                launch = math.degrees(math.atan2(r["bean_vz"], sp))
                break
        grounded_seg = [r for r in seg if r["grounded"] > 0.5]
        slide_cm = 0.0
        if len(grounded_seg) >= 2:
            slide_cm = math.dist(
                (grounded_seg[0]["bean_x"], grounded_seg[0]["bean_y"]),
                (grounded_seg[-1]["bean_x"], grounded_seg[-1]["bean_y"]))
        # time-to-feet: dive start until first Idle/Walk after the dive
        t_start = seg[0]["t"]
        t_feet = None
        for r in rows[b:]:
            if r["state"] in ("Idle", "Walk"):
                t_feet = r["t"] - t_start
                break
        result["dive_signature"] = {
            "launch_angle_deg": round(launch, 1) if launch is not None else None,
            "prone_slide_cm": round(slide_cm, 1),
            "time_to_feet_s": round(t_feet, 2) if t_feet else None,
            "target": "angle 15..45; slide 60..260cm; to-feet < 4.5s (provisional)",
            "pass": launch is not None and 15.0 <= launch <= 45.0
                    and (t_feet or 99) < 4.5,
        }

    # ---- axis 6: knockdown ---------------------------------------------
    kd_spans = spans_of_state(rows, "Knockdown")
    if kd_spans:
        a, b = kd_spans[0]
        seg = rows[a:b]
        t0 = seg[0]["t"]
        settle_t = None
        run = 0
        for r in seg:
            if r["pelvis_speed"] < 120.0:
                run += 1
                if run >= 12 and settle_t is None:  # ~0.2s sustained
                    settle_t = r["t"] - t0
            else:
                run = 0
        getup_spans = spans_of_state(rows, "GetUp")
        getup_s = None
        for ga, gb in getup_spans:
            if rows[ga]["t"] >= rows[a]["t"]:
                getup_s = rows[min(gb, len(rows) - 1)]["t"] - rows[ga]["t"]
                break
        result["knockdown"] = {
            "settle_s": round(settle_t, 2) if settle_t else None,
            "knockdown_total_s": round(seg[-1]["t"] - t0, 2),
            "getup_s": round(getup_s, 2) if getup_s else None,
            "target": "settle < 2.5s; getup 1.0..2.5s (provisional)",
            "pass": settle_t is not None and settle_t < 2.5,
        }

    # ---- axis 7: upright spring (post-landing wobble) ----------------------
    # Count bean pitch sign flips in the 1.5s after each jump landing.
    overshoots = []
    for a, b in jump_spans:
        seg = rows[a:b]
        landed = [i for i, r in enumerate(seg) if r["grounded"] > 0.5 and (r["t"] - seg[0]["t"]) > 0.15]
        if not landed:
            continue
        li = landed[0]
        t_land = seg[li]["t"]
        window = [r for r in rows if t_land <= r["t"] <= t_land + 1.5]
        flips = 0
        prev = None
        for r in window:
            sign = 1 if r["bean_pitch"] > 1.0 else (-1 if r["bean_pitch"] < -1.0 else 0)
            if prev is not None and sign != 0 and prev != 0 and sign != prev:
                flips += 1
            if sign != 0:
                prev = sign
        overshoots.append(flips)
    if overshoots:
        result["upright_spring"] = {
            "post_landing_pitch_flips": overshoots,
            "target": "1..3 flips (provisional: visible wobble, no oscillation tail)",
            "pass": all(f <= 4 for f in overshoots),
        }

    # ---- axis 8: responsiveness --------------------------------------------
    # First sustained input is the walkcircle 'walk fwd' step at t=0.5.
    moving = [r for r in rows if planar_speed(r) > 50.0]
    if moving and rows:
        t_first_move = moving[0]["t"]
        latency = t_first_move - 0.5
        if 0.0 <= latency <= 2.0:
            result["responsiveness"] = {
                "input_to_motion_s": round(latency, 3),
                "target": "< 0.25s (provisional)",
                "pass": latency < 0.25,
            }

    result["notes"] = notes
    axis_passes = [v.get("pass") for k, v in result.items() if isinstance(v, dict) and "pass" in v]
    result["summary"] = {
        "axes_evaluated": len(axis_passes),
        "axes_passed": sum(1 for p in axis_passes if p),
    }
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("csv_path")
    parser.add_argument("--json", dest="json_out", default=None)
    args = parser.parse_args()

    rows = load_rows(args.csv_path)
    if not rows:
        print("EMPTY telemetry CSV")
        sys.exit(1)

    result = analyze(rows)
    text = json.dumps(result, indent=2)
    print(text)
    if args.json_out:
        with open(args.json_out, "w") as f:
            f.write(text)
    print(f"MOTIONRIG_ANALYZE_SUMMARY={result['summary']['axes_passed']}/{result['summary']['axes_evaluated']} axes pass; invariants={'PASS' if result['invariants']['pass'] else 'FAIL'}")


if __name__ == "__main__":
    main()
