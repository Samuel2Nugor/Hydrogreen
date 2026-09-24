#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "math.h"

#include "hardware/adc.h"

#define led 16
#define soilTemp 26
// #define humidityTemp 27

double Thermister(int rawAdc)
{
    double temp;
    temp = log(((10240000/rawAdc) - 10000));
    temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * temp * temp))* temp);
    temp = temp - 273.15;
    return temp;
}

int main()
{
    stdio_init_all();

    adc_init();
    adc_gpio_init(soilTemp);
    // adc_gpio_init(humidityTemp);

    gpio_init(led);
    gpio_set_dir(led, GPIO_OUT);

    uint16_t raw;

    // // Initialise the Wi-Fi chip
    // if (cyw43_arch_init()) {
    //     printf("Wi-Fi init failed\n");
    //     return -1;
    // }

    // // Enable wifi station
    // cyw43_arch_enable_sta_mode();

    // printf("Connecting to Wi-Fi...\n");
    // if (cyw43_arch_wifi_connect_timeout_ms("Your Wi-Fi SSID", "Your Wi-Fi Password", CYW43_AUTH_WPA2_AES_PSK, 30000)) {
    //     printf("failed to connect.\n");
    //     return 1;
    // } else {
    //     printf("Connected.\n");
    //     // Read the ip address in a human readable way
    //     uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
    //     printf("IP address %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    // }

    while (true) {
        gpio_put(led, true);
        adc_select_input(0);
        raw = adc_read();
        double temp = Thermister(raw);
        printf("Soil temp: %lfC\n", temp);
        sleep_ms(1000);
        gpio_put(led, false);
        sleep_ms(300);

    }
}
