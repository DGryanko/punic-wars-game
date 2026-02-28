#!/usr/bin/env python3
"""
Генератор простого placeholder тайлсету для ізометричної карти
Створює файл assets/isometric_tileset.png розміром 256x128
"""

from PIL import Image, ImageDraw

def create_isometric_tileset():
    # Створюємо зображення 256x128
    img = Image.new('RGBA', (256, 128), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Кольори для кожного типу місцевості
    colors = {
        'grass': (76, 175, 80, 255),    # Зелений
        'water': (33, 150, 243, 255),   # Синій
        'sand': (245, 222, 179, 255),   # Бежевий
        'road': (139, 69, 19, 255)      # Коричневий
    }
    
    # Функція для малювання ізометричного ромба
    def draw_isometric_tile(x_offset, y_offset, color):
        # Координати вершин ромба (відносно offset)
        points = [
            (x_offset + 64, y_offset + 0),   # Верх
            (x_offset + 128, y_offset + 32), # Право
            (x_offset + 64, y_offset + 64),  # Низ
            (x_offset + 0, y_offset + 32)    # Ліво
        ]
        
        # Малюємо заповнений ромб
        draw.polygon(points, fill=color, outline=(0, 0, 0, 255))
    
    # Малюємо 4 тайли
    # Трава (0, 0)
    draw_isometric_tile(0, 0, colors['grass'])
    
    # Вода (128, 0)
    draw_isometric_tile(128, 0, colors['water'])
    
    # Пісок (0, 64)
    draw_isometric_tile(0, 64, colors['sand'])
    
    # Дорога (128, 64)
    draw_isometric_tile(128, 64, colors['road'])
    
    # Додаємо текст для ідентифікації (опціонально)
    from PIL import ImageFont
    try:
        font = ImageFont.truetype("arial.ttf", 12)
    except:
        font = ImageFont.load_default()
    
    # Підписи (білий текст з чорним обведенням)
    labels = [
        ("G", 60, 28, colors['grass']),
        ("W", 188, 28, colors['water']),
        ("S", 60, 92, colors['sand']),
        ("R", 188, 92, colors['road'])
    ]
    
    for text, x, y, bg_color in labels:
        # Чорне обведення
        for dx in [-1, 0, 1]:
            for dy in [-1, 0, 1]:
                if dx != 0 or dy != 0:
                    draw.text((x + dx, y + dy), text, fill=(0, 0, 0, 255), font=font)
        # Білий текст
        draw.text((x, y), text, fill=(255, 255, 255, 255), font=font)
    
    # Зберігаємо
    img.save('assets/isometric_tileset.png')
    print("✅ Тайлсет створено: assets/isometric_tileset.png")
    print("📐 Розмір: 256x128 пікселів")
    print("🎨 4 ізометричні тайли (Трава, Вода, Пісок, Дорога)")
    print("\n⚠️  Це простий placeholder!")
    print("💡 Для кращого вигляду створіть власний тайлсет з текстурами.")
    print("📖 Дивіться assets/TILESET_INSTRUCTIONS.md для інструкцій.")

if __name__ == "__main__":
    try:
        create_isometric_tileset()
    except Exception as e:
        print(f"❌ Помилка: {e}")
        print("\n💡 Переконайтеся, що встановлено Pillow:")
        print("   pip install Pillow")
