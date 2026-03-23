"""
UI Panel Simulator — real-time layout preview for Punic Wars panels.
Requires: pip install pygame
Run: python ui_simulator.py
"""
import pygame
import sys
import os

pygame.init()

SCREEN_W, SCREEN_H = 1280, 720
screen = pygame.display.set_mode((SCREEN_W, SCREEN_H), pygame.RESIZABLE)
pygame.display.set_caption("UI Panel Simulator — Punic Wars")
clock = pygame.time.Clock()

# ── Fonts ──────────────────────────────────────────────────────────────────
font_sm = pygame.font.SysFont("consolas", 13)
font_md = pygame.font.SysFont("consolas", 16)
font_lg = pygame.font.SysFont("consolas", 22, bold=True)

# ── Asset paths ────────────────────────────────────────────────────────────
ASSET_DIR = os.path.join(os.path.dirname(__file__), "cpp", "assets", "sprites", "ui")

def try_load(path):
    if os.path.exists(path):
        try:
            return pygame.image.load(path).convert_alpha()
        except Exception as e:
            print(f"[WARN] Could not load {path}: {e}")
    return None

panel_bg_img   = try_load(os.path.join(ASSET_DIR, "panel_bg.png"))
btn_attack_img = try_load(os.path.join(ASSET_DIR, "buttons", "btn_attack_hover.png"))

