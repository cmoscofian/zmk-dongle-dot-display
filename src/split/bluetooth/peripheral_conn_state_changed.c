/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#include <zephyr/init.h>
#include <zephyr/types.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/sys/byteorder.h>

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/ble.h>

#include <zmk/events/peripheral_conn_state_changed.h>

struct zmk_peripheral_conn
{
	enum zmk_peripheral_conn_state state;
	struct bt_conn                *conn;
};

static struct zmk_peripheral_conn peripherals[ZMK_SPLIT_BLE_PERIPHERAL_COUNT];

static int
peripheral_index_by_conn(struct bt_conn *conn)
{
	int i;

	for (i = 0; i < ZMK_SPLIT_BLE_PERIPHERAL_COUNT; i++)
	{
		if (peripherals[i].conn == conn)
		{
			return i;
		}
	}
	return -EINVAL;
}

static struct zmk_peripheral_conn *
peripheral_by_conn(struct bt_conn *conn)
{
	int i;

	if ((i = peripheral_index_by_conn(conn)) < 0)
	{
		return NULL;
	}
	return &peripherals[i];
}

static int
release_peripheral_by_index(int i)
{
	struct zmk_peripheral_conn *source;

	if (i < 0 || i >= ZMK_SPLIT_BLE_PERIPHERAL_COUNT)
	{
		return -EINVAL;
	}

	source = &peripherals[i];
	if (source->state == PERIPHERAL_CONN_STATE_OPEN)
	{
		return -EINVAL;
	}

	LOG_DBG("Releasing peripheral conn at %d", i);
	if (source->conn != NULL)
	{
		source->conn = NULL;
	}
	source->state = PERIPHERAL_CONN_STATE_OPEN;

	return 0;
}

int
release_peripheral_by_conn(struct bt_conn *conn)
{
	int i;

	if ((i = peripheral_index_by_conn(conn)) < 0)
	{
		return i;
	}

	return release_peripheral_by_index(i);
}

static int
reserve_peripheral_by_conn(struct bt_conn *conn)
{
	int i;

#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_PREF_WEAK_BOND)
	for (i = 0; i < ZMK_SPLIT_BLE_PERIPHERAL_COUNT; i++)
	{
		if (peripherals[i].state == PERIPHERAL_CONN_STATE_OPEN)
		{
			// Ensure peripheral is fully reinitialized
			release_peripheral_by_index(i);
			peripherals[i].conn  = conn;
			peripherals[i].state = PERIPHERAL_CONN_STATE_CONNECTED;
			return i;
		}
	}
#else
	i = zmk_ble_put_peripheral_addr(bt_conn_get_dst(conn));
	if (i >= 0)
	{
		if (peripherals[i].state == PERIPHERAL_CONN_STATE_OPEN)
		{
			// Ensure peripheral is fully reinitialized
			release_peripheral_by_index(i);
			peripherals[i].conn  = conn;
			peripherals[i].state = PERIPHERAL_CONN_STATE_CONNECTED;
			return i;
		}
	}
#endif // IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_PREF_WEAK_BOND)

	return -ENOMEM;
}

static void
peripheral_state_process_connection(struct bt_conn *conn)
{
	LOG_DBG("Current security for connection: %d", bt_conn_get_security(conn));

	struct zmk_peripheral_conn *peripheral;
	struct bt_conn_info         info;

	if ((peripheral = peripheral_by_conn(conn)) == NULL)
	{
		LOG_ERR("No peripheral state found for connection");
		return;
	}

	bt_conn_get_info(conn, &info);

	LOG_DBG("New connection params: Interval: %d, Latency: %d, PHY: %d", info.le.interval, info.le.latency, info.le.phy->rx_phy);
	raise_zmk_peripheral_conn_state_changed((struct zmk_peripheral_conn_state_changed){
	    .source    = peripheral_index_by_conn(conn),
	    .connected = true,
	});
}

static void
peripheral_state_connected(struct bt_conn *conn, uint8_t conn_err)
{
	int                 i;
	char                addr_str[BT_ADDR_LE_STR_LEN];
	struct bt_conn_info info;

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr_str, sizeof(addr_str));
	bt_conn_get_info(conn, &info);

	if (info.role != BT_CONN_ROLE_CENTRAL)
	{
		LOG_DBG("SKIPPING FOR ROLE %d", info.role);
		return;
	}

	if (conn_err)
	{
		LOG_ERR("Failed to connect to %s (%u)", addr_str, conn_err);
		release_peripheral_by_conn(conn);
		return;
	}

	LOG_DBG("Connected: %s", addr_str);

	if ((i = reserve_peripheral_by_conn(conn)) < 0)
	{
		LOG_ERR("Unable to reserve peripheral source for connection (err %d)", i);
		return;
	}

	peripheral_state_process_connection(conn);
}

static void
peripheral_state_disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr_str[BT_ADDR_LE_STR_LEN];
	int  err;

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr_str, sizeof(addr_str));
	LOG_DBG("Disconnected: %s (reason %d)", addr_str, reason);

	raise_zmk_peripheral_conn_state_changed((struct zmk_peripheral_conn_state_changed){
	    .source    = peripheral_index_by_conn(conn),
	    .connected = false,
	});

	if ((err = release_peripheral_by_conn(conn)) < 0)
	{
		return;
	}
}

static struct bt_conn_cb conn_callbacks = {
    .connected    = peripheral_state_connected,
    .disconnected = peripheral_state_disconnected,
};

static int
zmk_peripheral_bt_init(void)
{
	bt_conn_cb_register(&conn_callbacks);
	return 0;
}

SYS_INIT(zmk_peripheral_bt_init, APPLICATION, CONFIG_ZMK_BLE_INIT_PRIORITY);
