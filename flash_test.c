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
#define FLASH_TEST_RETRY 3U
#endif

#ifndef FLASH_TEST_CASES
#define FLASH_TEST_CASES 8U
#endif

static void FillPattern(uint8_t *buf, uint32_t len, uint32_t seed) {
    for (uint32_t i = 0; i < len; i++) {
        buf[i] = (uint8_t) ((seed + i * 37U) ^ (0xA5U + (i << 1)));
    }
}

static uint8_t VerifyAndSnapshot(const uint8_t *exp, const uint8_t *act, uint32_t len,
        uint8_t *snapshot_exp, uint8_t *snapshot_act) {
    for (uint32_t i = 0; i < len; i++) {
        if (exp[i] != act[i]) {
            uint32_t copy_len = (len > 16U) ? 16U : len;
            memcpy(snapshot_exp, exp, copy_len);
            memcpy(snapshot_act, act, copy_len);
            return 0U;
        }
    }
    return 1U;
}

static void OLED_ShowFail(uint32_t addr) {
    char line[24];
    OLED_ShowString(OLED_I2C_ch, OLED_type, 0, 1, "Flash Test FAIL   ");
    snprintf(line, sizeof(line), "Addr:%06lX", (unsigned long) addr);
    OLED_ShowString(OLED_I2C_ch, OLED_type, 0, 2, line);
}

FlashTestResult Flash_Test_Run(void) {
    FlashTestResult result;
    memset(&result, 0, sizeof(result));

    static const uint32_t test_addrs[FLASH_TEST_CASES] = {
            Sys_Addr_DispTest,
            Sys_Addr_DispX0,
            Sys_Addr_DispX1,
            Sys_Addr_DispY0,
            Sys_Addr_DispY1,
            Sys_Addr_DispTest + 0x50,
            Sys_Addr_DispTest + 0x80,
            Sys_Addr_DispTest + 0x100
    };

    uint8_t wr[FLASH_TEST_PATTERN_SIZE];
    uint8_t rd[FLASH_TEST_PATTERN_SIZE];

    SPI_Flash_Start(Flash_SPI);

    for (uint32_t case_idx = 0; case_idx < FLASH_TEST_CASES; case_idx++) {
        uint32_t addr = test_addrs[case_idx];
        uint8_t pass = 0U;

        result.total_cases++;
        FillPattern(wr, FLASH_TEST_PATTERN_SIZE, addr + case_idx * 13U);

        for (uint32_t retry = 0; retry < FLASH_TEST_RETRY; retry++) {
            SPI_Flash_WtritEnable();
            SPI_Flash_WriteSomeBytes(wr, addr, FLASH_TEST_PATTERN_SIZE);
            SPI_Flash_WaitNoBusy();

            memset(rd, 0, sizeof(rd));
            SPI_Flash_ReadBytes(rd, addr, FLASH_TEST_PATTERN_SIZE);

            if (VerifyAndSnapshot(wr, rd, FLASH_TEST_PATTERN_SIZE,
                    result.first_expect, result.first_actual)) {
                pass = 1U;
                break;
            }
        }

        if (pass) {
            result.passed_cases++;
        } else {
            result.failed_cases++;
            if (result.first_fail_addr == 0U) {
                result.first_fail_addr = addr;
                OLED_ShowFail(addr);
            }
        }
    }

    if (result.failed_cases == 0U) {
        OLED_ShowString(OLED_I2C_ch, OLED_type, 0, 1, "Flash Test PASS   ");
    }

    return result;
}
