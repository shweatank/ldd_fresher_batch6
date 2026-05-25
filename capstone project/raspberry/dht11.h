#define GPIO_DHT (4 + 512)
#define PULSE_TIMEOUT   200

int dht11_read_data(int *temp, int *hum);
int dht11_init(void);
void dht11_exit(void);
