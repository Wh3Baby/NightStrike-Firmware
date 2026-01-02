#!/bin/bash
# Автоматический коммит с автоопределением типа изменений

set -e

# Цвета
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

info() { echo -e "${GREEN}[INFO]${NC} $1"; }
step() { echo -e "${BLUE}[STEP]${NC} $1"; }

# Проверка git репозитория
if ! git rev-parse --git-dir > /dev/null 2>&1; then
    echo "❌ Не найден git репозиторий!"
    exit 1
fi

# Проверка изменений
if [ -z "$(git status --porcelain)" ]; then
    echo "ℹ️  Нет изменений для коммита"
    exit 0
fi

# Получение измененных файлов
CHANGED_FILES=$(git status --porcelain | awk '{print $2}')

# Автоопределение типа изменений
COMMIT_TYPE="Update"
COMMIT_SCOPE=""

# Проверка изменений в src/
if echo "$CHANGED_FILES" | grep -q "^src/"; then
    if echo "$CHANGED_FILES" | grep -q "src/modules/"; then
        MODULE=$(echo "$CHANGED_FILES" | grep "src/modules/" | head -1 | cut -d'/' -f3)
        COMMIT_TYPE="feat"
        COMMIT_SCOPE="($MODULE)"
    elif echo "$CHANGED_FILES" | grep -q "src/core/"; then
        COMMIT_TYPE="refactor"
        COMMIT_SCOPE="(core)"
    else
        COMMIT_TYPE="update"
    fi
fi

# Проверка изменений в include/
if echo "$CHANGED_FILES" | grep -q "^include/"; then
    COMMIT_TYPE="refactor"
    COMMIT_SCOPE="(headers)"
fi

# Проверка изменений в README
if echo "$CHANGED_FILES" | grep -q "README.md"; then
    COMMIT_TYPE="docs"
    COMMIT_SCOPE="(README)"
fi

# Проверка изменений в platformio.ini или build.sh
if echo "$CHANGED_FILES" | grep -qE "(platformio.ini|build.sh|partitions.csv)"; then
    COMMIT_TYPE="build"
    COMMIT_SCOPE="(config)"
fi

# Проверка изменений в boards/
if echo "$CHANGED_FILES" | grep -q "^boards/"; then
    COMMIT_TYPE="feat"
    COMMIT_SCOPE="(boards)"
fi

# Подсчет изменений
ADDED=$(git diff --cached --numstat 2>/dev/null | wc -l)
MODIFIED=$(git diff --numstat 2>/dev/null | wc -l)

# Формирование сообщения
TIMESTAMP=$(date +"%Y-%m-%d %H:%M:%S")
COMMIT_MSG="$COMMIT_TYPE$COMMIT_SCOPE: автоматический коммит ($TIMESTAMP)"

# Если передано сообщение как аргумент
if [ -n "$1" ]; then
    COMMIT_MSG="$1"
fi

# Добавление всех изменений
step "Добавляем изменения..."
git add .

# Создание коммита
step "Создаем коммит: $COMMIT_MSG"
git commit -m "$COMMIT_MSG"

# Отправка на GitHub
CURRENT_BRANCH=$(git branch --show-current)
step "Отправляем на GitHub (ветка: $CURRENT_BRANCH)..."
git push origin "$CURRENT_BRANCH"

info "✅ Готово! Коммит создан и отправлен."

