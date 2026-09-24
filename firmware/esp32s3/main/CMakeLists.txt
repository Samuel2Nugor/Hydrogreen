idf_component_register(
    SRCS "main.c" "wifi.c" "mqtt.c" "sensors/sht31.c" "sensors/ds18b20_roles.c"
    INCLUDE_DIRS "." "sensors"
    REQUIRES esp_wifi esp_event esp_netif nvs_flash
             driver esp_timer json mqtt
             onewire_bus ds18b20
)
