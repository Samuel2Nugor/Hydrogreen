#pragma once

/* Brings up WiFi station mode and registers reconnect handling. Blocks up to
 * 30 s for the initial connection, then returns regardless — the driver
 * keeps retrying in the background. */
void wifi_init(void);
