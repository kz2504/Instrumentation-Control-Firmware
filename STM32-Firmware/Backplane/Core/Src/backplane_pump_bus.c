#include "backplane_pump_bus.h"

#include "backplane_card_bus.h"
#include "protocol.h"

#include <string.h>

#define PUMP_BUS_DISCOVERY_PASSES    6U

#define SCHED_ACTION_HEADER_SIZE     4U
#define SCHED_PUMP_PAYLOAD_SIZE      8U

typedef struct
{
  uint8_t card_slot;
  uint16_t local_event_index;
} PumpExecTarget;

typedef struct
{
  uint32_t global_event_id;
  uint8_t target_count;
  PumpExecTarget targets[CARD_BUS_SLOT_COUNT];
} PumpExecMapEntry;

typedef struct
{
  PumpExecMapEntry exec_map[MAX_EVENTS];
  uint16_t exec_map_count;
  uint16_t next_local_event_index[CARD_BUS_SLOT_COUNT];
  uint8_t last_status;
  uint8_t last_detail;
} PumpBusContext;

static PumpBusContext g_pump_bus;

static void pump_bus_set_error(uint8_t status, uint8_t detail)
{
  g_pump_bus.last_status = status;
  g_pump_bus.last_detail = detail;
}

static void pump_bus_reset_distribution_state(void)
{
  memset(g_pump_bus.exec_map, 0, sizeof(g_pump_bus.exec_map));
  memset(g_pump_bus.next_local_event_index, 0, sizeof(g_pump_bus.next_local_event_index));
  g_pump_bus.exec_map_count = 0U;
}

static uint32_t pump_bus_flow_nl_to_ul_min(uint32_t flow_nl_min)
{
  return (uint32_t)(((uint64_t)flow_nl_min + 500ULL) / 1000ULL);
}

static bool pump_bus_send_simple_command(uint8_t slot, uint8_t command_id)
{
  if (!card_bus_command(slot, command_id, 0, 0U, 0))
  {
    pump_bus_set_error(card_bus_get_last_status(), card_bus_get_last_detail());
    return false;
  }

  pump_bus_set_error(PUMP_STATUS_OK, 0U);
  return true;
}

static PumpExecMapEntry *pump_bus_find_exec_map(uint32_t global_event_id)
{
  uint16_t index;

  for (index = 0U; index < g_pump_bus.exec_map_count; index++)
  {
    if (g_pump_bus.exec_map[index].global_event_id == global_event_id)
    {
      return &g_pump_bus.exec_map[index];
    }
  }

  return 0;
}

static bool pump_bus_collect_required_slots(const Schedule *schedule, uint8_t *required_mask_out)
{
  uint16_t event_index;
  uint8_t required_mask = 0U;

  if ((schedule == 0) || (required_mask_out == 0))
  {
    pump_bus_set_error(PUMP_STATUS_BAD_LEN, 0U);
    return false;
  }

  for (event_index = 0U; event_index < schedule->event_count; event_index++)
  {
    const ScheduleEvent *event = &schedule->events[event_index];
    uint16_t offset = 0U;
    uint8_t action_index;

    for (action_index = 0U; action_index < event->action_count; action_index++)
    {
      uint8_t module_type;
      uint8_t module_id;
      uint8_t action_len;
      uint8_t slot;

      if ((uint16_t)(offset + SCHED_ACTION_HEADER_SIZE) > event->action_bytes_len)
      {
        pump_bus_set_error(PUMP_STATUS_BAD_LEN, action_index);
        return false;
      }

      module_type = event->action_bytes[offset + 0U];
      module_id = event->action_bytes[offset + 1U];
      action_len = event->action_bytes[offset + 3U];

      if ((uint16_t)(offset + SCHED_ACTION_HEADER_SIZE + action_len) > event->action_bytes_len)
      {
        pump_bus_set_error(PUMP_STATUS_BAD_LEN, action_index);
        return false;
      }

      if (module_type == MODULE_PUMP_PERISTALTIC)
      {
        slot = (uint8_t)(module_id / PUMP_CARD_PUMPS_PER_CARD);
        if (slot >= CARD_BUS_SLOT_COUNT)
        {
          pump_bus_set_error(PUMP_STATUS_BAD_INDEX, module_id);
          return false;
        }

        required_mask |= (uint8_t)(1U << slot);
      }

      offset = (uint16_t)(offset + SCHED_ACTION_HEADER_SIZE + action_len);
    }

    if (offset != event->action_bytes_len)
    {
      pump_bus_set_error(PUMP_STATUS_BAD_LEN, event->action_count);
      return false;
    }
  }

  *required_mask_out = required_mask;
  pump_bus_set_error(PUMP_STATUS_OK, 0U);
  return true;
}

