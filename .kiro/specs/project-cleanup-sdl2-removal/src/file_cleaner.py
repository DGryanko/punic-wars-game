"""FileCleaner для роботи з файлами SDL2 Cleanup System"""

import os
import re
from typing import List, Dict
from pathlib import Path

from .exceptions import FileSystemError


class FileCleaner:
    """Клас для видалення SDL2 файлів та очищення документації
    
    Відповідає за:
    - Пошук згадок про SDL2 в файлах
    - Видалення SDL2-специфічних файлів
    - Очищення документації від SDL2 згадок
    - Пошук застарілих файлів
    """
    
    def __init__(self, project_root: str = "."):
        """Ініціалізація FileCleaner
        
        Args:
            project_root: Кореневий каталог проекту
        """
        self.project_root = Path(project_root)
        
        # Файли для видалення
        self.files_to_delete = [
            "cpp/SDL2_MIGRATION_TODO.md",
            "cpp/QUICK_START_SDL2.md"
        ]
        
        # Патерни для пошуку SDL2 згадок (case-insensitive)
        self.sdl2_patterns = [
            re.compile(r'SDL2', re.IGNORECASE),
            re.compile(r'\bSDL\b', re.IGNORECASE),
            re.compile(r'sdl2', re.IGNORECASE),
            re.compile(r'\bsdl\b', re.IGNORECASE)
        ]
    
    def find_sdl2_references(self, search_paths: List[str]) -> Dict[str, List[str]]:
        """Знаходить всі згадки про SDL2 в файлах
        
        Args:
            search_paths: Список директорій для пошуку
            
        Returns:
            references: Словник {файл: [рядки зі згадками SDL2]}
        """
        references = {}
        
        for search_path in search_paths:
            full_path = self.project_root / search_path
            
            if not full_path.exists():
                continue
            
            # Якщо це файл
            if full_path.is_file():
                lines = self._find_sdl2_in_file(full_path)
                if lines:
                    references[str(full_path.relative_to(self.project_root))] = lines
            
            # Якщо це директорія
            elif full_path.is_dir():
                for root, dirs, files in os.walk(full_path):
                    # Пропускаємо .git, node_modules, __pycache__ тощо
                    dirs[:] = [d for d in dirs if not d.startswith('.') and d not in ['node_modules', '__pycache__']]
                    
                    for file in files:
                        # Шукаємо тільки в текстових файлах
                        if self._is_text_file(file):
                            file_path = Path(root) / file
                            lines = self._find_sdl2_in_file(file_path)
                            if lines:
                                references[str(file_path.relative_to(self.project_root))] = lines
        
        return references
    
    def _find_sdl2_in_file(self, file_path: Path) -> List[str]:
        """Знаходить рядки зі згадками SDL2 у файлі
        
        Args:
            file_path: Шлях до файлу
            
        Returns:
            lines: Список рядків зі згадками SDL2
        """
        matching_lines = []
        
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                for line_num, line in enumerate(f, 1):
                    # Перевіряємо чи містить рядок SDL2 згадки
                    if any(pattern.search(line) for pattern in self.sdl2_patterns):
                        matching_lines.append(f"Line {line_num}: {line.rstrip()}")
        except Exception as e:
            # Ігноруємо файли, які не можна прочитати
            pass
        
        return matching_lines
    
    def _is_text_file(self, filename: str) -> bool:
        """Перевіряє чи є файл текстовим
        
        Args:
            filename: Ім'я файлу
            
        Returns:
            bool: True якщо файл текстовий
        """
        text_extensions = {
            '.md', '.txt', '.cpp', '.h', '.hpp', '.c', '.py', '.js', '.ts',
            '.json', '.xml', '.yaml', '.yml', '.ini', '.cfg', '.conf',
            '.sh', '.bat', '.cmd', '.ps1', '.rst', '.html', '.css'
        }
        
        ext = Path(filename).suffix.lower()
        return ext in text_extensions or filename in ['README', 'LICENSE', 'Makefile']
    
    def delete_sdl2_files(self) -> List[str]:
        """Видаляє SDL2-специфічні файли документації
        
        Returns:
            deleted_files: Список видалених файлів
        """
        deleted_files = []
        
        for file_path in self.files_to_delete:
            full_path = self.project_root / file_path
            
            if full_path.exists():
                try:
                    os.remove(full_path)
                    deleted_files.append(file_path)
                except Exception as e:
                    raise FileSystemError(f"Не вдалося видалити файл {file_path}: {e}")
        
        return deleted_files

    
    def clean_documentation(self, files: List[str]) -> Dict[str, str]:
        """Очищає згадки про SDL2 з документації
        
        Args:
            files: Список файлів для очищення
            
        Returns:
            modifications: Словник {файл: опис змін}
        """
        modifications = {}
        
        for file_path in files:
            full_path = self.project_root / file_path
            
            if not full_path.exists():
                modifications[file_path] = "Файл не знайдено"
                continue
            
            try:
                # Читаємо вміст файлу
                with open(full_path, 'r', encoding='utf-8', errors='ignore') as f:
                    original_content = f.read()
                    lines = original_content.split('\n')
                
                # Знаходимо рядки зі згадками SDL2
                modified_lines = []
                changes_count = 0
                
                for line in lines:
                    # Перевіряємо чи містить рядок SDL2 згадки
                    has_sdl2 = any(pattern.search(line) for pattern in self.sdl2_patterns)
                    
                    if has_sdl2:
                        # Перевіряємо чи це історичний контекст
                        if self._is_historical_context(line):
                            # Зберігаємо історичний контекст
                            modified_lines.append(line)
                        else:
                            # Видаляємо або коментуємо рядок
                            # Для документації просто пропускаємо рядок
                            changes_count += 1
                            continue
                    
                    modified_lines.append(line)
                
                # Якщо були зміни, записуємо файл
                if changes_count > 0:
                    modified_content = '\n'.join(modified_lines)
                    with open(full_path, 'w', encoding='utf-8') as f:
                        f.write(modified_content)
                    
                    modifications[file_path] = f"Видалено {changes_count} рядків зі згадками SDL2"
                else:
                    modifications[file_path] = "Згадок SDL2 не знайдено"
                    
            except Exception as e:
                modifications[file_path] = f"Помилка при обробці: {e}"
        
        return modifications
    
    def _is_historical_context(self, line: str) -> bool:
        """Перевіряє чи є рядок частиною історичного контексту
        
        Args:
            line: Рядок для перевірки
            
        Returns:
            bool: True якщо це історичний контекст
        """
        historical_markers = [
            'історія', 'history', 'було', 'was', 'спроба', 'attempt',
            'раніше', 'previously', 'минуле', 'past', 'архів', 'archive'
        ]
        
        line_lower = line.lower()
        return any(marker in line_lower for marker in historical_markers)
    
    def find_legacy_files(self, directory: str) -> List[Dict[str, str]]:
        """Знаходить потенційно застарілі файли
        
        Args:
            directory: Директорія для пошуку
            
        Returns:
            legacy_files: Список словників з інформацією про файли
        """
        legacy_files = []
        full_path = self.project_root / directory
        
        if not full_path.exists():
            return legacy_files
        
        # Патерни для пошуку застарілих файлів
        legacy_patterns = [
            re.compile(r'sdl', re.IGNORECASE),
            re.compile(r'SDL', re.IGNORECASE),
            re.compile(r'old', re.IGNORECASE),
            re.compile(r'backup', re.IGNORECASE),
            re.compile(r'deprecated', re.IGNORECASE),
            re.compile(r'legacy', re.IGNORECASE),
            re.compile(r'unused', re.IGNORECASE),
            re.compile(r'obsolete', re.IGNORECASE)
        ]
        
        for root, dirs, files in os.walk(full_path):
            # Пропускаємо .git, node_modules, __pycache__ тощо
            dirs[:] = [d for d in dirs if not d.startswith('.') and d not in ['node_modules', '__pycache__']]
            
            for file in files:
                file_path = Path(root) / file
                relative_path = file_path.relative_to(self.project_root)
                
                # Перевіряємо ім'я файлу
                if any(pattern.search(file) for pattern in legacy_patterns):
                    legacy_info = {
                        'path': str(relative_path),
                        'name': file,
                        'reason': 'Ім\'я файлу містить застарілі маркери',
                        'recommendation': 'Перевірити та видалити якщо не використовується'
                    }
                    legacy_files.append(legacy_info)
                    continue
                
                # Перевіряємо вміст файлу для .bat та .sh
                if file.endswith(('.bat', '.sh')):
                    if self._contains_sdl2_commands(file_path):
                        legacy_info = {
                            'path': str(relative_path),
                            'name': file,
                            'reason': 'Скрипт містить команди компіляції з SDL2',
                            'recommendation': 'Оновити або видалити скрипт'
                        }
                        legacy_files.append(legacy_info)
        
        return legacy_files
    
    def _contains_sdl2_commands(self, file_path: Path) -> bool:
        """Перевіряє чи містить скрипт команди SDL2
        
        Args:
            file_path: Шлях до файлу
            
        Returns:
            bool: True якщо містить SDL2 команди
        """
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                
                # Шукаємо SDL2 згадки в командах компіляції
                sdl2_command_patterns = [
                    re.compile(r'-lSDL2', re.IGNORECASE),
                    re.compile(r'SDL2\.lib', re.IGNORECASE),
                    re.compile(r'SDL2\.dll', re.IGNORECASE),
                    re.compile(r'SDL2/', re.IGNORECASE),
                    re.compile(r'sdl2-config', re.IGNORECASE)
                ]
                
                return any(pattern.search(content) for pattern in sdl2_command_patterns)
        except Exception:
            return False
