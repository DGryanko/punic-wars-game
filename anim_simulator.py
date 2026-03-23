"""
Animation Simulator - Punic Wars: Castra
Дозволяє переглядати анімації юнітів та будівель без запуску гри.
"""
import pygame
import os
import sys
import math

# ── Конфіг ──────────────────────────────────────────────────────────────────
ASSETS_DIR = os.path.join("cpp", "assets", "sprites", "isometric")
UNITS_DIR  = os.path.join(ASSETS_DIR, "units")
BLDG_DIR   = os.path.join(ASSETS_DIR, "buildings")

WIN_W, WIN_H = 900, 600
BG_COLOR     = (30, 30, 40)
PANEL_COLOR  = (20, 20, 30)
SEL_COLOR    = (255, 200, 50)
TEXT_COLOR   = (220, 220, 220)
DIM_COLOR    = (120, 120, 120)
FRAME_COLOR  = (60, 60, 80)

FPS_MAP = {"idle": 4, "walk": 8, "attack": 12, "death": 6}

# ── Утиліти ──────────────────────────────────────────────────────────────────
def load_spritesheet(path, fw, fh):
    """Завантажує спрайтшіт і повертає список фреймів."""
    if not os.path.exists(path):
        return []
    try:
        sheet = pygame.image.load(path).convert_alpha()
        cols = sheet.get_width() // fw
        rows = sheet.get_height() // fh
        frames = []
        for r in range(rows):
            for c in range(cols):
                surf = pygame.Surface((fw, fh), pygame.SRCALPHA)
                surf.blit(sheet, (0, 0), (c * fw, r * fh, fw, fh))
                frames.append(surf)
        return frames
    except Exception as e:
        print(f"[WARN] Cannot load {path}: {e}")
        return []

def load_frame_folder(folder):
    """Завантажує окремі файли 0.png, 1.png... з папки."""
    frames = []
    i = 0
    while True:
        p = os.path.join(folder, f"{i}.png")
        if not os.path.exists(p):
            break
        try:
            frames.append(pygame.image.load(p).convert_alpha())
        except:
            break
        i += 1
    return frames

def load_single(path):
    """Завантажує один PNG як один фрейм."""
    if os.path.exists(path):
        try:
            return [pygame.image.load(path).convert_alpha()]
        except:
            pass
    return []

def scale_frames(frames, scale):
    if scale == 1.0:
        return frames
    return [pygame.transform.scale(f, (int(f.get_width() * scale), int(f.get_height() * scale)))
            for f in frames]

# ── Дані про юнітів та будівлі ───────────────────────────────────────────────
UNIT_STATES = ["idle", "walk", "attack", "death"]
UNIT_DIRS   = ["front_left", "front_right", "back_left", "back_right"]

def discover_units():
    units = []
    if not os.path.exists(UNITS_DIR):
        return units
    for name in sorted(os.listdir(UNITS_DIR)):
        full = os.path.join(UNITS_DIR, name)
        if os.path.isdir(full):
            units.append(name)
    return units

def discover_buildings():
    bldgs = []
    if not os.path.exists(BLDG_DIR):
        return bldgs
    for f in sorted(os.listdir(BLDG_DIR)):
        if f.lower().endswith(".png"):
            bldgs.append(f[:-4])
    return bldgs

def load_unit_animations(unit_name):
    """Повертає dict: {(state, dir): [frames]}"""
    base = os.path.join(UNITS_DIR, unit_name)
    anims = {}
    fw = fh = 64

    for state in UNIT_STATES:
        if state == "death":
            dirs_to_try = [None]
        else:
            dirs_to_try = UNIT_DIRS

        for d in dirs_to_try:
            key = (state, d)
            fname = state if d is None else f"{state}_{d}"

            # 1. Спрайтшіт у папці юніта
            sheet_path = os.path.join(base, f"{fname}.png")
            frames = load_spritesheet(sheet_path, fw, fh)

            # 2. Папка з окремими файлами (0.png, 1.png...)
            if not frames:
                folder = os.path.join(base, fname)
                frames = load_frame_folder(folder)

            # 3. Папка з одним файлом що має ту саму назву (walk_front_right/walk_front_right.png)
            if not frames:
                nested = os.path.join(base, fname, f"{fname}.png")
                frames = load_spritesheet(nested, fw, fh)
                if not frames:
                    frames = load_single(nested)

            # 4. Fallback: статичний спрайт поруч з папкою юніта (legionary_rome.png)
            if not frames:
                fallback = os.path.join(UNITS_DIR, f"{unit_name}.png")
                frames = load_single(fallback)

            if state == "death":
                for dd in UNIT_DIRS:
                    anims[("death", dd)] = frames
            else:
                anims[key] = frames

    return anims