static bool pump_bus_collect_event_actions(const ScheduleEvent *event,
                                           PumpCardEvent per_slot_events[CARD_BUS_SLOT_COUNT],
                                           uint8_t slot_used[CARD_BUS_SLOT_COUNT],
                                           bool require_present_cards)
{
  uint16_t offset = 0U;
  uint8_t action_index;
  uint8_t seen_local_pumps[CARD_BUS_SLOT_COUNT];

  memset(per_slot_events, 0, sizeof(PumpCardEvent) * CARD_BUS_SLOT_COUNT);
  memset(slot_used, 0, CARD_BUS_SLOT_COUNT);
  memset(seen_local_pumps, 0, sizeof(seen_local_pumps));

  for (action_index = 0U; action_index < event->action_count; action_index++)
  {
    uint8_t module_type;
    uint8_t module_id;
    uint8_t action_type;
    uint8_t action_len;

    if ((uint16_t)(offset + SCHED_ACTION_HEADER_SIZE) > event->action_bytes_len)
    {
      pump_bus_set_error(PUMP_STATUS_BAD_LEN, action_index);
      return false;
    }

    module_type = event->action_bytes[offset + 0U];
    module_id = event->action_bytes[offset + 1U];
    action_type = event->action_bytes[offset + 2U];
    action_len = event->action_bytes[offset + 3U];

    if ((uint16_t)(offset + SCHED_ACTION_HEADER_SIZE + action_len) > event->action_bytes_len)
    {
      pump_bus_set_error(PUMP_STATUS_BAD_LEN, action_index);
      return false;
    }

    if (module_type == MODULE_PUMP_PERISTALTIC)
    {
      uint8_t slot;
      uint8_t local_pump_id;
      PumpCardEvent *slot_event;
      PumpAction *slot_action;
      const uint8_t *payload = &event->action_bytes[offset + SCHED_ACTION_HEADER_SIZE];

      if ((action_type != PUMP_SET_STATE) || (action_len != SCHED_PUMP_PAYLOAD_SIZE))
      {
        pump_bus_set_error(PUMP_STATUS_BAD_LEN, module_id);
        return false;
      }

      slot = (uint8_t)(module_id / PUMP_CARD_PUMPS_PER_CARD);
      local_pump_id = (uint8_t)(module_id % PUMP_CARD_PUMPS_PER_CARD);

      if (slot >= CARD_BUS_SLOT_COUNT)
      {
        pump_bus_set_error(PUMP_STATUS_BAD_LEN, module_id);
        return false;
      }

      if (require_present_cards && !card_bus_is_slot_present(slot, CARD_TYPE_PUMP_PERISTALTIC))
      {
        pump_bus_set_error(PUMP_STATUS_HW_ERROR, module_id);
        return false;
      }

      if ((seen_local_pumps[slot] & (uint8_t)(1U << local_pump_id)) != 0U)
      {
        pump_bus_set_error(PUMP_STATUS_BAD_LEN, module_id);
        return false;
      }

      seen_local_pumps[slot] |= (uint8_t)(1U << local_pump_id);
      slot_event = &per_slot_events[slot];
      if (slot_event->action_count >= PUMP_CARD_MAX_ACTIONS_PER_EVENT)
      {
        pump_bus_set_error(PUMP_STATUS_FULL, module_id);
        return false;
      }

      slot_event->global_event_id = event->event_id;
      slot_action = &slot_event->actions[slot_event->action_count];
      slot_action->local_pump_id = local_pump_id;
      slot_action->enable = payload[0];
      slot_action->direction = payload[1];
      slot_action->reserved = 0U;
      slot_action->flow_ul_min = pump_bus_flow_nl_to_ul_min(read_u32_le(&payload[4]));

      slot_event->action_count++;
      slot_used[slot] = 1U;
    }

    offset = (uint16_t)(offset + SCHED_ACTION_HEADER_SIZE + action_len);
  }

  if (offset != event->action_bytes_len)
  {
    pump_bus_set_error(PUMP_STATUS_BAD_LEN, event->action_count);
    return false;
  }

  pump_bus_set_error(PUMP_STATUS_OK, 0U);
  return true;
}

