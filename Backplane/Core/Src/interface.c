#include "main.h"
#include "interface.h"
#include "spi.h"
#include <string.h>

#define CMD_CLEAR 0x01
#define CMD_START 0x02
#define CMD_STOP 0x03
#define CMD_TRIGGER_RDY 0x04
#define CMD_BLOCK 0x10

uint32_t SPI_TIMEOUT = 100;

void setCS(uint8_t line) {
	line = 7u - line;
	uint8_t A0 = (line >> 0) & 1;
	uint8_t A1 = (line >> 1) & 1;
	uint8_t A2 = (line >> 2) & 1;
	HAL_GPIO_WritePin(EN_CS_GPIO_Port, EN_CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(A0_CS_GPIO_Port, A0_CS_Pin, A0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(A1_CS_GPIO_Port, A1_CS_Pin, A1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(A2_CS_GPIO_Port, A2_CS_Pin, A2 ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(EN_CS_GPIO_Port, EN_CS_Pin, GPIO_PIN_RESET);
}

void resetCS(void) {
	HAL_GPIO_WritePin(EN_CS_GPIO_Port, EN_CS_Pin, GPIO_PIN_SET);
}

static int spi_tx_frame(uint8_t card, uint8_t cmd, const void *payload, uint8_t len) {
    uint8_t hdr[2] = { cmd, len };

    setCS(card);

    if (HAL_SPI_Transmit(&hspi1, hdr, sizeof(hdr), SPI_TIMEOUT) != HAL_OK) {
        resetCS();
        return 0;
    }

    if (len && payload) {
        if (HAL_SPI_Transmit(&hspi1, (uint8_t*)payload, len, SPI_TIMEOUT) != HAL_OK) {
            resetCS();
            return 0;
        }
    }

    resetCS();
    return 1;
}

int spi_send_clear(uint8_t card) {
    return spi_tx_frame(card, CMD_CLEAR, NULL, 0);
}

int spi_send_start(uint8_t card) {
    return spi_tx_frame(card, CMD_START, NULL, 0);
}

int spi_send_stop(uint8_t card) {
    return spi_tx_frame(card, CMD_STOP, NULL, 0);
}

int spi_send_trigger_ready(uint8_t card, uint8_t ready) {
    uint8_t p = (ready ? 1u : 0u);
    return spi_tx_frame(card, CMD_TRIGGER_RDY, &p, 1);
}

int spi_send_block(uint8_t card, const void *blk, uint16_t len) {
    if (!blk) return 0;
    if (len > 255) return 0;
    return spi_tx_frame(card, CMD_BLOCK, blk, (uint8_t)len);
}
