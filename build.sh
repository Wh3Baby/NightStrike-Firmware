#!/bin/bash
# Единый скрипт сборки NightStrike Firmware
# Устанавливает платформу через git и собирает прошивку

set -e

# Цвета
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

info() { echo -e "${GREEN}[INFO]${NC} $1"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
error() { echo -e "${RED}[ERROR]${NC} $1"; }
step() { echo -e "${BLUE}[STEP]${NC} $1"; }

# Параметры
ENV="${1:-m5stickc-plus2}"
ACTION="${2:-build}"

# Отключаем все загрузки PlatformIO
export PLATFORMIO_LIB_EXTRA_DIRS=".pio/lib"
export PLATFORMIO_DISABLE_PROGRESSBAR="true"
export PLATFORMIO_AUTO_UPLOAD_PORT=""
export PLATFORMIO_CORE_DIR="${HOME}/.platformio"
export PLATFORMIO_DISABLE_AUTO_CHECK="true"
export PLATFORMIO_DISABLE_UPDATES="true"

# Проверка PlatformIO
if ! command -v pio &> /dev/null; then
    error "PlatformIO не найден! Установите: pipx install platformio"
    exit 1
fi

info "PlatformIO найден: $(pio --version)"

# Проверка git
if ! command -v git &> /dev/null; then
    error "Git не найден! Установите: sudo apt-get install git"
    exit 1
fi

# Установка платформы через git (если нужно)
PLATFORM_DIR="${HOME}/.platformio/platforms/espressif32"
REPO_URL="https://github.com/platformio/platform-espressif32.git"

step "Проверяем платформу espressif32..."
if [ ! -d "$PLATFORM_DIR" ] || [ ! -f "$PLATFORM_DIR/platform.json" ]; then
    warn "Платформа espressif32 не установлена!"
    step "Устанавливаем платформу через git (это займет 2-3 минуты)..."
    
    mkdir -p "${HOME}/.platformio/platforms"
    
    if [ -d "$PLATFORM_DIR" ]; then
        warn "Директория существует, но платформа неполная. Удаляем..."
        rm -rf "$PLATFORM_DIR"
    fi
    
    info "Клонируем платформу из GitHub..."
    if git clone --depth 1 "$REPO_URL" "$PLATFORM_DIR" 2>&1; then
        info "Платформа успешно установлена через git"
    else
        error "Не удалось клонировать платформу"
        error "Проверьте подключение к интернету"
        exit 1
    fi
else
    info "Платформа espressif32 установлена"
fi

# Проверка библиотек
step "Проверяем локальные библиотеки..."
if [ ! -d ".pio/lib" ] || [ -z "$(ls -A .pio/lib 2>/dev/null)" ]; then
    warn "Локальные библиотеки не найдены!"
    info "Библиотеки будут установлены автоматически через PlatformIO"
fi

info "Локальные библиотеки найдены:"
ls -1 .pio/lib/ | sed 's/^/  - /'

# Выполнение команды
step "Выполняем: pio $ACTION -e $ENV"
echo ""

# Запускаем PlatformIO с отключенными загрузками
case "$ACTION" in
    build|run)
        PLATFORMIO_LIB_EXTRA_DIRS=".pio/lib" \
        PLATFORMIO_DISABLE_PROGRESSBAR="true" \
        PLATFORMIO_DISABLE_AUTO_CHECK="true" \
        PLATFORMIO_DISABLE_UPDATES="true" \
        pio run -e "$ENV" || {
            error "Сборка не удалась"
            exit 1
        }
        ;;
    upload)
        if [ -n "$3" ]; then
            PLATFORMIO_LIB_EXTRA_DIRS=".pio/lib" \
            PLATFORMIO_DISABLE_PROGRESSBAR="true" \
            PLATFORMIO_DISABLE_AUTO_CHECK="true" \
            PLATFORMIO_DISABLE_UPDATES="true" \
            pio run -e "$ENV" -t upload --upload-port "$3" || {
                error "Загрузка не удалась"
                exit 1
            }
        else
            PLATFORMIO_LIB_EXTRA_DIRS=".pio/lib" \
            PLATFORMIO_DISABLE_PROGRESSBAR="true" \
            PLATFORMIO_DISABLE_AUTO_CHECK="true" \
            PLATFORMIO_DISABLE_UPDATES="true" \
            pio run -e "$ENV" -t upload || {
                error "Загрузка не удалась"
                exit 1
            }
        fi
        ;;
    monitor)
        if [ -n "$3" ]; then
            pio device monitor --port "$3" -b 115200
        else
            pio device monitor -b 115200
        fi
        ;;
    clean)
        pio run -t clean
        ;;
    *)
        error "Неизвестное действие: $ACTION"
        echo "Доступные действия: build, run, upload, monitor, clean"
        echo ""
        echo "Примеры:"
        echo "  ./build.sh m5stickc-plus2 build"
        echo "  ./build.sh m5stickc-plus2 upload"
        echo "  ./build.sh m5stickc-plus2 upload /dev/ttyUSB0"
        echo "  ./build.sh m5stickc-plus2 monitor"
        exit 1
        ;;
esac

info "Готово!"