void pump_bus_init(void)
{
  memset(&g_pump_bus, 0, sizeof(g_pump_bus));
  card_bus_init();
}

void pump_bus_discover(void)
{
  card_bus_discover_all();
  pump_bus_set_error(card_bus_get_last_status(), card_bus_get_last_detail());
}

bool pump_bus_discover_required(const Schedule *schedule)
{
  uint8_t required_mask;

  if (!pump_bus_collect_required_slots(schedule, &required_mask))
  {
    return false;
  }

  if (!card_bus_discover_type_slots(required_mask, CARD_TYPE_PUMP_PERISTALTIC, PUMP_BUS_DISCOVERY_PASSES))
  {
    pump_bus_set_error(PUMP_STATUS_HW_ERROR,
                       (uint8_t)(card_bus_get_last_detail() * PUMP_CARD_PUMPS_PER_CARD));
    return false;
  }

  pump_bus_set_error(PUMP_STATUS_OK, 0U);
  return true;
}

bool pump_bus_validate_schedule(const Schedule *schedule)
{
  uint16_t event_index;
  PumpCardEvent per_slot_events[CARD_BUS_SLOT_COUNT];
  uint8_t slot_used[CARD_BUS_SLOT_COUNT];

  if (schedule == 0)
  {
    pump_bus_set_error(PUMP_STATUS_BAD_LEN, 0U);
    return false;
  }

  for (event_index = 0U; event_index < schedule->event_count; event_index++)
  {
    if (!pump_bus_collect_event_actions(&schedule->events[event_index],
                                        per_slot_events,
                                        slot_used,
                                        true))
    {
      return false;
    }
  }

  pump_bus_set_error(PUMP_STATUS_OK, 0U);
  return true;
}

