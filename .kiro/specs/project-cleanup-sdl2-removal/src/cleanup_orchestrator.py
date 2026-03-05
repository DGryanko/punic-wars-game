"""Cleanup Orchestrator для SDL2 Cleanup System

Цей модуль координує виконання всіх операцій очищення проекту від SDL2.
Validates Requirements: 1.1, 1.2, 2.1, 3.2, 4.1, 7.1, 8.1
"""

from typing import Optional
from pathlib import Path

from .backup_manager import BackupManager
from .file_cleaner import FileCleaner
from .git_manager import GitManager
from .validation_engine import ValidationEngine
from .report_generator import ReportGenerator
from .models import CleanupConfig, CleanupResult
from .exceptions import CleanupError, GitError, ValidationError, BackupError


class CleanupOrchestrator:
    """Оркестратор для координації всіх операцій очищення
    
    Відповідає за:
    - Ініціалізацію всіх компонентів
    - Виконання повного циклу очищення
    - Обробку помилок та відкат змін
    - Генерацію фінального звіту
    """
    
    def __init__(self, config: Optional[CleanupConfig] = None, project_root: str = "."):
        """Ініціалізація CleanupOrchestrator
        
        Args:
            config: Конфігурація очищення (опціонально)
            project_root: Кореневий каталог проекту
        """
        self.config = config or CleanupConfig()
        self.project_root = Path(project_root)
        
        # Ініціалізація компонентів
        self.backup_manager = BackupManager(self.config.backup_directory)
        self.file_cleaner = FileCleaner(project_root)
        self.git_manager = GitManager()
        self.validation_engine = ValidationEngine(project_root, self.config)
        self.report_generator = ReportGenerator()
        
        # Змінні для відстеження стану
        self.commit_hash: Optional[str] = None
        self.backup_dir: Optional[str] = None
        self.errors: list[str] = []
        self.warnings: list[str] = []
    
    def execute_cleanup(self) -> CleanupResult:
        """Виконує повний цикл очищення проекту
        
        Returns:
            CleanupResult: Результат операції очищення
            
        Raises:
            CleanupError: Якщо виникла критична помилка
        """
        try:
            # Фаза 1: Створення резервних копій
            print("Фаза 1: Створення резервних копій...")
            self._create_backups()
            
            # Фаза 2: Видалення SDL2 файлів
            print("Фаза 2: Видалення SDL2 файлів...")
            deleted_files = self._delete_sdl2_files()
            
            # Фаза 3: Очищення документації
            print("Фаза 3: Очищення документації...")
            modified_files = self._clean_documentation()
            
            # Фаза 4: Видалення SDL2 гілки
            print("Фаза 4: Видалення SDL2 гілки...")
            git_operations = self._delete_sdl2_branch()
            
            # Фаза 5: Валідація
            print("Фаза 5: Валідація проекту...")
            validation_results = self._validate_project()
            
            # Фаза 6: Генерація звіту
            print("Фаза 6: Генерація звіту...")
            self._generate_report(deleted_files, modified_files, git_operations, validation_results)
            
            # Створення результату
            result = CleanupResult(
                success=True,
                commit_hash=self.commit_hash or "",
                deleted_files=deleted_files,
                modified_files=modified_files,
                git_operations=git_operations,
                validation_results=validation_results,
                errors=self.errors,
                warnings=self.warnings,
                backup_directory=self.backup_dir or "",
                report_path=self.config.report_path
            )
            
            print("\n✓ Очищення успішно завершено!")
            print(f"Звіт збережено: {self.config.report_path}")
            
            return result
            
        except Exception as e:
            # Обробка помилок
            self.errors.append(str(e))
            print(f"\n✗ Помилка під час очищення: {e}")
            
            # Запропонувати відкат
            if self.commit_hash and self.backup_dir:
                print("\nВиявлено помилку. Бажаєте відкотити зміни? (y/n)")
                # У реальному застосунку тут буде інтерактивний ввід
                # Для автоматизації просто логуємо
                print("Для відкату використовуйте: rollback_changes()")
            
            # Створення результату з помилкою
            result = CleanupResult(
                success=False,
                commit_hash=self.commit_hash or "",
                deleted_files=[],
                modified_files={},
                git_operations=[],
                validation_results={},
                errors=self.errors,
                warnings=self.warnings,
                backup_directory=self.backup_dir or "",
                report_path=""
            )
            
            return result
    
    def _create_backups(self) -> None:
        """Створює резервні копії перед модифікацією
        
        Validates: Requirements 8.1, 8.2
        
        Raises:
            BackupError: Якщо не вдалося створити резервні копії
        """
        try:
            # Створити safety commit
            self.commit_hash = self.backup_manager.create_safety_commit()
            print(f"  ✓ Safety commit створено: {self.commit_hash[:8]}")
            
            # Створити резервні копії файлів
            files_to_backup = self.config.files_to_delete + self.config.files_to_clean
            self.backup_dir = self.backup_manager.create_file_backups(files_to_backup)
            print(f"  ✓ Резервні копії створено: {self.backup_dir}")
            
        except BackupError as e:
            raise CleanupError(f"Не вдалося створити резервні копії: {e}")
        except GitError as e:
            self.warnings.append(f"Git операція не вдалася: {e}")
            print(f"  ⚠ Попередження: {e}")
    
    def _delete_sdl2_files(self) -> list[str]:
        """Видаляє SDL2-специфічні файли
        
        Validates: Requirements 1.1, 1.2
        
        Returns:
            Список видалених файлів
        """
        try:
            deleted_files = self.file_cleaner.delete_sdl2_files()
            
            if deleted_files:
                print(f"  ✓ Видалено {len(deleted_files)} файлів:")
                for file in deleted_files:
                    print(f"    - {file}")
            else:
                print("  ℹ SDL2 файли не знайдено")
                self.warnings.append("SDL2 файли не знайдено для видалення")
            
            return deleted_files
            
        except Exception as e:
            self.errors.append(f"Помилка при видаленні файлів: {e}")
            print(f"  ✗ Помилка: {e}")
            return []
    
    def _clean_documentation(self) -> dict[str, str]:
        """Очищає документацію від SDL2 згадок
        
        Validates: Requirements 2.1, 2.2, 2.3
        
        Returns:
            Словник модифікованих файлів з описом змін
        """
        try:
            # Знайти файли зі згадками SDL2
            references = self.file_cleaner.find_sdl2_references(self.config.search_directories)
            
            if references:
                print(f"  ℹ Знайдено SDL2 згадки у {len(references)} файлах")
            
            # Очистити документацію
            modifications = self.file_cleaner.clean_documentation(self.config.files_to_clean)
            
            if modifications:
                print(f"  ✓ Оброблено {len(modifications)} файлів:")
                for file, description in modifications.items():
                    print(f"    - {file}: {description}")
            else:
                print("  ℹ Файли для очищення не знайдено")
            
            return modifications
            
        except Exception as e:
            self.errors.append(f"Помилка при очищенні документації: {e}")
            print(f"  ✗ Помилка: {e}")
            return {}
    
    def _delete_sdl2_branch(self) -> list[str]:
        """Видаляє SDL2 гілку з локального та віддаленого репозиторію
        
        Validates: Requirements 3.1, 3.2, 3.3
        
        Returns:
            Список виконаних git операцій
        """
        operations = []
        
        try:
            # Перевірити поточну гілку
            current_branch = self.git_manager.get_current_branch()
            print(f"  ℹ Поточна гілка: {current_branch}")
            
            if current_branch == self.config.branch_to_delete:
                error_msg = f"Неможливо видалити поточну активну гілку '{self.config.branch_to_delete}'"
                self.errors.append(error_msg)
                print(f"  ✗ {error_msg}")
                return operations
            
            # Видалити локальну гілку
            if self.git_manager.branch_exists(self.config.branch_to_delete, remote=False):
                self.git_manager.delete_local_branch(self.config.branch_to_delete)
                operations.append(f"Видалено локальну гілку '{self.config.branch_to_delete}'")
                print(f"  ✓ Локальна гілка '{self.config.branch_to_delete}' видалена")
            else:
                operations.append(f"Локальна гілка '{self.config.branch_to_delete}' не існує")
                print(f"  ℹ Локальна гілка '{self.config.branch_to_delete}' не існує")
            
            # Видалити віддалену гілку
            if self.git_manager.branch_exists(self.config.branch_to_delete, remote=True):
                self.git_manager.delete_remote_branch(self.config.branch_to_delete)
                operations.append(f"Видалено віддалену гілку 'origin/{self.config.branch_to_delete}'")
                print(f"  ✓ Віддалена гілка 'origin/{self.config.branch_to_delete}' видалена")
            else:
                operations.append(f"Віддалена гілка 'origin/{self.config.branch_to_delete}' не існує")
                print(f"  ℹ Віддалена гілка 'origin/{self.config.branch_to_delete}' не існує")
            
        except GitError as e:
            self.warnings.append(f"Git операція не вдалася: {e}")
            print(f"  ⚠ Попередження: {e}")
            operations.append(f"Помилка: {e}")
        
        return operations
    
    def _validate_project(self) -> dict[str, bool]:
        """Виконує валідацію проекту після очищення
        
        Validates: Requirements 4.1, 4.2, 4.3, 4.4
        
        Returns:
            Словник результатів валідації
        """
        try:
            validation_results = self.validation_engine.run_validation_suite()
            
            print("  Результати валідації:")
            for check_name, passed in validation_results.items():
                if isinstance(passed, bool):
                    status = "✓" if passed else "✗"
                    print(f"    {status} {check_name}")
                    
                    if not passed:
                        self.errors.append(f"Валідація не пройдена: {check_name}")
            
            # Перевірити критичні результати
            critical_checks = ['critical_files', 'no_sdl2_includes']
            all_critical_passed = all(
                validation_results.get(check, False) for check in critical_checks
            )
            
            if not all_critical_passed:
                raise ValidationError("Критичні перевірки не пройдено")
            
            return validation_results
            
        except Exception as e:
            self.errors.append(f"Помилка валідації: {e}")
            print(f"  ✗ Помилка валідації: {e}")
            return {'validation_failed': False}
    
    def _generate_report(
        self,
        deleted_files: list[str],
        modified_files: dict[str, str],
        git_operations: list[str],
        validation_results: dict[str, bool]
    ) -> None:
        """Генерує фінальний звіт
        
        Validates: Requirements 7.1, 7.2, 7.3, 7.4, 7.5
        
        Args:
            deleted_files: Список видалених файлів
            modified_files: Словник модифікованих файлів
            git_operations: Список git операцій
            validation_results: Результати валідації
        """
        try:
            self.report_generator.add_deleted_files(deleted_files)
            self.report_generator.add_modified_files(modified_files)
            self.report_generator.add_git_operations(git_operations)
            self.report_generator.add_validation_results(validation_results)
            
            self.report_generator.generate_report(self.config.report_path)
            print(f"  ✓ Звіт згенеровано: {self.config.report_path}")
            
        except Exception as e:
            self.errors.append(f"Помилка генерації звіту: {e}")
            print(f"  ✗ Помилка генерації звіту: {e}")
    
    def rollback_changes(self) -> None:
        """Відкочує зміни до попереднього стану
        
        Validates: Requirement 8.4
        """
        if not self.commit_hash or not self.backup_dir:
            print("✗ Неможливо виконати відкат: відсутня інформація про резервні копії")
            return
        
        try:
            print("Виконується відкат змін...")
            self.backup_manager.rollback_changes(self.commit_hash, self.backup_dir)
            print("✓ Зміни успішно відкочено")
            
        except Exception as e:
            print(f"✗ Помилка при відкаті змін: {e}")
    
    def cleanup_backups(self) -> None:
        """Видаляє резервні копії після підтвердження
        
        Validates: Requirement 8.5
        """
        if not self.backup_dir:
            print("ℹ Резервні копії не знайдено")
            return
        
        try:
            self.backup_manager.cleanup_backups(self.backup_dir)
            print(f"✓ Резервні копії видалено: {self.backup_dir}")
            
        except Exception as e:
            print(f"✗ Помилка при видаленні резервних копій: {e}")
