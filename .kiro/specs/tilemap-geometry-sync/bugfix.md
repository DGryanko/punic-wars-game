# Bugfix Requirements Document

## Introduction

Після адаптації гри до ізометричної проекції виявлено критичну проблему несинхронізації геометрії: система координат для візуалізації (CoordinateConverter з тайлами 128x64) не відповідає системі координат для pathfinding (NavigationGrid з клітинками 16x16). Це призводить до невідповідності між візуальним розташуванням тайлів та областями взаємодії (pathfinding, mouse clicks, collision detection).

Проблема виникла через те, що NavigationGrid була створена для прямокутної проекції і не була адаптована під ізометричну геометрію з тайлами 128x64 пікселів.

## Bug Analysis

### Current Behavior (Defect)

1.1 WHEN NavigationGrid ініціалізується THEN система використовує cellSize = 16 пікселів замість ізометричних тайлів 128x64

1.2 WHEN pathfinding обчислює шлях THEN система використовує прямокутну сітку 16x16 пікселів замість ізометричної сітки тайлів

1.3 WHEN NavigationGrid конвертує координати (worldToGrid/gridToWorld) THEN система використовує прямокутну математику (ділення на 16) замість ізометричної конвертації через CoordinateConverter

1.4 WHEN юніти рухаються по шляху THEN вони рухаються по дрібній сітці 16x16 замість центрів ізометричних тайлів 128x64

1.5 WHEN NavigationGrid.updateFromBuildings() позначає перешкоди THEN система використовує прямокутну сітку 16x16, що не відповідає візуальній геометрії будівель на ізометричних тайлах

### Expected Behavior (Correct)

2.1 WHEN NavigationGrid ініціалізується THEN система SHALL використовувати ізометричну геометрію з тайлами 128x64 пікселів

2.2 WHEN pathfinding обчислює шлях THEN система SHALL використовувати ізометричну сітку тайлів, де кожна клітинка відповідає одному тайлу мапи

2.3 WHEN NavigationGrid конвертує координати (worldToGrid/gridToWorld) THEN система SHALL використовувати CoordinateConverter для ізометричної конвертації

2.4 WHEN юніти рухаються по шляху THEN вони SHALL рухатися по центрах ізометричних тайлів, використовуючи CoordinateConverter.gridToScreen()

2.5 WHEN NavigationGrid.updateFromBuildings() позначає перешкоди THEN система SHALL використовувати GridCoords (row, col) для позначення тайлів як перешкод

2.6 WHEN NavigationGrid перевіряє прохідність THEN система SHALL використовувати TileMap.isPassable(row, col) для визначення прохідності тайлів

### Unchanged Behavior (Regression Prevention)

3.1 WHEN A* pathfinding обчислює шлях THEN алгоритм SHALL CONTINUE TO працювати з 8 напрямками руху та евристикою Manhattan distance

3.2 WHEN PathfindingManager кешує шляхи THEN система SHALL CONTINUE TO інвалідувати кеш при зміні перешкод

3.3 WHEN юніти отримують команду руху через HandleClicks THEN система SHALL CONTINUE TO використовувати GridCoords для цільової позиції

3.4 WHEN NavigationGrid.getNeighbors() повертає сусідів THEN система SHALL CONTINUE TO перевіряти прохідність сусідніх клітинок

3.5 WHEN PathfindingManager обробляє запити THEN система SHALL CONTINUE TO обмежувати кількість обчислень на кадр (maxCalculationsPerFrame)