bool pump_bus_distribute_schedule(const Schedule *schedule)
{
  uint8_t slot;
  uint16_t event_index;
  PumpCardEvent per_slot_events[CARD_BUS_SLOT_COUNT];
  uint8_t slot_used[CARD_BUS_SLOT_COUNT];
  uint8_t payload[PUMP_CARD_CMD_PAYLOAD_MAX];
  uint16_t payload_len;

  if (schedule == 0)
  {
    pump_bus_set_error(PUMP_STATUS_BAD_LEN, 0U);
    return false;
  }

  pump_bus_reset_distribution_state();

  for (slot = 0U; slot < CARD_BUS_SLOT_COUNT; slot++)
  {
    if (card_bus_is_slot_present(slot, CARD_TYPE_PUMP_PERISTALTIC)
        && !pump_bus_send_simple_command(slot, CMD_CLEAR))
    {
      return false;
    }
  }

  for (event_index = 0U; event_index < schedule->event_count; event_index++)
  {
    PumpExecMapEntry *map_entry = 0;

    if (!pump_bus_collect_event_actions(&schedule->events[event_index],
                                        per_slot_events,
                                        slot_used,
                                        true))
    {
      return false;
    }

    for (slot = 0U; slot < CARD_BUS_SLOT_COUNT; slot++)
    {
      if (slot_used[slot] == 0U)
      {
        continue;
      }

      per_slot_events[slot].local_event_index = g_pump_bus.next_local_event_index[slot];
      if (!pump_card_build_load_event_payload(&per_slot_events[slot],
                                              payload,
                                              sizeof(payload),
                                              &payload_len))
      {
        pump_bus_set_error(PUMP_STATUS_BAD_LEN, slot);
        return false;
      }

      if (!card_bus_command(slot, CMD_LOAD_EVENT, payload, payload_len, 0))
      {
        pump_bus_set_error(card_bus_get_last_status(), card_bus_get_last_detail());
        return false;
      }

      if (map_entry == 0)
      {
        if (g_pump_bus.exec_map_count >= MAX_EVENTS)
        {
          pump_bus_set_error(PUMP_STATUS_FULL, slot);
          return false;
        }

        map_entry = &g_pump_bus.exec_map[g_pump_bus.exec_map_count++];
        memset(map_entry, 0, sizeof(*map_entry));
        map_entry->global_event_id = schedule->events[event_index].event_id;
      }

      if (map_entry->target_count >= CARD_BUS_SLOT_COUNT)
      {
        pump_bus_set_error(PUMP_STATUS_FULL, slot);
        return false;
      }

      map_entry->targets[map_entry->target_count].card_slot = slot;
      map_entry->targets[map_entry->target_count].local_event_index = per_slot_events[slot].local_event_index;
      map_entry->target_count++;

      g_pump_bus.next_local_event_index[slot]++;
    }
  }

  for (slot = 0U; slot < CARD_BUS_SLOT_COUNT; slot++)
  {
    if (card_bus_is_slot_present(slot, CARD_TYPE_PUMP_PERISTALTIC)
        && (g_pump_bus.next_local_event_index[slot] > 0U))
    {
      if (!pump_bus_send_simple_command(slot, CMD_ARM))
      {
        return false;
      }
    }
  }

  pump_bus_set_error(PUMP_STATUS_OK, 0U);
  return true;
}

bool pump_bus_exec_event(uint32_t global_event_id)
{
  PumpExecMapEntry *map_entry;
  uint8_t target_index;
  uint8_t payload[PUMP_CARD_EXEC_PAYLOAD_SIZE];
  uint16_t payload_len = 0U;

  map_entry = pump_bus_find_exec_map(global_event_id);
  if (map_entry == 0)
  {
    return true;
  }

  for (target_index = 0U; target_index < map_entry->target_count; target_index++)
  {
    pump_card_build_exec_event_payload(map_entry->targets[target_index].local_event_index,
                                       global_event_id,
                                       payload,
                                       &payload_len);

    if (!card_bus_command(map_entry->targets[target_index].card_slot,
                          CMD_EXEC_EVENT,
                          payload,
                          payload_len,
                          0))
    {
      pump_bus_set_error(card_bus_get_last_status(), card_bus_get_last_detail());
      return false;
    }
  }

  pump_bus_set_error(PUMP_STATUS_OK, 0U);
  return true;
}

void pump_bus_stop_all(void)
{
  uint8_t slot;

  for (slot = 0U; slot < CARD_BUS_SLOT_COUNT; slot++)
  {
    if (card_bus_is_slot_present(slot, CARD_TYPE_PUMP_PERISTALTIC))
    {
      (void)pump_bus_send_simple_command(slot, CMD_STOP);
    }
  }
}

uint8_t pump_bus_get_last_status(void)
{
  return g_pump_bus.last_status;
}

uint8_t pump_bus_get_last_detail(void)
{
  return g_pump_bus.last_detail;
}
