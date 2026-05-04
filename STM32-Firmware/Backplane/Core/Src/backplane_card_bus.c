#include "backplane_card_bus.h"

#include "main.h"
#include "spi.h"
#include "spi_frame.h"

#include <string.h>

#define SPI_SETTLE_DELAY_CYCLES       200U
#define CARD_BUS_MAX_RETRIES          3U
#define CARD_BUS_TX_TIMEOUT_MS        100U
#define CARD_BUS_RX_TIMEOUT_MS        100U
#define CARD_BUS_INTERPHASE_DELAY_MS  1U
#define CARD_BUS_RETRY_DELAY_MS       2U
#define CARD_BUS_DISCOVERY_DELAY_MS   25U
#define CARD_BUS_INVENTORY_MISS_LIMIT 2U

typedef struct
{
  CardSlotInfo slots[CARD_BUS_SLOT_COUNT];
  uint8_t miss_count[CARD_BUS_SLOT_COUNT];
  uint8_t last_status;
  uint8_t last_detail;
} CardBusContext;

static CardBusContext g_card_bus;

static void card_bus_set_error(uint8_t status, uint8_t detail)
{
  g_card_bus.last_status = status;
  g_card_bus.last_detail = detail;
}

static void spi_delay_cycles(volatile uint32_t cycles)
{
  while (cycles-- > 0U)
  {
    __NOP();
  }
}

