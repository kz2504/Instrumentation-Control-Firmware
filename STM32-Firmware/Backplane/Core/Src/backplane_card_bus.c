#include "backplane_card_bus.h"

#include "main.h"
#include "spi.h"

#include <string.h>

#define SPI_SETTLE_DELAY_CYCLES      200U
#define CARD_BUS_MAX_RETRIES         3U
#define CARD_BUS_TX_TIMEOUT_MS       100U
#define CARD_BUS_RX_TIMEOUT_MS       100U
#define CARD_BUS_INTERPHASE_DELAY_MS 1U
#define CARD_BUS_RETRY_DELAY_MS      2U
#define CARD_BUS_DISCOVERY_DELAY_MS  25U
#define CARD_BUS_INVENTORY_MISS_THRESHOLD  2U

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
                                     uint16_t tx_len,
                                     uint8_t *rx_buf,
                                     uint16_t rx_len)
{
  uint8_t attempt;
  bool ok;

  for (attempt = 0U; attempt < CARD_BUS_MAX_RETRIES; attempt++)
  {
    ok = false;
    card_bus_deselect_all();
    card_bus_select_slot(slot);

    if (HAL_SPI_Transmit(&hspi1, (uint8_t *)tx_buf, tx_len, CARD_BUS_TX_TIMEOUT_MS) == HAL_OK)
    {
      HAL_Delay(CARD_BUS_INTERPHASE_DELAY_MS);

      if (rx_len == 0U)
      {
        ok = true;
      }
      else if (HAL_SPI_Receive(&hspi1, rx_buf, rx_len, CARD_BUS_RX_TIMEOUT_MS) == HAL_OK)
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

static void card_bus_store_identify_info(uint8_t slot, const PumpCardIdentifyInfo *identify)
{
  g_card_bus.slots[slot].present = 1U;
  g_card_bus.slots[slot].card_type = identify->card_type;
  g_card_bus.slots[slot].firmware_major = identify->firmware_major;
  g_card_bus.slots[slot].firmware_minor = identify->firmware_minor;
  g_card_bus.slots[slot].capabilities = identify->capabilities;
  g_card_bus.slots[slot].max_local_events = identify->max_local_events;
  g_card_bus.miss_count[slot] = 0U;
}

static bool card_bus_identify_slot(uint8_t slot)
{
  CardBusResponse response;
  PumpCardIdentifyInfo identify;

  if (slot >= CARD_BUS_SLOT_COUNT)
  {
    card_bus_set_error(PUMP_STATUS_BAD_INDEX, slot);
    return false;
  }

  memset(&identify, 0, sizeof(identify));

  if (!card_bus_command(slot, CMD_IDENTIFY, 0, 0U, &response))
  {
    return false;
  }

  if (!pump_card_parse_identify_payload(response.payload, response.payload_len, &identify))
  {
    card_bus_set_error(PUMP_STATUS_BAD_LEN, slot);
    return false;
  }

  card_bus_store_identify_info(slot, &identify);
  card_bus_set_error(PUMP_STATUS_OK, 0U);
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

      if (g_card_bus.miss_count[slot] >= CARD_BUS_INVENTORY_MISS_THRESHOLD)
      {
        memset(&g_card_bus.slots[slot], 0, sizeof(g_card_bus.slots[slot]));
      }
    }
  }

  card_bus_set_error(PUMP_STATUS_OK, 0U);
}

bool card_bus_discover_type_slots(uint8_t slot_mask, uint8_t required_card_type, uint8_t pass_count)
{
  uint8_t pass;

  if (slot_mask == 0U)
  {
    card_bus_set_error(PUMP_STATUS_OK, 0U);
    return true;
  }

  card_bus_clear_slots(slot_mask);

  for (pass = 0U; pass < pass_count; pass++)
  {
    uint8_t slot;
    uint8_t missing_slot;

    for (slot = 0U; slot < CARD_BUS_SLOT_COUNT; slot++)
    {
      uint8_t slot_bit = (uint8_t)(1U << slot);

      if ((slot_mask & slot_bit) == 0U)
      {
        continue;
      }

      if (card_bus_is_slot_present(slot, required_card_type))
      {
        continue;
      }

      (void)card_bus_identify_slot(slot);
    }

    missing_slot = card_bus_find_missing_slot(slot_mask, required_card_type);
    if (missing_slot >= CARD_BUS_SLOT_COUNT)
    {
      card_bus_set_error(PUMP_STATUS_OK, 0U);
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
      card_bus_set_error(PUMP_STATUS_HW_ERROR, missing_slot);
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

const CardSlotInfo *card_bus_get_slots(void)
{
  return g_card_bus.slots;
}

bool card_bus_command(uint8_t slot,
                      uint8_t command_id,
                      const uint8_t *payload,
                      uint16_t payload_len,
                      CardBusResponse *response_out)
{
  uint8_t tx_frame[PUMP_CARD_CMD_FRAME_SIZE];
  uint8_t rx_frame[PUMP_CARD_RESP_FRAME_SIZE];
  PumpCardResponse decoded_response;
  uint8_t decode_status;

  if (slot >= CARD_BUS_SLOT_COUNT)
  {
    card_bus_set_error(PUMP_STATUS_BAD_INDEX, slot);
    return false;
  }

  if (!pump_card_encode_command_frame(command_id, payload, payload_len, tx_frame, sizeof(tx_frame)))
  {
    card_bus_set_error(PUMP_STATUS_BAD_LEN, command_id);
    return false;
  }

  memset(&decoded_response, 0, sizeof(decoded_response));
  memset(rx_frame, 0, sizeof(rx_frame));

  if (!card_bus_spi_transaction(slot, tx_frame, sizeof(tx_frame), rx_frame, sizeof(rx_frame)))
  {
    card_bus_set_error(PUMP_STATUS_HW_ERROR, slot);
    return false;
  }

  decode_status = pump_card_decode_response_frame(rx_frame, sizeof(rx_frame), &decoded_response);
  if (decode_status != PUMP_STATUS_OK)
  {
    card_bus_set_error(decode_status, slot);
    return false;
  }

  if ((decoded_response.ack != PUMP_ACK_OK) || (decoded_response.status != PUMP_STATUS_OK))
  {
    card_bus_set_error(decoded_response.status, slot);
    return false;
  }

  if (response_out != 0)
  {
    memset(response_out, 0, sizeof(*response_out));
    response_out->ack = decoded_response.ack;
    response_out->status = decoded_response.status;
    response_out->payload_len = decoded_response.payload_len;
    memcpy(response_out->payload, decoded_response.payload, decoded_response.payload_len);
  }

  card_bus_set_error(PUMP_STATUS_OK, 0U);
  return true;
}

uint8_t card_bus_get_last_status(void)
{
  return g_card_bus.last_status;
}

uint8_t card_bus_get_last_detail(void)
{
  return g_card_bus.last_detail;
}
