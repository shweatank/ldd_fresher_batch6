

#define led_1 1<<9
void delay_ms(int);

int main(){

}

void delay_ms(int ms){
T0PR=15000-1;
T0TCR=0x01;
while(TOTC<ms);
T0TCR=0x03;
}

