#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/adc.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>

#define LED_PORT DT_ALIAS_LED0_GPIOS_CONTROLLER
#define LED_PIN DT_ALIAS_LED0_GPIOS_PIN

#define ADC_DEVICE_NAME DT_LABEL(DT_INST(0, nordic_nrf_saadc))

static struct bt_conn *default_conn;

void adc_callback(const struct device *dev, struct adc_seq_table *seq_table,
                  int error, u32_t *optional_results)
{
    if (error)
    {
        printk("ADC reading error: %d\n", error);
        return;
    }

    // Convert the ADC reading to voltage (assuming 3.3V reference voltage)
    float voltage = (optional_results[0] * 3.3) / 4096.0;

    // Send the voltage value via BLE
    bt_gatt_notify(default_conn, &attrs[0], &voltage, sizeof(voltage));

    // Toggle the LED while reading
    static bool led_state = true;
    led_state = !led_state;
    gpio_pin_set(gpio_dev, LED_PIN, led_state);
}

static void connected(struct bt_conn *conn, u8_t err)
{
    if (err)
    {
        printk("Connection failed (err %u)\n", err);
    }
    else
    {
        printk("Connected\n");
        if (!default_conn)
        {
            default_conn = bt_conn_ref(conn);
        }
    }
}

static void disconnected(struct bt_conn *conn, u8_t reason)
{
    printk("Disconnected (reason %u)\n", reason);
    if (default_conn)
    {
        bt_conn_unref(default_conn);
        default_conn = NULL;
    }
}

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
};

void main(void)
{
    int ret;
    struct device *adc_dev;
    struct adc_seq_entry entry = ADC_SEQ_ENTRY(0, ADC_CHANNEL_0, ADC_GAIN_1_6);
    struct adc_seq_table table = {
        .entries = &entry,
        .num_entries = 1,
    };

    ret = adc_read(adc_dev, &table);
    if (ret)
    {
        printk("Failed to read ADC: %d\n", ret);
        return;
    }

    adc_dev = device_get_binding(ADC_DEVICE_NAME);
    if (!adc_dev)
    {
        printk("ADC device not found\n");
        return;
    }

    ret = adc_read_async(adc_dev, &table, adc_callback, K_SECONDS(1));
    if (ret)
    {
        printk("Failed to read ADC asynchronously: %d\n", ret);
        return;
    }

    ret = bt_enable(NULL);
    if (ret)
    {
        printk("Bluetooth init failed (err %d)\n", ret);
        return;
    }

    printk("Bluetooth initialized\n");

    bt_conn_cb_register(&conn_callbacks);

    while (1)
    {
        k_sleep(K_FOREVER);
    }
}