def load_building_frames(bldg_name):
    path = os.path.join(BLDG_DIR, f"{bldg_name}.png")
    return load_single(path)

# ── UI компоненти ─────────────────────────────────────────────────────────────
class Button:
    def __init__(self, rect, label, font):
        self.rect  = pygame.Rect(rect)
        self.label = label
        self.font  = font
        self.active = False

    def draw(self, surf):
        color = SEL_COLOR if self.active else FRAME_COLOR
        pygame.draw.rect(surf, color, self.rect, border_radius=4)
        pygame.draw.rect(surf, (80, 80, 100), self.rect, 1, border_radius=4)
        txt = self.font.render(self.label, True, (20, 20, 20) if self.active else TEXT_COLOR)
        surf.blit(txt, txt.get_rect(center=self.rect.center))

    def hit(self, pos):
        return self.rect.collidepoint(pos)

class ScrollList:
    """Прокручуваний список кнопок."""
    def __init__(self, x, y, w, h, items, font):
        self.rect   = pygame.Rect(x, y, w, h)
        self.items  = items
        self.font   = font
        self.scroll = 0
        self.selected = 0
        self.item_h = 28

    def draw(self, surf):
        pygame.draw.rect(surf, PANEL_COLOR, self.rect)
        pygame.draw.rect(surf, FRAME_COLOR, self.rect, 1)
        clip = surf.get_clip()
        surf.set_clip(self.rect)
        for i, item in enumerate(self.items):
            iy = self.rect.y + i * self.item_h - self.scroll
            if iy + self.item_h < self.rect.y or iy > self.rect.bottom:
                continue
            color = SEL_COLOR if i == self.selected else PANEL_COLOR
            pygame.draw.rect(surf, color, (self.rect.x, iy, self.rect.w, self.item_h - 2))
            tc = (20, 20, 20) if i == self.selected else TEXT_COLOR
            txt = self.font.render(item, True, tc)
            surf.blit(txt, (self.rect.x + 6, iy + 5))
        surf.set_clip(clip)

    def handle_event(self, e):
        if e.type == pygame.MOUSEBUTTONDOWN and self.rect.collidepoint(e.pos):
            if e.button == 4:
                self.scroll = max(0, self.scroll - self.item_h)
            elif e.button == 5:
                max_scroll = max(0, len(self.items) * self.item_h - self.rect.h)
                self.scroll = min(max_scroll, self.scroll + self.item_h)
            elif e.button == 1:
                idx = (e.pos[1] - self.rect.y + self.scroll) // self.item_h
                if 0 <= idx < len(self.items):
                    self.selected = idx
                    return True
        return False

