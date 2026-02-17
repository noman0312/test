#!/usr/bin/env python3
"""Detailed grid-forming inverter simulation and dependency-free plotting.

This script simulates a balanced three-phase grid-forming inverter connected
through an L-R filter to a stiff grid. The controller uses:
- P-f droop (active power to frequency)
- Q-V droop (reactive power to voltage magnitude)

To avoid external package limitations, plotting is done by writing a multi-panel
SVG file directly (no numpy/matplotlib required).
"""

from __future__ import annotations

import argparse
import math
from dataclasses import dataclass
from typing import Iterable


@dataclass
class InverterParams:
    f0: float = 50.0
    v_ll_rms: float = 400.0
    Lf: float = 2.5e-3
    Rf: float = 0.12
    Kp_droop: float = 3.0e-5
    Kq_droop: float = 4.0e-4
    tau_pq: float = 0.02
    P_ref_1: float = 8_000.0
    P_ref_2: float = 18_000.0
    Q_ref_1: float = 0.0
    Q_ref_2: float = 4_000.0
    t_end: float = 0.60
    dt: float = 2e-5
    step_time: float = 0.25


def clarke_transform(a: float, b: float, c: float) -> tuple[float, float]:
    alpha = (2.0 / 3.0) * (a - 0.5 * b - 0.5 * c)
    beta = (2.0 / 3.0) * ((math.sqrt(3.0) / 2.0) * (b - c))
    return alpha, beta


def three_phase_from_angle(v_peak: float, theta: float) -> tuple[float, float, float]:
    va = v_peak * math.sin(theta)
    vb = v_peak * math.sin(theta - 2.0 * math.pi / 3.0)
    vc = v_peak * math.sin(theta + 2.0 * math.pi / 3.0)
    return va, vb, vc


def simulate(params: InverterParams) -> dict[str, list[float]]:
    w0 = 2.0 * math.pi * params.f0
    v_phase_rms = params.v_ll_rms / math.sqrt(3.0)
    v_phase_peak_nom = math.sqrt(2.0) * v_phase_rms
    n = int(params.t_end / params.dt)

    t = [k * params.dt for k in range(n)]
    ia = [0.0] * n
    ib = [0.0] * n
    ic = [0.0] * n
    theta = [0.0] * n
    p_f = [0.0] * n
    q_f = [0.0] * n
    omega = [w0] * n
    e_mag = [v_phase_peak_nom] * n

    va_inv = [0.0] * n
    va_grid = [0.0] * n
    p_inst = [0.0] * n
    q_inst = [0.0] * n
    p_ref_hist = [0.0] * n
    q_ref_hist = [0.0] * n

    delta_grid = math.radians(2.0)

    for k in range(n - 1):
        tk = t[k]
        p_ref = params.P_ref_1 if tk < params.step_time else params.P_ref_2
        q_ref = params.Q_ref_1 if tk < params.step_time else params.Q_ref_2
        p_ref_hist[k] = p_ref
        q_ref_hist[k] = q_ref

        omega[k] = w0 - params.Kp_droop * (p_f[k] - p_ref)
        e_mag[k] = v_phase_peak_nom - params.Kq_droop * (q_f[k] - q_ref)

        vai, vbi, vci = three_phase_from_angle(e_mag[k], theta[k])
        th_grid = w0 * tk + delta_grid
        vag, vbg, vcg = three_phase_from_angle(v_phase_peak_nom, th_grid)

        dia = (vai - vag - params.Rf * ia[k]) / params.Lf
        dib = (vbi - vbg - params.Rf * ib[k]) / params.Lf
        dic = (vci - vcg - params.Rf * ic[k]) / params.Lf

        ia[k + 1] = ia[k] + params.dt * dia
        ib[k + 1] = ib[k] + params.dt * dib
        ic[k + 1] = ic[k] + params.dt * dic

        v_alpha, v_beta = clarke_transform(vai, vbi, vci)
        i_alpha, i_beta = clarke_transform(ia[k], ib[k], ic[k])

        p = 1.5 * (v_alpha * i_alpha + v_beta * i_beta)
        q = 1.5 * (v_beta * i_alpha - v_alpha * i_beta)

        p_inst[k] = p
        q_inst[k] = q

        p_f[k + 1] = p_f[k] + params.dt * ((p - p_f[k]) / params.tau_pq)
        q_f[k + 1] = q_f[k] + params.dt * ((q - q_f[k]) / params.tau_pq)

        theta_next = theta[k] + params.dt * omega[k]
        theta[k + 1] = theta_next % (2.0 * math.pi)

        va_inv[k] = vai
        va_grid[k] = vag

    for key in (p_ref_hist, q_ref_hist, omega, e_mag, va_inv, va_grid, p_inst, q_inst):
        key[-1] = key[-2]

    return {
        "t": t,
        "ia": ia,
        "va_inv": va_inv,
        "va_grid": va_grid,
        "p_inst": p_inst,
        "q_inst": q_inst,
        "p_f": p_f,
        "q_f": q_f,
        "omega": omega,
        "E_mag": e_mag,
        "p_ref": p_ref_hist,
        "q_ref": q_ref_hist,
    }


