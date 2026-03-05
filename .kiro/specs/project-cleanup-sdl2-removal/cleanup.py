#!/usr/bin/env python3
"""SDL2 Cleanup Tool - Головний скрипт для очищення проекту від SDL2

Цей скрипт виконує автоматизоване очищення проекту "Punic Wars: Castra"
від застарілих файлів та документації, пов'язаних з невдалою міграцією на SDL2.

Usage:
    python cleanup.py [options]

Options:
    --dry-run           Показати що буде зроблено без виконання змін
    --no-backup         Пропустити створення резервних копій (небезпечно!)
    --skip-git          Пропустити видалення SDL2 гілки
    --verbose           Детальний вивід
    -h, --help          Показати це повідомлення
"""

import sys
import argparse
from pathlib import Path

# Додати src директорію до Python path
sys.path.insert(0, str(Path(__file__).parent / "src"))

from src.cleanup_orchestrator import CleanupOrchestrator
from src.models import CleanupConfig
from src.exceptions import CleanupError


class Colors:
    """ANSI кольори для консольного виводу"""
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def print_colored(message: str, color: str = Colors.ENDC) -> None:
    """Виводить кольорове повідомлення
    
    Args:
        message: Текст повідомлення
        color: ANSI код кольору
    """
    print(f"{color}{message}{Colors.ENDC}")


def print_banner() -> None:
    """Виводить банер програми"""
    banner = """
╔═══════════════════════════════════════════════════════════╗
║         SDL2 Cleanup Tool for Punic Wars: Castra         ║
║                                                           ║
║  Автоматизоване очищення проекту від SDL2 файлів         ║
╚═══════════════════════════════════════════════════════════╝
    """
    print_colored(banner, Colors.OKCYAN)


def confirm_action(message: str) -> bool:
    """Запитує підтвердження користувача
    
    Args:
        message: Повідомлення для підтвердження
        
    Returns:
        True якщо користувач підтвердив
    """
    while True:
        response = input(f"{message} (y/n): ").lower().strip()
        if response in ['y', 'yes', 'так', 'т']:
            return True
        elif response in ['n', 'no', 'ні', 'н']:
            return False
        else:
            print("Будь ласка, введіть 'y' або 'n'")


def show_dry_run_info(config: CleanupConfig) -> None:
    """Показує інформацію про те, що буде зроблено
    
    Args:
        config: Конфігурація очищення
    """
    print_colored("\n=== DRY RUN MODE ===", Colors.WARNING)
    print_colored("Наступні операції будуть виконані:\n", Colors.BOLD)
    
    print_colored("1. Створення резервних копій:", Colors.OKBLUE)
    print(f"   - Safety commit у git")
    print(f"   - Резервні копії файлів у {config.backup_directory}")
    
    print_colored("\n2. Видалення SDL2 файлів:", Colors.OKBLUE)
    for file in config.files_to_delete:
        print(f"   - {file}")
    
    print_colored("\n3. Очищення документації:", Colors.OKBLUE)
    for file in config.files_to_clean:
        print(f"   - {file}")
    
    print_colored("\n4. Видалення git гілки:", Colors.OKBLUE)
    print(f"   - Локальна гілка: {config.branch_to_delete}")
    print(f"   - Віддалена гілка: origin/{config.branch_to_delete}")
    
    print_colored("\n5. Валідація проекту:", Colors.OKBLUE)
    print(f"   - Перевірка критичних файлів")
    print(f"   - Перевірка відсутності SDL2 включень")
    print(f"   - Перевірка документації")
    
    print_colored("\n6. Генерація звіту:", Colors.OKBLUE)
    print(f"   - Звіт буде збережено: {config.report_path}")
    
    print_colored("\n===================\n", Colors.WARNING)