# ── Головний симулятор ────────────────────────────────────────────────────────
class AnimSimulator:
    def __init__(self):
        pygame.init()
        self.screen = pygame.display.set_mode((WIN_W, WIN_H))
        pygame.display.set_caption("Punic Wars — Animation Simulator")
        self.clock  = pygame.time.Clock()

        self.font_sm = pygame.font.SysFont("segoeui", 14)
        self.font_md = pygame.font.SysFont("segoeui", 16, bold=True)
        self.font_lg = pygame.font.SysFont("segoeui", 20, bold=True)

        self.units    = discover_units()
        self.buildings = discover_buildings()
        self.all_items = (
            [("unit", u) for u in self.units] +
            [("building", b) for b in self.buildings]
        )
        labels = [f"[U] {u}" for u in self.units] + [f"[B] {b}" for b in self.buildings]

        # Ліва панель — список
        self.item_list = ScrollList(10, 50, 200, WIN_H - 60, labels, self.font_sm)

        # Кнопки стану
        states = ["idle", "walk", "attack", "death"]
        self.state_btns = [
            Button((220 + i * 90, 10, 84, 28), s, self.font_sm)
            for i, s in enumerate(states)
        ]
        self.state_btns[0].active = True

        # Кнопки напрямку
        dirs = ["front_left", "front_right", "back_left", "back_right"]
        self.dir_btns = [
            Button((220 + i * 110, 44, 104, 28), d.replace("_", " "), self.font_sm)
            for i, d in enumerate(dirs)
        ]
        self.dir_btns[0].active = True

        # Масштаб
        self.scale_btns = [
            Button((680, 10, 40, 28), f"{s}x", self.font_sm)
            for s in [1, 2, 4, 6]
        ]
        self.scale_btns[2].active = True  # 4x за замовчуванням
        self.scales = [1, 2, 4, 6]
        self.scale  = 4

        # FPS повзунок
        self.fps_slider_rect = pygame.Rect(220, 78, 440, 14)
        self.fps_min = 1
        self.fps_max = 24
        self.fps_dragging = False

        # Стан
        self.cur_state = "idle"
        self.cur_dir   = "front_left"
        self.frames    = []
        self.timer     = 0.0
        self.fps_anim  = 4
        self.loop      = True
        self.paused    = False
        self.show_grid = True
        self.show_info = True
        self.anim_source  = ""
        self.is_fallback  = False

        self._load_selected()

    def _load_selected(self):
        idx = self.item_list.selected
        if idx >= len(self.all_items):
            return
        kind, name = self.all_items[idx]
        self.timer = 0.0

        if kind == "building":
            self.frames = scale_frames(load_building_frames(name), self.scale)
            self.fps_anim = 1
            self.loop = True
            self.cur_state = "static"
            self.anim_source = os.path.join(BLDG_DIR, f"{name}.png")
            self.is_fallback = False
        else:
            anims = load_unit_animations(name)
            key = (self.cur_state, self.cur_dir)
            raw = anims.get(key, [])
            self.frames = scale_frames(raw, self.scale)
            self.fps_anim = FPS_MAP.get(self.cur_state, 8)
            self.loop = (self.cur_state != "death")
            # Визначаємо джерело для відображення
            fname = self.cur_state if self.cur_dir is None else f"{self.cur_state}_{self.cur_dir}"
            sheet = os.path.join(UNITS_DIR, name, f"{fname}.png")
            nested = os.path.join(UNITS_DIR, name, fname, f"{fname}.png")
            fallback = os.path.join(UNITS_DIR, f"{name}.png")
            if os.path.exists(sheet):
                self.anim_source = sheet; self.is_fallback = False
            elif os.path.exists(nested):
                self.anim_source = nested; self.is_fallback = False
            elif os.path.exists(fallback) and raw:
                self.anim_source = fallback; self.is_fallback = True
            else:
                self.anim_source = ""; self.is_fallback = False

    def _set_state(self, state):
        self.cur_state = state
        for b in self.state_btns:
            b.active = (b.label == state)
        self._load_selected()

    def _set_dir(self, d):
        self.cur_dir = d
        label = d.replace("_", " ")
        for b in self.dir_btns:
            b.active = (b.label == label)
        self._load_selected()

    def _set_scale(self, s):
        self.scale = s
        for b, sv in zip(self.scale_btns, self.scales):
            b.active = (sv == s)
        self._load_selected()

    def _update_fps_from_mouse(self, mx):
        r = self.fps_slider_rect
        t = max(0.0, min(1.0, (mx - r.left) / r.width))
        self.fps_anim = max(self.fps_min, round(t * (self.fps_max - self.fps_min) + self.fps_min))

    def handle_events(self):
        for e in pygame.event.get():
            if e.type == pygame.QUIT:
                return False
            if e.type == pygame.KEYDOWN:
                if e.key == pygame.K_ESCAPE:
                    return False
                if e.key == pygame.K_SPACE:
                    self.paused = not self.paused
                if e.key == pygame.K_r:
                    self.timer = 0.0
                if e.key == pygame.K_g:
                    self.show_grid = not self.show_grid
                if e.key == pygame.K_i:
                    self.show_info = not self.show_info

            if self.item_list.handle_event(e):
                self._load_selected()

            if e.type == pygame.MOUSEBUTTONDOWN and e.button == 1:
                for b in self.state_btns:
                    if b.hit(e.pos):
                        self._set_state(b.label)
                for b in self.dir_btns:
                    if b.hit(e.pos):
                        self._set_dir(b.label.replace(" ", "_"))
                for b, s in zip(self.scale_btns, self.scales):
                    if b.hit(e.pos):
                        self._set_scale(s)
                # FPS повзунок
                if self.fps_slider_rect.collidepoint(e.pos):
                    self.fps_dragging = True
                    self._update_fps_from_mouse(e.pos[0])

            if e.type == pygame.MOUSEBUTTONUP and e.button == 1:
                self.fps_dragging = False

            if e.type == pygame.MOUSEMOTION and self.fps_dragging:
                self._update_fps_from_mouse(e.pos[0])

            # Колесо миші у зоні перегляду — змінює масштаб
            if e.type == pygame.MOUSEWHEEL:
                view_rect = pygame.Rect(215, 100, WIN_W - 215, WIN_H - 100)
                if view_rect.collidepoint(pygame.mouse.get_pos()):
                    idx = self.scales.index(self.scale)
                    new_idx = max(0, min(len(self.scales) - 1, idx + e.y))
                    self._set_scale(self.scales[new_idx])
        return True

    def update(self, dt):
        if not self.paused and self.frames:
            if self.loop:
                self.timer += dt
            else:
                max_t = len(self.frames) / max(self.fps_anim, 1)
                if self.timer < max_t:
                    self.timer += dt

    def draw(self):
        self.screen.fill(BG_COLOR)

        # Ліва панель
        pygame.draw.rect(self.screen, PANEL_COLOR, (0, 0, 215, WIN_H))
        label = self.font_md.render("Юніти / Будівлі", True, TEXT_COLOR)
        self.screen.blit(label, (10, 28))
        self.item_list.draw(self.screen)

        # Кнопки
        for b in self.state_btns:
            b.draw(self.screen)
        for b in self.dir_btns:
            b.draw(self.screen)
        for b in self.scale_btns:
            b.draw(self.screen)

        # Підписи
        self.screen.blit(self.font_sm.render("Масштаб:", True, DIM_COLOR), (620, 16))

        # FPS повзунок
        r = self.fps_slider_rect
        t = (self.fps_anim - self.fps_min) / (self.fps_max - self.fps_min)
        # Трек
        pygame.draw.rect(self.screen, FRAME_COLOR, r, border_radius=3)
        # Заповнення
        fill_w = max(6, int(r.width * t))
        pygame.draw.rect(self.screen, (80, 140, 200), (r.x, r.y, fill_w, r.height), border_radius=3)
        # Ручка
        hx = r.x + int(r.width * t)
        pygame.draw.circle(self.screen, SEL_COLOR, (hx, r.centery), 8)
        pygame.draw.circle(self.screen, (20, 20, 30), (hx, r.centery), 5)
        # Підписи
        self.screen.blit(self.font_sm.render(f"{self.fps_min}", True, DIM_COLOR), (r.x - 14, r.y))
        self.screen.blit(self.font_sm.render(f"{self.fps_max}", True, DIM_COLOR), (r.right + 4, r.y))
        fps_lbl = self.font_md.render(f"FPS: {self.fps_anim}", True, SEL_COLOR)
        self.screen.blit(fps_lbl, (r.right + 30, r.y - 2))

        # Область перегляду
        view_rect = pygame.Rect(215, 100, WIN_W - 215, WIN_H - 100)
        pygame.draw.rect(self.screen, (22, 22, 32), view_rect)

        cx = view_rect.centerx
        cy = view_rect.centery

        # Сітка
        if self.show_grid:
            for gx in range(view_rect.left, view_rect.right, 32):
                pygame.draw.line(self.screen, (35, 35, 50), (gx, view_rect.top), (gx, view_rect.bottom))
            for gy in range(view_rect.top, view_rect.bottom, 32):
                pygame.draw.line(self.screen, (35, 35, 50), (view_rect.left, gy), (view_rect.right, gy))
            # Центральний хрест
            pygame.draw.line(self.screen, (50, 50, 70), (cx, view_rect.top), (cx, view_rect.bottom))
            pygame.draw.line(self.screen, (50, 50, 70), (view_rect.left, cy), (view_rect.right, cy))

        # Анімація
        if self.frames:
            total = len(self.frames)
            idx = int(self.timer * self.fps_anim)
            if self.loop:
                idx = idx % total
            else:
                idx = min(idx, total - 1)

            frame = self.frames[idx]
            fw, fh = frame.get_size()
            # Anchor: bottom-center (як в грі)
            draw_x = cx - fw // 2
            draw_y = cy - fh
            self.screen.blit(frame, (draw_x, draw_y))

            # Anchor point
            pygame.draw.circle(self.screen, (255, 80, 80), (cx, cy), 3)

            # Інфо
            if self.show_info:
                idx_name = self.item_list.selected
                kind, name = self.all_items[idx_name]
                fallback_tag = "  [FALLBACK]" if self.is_fallback else ""
                src_short = os.path.basename(self.anim_source) if self.anim_source else "—"
                info_lines = [
                    f"{name}",
                    f"Стан: {self.cur_state}  Напрямок: {self.cur_dir}",
                    f"Фрейм: {idx + 1}/{total}  FPS: {self.fps_anim}",
                    f"Розмір: {fw}x{fh}px  Масштаб: {self.scale}x",
                    f"Файл: {src_short}{fallback_tag}",
                ]
                for li, line in enumerate(info_lines):
                    if li == 0:
                        color = SEL_COLOR
                    elif li == 4 and self.is_fallback:
                        color = (255, 160, 60)
                    else:
                        color = TEXT_COLOR
                    txt = self.font_sm.render(line, True, color)
                    self.screen.blit(txt, (view_rect.left + 8, view_rect.top + 8 + li * 18))

            # Прогрес-бар
            bar_w = view_rect.width - 20
            bar_x = view_rect.left + 10
            bar_y = view_rect.bottom - 20
            pygame.draw.rect(self.screen, FRAME_COLOR, (bar_x, bar_y, bar_w, 8), border_radius=4)
            prog = (idx + 1) / max(total, 1)
            pygame.draw.rect(self.screen, SEL_COLOR, (bar_x, bar_y, int(bar_w * prog), 8), border_radius=4)

        else:
            # Немає фреймів
            msg = self.font_lg.render("Немає спрайтів для цієї анімації", True, (180, 80, 80))
            self.screen.blit(msg, msg.get_rect(center=(cx, cy - 20)))
            idx_name = self.item_list.selected
            kind, name = self.all_items[idx_name]
            if kind == "unit":
                fname = f"{self.cur_state}_{self.cur_dir}"
                expected = os.path.join(UNITS_DIR, name, f"{fname}.png")
                hint1 = self.font_sm.render("Очікуваний файл:", True, DIM_COLOR)
                hint2 = self.font_sm.render(expected, True, (100, 160, 220))
                self.screen.blit(hint1, hint1.get_rect(center=(cx, cy + 14)))
                self.screen.blit(hint2, hint2.get_rect(center=(cx, cy + 32)))

        # Підказки
        hints = "[Space] пауза  [R] перезапуск  [G] сітка  [I] інфо  [Esc] вихід"
        htxt = self.font_sm.render(hints, True, DIM_COLOR)
        self.screen.blit(htxt, (220, WIN_H - 22))

        # Статус паузи
        if self.paused:
            ptxt = self.font_md.render("⏸ ПАУЗА", True, SEL_COLOR)
            self.screen.blit(ptxt, (WIN_W - 100, WIN_H - 22))

        pygame.display.flip()

    def run(self):
        while True:
            dt = self.clock.tick(60) / 1000.0
            if not self.handle_events():
                break
            self.update(dt)
            self.draw()
        pygame.quit()

if __name__ == "__main__":
    sim = AnimSimulator()
    sim.run()