# ── Slider helper ──────────────────────────────────────────────────────────
class Slider:
    def __init__(self, x, y, w, label, vmin, vmax, value):
        self.rect  = pygame.Rect(x, y, w, 18)
        self.label = label
        self.vmin, self.vmax = vmin, vmax
        self.value = value
        self.dragging = False

    def handle(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN and self.rect.collidepoint(event.pos):
            self.dragging = True
        if event.type == pygame.MOUSEBUTTONUP:
            self.dragging = False
        if event.type == pygame.MOUSEMOTION and self.dragging:
            ratio = (event.pos[0] - self.rect.x) / self.rect.w
            self.value = int(self.vmin + ratio * (self.vmax - self.vmin))
            self.value = max(self.vmin, min(self.vmax, self.value))

    def draw(self, surf):
        pygame.draw.rect(surf, (60, 60, 60), self.rect)
        ratio = (self.value - self.vmin) / (self.vmax - self.vmin)
        fill = pygame.Rect(self.rect.x, self.rect.y, int(self.rect.w * ratio), self.rect.h)
        pygame.draw.rect(surf, (140, 110, 50), fill)
        pygame.draw.rect(surf, (200, 180, 100), self.rect, 1)
        lbl = font_sm.render(f"{self.label}: {self.value}", True, (230, 220, 180))
        surf.blit(lbl, (self.rect.x, self.rect.y - 16))

# ── Panel params (sliders) ─────────────────────────────────────────────────
SX = 20   # sidebar x
SY = 20   # sidebar y start
SW = 200  # slider width

sliders = {
    "panel_x":  Slider(SX, SY+30,  SW, "panel_x",  0,   800,  10),
    "panel_y":  Slider(SX, SY+80,  SW, "panel_y",  0,   600,  500),
    "panel_w":  Slider(SX, SY+130, SW, "panel_w",  200, 1100, 714),
    "panel_h":  Slider(SX, SY+180, SW, "panel_h",  80,  400,  196),
    "icon_pad": Slider(SX, SY+230, SW, "icon_pad", 4,   30,   12),
    "btn_gap":  Slider(SX, SY+280, SW, "btn_gap",  2,   20,   6),
    "font_sz":  Slider(SX, SY+330, SW, "font_sz",  7,   20,   10),
}

# ── Colors ─────────────────────────────────────────────────────────────────
COL_BG        = (34, 85, 34)
COL_PANEL_FB  = (30, 22, 14)       # fallback panel bg
COL_PANEL_BRD = (180, 160, 100)
COL_BTN       = (30, 22, 12)
COL_BTN_ACT   = (100, 80, 30)
COL_BTN_BRD   = (120, 100, 60)
COL_BTN_ABRD  = (255, 210, 60)
COL_ICON_BG   = (20, 15, 10)
COL_HP_BG     = (40, 40, 40)
COL_HP_FG     = (60, 200, 60)

# ── State ──────────────────────────────────────────────────────────────────
active_behavior  = 0   # 0=Stand 1=Passive 2=Active 3=Guard 4=Scout 5=ScoutAtk
active_formation = -1  # -1=none 0=Line 1=Square 2=Chess
hp_pct = 0.75

# ── Draw helpers ───────────────────────────────────────────────────────────
def draw_panel_bg(surf, rect, img):
    if img:
        scaled = pygame.transform.scale(img, (rect.w, rect.h))
        surf.blit(scaled, rect.topleft)
    else:
        pygame.draw.rect(surf, COL_PANEL_FB, rect)
        pygame.draw.rect(surf, COL_PANEL_BRD, rect, 2)

def draw_btn(surf, rect, label, icon_img, is_active, fsz):
    bg  = COL_BTN_ACT if is_active else COL_BTN
    brd = COL_BTN_ABRD if is_active else COL_BTN_BRD
    pygame.draw.rect(surf, bg, rect)
    pygame.draw.rect(surf, brd, rect, 1)
    if is_active:
        outer = rect.inflate(2, 2)
        pygame.draw.rect(surf, (255, 200, 50), outer, 1)

    if icon_img:
        icon_r = rect.inflate(-6, -6)
        scaled = pygame.transform.scale(icon_img, (icon_r.w, icon_r.h))
        tint_surf = scaled.copy()
        if is_active:
            tint = pygame.Surface(tint_surf.get_size(), pygame.SRCALPHA)
            tint.fill((255, 230, 120, 60))
            tint_surf.blit(tint, (0, 0))
        surf.blit(tint_surf, icon_r.topleft)
    else:
        f = pygame.font.SysFont("consolas", fsz)
        txt = f.render(label, True, (255, 220, 80) if is_active else (200, 200, 200))
        tx = rect.x + (rect.w - txt.get_width()) // 2
        ty = rect.y + (rect.h - txt.get_height()) // 2
        surf.blit(txt, (tx, ty))

def compute_layout(px, py, pw, ph, pad, gap):
    icon_w = ph - pad * 2
    bx     = px + icon_w + pad + 8
    avail  = (px + pw - 8) - bx

    bh = (ph - pad * 2 - 2 * gap) / 3.0

    bw1 = (avail - 3 * gap) / 4.0
    bw2 = (avail - 2 * gap) / 3.0
    bw3 = (avail - 1 * gap) / 2.0

    by = py + pad

    row1 = [pygame.Rect(int(bx + i*(bw1+gap)), int(by),              int(bw1), int(bh)) for i in range(4)]
    row2 = [pygame.Rect(int(bx + i*(bw2+gap)), int(by + bh + gap),   int(bw2), int(bh)) for i in range(3)]
    row3 = [pygame.Rect(int(bx + i*(bw3+gap)), int(by + 2*(bh+gap)), int(bw3), int(bh)) for i in range(2)]

    icon_rect = pygame.Rect(px + pad, py + pad, icon_w, icon_w)
    return icon_rect, row1, row2, row3

def draw_panel(surf, sv):
    px   = sv["panel_x"]
    py   = sv["panel_y"]
    pw   = sv["panel_w"]
    ph   = sv["panel_h"]
    pad  = sv["icon_pad"]
    gap  = sv["btn_gap"]
    fsz  = sv["font_sz"]

    panel_rect = pygame.Rect(px, py, pw, ph)
    draw_panel_bg(surf, panel_rect, panel_bg_img)

    icon_rect, row1, row2, row3 = compute_layout(px, py, pw, ph, pad, gap)

    # Icon
    pygame.draw.rect(surf, COL_ICON_BG, icon_rect)
    f_big = pygame.font.SysFont("consolas", max(12, icon_rect.h // 2), bold=True)
    lbl = f_big.render("L", True, (255, 255, 255))
    surf.blit(lbl, (icon_rect.x + (icon_rect.w - lbl.get_width()) // 2,
                    icon_rect.y + (icon_rect.h - lbl.get_height()) // 2))

    # Count
    cnt = font_sm.render("x3", True, (180, 180, 180))
    surf.blit(cnt, (icon_rect.x + 2, icon_rect.bottom + 3))

    # HP bar
    hp_rect = pygame.Rect(icon_rect.x, icon_rect.bottom + 18, icon_rect.w, 8)
    pygame.draw.rect(surf, COL_HP_BG, hp_rect)
    pygame.draw.rect(surf, COL_HP_FG, pygame.Rect(hp_rect.x, hp_rect.y, int(hp_rect.w * hp_pct), hp_rect.h))

    # Row 1: behavior
    r1_labels = ["Stand", "Passive", "Active", "Guard"]
    for i, r in enumerate(row1):
        icon = btn_attack_img if (i == 2 and btn_attack_img) else None
        draw_btn(surf, r, r1_labels[i], icon, active_behavior == i, fsz)

    # Row 2: formation
    r2_labels = ["Line", "Square", "Chess"]
    for i, r in enumerate(row2):
        draw_btn(surf, r, r2_labels[i], None, active_formation == i, fsz)

    # Row 3: scout
    r3_labels = ["Scout", "Scout Atk"]
    for i, r in enumerate(row3):
        draw_btn(surf, r, r3_labels[i], None, active_behavior == 4 + i, fsz)

    # Dimension overlay
    dim = font_sm.render(f"{pw}x{ph}  btn_h≈{int((ph - pad*2 - 2*gap)/3)}", True, (255, 255, 100))
    surf.blit(dim, (px + 4, py - 18))

    return row1, row2, row3

# ── Sidebar ────────────────────────────────────────────────────────────────
def draw_sidebar(surf):
    pygame.draw.rect(surf, (20, 20, 20), pygame.Rect(0, 0, 240, SCREEN_H))
    title = font_lg.render("UI Simulator", True, (220, 200, 120))
    surf.blit(title, (10, 4))
    hint = font_sm.render("Drag sliders | Click buttons", True, (140, 140, 140))
    surf.blit(hint, (10, SY + 10))
    for s in sliders.values():
        s.draw(surf)

    # HP slider (manual)
    hp_lbl = font_sm.render(f"hp_pct: {int(hp_pct*100)}%  (↑↓ keys)", True, (180, 220, 180))
    surf.blit(hp_lbl, (SX, SY + 370))

    # Asset status
    y = SY + 400
    for name, img in [("panel_bg", panel_bg_img), ("btn_attack_hover", btn_attack_img)]:
        col = (80, 200, 80) if img else (200, 80, 80)
        txt = font_sm.render(f"{'✓' if img else '✗'} {name}", True, col)
        surf.blit(txt, (SX, y)); y += 18

    # Controls hint
    y += 10
    for line in ["[↑↓] HP", "[R] reset", "[Q/Esc] quit"]:
        surf.blit(font_sm.render(line, True, (140, 140, 140)), (SX, y)); y += 16

# ── Main loop ──────────────────────────────────────────────────────────────
def main():
    global active_behavior, active_formation, hp_pct, SCREEN_W, SCREEN_H

    running = True
    while running:
        sv = {k: s.value for k, s in sliders.items()}

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            if event.type == pygame.VIDEORESIZE:
                SCREEN_W, SCREEN_H = event.w, event.h
            if event.type == pygame.KEYDOWN:
                if event.key in (pygame.K_q, pygame.K_ESCAPE):
                    running = False
                if event.key == pygame.K_UP:
                    hp_pct = min(1.0, hp_pct + 0.05)
                if event.key == pygame.K_DOWN:
                    hp_pct = max(0.0, hp_pct - 0.05)
                if event.key == pygame.K_r:
                    for s in sliders.values():
                        s.value = (s.vmin + s.vmax) // 2

            for s in sliders.values():
                s.handle(event)

            # Click on buttons in preview
            if event.type == pygame.MOUSEBUTTONDOWN:
                px, py = sv["panel_x"], sv["panel_y"]
                pw, ph = sv["panel_w"], sv["panel_h"]
                pad, gap = sv["icon_pad"], sv["btn_gap"]
                _, row1, row2, row3 = compute_layout(px, py, pw, ph, pad, gap)
                for i, r in enumerate(row1):
                    if r.collidepoint(event.pos):
                        active_behavior = i
                for i, r in enumerate(row2):
                    if r.collidepoint(event.pos):
                        active_formation = i if active_formation != i else -1
                for i, r in enumerate(row3):
                    if r.collidepoint(event.pos):
                        active_behavior = 4 + i

        # Draw
        screen.fill(COL_BG)
        draw_panel(screen, sv)
        draw_sidebar(screen)

        pygame.display.flip()
        clock.tick(60)

    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()
