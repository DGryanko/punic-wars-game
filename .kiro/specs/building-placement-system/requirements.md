# Requirements Document

## Introduction

Система розміщення будівель та взаємодії з ними для ізометричної стратегічної гри. Система забезпечує правильне розміщення будівель на тайловій сітці, блокування тайлів для патфайндингу, спавн головних наметів фракцій, панель замовлення юнітів та коректне відображення ресурсів на UI панелі.

## Glossary

- **Building**: Будівля в грі (намет, казарма, квесторіум тощо)
- **Tile**: Одна клітинка ізометричної тайлової сітки
- **Grid**: Тайлова сітка карти
- **Pathfinding**: Система пошуку шляху для юнітів
- **Faction**: Фракція (Рим або Карфаген)
- **HQ**: Головний намет фракції (Praetorium для Риму, Main Tent для Карфагену)
- **UI_Panel**: Панель користувацького інтерфейсу
- **Resource_Display**: Відображення ресурсів (їжа, гроші)
- **Viewport**: Видима область екрану
- **Unit_Order_Panel**: Панель замовлення юнітів
- **Worker**: Робітник/раб - базовий юніт для збору ресурсів

## Requirements

### Requirement 1: Розміщення будівель на одному тайлі

**User Story:** Як гравець, я хочу щоб будівлі займали рівно один тайл, щоб карта виглядала організовано та передбачувано.

#### Acceptance Criteria

1. WHEN будівля створюється, THE Building_System SHALL розмістити її центр на координатах одного тайла
2. WHEN будівля малюється, THE Rendering_System SHALL відображати спрайт будівлі з прив'язкою до тайлової сітки
3. THE Building_System SHALL зберігати координати будівлі в тайлових координатах (row, col)
4. WHEN будівля розміщується, THE Coordinate_Converter SHALL конвертувати тайлові координати в екранні координати

### Requirement 2: Блокування тайлів під будівлями

**User Story:** Як розробник системи патфайндингу, я хочу щоб тайли під будівлями були заблоковані, щоб юніти не проходили крізь будівлі.

#### Acceptance Criteria

1. WHEN будівля створюється, THE Pathfinding_System SHALL позначити тайл під будівлею як непрохідний
2. WHEN будівля видаляється, THE Pathfinding_System SHALL позначити тайл як прохідний
3. WHEN юніт шукає шлях, THE Pathfinding_System SHALL обходити заблоковані тайли
4. THE Building_System SHALL повідомляти Pathfinding_System про зміни в розміщенні будівель

### Requirement 3: Спавн головних наметів фракцій

**User Story:** Як гравець, я хочу бачити головні намети обох фракцій на карті, щоб розуміти стартові позиції.

#### Acceptance Criteria

1. WHEN карта генерується, THE Map_Generator SHALL вибрати випадковий прохідний тайл для HQ кожної фракції
2. WHEN HQ розміщується, THE Building_System SHALL створити будівлю типу HQ_ROME або HQ_CARTHAGE
3. THE Map_Generator SHALL забезпечити мінімальну відстань між HQ двох фракцій (мінімум 20 тайлів)
4. WHEN HQ створюється, THE Pathfinding_System SHALL заблокувати відповідний тайл

### Requirement 4: Панель замовлення юнітів при кліку на намет

**User Story:** Як гравець, я хочу бачити панель замовлення юнітів коли клікаю на головний намет, щоб створювати робітників.

#### Acceptance Criteria

1. WHEN гравець клікає на HQ своєї фракції, THE UI_System SHALL відобразити Unit_Order_Panel знизу екрану
2. WHEN Unit_Order_Panel відображається для HQ, THE UI_System SHALL показати кнопку замовлення Worker
3. WHEN гравець клікає на кнопку Worker, THE Production_System SHALL почати виробництво Worker якщо є достатньо ресурсів
4. WHEN виробництво завершується, THE Spawn_System SHALL створити Worker поруч з HQ
5. WHEN гравець клікає поза HQ, THE UI_System SHALL приховати Unit_Order_Panel

### Requirement 5: Коректне відображення ресурсів на панелі

**User Story:** Як гравець, я хочу бачити правильну кількість доступних ресурсів на верхній панелі, щоб планувати свої дії.

#### Acceptance Criteria

1. WHEN Resource_Display малюється, THE UI_System SHALL відобразити кількість їжі зліва на панелі (біля амфори)
2. WHEN Resource_Display малюється, THE UI_System SHALL відобразити кількість грошей справа на панелі (біля монети)
3. THE Resource_Display SHALL показувати доступні ресурси (total - reserved)
4. WHEN ресурси змінюються, THE Resource_Display SHALL оновити відображення в реальному часі
5. THE UI_System SHALL НЕ відображати загальну кількість юнітів на Resource_Display

### Requirement 6: Повне відображення тайлової мапи у в'юпорті

**User Story:** Як гравець, я хочу бачити всю тайлову мапу що попадає у в'юпорт, щоб орієнтуватися на місцевості.

#### Acceptance Criteria

1. WHEN карта малюється, THE Rendering_System SHALL відобразити всі тайли що попадають у Viewport
2. THE Rendering_System SHALL розраховувати видимі тайли на основі позиції камери та зуму
3. THE Rendering_System SHALL малювати тайли з невеликим запасом за межами Viewport (culling margin)
4. WHEN камера рухається, THE Rendering_System SHALL динамічно оновлювати список видимих тайлів
5. THE Rendering_System SHALL НЕ обмежувати відображення фіксованою кількістю тайлів (12 тайлів)

### Requirement 7: Оптимізоване відображення мапи

**User Story:** Як розробник, я хочу щоб рендеринг мапи був оптимізованим, щоб гра працювала плавно навіть на великих картах.

#### Acceptance Criteria

1. WHEN карта малюється, THE Rendering_System SHALL використовувати frustum culling для відсікання невидимих тайлів
2. THE Rendering_System SHALL розраховувати видимі тайли один раз на кадр
3. WHEN зум змінюється, THE Rendering_System SHALL перераховувати видимі тайли
4. THE Rendering_System SHALL малювати тільки тайли в межах екрану плюс culling margin
5. THE Rendering_System SHALL підтримувати мінімум 60 FPS на картах розміром 50x50 тайлів
