#ifndef FLASH_TEST_H
#define FLASH_TEST_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t total_cases;
    uint32_t passed_cases;
    uint32_t failed_cases;
    uint32_t first_fail_addr;
    uint8_t first_expect[16];
    uint8_t first_actual[16];
} FlashTestResult;

/**
 * @brief Run flash R/W verification in SYS_CONFIG area.
 *
 * This routine writes deterministic patterns, reads them back,
 * and reports the first mismatch for quick debugging.
 */
FlashTestResult Flash_Test_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* FLASH_TEST_H */
