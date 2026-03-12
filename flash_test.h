#ifndef FLASH_TEST_H
#define FLASH_TEST_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FLASH_TEST_STAGE_NONE = 0,
    FLASH_TEST_STAGE_WRITE_A = 1,
    FLASH_TEST_STAGE_VERIFY_A = 2,
    FLASH_TEST_STAGE_WRITE_B = 3,
    FLASH_TEST_STAGE_VERIFY_B = 4,
    FLASH_TEST_STAGE_CROSS_VERIFY = 5
} FlashTestStage;

typedef struct {
    uint32_t total_cases;
    uint32_t passed_cases;
    uint32_t failed_cases;
    uint32_t total_writes;
    uint32_t total_retries;
    uint32_t first_fail_addr;
    uint8_t first_fail_sr;
    FlashTestStage first_fail_stage;
    uint8_t first_expect[16];
    uint8_t first_actual[16];
} FlashTestResult;

/**
 * @brief Run flash R/W verification focused on overwrite and sector consistency.
 *
 * - Each address is written twice with different patterns (A then B).
 * - Readback is verified after each write.
 * - Neighbor addresses in the same sector are re-checked to catch collateral corruption.
 */
FlashTestResult Flash_Test_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* FLASH_TEST_H */
