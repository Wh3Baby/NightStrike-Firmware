#!/bin/bash
# Скрипт для создания приватного репозитория на GitHub

set -e

REPO_NAME="NightStrike-Firmware"
GITHUB_USER="wh3baby"

echo "🌑 Настройка GitHub репозитория для NightStrike Firmware"
echo ""

# Проверка GitHub CLI
if command -v gh &> /dev/null; then
    echo "✅ GitHub CLI установлен"
    echo ""
    echo "Для создания приватного репозитория выполните:"
    echo "  gh auth login"
    echo "  gh repo create $REPO_NAME --private --source=. --remote=origin --push"
    echo ""
    echo "Или используйте Personal Access Token:"
    echo "  git remote add origin https://github.com/$GITHUB_USER/$REPO_NAME.git"
    echo "  git push -u origin main"
else
    echo "⚠️ GitHub CLI не установлен"
    echo ""
    echo "Создайте репозиторий вручную:"
    echo "1. Перейдите на https://github.com/new"
    echo "2. Название: $REPO_NAME"
    echo "3. Выберите: Private"
    echo "4. НЕ инициализируйте с README (у нас уже есть)"
    echo "5. Нажмите 'Create repository'"
    echo ""
    echo "Затем выполните:"
    echo "  git remote add origin https://github.com/$GITHUB_USER/$REPO_NAME.git"
    echo "  git branch -M main"
    echo "  git push -u origin main"
    echo ""
    echo "Для 2FA используйте Personal Access Token вместо пароля"
fi
