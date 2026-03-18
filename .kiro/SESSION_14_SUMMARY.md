# Сесія 14 — Система анімацій юнітів (AnimTrack fix)

## Статус: в процесі (анімація ще не відображається)

## Що зроблено

### unit_animator.h — повний рефакторинг
- `AnimTrack` отримав правильний move constructor/assignment що обнуляє `o.sheet = {0}` після переносу
- Заборонено копіювання (`= delete`) — Texture2D є GPU-ресурсом
- `loadSheets` тепер завантажує death окремо для кожного напрямку (не копіює через `=`)
- `loaded` flag перевіряє всі 16 слотів `tracks[4][4]`
- Додано `[ANIM]` логи для діагностики

### main.cpp
- `units.push_back(unit)` → `units.push_back(std::move(unit))` у 3 місцях
- Це необхідно бо `UnitAnimator` містить non-copyable `AnimTrack`

## Поточна проблема

В логу видно:
```
walk_front_right.png → TEXTURE [ID 34] loaded (256x64)
TEXTURE [ID 34] Unloaded   ← одразу вивантажується!
[UNIT] Static sprite loaded for legionary_rome
```

`[ANIM]` логи не з'являються — значить або:
1. Exe не перекомпілювався (але compile.bat каже "Compilation successful")
2. `loadSheets` не викликається через якийсь інший шлях

## Файли спрайтів

- `cpp/assets/sprites/isometric/units/legionary_rome/walk_front_right.png` — 256x64, 4 кадри ✅
- Інші напрямки walk — відсутні (файли не існують)
- idle.png — відсутній

## Наступний крок

Перевірити чи `[ANIM] loadSheets:` з'являється в логу після останньої компіляції.
Якщо ні — шукати чому `loadSheets` не викликається (можливо `animBasePath` неправильний).

## Ключові файли
- `cpp/src/unit_animator.h` — AnimTrack з правильним move
- `cpp/src/unit.h` — виклик `animator.loadSheets(animBasePath, 64, 64)`
- `cpp/assets/sprites/isometric/units/legionary_rome/walk_front_right.png`
