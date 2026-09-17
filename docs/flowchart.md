```mermaid
graph TD;
    start-->data[system collect data from sensor];
    data-->send{send sensor data to server}
    send-->data
```