def _min_max(series: Iterable[float]) -> tuple[float, float]:
    vals = list(series)
    lo, hi = min(vals), max(vals)
    if abs(hi - lo) < 1e-12:
        hi += 1.0
        lo -= 1.0
    return lo, hi


def _polyline_points(xs: list[float], ys: list[float], x0: float, y0: float, w: float, h: float) -> str:
    xmin, xmax = xs[0], xs[-1]
    ymin, ymax = _min_max(ys)
    pts = []
    for x, y in zip(xs, ys):
        px = x0 + (x - xmin) / (xmax - xmin) * w
        py = y0 + h - (y - ymin) / (ymax - ymin) * h
        pts.append(f"{px:.2f},{py:.2f}")
    return " ".join(pts)


def write_svg(data: dict[str, list[float]], params: InverterParams, output: str) -> None:
    t = data["t"]
    width, height = 1280, 1400
    margin = 70
    panel_h = (height - 2 * margin - 90) / 4
    panel_w = width - 2 * margin

    panels = [
        [("va_inv", "#1f77b4"), ("va_grid", "#ff7f0e"), ("ia", "#000000")],
        [("p_inst", "#9467bd"), ("p_f", "#2ca02c"), ("p_ref", "#d62728")],
        [("q_inst", "#8c564b"), ("q_f", "#17becf"), ("q_ref", "#bcbd22")],
        [("omega", "#e377c2"), ("E_mag", "#7f7f7f")],
    ]
    titles = [
        "Phase-a voltage/current",
        "Active power response",
        "Reactive power response",
        "Frequency and voltage magnitude",
    ]

    svg = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}">']
    svg.append('<rect width="100%" height="100%" fill="white"/>')
    svg.append('<text x="640" y="40" text-anchor="middle" font-size="24" font-family="Arial">Grid-Forming Inverter Response</text>')

    for i, series in enumerate(panels):
        y = margin + 30 + i * (panel_h + 20)
        svg.append(f'<rect x="{margin}" y="{y}" width="{panel_w}" height="{panel_h}" fill="none" stroke="#cccccc"/>')
        svg.append(f'<text x="{margin + 5}" y="{y - 8}" font-size="16" font-family="Arial">{titles[i]}</text>')
        step_x = margin + (params.step_time - t[0]) / (t[-1] - t[0]) * panel_w
        svg.append(f'<line x1="{step_x:.2f}" y1="{y}" x2="{step_x:.2f}" y2="{y + panel_h}" stroke="#999" stroke-dasharray="5,5"/>')

        lx = margin + 15
        ly = y + 20
        for name, color in series:
            yvals = data[name]
            points = _polyline_points(t, yvals, margin, y, panel_w, panel_h)
            svg.append(f'<polyline points="{points}" fill="none" stroke="{color}" stroke-width="1.4"/>')
            svg.append(f'<line x1="{lx}" y1="{ly}" x2="{lx + 20}" y2="{ly}" stroke="{color}" stroke-width="2"/>')
            svg.append(f'<text x="{lx + 26}" y="{ly + 4}" font-size="12" font-family="Arial">{name}</text>')
            ly += 16

    svg.append(f'<text x="{width/2:.0f}" y="{height-20}" text-anchor="middle" font-size="14" font-family="Arial">Time [s]</text>')
    svg.append('</svg>')

    with open(output, "w", encoding="utf-8") as f:
        f.write("\n".join(svg))


def main() -> None:
    parser = argparse.ArgumentParser(description="Simulate a grid-forming inverter and plot to SVG.")
    parser.add_argument("--output", default="grid_forming_inverter_response.svg", help="Output SVG plot path")
    args = parser.parse_args()

    params = InverterParams()
    data = simulate(params)
    write_svg(data, params, args.output)
    print(f"Saved plot: {args.output}")


if __name__ == "__main__":
    main()