def main():
    """Головна функція програми"""
    # Парсинг аргументів командного рядка
    parser = argparse.ArgumentParser(
        description="SDL2 Cleanup Tool - Очищення проекту від SDL2 файлів",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Приклади використання:
  python cleanup.py                    # Звичайне виконання з підтвердженням
  python cleanup.py --dry-run          # Показати що буде зроблено
  python cleanup.py --verbose          # Детальний вивід
  python cleanup.py --skip-git         # Пропустити видалення git гілки
  python cleanup.py --no-backup        # Без резервних копій (небезпечно!)

Примітка: Рекомендується спочатку запустити з --dry-run
        """
    )
    
    parser.add_argument(
        '--dry-run',
        action='store_true',
        help='Показати що буде зроблено без виконання змін'
    )
    
    parser.add_argument(
        '--no-backup',
        action='store_true',
        help='Пропустити створення резервних копій (небезпечно!)'
    )
    
    parser.add_argument(
        '--skip-git',
        action='store_true',
        help='Пропустити видалення SDL2 гілки'
    )
    
    parser.add_argument(
        '--verbose',
        action='store_true',
        help='Детальний вивід'
    )
    
    args = parser.parse_args()
    
    # Вивести банер
    print_banner()
    
    # Створити конфігурацію
    config = CleanupConfig()
    
    # Визначити кореневий каталог проекту (2 рівні вгору від cleanup.py)
    script_dir = Path(__file__).parent
    project_root = script_dir.parent.parent
    
    # Dry run режим
    if args.dry_run:
        show_dry_run_info(config)
        print_colored("Для виконання очищення запустіть без --dry-run", Colors.OKGREEN)
        return 0
    
    # Попередження про no-backup
    if args.no_backup:
        print_colored("\n⚠ УВАГА: Режим --no-backup вимкнено!", Colors.FAIL)
        print_colored("Резервні копії не будуть створені. Це небезпечно!", Colors.WARNING)
        if not confirm_action("Ви впевнені що хочете продовжити?"):
            print_colored("Операцію скасовано", Colors.WARNING)
            return 1
    
    # Підтвердження виконання
    print_colored("\nЦей скрипт виконає наступні операції:", Colors.BOLD)
    print("  1. Створить резервні копії (safety commit + файли)")
    print("  2. Видалить SDL2 файли документації")
    print("  3. Очистить документацію від SDL2 згадок")
    if not args.skip_git:
        print("  4. Видалить SDL2 гілку (локальну та віддалену)")
    else:
        print("  4. Пропустить видалення SDL2 гілки (--skip-git)")
    print("  5. Виконає валідацію проекту")
    print("  6. Згенерує детальний звіт")
    
    print_colored("\n⚠ Переконайтеся що у вас немає незбережених змін!", Colors.WARNING)
    
    if not confirm_action("\nПродовжити виконання?"):
        print_colored("Операцію скасовано", Colors.WARNING)
        return 1
    
    # Створити оркестратор
    try:
        print_colored("\n" + "="*60, Colors.OKBLUE)
        print_colored("Початок очищення проекту...", Colors.BOLD)
        print_colored("="*60 + "\n", Colors.OKBLUE)
        
        orchestrator = CleanupOrchestrator(config, str(project_root))
        
        # Модифікувати конфігурацію якщо потрібно
        if args.skip_git:
            config.branch_to_delete = ""
        
        # Виконати очищення
        result = orchestrator.execute_cleanup()
        
        # Вивести результати
        print_colored("\n" + "="*60, Colors.OKBLUE)
        if result.success:
            print_colored("✓ Очищення успішно завершено!", Colors.OKGREEN)
        else:
            print_colored("✗ Очищення завершено з помилками", Colors.FAIL)
        print_colored("="*60 + "\n", Colors.OKBLUE)
        
        # Детальна інформація
        if args.verbose or not result.success:
            print_colored("Детальна інформація:", Colors.BOLD)
            print(f"  Commit hash: {result.commit_hash}")
            print(f"  Видалено файлів: {len(result.deleted_files)}")
            print(f"  Модифіковано файлів: {len(result.modified_files)}")
            print(f"  Git операцій: {len(result.git_operations)}")
            print(f"  Резервні копії: {result.backup_directory}")
            print(f"  Звіт: {result.report_path}")
            
            if result.errors:
                print_colored("\nПомилки:", Colors.FAIL)
                for error in result.errors:
                    print(f"  ✗ {error}")
            
            if result.warnings:
                print_colored("\nПопередження:", Colors.WARNING)
                for warning in result.warnings:
                    print(f"  ⚠ {warning}")
        
        # Запропонувати видалити резервні копії
        if result.success and result.backup_directory:
            print()
            if confirm_action("Видалити резервні копії?"):
                orchestrator.cleanup_backups()
            else:
                print_colored(f"Резервні копії збережено: {result.backup_directory}", Colors.OKBLUE)
        
        # Запропонувати відкат при помилках
        if not result.success and result.commit_hash:
            print()
            if confirm_action("Відкотити зміни?"):
                orchestrator.rollback_changes()
        
        return 0 if result.success else 1
        
    except CleanupError as e:
        print_colored(f"\n✗ Критична помилка: {e}", Colors.FAIL)
        return 1
    except KeyboardInterrupt:
        print_colored("\n\n✗ Операцію перервано користувачем", Colors.WARNING)
        return 1
    except Exception as e:
        print_colored(f"\n✗ Неочікувана помилка: {e}", Colors.FAIL)
        if args.verbose:
            import traceback
            traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())
