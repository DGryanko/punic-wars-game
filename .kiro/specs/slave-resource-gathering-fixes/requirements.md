# Requirements Document: Slave Resource Gathering System Fixes

## Introduction

Система збору ресурсів рабами має критичні баги, що роблять автоматичний збір ресурсів неможливим. Раби не обходять будівлі при русі, не здають зібрані ресурси, і не повертаються автоматично до ресурсу після здачі. Цей документ описує вимоги для виправлення цих проблем та створення повноцінного автоматичного циклу збору ресурсів.

## Glossary

- **Slave (Раб)**: Юніт з можливістю збору ресурсів (unit.can_harvest == true)
- **Resource_Point (Ресурсна_Точка)**: Джерело їжі або золота на мапі
- **Dropoff_Building (Будівля_Здачі)**: Questorium або HQ, де раб здає зібрані ресурси
- **Harvest_Cycle (Цикл_Збору)**: Повний цикл: рух до ресурсу → збір → рух до будівлі → здача → повернення до ресурсу
- **NavigationGrid**: Система pathfinding, що визначає прохідність тайлів
- **TileMap**: Карта гри з інформацією про прохідність тайлів
- **GridCoords**: Координати тайлу на сітці (row, col)
- **Building_Obstacle (Перешкода_Будівлі)**: Тайли, зайняті будівлею, що мають бути непрохідними

## Requirements

### Requirement 1: Automatic Resource Gathering Cycle

**User Story:** Як гравець, я хочу щоб раб автоматично збирав ресурси в циклі (збір → здача → повернення), щоб мені не потрібно було постійно давати команди вручну.

#### Acceptance Criteria

1. WHEN гравець відправляє раба до ресурсу (правий клік на ресурсі), THEN раб SHALL автоматично розпочати цикл збору ресурсів
2. WHEN раб досягає ресурсу, THEN раб SHALL автоматично почати збирати ресурс без додаткових команд
3. WHEN раб заповнює інвентар (carrying >= max_carry_capacity), THEN раб SHALL автоматично припинити збір і рухатися до найближчої будівлі здачі
4. WHEN раб досягає будівлі здачі (в межах 3 тайлів), THEN раб SHALL автоматично здати всі зібрані ресурси
5. WHEN раб здає ресурси, THEN раб SHALL автоматично повернутися до призначеного ресурсу і продовжити збір
6. WHEN ресурс вичерпується (depleted == true), THEN раб SHALL зупинити цикл збору і очікувати нових команд
7. WHEN гравець дає рабу нову команду руху, THEN поточний цикл збору SHALL бути скасований

### Requirement 2: Building Obstacle Integration

**User Story:** Як гравець, я хочу щоб юніти обходили будівлі при русі, щоб вони не застрягали і не проходили крізь будівлі.

#### Acceptance Criteria

1. WHEN будівля розміщується на мапі, THEN всі тайли під будівлею SHALL бути позначені як непрохідні в TileMap
2. WHEN будівля видаляється з мапи, THEN всі тайли під будівлею SHALL бути позначені як прохідні в TileMap
3. WHEN NavigationGrid перевіряє прохідність тайлу, THEN система SHALL використовувати TileMap.isPassable() що враховує будівлі
4. WHEN pathfinding обчислює шлях, THEN шлях SHALL обходити всі тайли зайняті будівлями
5. WHEN юніт рухається до цілі, THEN юніт SHALL слідувати шляху що обходить будівлі
6. WHEN гравець намагається відправити юніта на тайл зайнятий будівлею, THEN система SHALL знайти найближчий вільний тайл або відхилити команду

### Requirement 3: Resource Dropoff Logic

**User Story:** Як гравець, я хочу щоб раб надійно здавав зібрані ресурси в будівлю, щоб ресурси не зникали і додавалися до загального запасу фракції.

#### Acceptance Criteria

1. WHEN раб заповнює інвентар, THEN система SHALL знайти найближчу будівлю здачі (Questorium або HQ) гравця
2. WHEN раб рухається до будівлі здачі, THEN раб SHALL використовувати pathfinding для обходу перешкод
3. WHEN раб досягає будівлі здачі (distToBuilding < 3.0 тайлів), THEN раб SHALL здати всі ресурси (food і gold)
4. WHEN раб здає ресурси, THEN ресурси SHALL бути додані до загального запасу фракції (rome_food, rome_money)
5. WHEN раб здає ресурси, THEN інвентар раба SHALL бути очищений (carrying_food = 0, carrying_gold = 0)
6. WHEN раб здає ресурси, THEN раб SHALL отримати команду повернутися до призначеного ресурсу
7. WHEN будівля здачі не знайдена, THEN раб SHALL зупинити збір і вивести помилку в лог