static void card_bus_select_slot(uint8_t slot)
{
  slot = (uint8_t)(7U - slot);

  HAL_GPIO_WritePin(EN_CS_GPIO_Port, EN_CS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(A0_CS_GPIO_Port, A0_CS_Pin,
                    (slot & 0x01U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(A1_CS_GPIO_Port, A1_CS_Pin,
                    (slot & 0x02U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(A2_CS_GPIO_Port, A2_CS_Pin,
                    (slot & 0x04U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(EN_CS_GPIO_Port, EN_CS_Pin, GPIO_PIN_RESET);
  spi_delay_cycles(SPI_SETTLE_DELAY_CYCLES);
}

static void card_bus_deselect_all(void)
{
  HAL_GPIO_WritePin(EN_CS_GPIO_Port, EN_CS_Pin, GPIO_PIN_SET);
  spi_delay_cycles(SPI_SETTLE_DELAY_CYCLES);
}

static bool card_bus_spi_transaction(uint8_t slot,
                                     const uint8_t *tx_buf,
                                     uint8_t *rx_buf)
{
  uint8_t attempt;
  bool ok;

  for (attempt = 0U; attempt < CARD_BUS_MAX_RETRIES; attempt++)
  {
    ok = false;
    card_bus_deselect_all();
    card_bus_select_slot(slot);

    if (HAL_SPI_Transmit(&hspi1,
                         (uint8_t *)tx_buf,
                         SPI_FRAME_CMD_SIZE,
                         CARD_BUS_TX_TIMEOUT_MS) == HAL_OK)
    {
      HAL_Delay(CARD_BUS_INTERPHASE_DELAY_MS);

      if (HAL_SPI_Receive(&hspi1,
                          rx_buf,
                          SPI_FRAME_RESP_SIZE,
                          CARD_BUS_RX_TIMEOUT_MS) == HAL_OK)
      {
        ok = true;
      }
    }

    card_bus_deselect_all();
    if (ok)
    {
      return true;
    }

    HAL_Delay(CARD_BUS_RETRY_DELAY_MS);
  }

  return false;
}

static bool card_bus_issue_command(uint8_t slot,
                                   uint8_t command_id,
                                   const uint8_t *payload,
                                   uint16_t payload_len,
                                   SpiFrameResponse *response_out)
{
  uint8_t tx_frame[SPI_FRAME_CMD_SIZE];
  uint8_t rx_frame[SPI_FRAME_RESP_SIZE];
  SpiFrameResponse response;
  uint8_t parse_status;

  if (slot >= CARD_BUS_SLOT_COUNT)
  {
    card_bus_set_error(STATUS_BAD_ADDR, slot);
    return false;
  }

  if (!spi_frame_build_command(command_id, payload, payload_len, tx_frame, sizeof(tx_frame)))
  {
    card_bus_set_error(STATUS_BAD_LEN, command_id);
    return false;
  }

  memset(&response, 0, sizeof(response));
  memset(rx_frame, 0, sizeof(rx_frame));

  if (!card_bus_spi_transaction(slot, tx_frame, rx_frame))
  {
    card_bus_set_error(STATUS_HW_ERROR, slot);
    return false;
  }

  parse_status = spi_frame_parse_response(rx_frame, sizeof(rx_frame), &response);
  if (parse_status != STATUS_OK)
  {
    card_bus_set_error(parse_status, slot);
    return false;
  }

  if ((response.ack != SPI_FRAME_ACK_OK) || (response.status != STATUS_OK))
  {
    card_bus_set_error(response.status, slot);
    return false;
  }

  if (response_out != 0)
  {
    *response_out = response;
  }

  card_bus_set_error(STATUS_OK, 0U);
  return true;
}

static void card_bus_clear_slots(uint8_t slot_mask)
{
  uint8_t slot;

  for (slot = 0U; slot < CARD_BUS_SLOT_COUNT; slot++)
  {
    if ((slot_mask & (uint8_t)(1U << slot)) != 0U)
    {
      memset(&g_card_bus.slots[slot], 0, sizeof(g_card_bus.slots[slot]));
      g_card_bus.miss_count[slot] = 0U;
    }
  }
}

static bool card_bus_identify_slot(uint8_t slot)
{
  SpiFrameResponse response;
  CardSlotInfo info;

  if (!card_bus_issue_command(slot, CMD_IDENTIFY, 0, 0U, &response))
  {
    return false;
  }

  if (response.payload_len != 6U)
  {
    card_bus_set_error(STATUS_BAD_LEN, slot);
    return false;
  }

  memset(&info, 0, sizeof(info));
  info.present = 1U;
  info.card_type = response.payload[0];
  info.firmware_major = response.payload[1];
  info.firmware_minor = response.payload[2];
  info.capabilities = spi_frame_read_u16_le(&response.payload[3]);
  info.max_local_events = response.payload[5];

  g_card_bus.slots[slot] = info;
  g_card_bus.miss_count[slot] = 0U;
  card_bus_set_error(STATUS_OK, 0U);
  return true;
}

static uint8_t card_bus_find_missing_slot(uint8_t slot_mask, uint8_t required_card_type)
{
  uint8_t slot;

  for (slot = 0U; slot < CARD_BUS_SLOT_COUNT; slot++)
  {
    if ((slot_mask & (uint8_t)(1U << slot)) == 0U)
    {
      continue;
    }

    if (!card_bus_is_slot_present(slot, required_card_type))
    {
      return slot;
    }
  }

  return CARD_BUS_SLOT_COUNT;
}

void card_bus_init(void)
{
  memset(&g_card_bus, 0, sizeof(g_card_bus));
  card_bus_deselect_all();
}

void card_bus_discover_all(void)
{
  uint8_t slot;

  for (slot = 0U; slot < CARD_BUS_SLOT_COUNT; slot++)
  {
    if (!card_bus_identify_slot(slot))
    {
      if (g_card_bus.miss_count[slot] < 0xFFU)
      {
        g_card_bus.miss_count[slot]++;
      }

      if (g_card_bus.miss_count[slot] >= CARD_BUS_INVENTORY_MISS_LIMIT)
      {
        memset(&g_card_bus.slots[slot], 0, sizeof(g_card_bus.slots[slot]));
      }
    }
  }

  card_bus_set_error(STATUS_OK, 0U);
}

bool card_bus_discover_type_slots(uint8_t slot_mask, uint8_t required_card_type, uint8_t pass_count)
{
  uint8_t pass;

  if (slot_mask == 0U)
  {
    card_bus_set_error(STATUS_OK, 0U);
    return true;
  }

  card_bus_clear_slots(slot_mask);

  for (pass = 0U; pass < pass_count; pass++)
  {
    uint8_t slot;
    uint8_t missing_slot;

    for (slot = 0U; slot < CARD_BUS_SLOT_COUNT; slot++)
    {
      if ((slot_mask & (uint8_t)(1U << slot)) == 0U)
      {
        continue;
      }

      if (!card_bus_is_slot_present(slot, required_card_type))
      {
        (void)card_bus_identify_slot(slot);
      }
    }

    missing_slot = card_bus_find_missing_slot(slot_mask, required_card_type);
    if (missing_slot >= CARD_BUS_SLOT_COUNT)
    {
      card_bus_set_error(STATUS_OK, 0U);
      return true;
    }

    if ((pass + 1U) < pass_count)
    {
      HAL_Delay(CARD_BUS_DISCOVERY_DELAY_MS);
    }
  }

  {
    uint8_t missing_slot = card_bus_find_missing_slot(slot_mask, required_card_type);

    if (missing_slot < CARD_BUS_SLOT_COUNT)
    {
      card_bus_set_error(STATUS_HW_ERROR, missing_slot);
    }
  }

  return false;
}

bool card_bus_is_slot_present(uint8_t slot, uint8_t required_card_type)
{
  if (slot >= CARD_BUS_SLOT_COUNT)
  {
    return false;
  }

  if (g_card_bus.slots[slot].present == 0U)
  {
    return false;
  }

  if ((required_card_type != 0U) && (g_card_bus.slots[slot].card_type != required_card_type))
  {
    return false;
  }

  return true;
}

uint8_t card_bus_find_first_slot(uint8_t required_card_type)
{
  uint8_t slot;

  for (slot = 0U; slot < CARD_BUS_SLOT_COUNT; slot++)
  {
    if (card_bus_is_slot_present(slot, required_card_type))
    {
      return slot;
    }
  }

  return CARD_BUS_SLOT_COUNT;
}

const CardSlotInfo *card_bus_get_slots(void)
{
  return g_card_bus.slots;
}

bool card_bus_read_reg(uint8_t slot, uint16_t reg_addr, uint32_t *value_out)
{
  uint8_t payload[2];
  SpiFrameResponse response;

  if (value_out == 0)
  {
    card_bus_set_error(STATUS_BAD_LEN, 0U);
    return false;
  }

  spi_frame_write_u16_le(payload, reg_addr);
  if (!card_bus_issue_command(slot, CMD_READ_REG, payload, sizeof(payload), &response))
  {
    return false;
  }

  if (response.payload_len != 4U)
  {
    card_bus_set_error(STATUS_BAD_LEN, slot);
    return false;
  }

  *value_out = spi_frame_read_u32_le(response.payload);
  card_bus_set_error(STATUS_OK, 0U);
  return true;
}

bool card_bus_write_reg(uint8_t slot, uint16_t reg_addr, uint32_t value)
{
  uint8_t payload[6];

  spi_frame_write_u16_le(payload, reg_addr);
  spi_frame_write_u32_le(&payload[2], value);
  return card_bus_issue_command(slot, CMD_WRITE_REG, payload, sizeof(payload), 0);
}

uint8_t card_bus_get_last_status(void)
{
  return g_card_bus.last_status;
}

uint8_t card_bus_get_last_detail(void)
{
  return g_card_bus.last_detail;
}
