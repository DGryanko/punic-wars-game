"""Backup Manager для SDL2 Cleanup System

Цей модуль забезпечує функціональність резервного копіювання та відкату змін.
Validates Requirements: 8.1, 8.2, 8.3, 8.4, 8.5
"""

import os
import shutil
import subprocess
from typing import List
from pathlib import Path

from .exceptions import BackupError, GitError


class BackupManager:
    """Менеджер для створення резервних копій та відкату змін
    
    Забезпечує:
    - Створення git commit з поточним станом проекту
    - Створення резервних копій файлів у тимчасовій директорії
    - Видалення резервних копій після підтвердження
    - Відкат змін до попереднього стану
    """
    
    def __init__(self, backup_directory: str = ".kiro/specs/project-cleanup-sdl2-removal/backups"):
        """Ініціалізація BackupManager
        
        Args:
            backup_directory: Шлях до директорії для резервних копій
        """
        self.backup_directory = backup_directory
    
    def create_safety_commit(self) -> str:
        """Створює git commit з поточним станом проекту
        
        Validates: Requirement 8.1
        
        Returns:
            commit_hash: SHA хеш створеного commit
            
        Raises:
            BackupError: Якщо не вдалося створити commit
            GitError: Якщо git недоступний або репозиторій не ініціалізований
        """
        try:
            # Перевірка, чи є це git репозиторій
            result = subprocess.run(
                ["git", "rev-parse", "--git-dir"],
                capture_output=True,
                text=True,
                check=False
            )
            
            if result.returncode != 0:
                raise GitError("Поточна директорія не є git репозиторієм")
            
            # Додати всі зміни до staging area
            subprocess.run(
                ["git", "add", "-A"],
                capture_output=True,
                text=True,
                check=True
            )
            
            # Створити commit
            commit_message = "Safety commit before SDL2 cleanup"
            result = subprocess.run(
                ["git", "commit", "-m", commit_message],
                capture_output=True,
                text=True,
                check=False
            )
            
            # Якщо немає змін для commit, отримати поточний commit hash
            if "nothing to commit" in result.stdout or "nothing added to commit" in result.stdout:
                result = subprocess.run(
                    ["git", "rev-parse", "HEAD"],
                    capture_output=True,
                    text=True,
                    check=True
                )
                commit_hash = result.stdout.strip()
            else:
                if result.returncode != 0:
                    raise BackupError(f"Не вдалося створити commit: {result.stderr}")
                
                # Отримати hash створеного commit
                result = subprocess.run(
                    ["git", "rev-parse", "HEAD"],
                    capture_output=True,
                    text=True,
                    check=True
                )
                commit_hash = result.stdout.strip()
            
            return commit_hash
            
        except subprocess.CalledProcessError as e:
            raise BackupError(f"Помилка при створенні safety commit: {e.stderr}")
        except FileNotFoundError:
            raise GitError("Git не знайдено. Переконайтеся, що git встановлено та доступний в PATH")
    
    def create_file_backups(self, files: List[str]) -> str:
        """Створює резервні копії файлів у тимчасовій директорії
        
        Validates: Requirement 8.2, 8.3
        
        Args:
            files: Список шляхів до файлів для резервування
            
        Returns:
            backup_dir: Шлях до директорії з резервними копіями
            
        Raises:
            BackupError: Якщо не вдалося створити резервні копії
        """
        try:
            # Створити директорію для резервних копій, якщо не існує
            backup_path = Path(self.backup_directory)
            backup_path.mkdir(parents=True, exist_ok=True)
            
            # Копіювати кожен файл
            for file_path in files:
                source = Path(file_path)
                
                # Пропустити файли, які не існують
                if not source.exists():
                    continue
                
                # Створити відповідну структуру директорій у backup
                relative_path = source
                destination = backup_path / relative_path
                
                # Створити батьківські директорії
                destination.parent.mkdir(parents=True, exist_ok=True)
                
                # Копіювати файл
                if source.is_file():
                    shutil.copy2(source, destination)
                elif source.is_dir():
                    shutil.copytree(source, destination, dirs_exist_ok=True)
            
            return str(backup_path)
            
        except (OSError, shutil.Error) as e:
            raise BackupError(f"Не вдалося створити резервні копії: {str(e)}")
    
    def cleanup_backups(self, backup_dir: str) -> None:
        """Видаляє тимчасові резервні копії після підтвердження
        
        Validates: Requirement 8.5
        
        Args:
            backup_dir: Шлях до директорії з резервними копіями
            
        Raises:
            BackupError: Якщо не вдалося видалити резервні копії
        """
        try:
            backup_path = Path(backup_dir)
            
            if backup_path.exists() and backup_path.is_dir():
                shutil.rmtree(backup_path)
                
        except (OSError, shutil.Error) as e:
            raise BackupError(f"Не вдалося видалити резервні копії: {str(e)}")
    
    def rollback_changes(self, commit_hash: str, backup_dir: str) -> None:
        """Відкочує зміни до попереднього стану
        
        Validates: Requirement 8.4
        
        Args:
            commit_hash: SHA хеш commit для відкату
            backup_dir: Шлях до директорії з резервними копіями
            
        Raises:
            BackupError: Якщо не вдалося відкотити зміни
            GitError: Якщо git операція не вдалася
        """
        try:
            # Відкотити git до вказаного commit
            result = subprocess.run(
                ["git", "reset", "--hard", commit_hash],
                capture_output=True,
                text=True,
                check=False
            )
            
            if result.returncode != 0:
                raise GitError(f"Не вдалося відкотити git до commit {commit_hash}: {result.stderr}")
            
            # Відновити файли з резервних копій
            backup_path = Path(backup_dir)
            
            if backup_path.exists() and backup_path.is_dir():
                # Пройтися по всіх файлах у backup директорії
                for backup_file in backup_path.rglob("*"):
                    if backup_file.is_file():
                        # Визначити відносний шлях
                        relative_path = backup_file.relative_to(backup_path)
                        destination = Path(relative_path)
                        
                        # Створити батьківські директорії
                        destination.parent.mkdir(parents=True, exist_ok=True)
                        
                        # Відновити файл
                        shutil.copy2(backup_file, destination)
            
        except subprocess.CalledProcessError as e:
            raise BackupError(f"Помилка при відкаті змін: {e.stderr}")
        except (OSError, shutil.Error) as e:
            raise BackupError(f"Не вдалося відновити файли з резервних копій: {str(e)}")
        except FileNotFoundError:
            raise GitError("Git не знайдено. Переконайтеся, що git встановлено та доступний в PATH")
