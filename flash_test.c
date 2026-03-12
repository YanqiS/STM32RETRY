#include "flash_test.h"

#include <string.h>
#include <stdio.h>

#include "main.h"
#include <FlashAddr.h>
#include <EasySyslib.h>
#include <OLED.h>

#ifndef FLASH_TEST_PATTERN_SIZE
#define FLASH_TEST_PATTERN_SIZE 16U
#endif

#ifndef FLASH_TEST_RETRY
#define FLASH_TEST_RETRY 5U
#endif

#ifndef FLASH_TEST_CASES
#define FLASH_TEST_CASES 5U
#endif

static void FillPattern(uint8_t *buf, uint32_t len, uint32_t seed) {
    for (uint32_t i = 0; i < len; i++) {
        uint8_t mix = (uint8_t) ((seed >> ((i & 0x3U) * 8U)) & 0xFFU);
        buf[i] = (uint8_t) (mix ^ (0x5AU + i * 29U));
    }
}

static void InvertPattern(uint8_t *buf, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        buf[i] = (uint8_t) ~buf[i];
    }
}

static uint8_t BufferEqual(const uint8_t *a, const uint8_t *b, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        if (a[i] != b[i]) {
            return 0U;
        }
    }
    return 1U;
}

static void SnapshotFail(FlashTestResult *result,
        uint32_t addr,
        FlashTestStage stage,
        const uint8_t *expect,
        const uint8_t *actual,
        uint32_t len) {
    if (result->first_fail_addr != 0U) {
        return;
    }

    result->first_fail_addr = addr;
    result->first_fail_stage = stage;
    result->first_fail_sr = SPI_Flash_ReadSR();

    uint32_t copy_len = (len > 16U) ? 16U : len;
    memcpy(result->first_expect, expect, copy_len);
    memcpy(result->first_actual, actual, copy_len);
}

static void OLED_ShowSummary(const FlashTestResult *result) {
    char line[24];

    if (result->failed_cases == 0U) {
        OLED_ShowString(OLED_I2C_ch, OLED_type, 0, 1, "Flash Test PASS   ");
        snprintf(line, sizeof(line), "P:%lu F:%lu", (unsigned long) result->passed_cases,
                (unsigned long) result->failed_cases);
        OLED_ShowString(OLED_I2C_ch, OLED_type, 0, 2, line);
    } else {
        OLED_ShowString(OLED_I2C_ch, OLED_type, 0, 1, "Flash Test FAIL   ");
        snprintf(line, sizeof(line), "A:%06lX S:%u", (unsigned long) result->first_fail_addr,
                (unsigned int) result->first_fail_stage);
        OLED_ShowString(OLED_I2C_ch, OLED_type, 0, 2, line);
    }
}

FlashTestResult Flash_Test_Run(void) {
    FlashTestResult result;
    memset(&result, 0, sizeof(result));

    static const uint32_t test_addrs[FLASH_TEST_CASES] = {
            Sys_Addr_DispTest,
            Sys_Addr_DispX0,
            Sys_Addr_DispX1,
            Sys_Addr_DispY0,
            Sys_Addr_DispY1
    };

    uint8_t pattern_a[FLASH_TEST_PATTERN_SIZE];
    uint8_t pattern_b[FLASH_TEST_PATTERN_SIZE];
    uint8_t readback[FLASH_TEST_PATTERN_SIZE];

    /* cache expected final values for cross verification */
    uint8_t expected[FLASH_TEST_CASES][FLASH_TEST_PATTERN_SIZE];
    memset(expected, 0, sizeof(expected));

    SPI_Flash_Start(Flash_SPI);

    for (uint32_t i = 0; i < FLASH_TEST_CASES; i++) {
        uint32_t addr = test_addrs[i];
        uint8_t pass = 0U;

        result.total_cases++;

        FillPattern(pattern_a, FLASH_TEST_PATTERN_SIZE, addr + i * 17U);
        memcpy(pattern_b, pattern_a, sizeof(pattern_b));
        InvertPattern(pattern_b, sizeof(pattern_b));

        for (uint32_t retry = 0; retry < FLASH_TEST_RETRY; retry++) {
            result.total_retries += (retry > 0U) ? 1U : 0U;

            /* write A */
            SPI_Flash_WtritEnable();
            SPI_Flash_WriteSomeBytes(pattern_a, addr, FLASH_TEST_PATTERN_SIZE);
            SPI_Flash_WaitNoBusy();
            result.total_writes++;

            memset(readback, 0, sizeof(readback));
            SPI_Flash_ReadBytes(readback, addr, FLASH_TEST_PATTERN_SIZE);
            if (!BufferEqual(pattern_a, readback, FLASH_TEST_PATTERN_SIZE)) {
                SnapshotFail(&result, addr, FLASH_TEST_STAGE_VERIFY_A,
                        pattern_a, readback, FLASH_TEST_PATTERN_SIZE);
                continue;
            }

            /* write B (overwrite test) */
            SPI_Flash_WtritEnable();
            SPI_Flash_WriteSomeBytes(pattern_b, addr, FLASH_TEST_PATTERN_SIZE);
            SPI_Flash_WaitNoBusy();
            result.total_writes++;

            memset(readback, 0, sizeof(readback));
            SPI_Flash_ReadBytes(readback, addr, FLASH_TEST_PATTERN_SIZE);
            if (!BufferEqual(pattern_b, readback, FLASH_TEST_PATTERN_SIZE)) {
                SnapshotFail(&result, addr, FLASH_TEST_STAGE_VERIFY_B,
                        pattern_b, readback, FLASH_TEST_PATTERN_SIZE);
                continue;
            }

            memcpy(expected[i], pattern_b, FLASH_TEST_PATTERN_SIZE);
            pass = 1U;
            break;
        }

        if (pass) {
            result.passed_cases++;
        } else {
            result.failed_cases++;
        }
    }

    /* cross verify all final values to catch same-sector collateral corruption */
    for (uint32_t i = 0; i < FLASH_TEST_CASES; i++) {
        memset(readback, 0, sizeof(readback));
        SPI_Flash_ReadBytes(readback, test_addrs[i], FLASH_TEST_PATTERN_SIZE);

        if (!BufferEqual(expected[i], readback, FLASH_TEST_PATTERN_SIZE)) {
            result.failed_cases++;
            SnapshotFail(&result, test_addrs[i], FLASH_TEST_STAGE_CROSS_VERIFY,
                    expected[i], readback, FLASH_TEST_PATTERN_SIZE);
        }
    }

    OLED_ShowSummary(&result);
    return result;
}
