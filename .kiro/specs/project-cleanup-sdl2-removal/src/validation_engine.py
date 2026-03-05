"""ValidationEngine для перевірки працездатності SDL2 Cleanup System"""

import os
import re
import subprocess
from typing import Tuple, Dict, List
from pathlib import Path

from .exceptions import ValidationError
from .models import CleanupConfig


class ValidationEngine:
    """Клас для перевірки працездатності проекту після очищення
    
    Відповідає за:
    - Перевірку наявності критичних файлів
    - Перевірку відсутності SDL2 включень у коді
    - Тестування компіляції проекту
    - Виконання повного набору перевірок
    """
    
    def __init__(self, project_root: str = ".", config: CleanupConfig = None):
        """Ініціалізація ValidationEngine
        
        Args:
            project_root: Кореневий каталог проекту
            config: Конфігурація очищення (опціонально)
        """
        self.project_root = Path(project_root)
        self.config = config or CleanupConfig()
        
        # Патерни для пошуку SDL2 включень
        self.sdl2_include_patterns = [
            re.compile(r'#include\s*[<"]SDL2/.*[>"]'),
            re.compile(r'#include\s*[<"]SDL\.h[>"]'),
            re.compile(r'#include\s*[<"]SDL_.*\.h[>"]')
        ]
    
    def verify_critical_files(self) -> Tuple[bool, List[str]]:
        """Перевіряє наявність всіх критичних файлів
        
        Returns:
            (success, missing_files): Результат перевірки та список відсутніх файлів
        """
        missing_files = []
        
        for file_path in self.config.critical_files:
            full_path = self.project_root / file_path
            
            if not full_path.exists():
                missing_files.append(file_path)
        
        success = len(missing_files) == 0
        return success, missing_files
    
    def check_sdl2_includes(self, directory: str) -> Dict[str, List[str]]:
        """Перевіряє відсутність SDL2 включень у коді
        
        Args:
            directory: Директорія для перевірки
            
        Returns:
            includes: Словник {файл: [знайдені включення SDL2]}
        """
        includes = {}
        full_path = self.project_root / directory
        
        if not full_path.exists():
            return includes
        
        # Шукаємо в .cpp та .h файлах
        for root, dirs, files in os.walk(full_path):
            # Пропускаємо .git, node_modules, __pycache__ тощо
            dirs[:] = [d for d in dirs if not d.startswith('.') and d not in ['node_modules', '__pycache__']]
            
            for file in files:
                if file.endswith(('.cpp', '.h', '.hpp', '.c')):
                    file_path = Path(root) / file
                    found_includes = self._find_sdl2_includes_in_file(file_path)
                    
                    if found_includes:
                        relative_path = str(file_path.relative_to(self.project_root))
                        includes[relative_path] = found_includes
        
        return includes
    
    def _find_sdl2_includes_in_file(self, file_path: Path) -> List[str]:
        """Знаходить SDL2 включення у файлі
        
        Args:
            file_path: Шлях до файлу
            
        Returns:
            includes: Список знайдених включень
        """
        found_includes = []
        
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                for line_num, line in enumerate(f, 1):
                    # Перевіряємо чи містить рядок SDL2 включення
                    for pattern in self.sdl2_include_patterns:
                        if pattern.search(line):
                            found_includes.append(f"Line {line_num}: {line.strip()}")
                            break
        except Exception as e:
            # Ігноруємо файли, які не можна прочитати
            pass
        
        return found_includes
    
    def test_compilation(self, script_path: str) -> Tuple[bool, str]:
        """Тестує компіляцію проекту
        
        Args:
            script_path: Шлях до скрипта компіляції
            
        Returns:
            (success, output): Результат компіляції та вивід
        """
        full_path = self.project_root / script_path
        
        if not full_path.exists():
            return False, f"Скрипт компіляції не знайдено: {script_path}"
        
        try:
            # Визначаємо команду залежно від розширення файлу
            if script_path.endswith('.bat'):
                # Windows batch файл
                result = subprocess.run(
                    [str(full_path)],
                    cwd=str(full_path.parent),
                    capture_output=True,
                    text=True,
                    timeout=300,  # 5 хвилин таймаут
                    shell=True
                )
            elif script_path.endswith('.sh'):
                # Unix shell скрипт
                result = subprocess.run(
                    ['bash', str(full_path)],
                    cwd=str(full_path.parent),
                    capture_output=True,
                    text=True,
                    timeout=300
                )
            else:
                return False, f"Непідтримуваний тип скрипта: {script_path}"
            
            # Перевіряємо код повернення
            success = result.returncode == 0
            
            # Формуємо вивід
            output = f"Return code: {result.returncode}\n"
            if result.stdout:
                output += f"STDOUT:\n{result.stdout}\n"
            if result.stderr:
                output += f"STDERR:\n{result.stderr}\n"
            
            return success, output
            
        except subprocess.TimeoutExpired:
            return False, "Компіляція перевищила таймаут (5 хвилин)"
        except Exception as e:
            return False, f"Помилка при виконанні компіляції: {e}"
    
    def run_validation_suite(self) -> Dict[str, bool]:
        """Виконує повний набір перевірок
        
        Returns:
            results: Словник {назва_перевірки: результат}
        """
        results = {}
        
        # 1. Перевірка критичних файлів
        critical_files_ok, missing_files = self.verify_critical_files()
        results['critical_files'] = critical_files_ok
        
        if not critical_files_ok:
            # Якщо критичні файли відсутні, не продовжуємо
            results['critical_files_details'] = f"Відсутні файли: {', '.join(missing_files)}"
            return results
        
        # 2. Перевірка відсутності SDL2 включень у cpp/src/
        sdl2_includes = self.check_sdl2_includes('cpp/src')
        results['no_sdl2_includes'] = len(sdl2_includes) == 0
        
        if sdl2_includes:
            results['sdl2_includes_details'] = sdl2_includes
        
        # 3. Перевірка main.cpp окремо
        main_cpp_includes = self.check_sdl2_includes('cpp/src/main.cpp')
        results['main_cpp_clean'] = len(main_cpp_includes) == 0
        
        # 4. Перевірка компіляції (опціонально, може бути довгою)
        # Перевіряємо чи існує compile.bat
        compile_script = self.project_root / 'cpp' / 'compile.bat'
        if compile_script.exists():
            # Не запускаємо компіляцію автоматично, тільки перевіряємо наявність
            results['compile_script_exists'] = True
        else:
            results['compile_script_exists'] = False
        
        # 5. Перевірка документації
        readme_path = self.project_root / 'cpp' / 'README.md'
        if readme_path.exists():
            results['readme_exists'] = True
            
            # Перевіряємо чи README містить SDL2 згадки
            sdl2_in_readme = self._check_file_for_sdl2(readme_path)
            results['readme_clean'] = not sdl2_in_readme
        else:
            results['readme_exists'] = False
            results['readme_clean'] = False
        
        # 6. Перевірка SETUP.md
        setup_path = self.project_root / 'cpp' / 'SETUP.md'
        if setup_path.exists():
            results['setup_exists'] = True
        else:
            results['setup_exists'] = False
        
        return results
    
    def _check_file_for_sdl2(self, file_path: Path) -> bool:
        """Перевіряє чи містить файл SDL2 згадки
        
        Args:
            file_path: Шлях до файлу
            
        Returns:
            bool: True якщо містить SDL2 згадки
        """
        sdl2_patterns = [
            re.compile(r'SDL2', re.IGNORECASE),
            re.compile(r'\bSDL\b', re.IGNORECASE)
        ]
        
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                return any(pattern.search(content) for pattern in sdl2_patterns)
        except Exception:
            return False