### Requirement 4: Pathfinding Building Awareness

**User Story:** Як розробник, я хочу щоб NavigationGrid синхронізувався з будівлями на мапі, щоб pathfinding завжди враховував актуальні перешкоди.

#### Acceptance Criteria

1. WHEN будівля додається до гри, THEN NavigationGrid.updateFromBuildings() SHALL позначити тайли будівлі як непрохідні в TileMap
2. WHEN будівля видаляється з гри, THEN NavigationGrid.updateFromBuildings() SHALL позначити тайли будівлі як прохідні в TileMap
3. WHEN NavigationGrid.updateFromBuildings() викликається, THEN система SHALL використовувати Building.getGridPosition() та Building.getFootprint() для визначення зайнятих тайлів
4. WHEN тайл позначається як непрохідний, THEN TileMap.setPassable(row, col, false) SHALL бути викликаний
5. WHEN pathfinding обчислює шлях після оновлення будівель, THEN новий шлях SHALL враховувати оновлені перешкоди

### Requirement 5: Resource Assignment Persistence

**User Story:** Як гравець, я хочу щоб раб запам'ятовував призначений ресурс і будівлю здачі, щоб він міг автоматично повертатися після здачі.

#### Acceptance Criteria

1. WHEN раб отримує команду збирати ресурс, THEN система SHALL зберегти GridCoords ресурсу в unit.assigned_resource_position
2. WHEN раб отримує команду збирати ресурс, THEN система SHALL знайти і зберегти GridCoords найближчої будівлі здачі в unit.assigned_dropoff_position
3. WHEN раб здає ресурси, THEN система SHALL використовувати збережені координати для повернення до ресурсу
4. WHEN ресурс вичерпується, THEN збережені координати SHALL бути очищені (assigned_resource_position = invalid)
5. WHEN раб отримує нову команду руху, THEN збережені координати SHALL бути очищені

### Requirement 6: TileMap Passability Management

**User Story:** Як розробник, я хочу щоб TileMap мав методи для управління прохідністю тайлів, щоб будівлі могли динамічно блокувати/розблоковувати тайли.

#### Acceptance Criteria

1. THE TileMap SHALL мати метод setPassable(row, col, passable) для зміни прохідності тайлу
2. WHEN setPassable(row, col, false) викликається, THEN тайл SHALL стати непрохідним для pathfinding
3. WHEN setPassable(row, col, true) викликається, THEN тайл SHALL стати прохідним для pathfinding
4. WHEN isPassable(row, col) викликається, THEN метод SHALL повертати актуальний стан прохідності тайлу
5. WHEN координати виходять за межі мапи, THEN методи SHALL повертати false або ігнорувати виклик

### Requirement 7: Unit State Management

**User Story:** Як розробник, я хочу щоб раб мав чіткі стани для управління циклом збору, щоб логіка була передбачуваною і легко дебажилася.

#### Acceptance Criteria

1. WHEN раб рухається до ресурсу, THEN unit.is_moving SHALL бути true і unit.is_harvesting SHALL бути false
2. WHEN раб збирає ресурс, THEN unit.is_harvesting SHALL бути true і unit.is_moving SHALL бути false
3. WHEN раб рухається до будівлі здачі, THEN unit.is_moving SHALL бути true і unit.is_harvesting SHALL бути false
4. WHEN раб здає ресурси, THEN обидва прапорці SHALL бути false на момент здачі
5. WHEN раб отримує нову команду, THEN is_harvesting SHALL бути скинутий до false

## Notes

- Поточна реалізація має баг: `NavigationGrid::updateFromBuildings()` не оновлює TileMap
- Поточна реалізація має баг: раб не повертається до ресурсу після здачі
- Поточна реалізація має баг: логіка здачі ресурсів не завжди спрацьовує
- Unit клас може потребувати нових полів: `assigned_resource_position`, `assigned_dropoff_position`
- TileMap клас потребує методу `setPassable(row, col, passable)`
