/* ヘッダーファイル */
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>

/* 定数宣言 */
// 各状態のアドレス
#define BLOCK_SIZE      4 * 1024
#define GPIO_BASE       0x3F200000
#define SET 7 // HIGH OUTPUT
#define CLEAR 10 // LOW OUTPUT
#define GET 13 // INPUT

// あれば状態がわかりやすくなるので定義
enum {
    LOW, HIGH, INPUT, OUTPUT
};
// 構造体
typedef struct {
    unsigned long gpio_base;
    int memory_fd;
    void *map;
    volatile unsigned int *addr;
} rpi_gpio;

//-----// 以下　関数群 //-----//
/*マップ設定 */
int mapGPIO(rpi_gpio *gpio) {
    gpio->memory_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (gpio->memory_fd < 0) {
        perror("Failed to open /dev/mem, try change permission.");
        return 1;
    }
    gpio->map = mmap(
            NULL,
            BLOCK_SIZE,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            gpio->memory_fd,
            gpio->gpio_base
    );
    if (gpio->map == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    gpio->addr = (volatile unsigned int *) gpio->map;
    return 0;
}

/* マップの開放 */
void unmapGPIO(rpi_gpio *gpio) {
    munmap(gpio->map, BLOCK_SIZE);
    close(gpio->memory_fd);
}

/* 現在時刻の表示 */
void nowTimeDisp(void) {
    time_t t;
    struct tm *t_st;
    /* 現在時刻の取得 */
    time(&t);
    /* 現在時刻を構造体に変換 */
    t_st = localtime(&t);
    printf("%02d時", t_st->tm_hour);
    printf("%02d分", t_st->tm_min);
    printf("%02d秒\n", t_st->tm_sec);
}

void pinMode(rpi_gpio *gpio, int pin, int staus) {
    int ten_value = pin / 10;
    int one_value = pin % 10;
    if (staus == OUTPUT) {
        *(gpio->addr + ten_value) = (unsigned int) (1 << (one_value * 3));
    } else if (staus == INPUT) {
        *(gpio->addr + ten_value) = (unsigned int) (0 << (one_value * 3));
    }
}

/* 入力処理 (指定されたピンから入力があれば0を返す) */
int gpioRead(rpi_gpio *gpio, int pin) {
    return !((*(gpio->addr + GET) & (1 << pin)));
}

/*   出力処理 (HIGH & LOW) */
void gpioWrite(rpi_gpio *gpio, int pin, int status) {
    if (status == HIGH) {
        *(gpio->addr + SET) = (unsigned int) (1 << pin);
    } else if (status == LOW) {
        *(gpio->addr + CLEAR) = (unsigned int) (1 << pin);
    }
}

/* cleanup処理 */
void cleanup(rpi_gpio *gpio, int pin) {
    int ten_value = pin / 10;
    *(gpio->addr + ten_value) = 0x00000000;
}

//-----// メイン処理 //-----//
int main(void) {
    rpi_gpio gpio = {GPIO_BASE};
    int map_status;
    map_status = mapGPIO(&gpio);
    if (map_status) {
        printf("Failed.\n");
        return map_status;
    }
    /* ここにメイン処理を書く */
    return 0;
}
