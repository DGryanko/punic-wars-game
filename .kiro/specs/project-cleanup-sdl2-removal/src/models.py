"""Моделі даних для SDL2 Cleanup System"""

from dataclasses import dataclass, field
from typing import List, Dict, Optional, Any


@dataclass
class CleanupConfig:
    """Конфігурація для операції очищення
    
    Attributes:
        files_to_delete: Список файлів для видалення
        files_to_clean: Список файлів для очищення від SDL2 згадок
        protected_files: Файли, які мають залишитися незмінними
        critical_files: Критичні файли для перевірки
        search_directories: Директорії для пошуку
        branch_to_delete: Git гілка для видалення
        report_path: Шлях для збереження звіту
        backup_directory: Директорія для резервних копій
    """
    
    # Файли для видалення
    files_to_delete: List[str] = field(default_factory=lambda: [
        "cpp/SDL2_MIGRATION_TODO.md",
        "cpp/QUICK_START_SDL2.md"
    ])
    
    # Файли для очищення від SDL2 згадок
    files_to_clean: List[str] = field(default_factory=lambda: [
        "ТЗ від грока",
        "Переписка з грок вихідні дані.txt",
        "cpp/README.md",
        "SESSION_CONTEXT.md"
    ])
    
    # Файли, які мають залишитися незмінними
    protected_files: List[str] = field(default_factory=lambda: [
        "cpp/SETUP.md",
        "cpp/src/main.cpp",
        "cpp/compile.bat"
    ])
    
    # Критичні файли для перевірки
    critical_files: List[str] = field(default_factory=lambda: [
        "cpp/src/main.cpp",
        "cpp/compile.bat",
        "cpp/README.md",
        "cpp/SETUP.md"
    ])
    
    # Директорії для пошуку
    search_directories: List[str] = field(default_factory=lambda: [
        "cpp/",
        "."
    ])
    
    # Git гілка для видалення
    branch_to_delete: str = "SDL2"
    
    # Шлях для збереження звіту
    report_path: str = ".kiro/specs/project-cleanup-sdl2-removal/cleanup_report.md"
    
    # Директорія для резервних копій
    backup_directory: str = ".kiro/specs/project-cleanup-sdl2-removal/backups"


@dataclass
class CleanupResult:
    """Результат операції очищення
    
    Attributes:
        success: Чи успішна операція
        commit_hash: SHA хеш створеного commit
        deleted_files: Список видалених файлів
        modified_files: Словник модифікованих файлів з описом змін
        git_operations: Список виконаних git операцій
        validation_results: Результати перевірок
        errors: Список помилок
        warnings: Список попереджень
        backup_directory: Шлях до директорії з резервними копіями
        report_path: Шлях до згенерованого звіту
    """
    
    success: bool
    commit_hash: str
    deleted_files: List[str]
    modified_files: Dict[str, str]
    git_operations: List[str]
    validation_results: Dict[str, bool]
    errors: List[str]
    warnings: List[str]
    backup_directory: str
    report_path: str


@dataclass
class FileReference:
    """Інформація про згадку SDL2 у файлі
    
    Attributes:
        file_path: Шлях до файлу
        line_number: Номер рядка
        line_content: Вміст рядка
        context_before: Рядки контексту перед згадкою
        context_after: Рядки контексту після згадки
        is_historical: Чи є це частиною історичного контексту
    """
    
    file_path: str
    line_number: int
    line_content: str
    context_before: List[str]
    context_after: List[str]
    is_historical: bool


@dataclass
class ValidationResult:
    """Результат перевірки працездатності
    
    Attributes:
        check_name: Назва перевірки
        passed: Чи пройшла перевірка
        message: Повідомлення про результат
        details: Додаткові деталі (опціонально)
    """
    
    check_name: str
    passed: bool
    message: str
    details: Optional[Dict[str, Any]] = None
