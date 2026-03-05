"""Report Generator для SDL2 Cleanup System"""

from typing import List, Dict
from datetime import datetime
import os


class ReportGenerator:
    """Генератор звітів про операції очищення
    
    Відповідає за створення детального звіту про всі виконані операції
    під час очищення проекту від SDL2 файлів та документації.
    """
    
    def __init__(self):
        """Ініціалізація генератора звітів"""
        self._deleted_files: List[str] = []
        self._modified_files: Dict[str, str] = {}
        self._git_operations: List[str] = []
        self._validation_results: Dict[str, bool] = {}
    
    def add_deleted_files(self, files: List[str]) -> None:
        """Додає інформацію про видалені файли
        
        Args:
            files: Список видалених файлів
        """
        self._deleted_files.extend(files)
    
    def add_modified_files(self, modifications: Dict[str, str]) -> None:
        """Додає інформацію про модифіковані файли
        
        Args:
            modifications: Словник {файл: опис змін}
        """
        self._modified_files.update(modifications)
    
    def add_git_operations(self, operations: List[str]) -> None:
        """Додає інформацію про git операції
        
        Args:
            operations: Список виконаних git операцій
        """
        self._git_operations.extend(operations)
    
    def add_validation_results(self, results: Dict[str, bool]) -> None:
        """Додає результати перевірки працездатності
        
        Args:
            results: Словник {назва_перевірки: результат}
        """
        self._validation_results.update(results)
    
    def generate_report(self, output_path: str) -> None:
        """Генерує та зберігає звіт
        
        Args:
            output_path: Шлях для збереження звіту
        """
        # Створити директорію якщо не існує
        os.makedirs(os.path.dirname(output_path), exist_ok=True)
        
        # Згенерувати вміст звіту
        report_content = self._build_report_content()
        
        # Зберегти звіт
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(report_content)
    
    def _build_report_content(self) -> str:
        """Будує вміст звіту у форматі Markdown
        
        Returns:
            Вміст звіту у форматі Markdown
        """
        lines = []
        
        # Заголовок
        lines.append("# SDL2 Cleanup Report")
        lines.append("")
        lines.append(f"**Generated:** {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        lines.append("")
        
        # Видалені файли
        lines.append("## Deleted Files")
        lines.append("")
        if self._deleted_files:
            for file in self._deleted_files:
                lines.append(f"- {file}")
        else:
            lines.append("*No files were deleted*")
        lines.append("")
        
        # Модифіковані файли
        lines.append("## Modified Files")
        lines.append("")
        if self._modified_files:
            for file, description in self._modified_files.items():
                lines.append(f"### {file}")
                lines.append(f"{description}")
                lines.append("")
        else:
            lines.append("*No files were modified*")
            lines.append("")
        
        # Git операції
        lines.append("## Git Operations")
        lines.append("")
        if self._git_operations:
            for operation in self._git_operations:
                lines.append(f"- {operation}")
        else:
            lines.append("*No git operations were performed*")
        lines.append("")
        
        # Результати валідації
        lines.append("## Validation Results")
        lines.append("")
        if self._validation_results:
            for check_name, passed in self._validation_results.items():
                status = "✓ PASSED" if passed else "✗ FAILED"
                lines.append(f"- **{check_name}**: {status}")
        else:
            lines.append("*No validation checks were performed*")
        lines.append("")
        
        return "\n".join(lines)
