#include <msp430.h>

/**
 * main.c
 *  Autor: C. Josué Espinosa Cruz
 *  Fecha: 10 de mayo de 2020
 *
 *
 *  El voltaje se varía con un potenciómetro externo.
 */

unsigned int valor_adc_binario1 = 0;
unsigned int valor_adc_binario2 = 0;
unsigned int valor_adc_binario3 = 0;
unsigned int valor_adc_binario4 = 0;

unsigned int tx_buffer[459]; //unsigned char

unsigned char i = 0;

unsigned char bandera_inicializar = 0x00;
unsigned int contador_general = 0;
unsigned int contador_timer = 0;
unsigned int contador_timer2 = 0;
unsigned int contador_timer3 = 0;

unsigned int contador_arreglo1 = 0;
unsigned int contador_arreglo2 = 8;
unsigned int contador_arreglo3 = 16;
unsigned int contador_arreglo4 = 24;

unsigned int contador_arreglo1_2 = 0;
unsigned int contador_arreglo2_2 = 0;
unsigned int contador_arreglo3_2 = 0;
unsigned int contador_arreglo4_2 = 0;

unsigned char banderaADC_Activar = 0x00;
unsigned char banderaTimer_Activar = 0x00;

void Configurar_GPIO(void)
{
    //Señal_Funcionamiento_TimerA0
    P1DIR |= 0x01;                                   // Se configura el bit 0 del puerto 1 como salida. Los demás bits, como entrada.
    P1OUT = 0x00;                                   // Se ponen todos los pines del puerto 1 en 0 (en LOW).

    //Output Contador_Multiplexor_Columnas
    P4DIR |= 0x09;                                  // Se configura el bit 0 y 3 del puerto 4 como salida. Los demás bits, como entrada.
    P3DIR |= BIT7;                                  // Se configura el bit 7 del puerto 3 como salida. Los demás bits, como entrada.

    P4OUT &= ~0x09;
    P3OUT &= ~BIT7;

    //Output Control_Shift_Registers
    P1DIR |= 0x3C;                                  // Se configura el bit 2, 3, 4 y 5 del puerto 1 como salida. Los demás bits, como entrada.
    P1OUT &= ~0x3C;                                 // Se mandan a 0 todos los bits
    P1OUT |= 0x1C;                                  // Se mandan a positivos para que inicie "0111"
}

void Configurar_TimerA(void)
{
    TA0CCTL0 = CCIE;                                // Activa la interrupción por CCR.
    TA0CCR0 = 130;                                // Configura el Capture Compare Register a aprox. (1/131072)*131 = 1ms

                                                    //Carga cuenta en TA1CCR0 0.1seg TA1CCR=(0.1*32768)-1
    TA0CTL |= TASSEL_2 | MC_1 | ID_3 | TACLR;       // SMCK= 1048576Hz | UP mode | Divisor de DCO / 8 | limpieza de registro TAR (contador del Timer)
}                                                   //1048576Hz/8 = 131072 Hz

void Configurar_ADC(void)
{
    P6SEL |= 0xF;            // Se selecciona el bit 0, 1, 2 y 3 del puerto 6 como Analog Input.
    ADC12CTL0 |= ADC12SHT0_2 | ADC12ON;     // Se enciende el ADC
    ADC12CTL1 = ADC12SHP;                   // Se selecciona el Sampling Timer como reloj de conversión
    ADC12MCTL0 |= ADC12INCH_0;                      // Se selecciona el canal A0 (P6.0)
    ADC12MCTL1 |= ADC12INCH_1;                      // Se selecciona el canal A1 (P6.1)
    ADC12MCTL2 |= ADC12INCH_2;                      // Se selecciona el canal A2 (P6.2)
    ADC12MCTL3 |= ADC12INCH_3;                      // Se selecciona el canal A3 (P6.3)
}

void Configurar_UART(void)
{
    P4SEL |= 0x30;                   // P4.4 TXD P4.5 RXD
    UCA1CTL1 |= UCSWRST;            // Configurar State Machine en modo Reset para modificar los demás registros
    UCA1CTL1 |= UCSSEL_2;           // Seleccionar SMCLK como reloj
    UCA1BR0 = 109;                  // 9600 bauds @ ~1MHz (User's guide, página 952)
    UCA1BR1 = 0;                    // 1MHz 9600
    UCA1MCTL = UCBRS_1;             // 9600 bauds @ ~1MHz (User's guide, página 952)

    UCA1CTL1 &= ~UCSWRST;           // Inicializa la State Machine del USCI
    UCA1IE |= UCRXIE;               // Activa las interrupciones de RX
}

void Adquirir_ADC_Canal0(void)
{
    ADC12CTL1 |= ADC12CSTARTADD_0;                  //Se selecciona la ubicación MEM a guardar la información (MEM0)

    ADC12CTL0 |= ADC12ENC;                          // Se activan las conversiones
    ADC12CTL0 |= ADC12SC;                           // Inicia la conversión

    while (!(ADC12IFG & 0x01));                    // Espera mientras esté ocupado el ADC.
    valor_adc_binario1 = ADC12MEM0;                 // Lee el valor del registro ADC12MEM0

    ADC12CTL0 &= ~ADC12ENC;                          // Se desactivan las conversiones
    ADC12CTL1 &= ~ADC12CSTARTADD_0;                  //Se selecciona la ubicación MEM a guardar la información (MEM0)
}

void Adquirir_ADC_Canal1(void)
{
    ADC12CTL1 |= ADC12CSTARTADD_1;                  //Se selecciona la ubicación MEM a guardar la información (MEM1)

    ADC12CTL0 |= ADC12ENC;                          // Se activan las conversiones
    ADC12CTL0 |= ADC12SC;                           // Inicia la conversión

    while (!(ADC12IFG & 0x02));                    // Espera mientras esté ocupado el ADC.
    valor_adc_binario2 = ADC12MEM1;                 // Lee el valor del registro ADC12MEM0

    ADC12CTL0 &= ~ADC12ENC;                          // Se desactivan las conversiones
    ADC12CTL1 &= ~ADC12CSTARTADD_1;                  //Se selecciona la ubicación MEM a guardar la información (MEM1)
}

void Adquirir_ADC_Canal2(void)
{
    ADC12CTL1 |= ADC12CSTARTADD_2;                  //Se selecciona la ubicación MEM a guardar la información (MEM2)

    ADC12CTL0 |= ADC12ENC;                          // Se activan las conversiones
    ADC12CTL0 |= ADC12SC;                           // Inicia la conversión

    while (!(ADC12IFG & 0x04));                    // Espera mientras esté ocupado el ADC.
    valor_adc_binario3 = ADC12MEM2;                 // Lee el valor del registro ADC12MEM0

    ADC12CTL0 &= ~ADC12ENC;                          // Se desactivan las conversiones
    ADC12CTL1 &= ~ADC12CSTARTADD_2;                  //Se selecciona la ubicación MEM a guardar la información (MEM2)
}

void Adquirir_ADC_Canal3(void)
{
    ADC12CTL1 |= ADC12CSTARTADD_3;                  //Se selecciona la ubicación MEM a guardar la información (MEM2)

    ADC12CTL0 |= ADC12ENC;                          // Se activan las conversiones
    ADC12CTL0 |= ADC12SC;                           // Inicia la conversión

    while (!(ADC12IFG & 0x08));                    // Espera mientras esté ocupado el ADC.
    valor_adc_binario4 = ADC12MEM3;                 // Lee el valor del registro ADC12MEM0

    ADC12CTL0 &= ~ADC12ENC;                          // Se desactivan las conversiones
    ADC12CTL1 &= ~ADC12CSTARTADD_3;                  //Se selecciona la ubicación MEM a guardar la información (MEM2)
}

void Transformar_Informacion_ADC1(void)
{

    if(contador_arreglo1_2<=7)
    {
        tx_buffer[contador_arreglo1] = valor_adc_binario1 / 16;
        contador_arreglo1++;
        contador_arreglo1_2++;
    }
    else if(contador_arreglo1_2==8)
    {
        contador_arreglo1=contador_arreglo1 + 20;
        contador_arreglo1_2 = 0;
    }
}

void Transformar_Informacion_ADC2(void)
{
    if(contador_arreglo2_2<=7)
    {
        tx_buffer[contador_arreglo2] = valor_adc_binario2 / 16;
        contador_arreglo2++;
        contador_arreglo2_2++;
    }
    else if(contador_arreglo2_2==8)
    {
        contador_arreglo2=contador_arreglo2 + 20;
        contador_arreglo2_2 = 0;
    }
}

void Transformar_Informacion_ADC3(void)
{
    if(contador_arreglo3_2<=7)
    {
        tx_buffer[contador_arreglo3] = valor_adc_binario3/16;
        contador_arreglo3++;
        contador_arreglo3_2++;
    }
    else if(contador_arreglo3_2==8)
    {
        contador_arreglo3=contador_arreglo3 + 20;
        contador_arreglo3_2 = 0;
    }

}

void Transformar_Informacion_ADC4(void)
{
    if(contador_arreglo4_2<=2)
    {
        tx_buffer[contador_arreglo4] = valor_adc_binario4 / 16;
    }
    else if(contador_arreglo4_2==7)
    {
        contador_arreglo4=contador_arreglo4 + 20;
        contador_arreglo4_2 = 0;
    }

    contador_arreglo4++;
    contador_arreglo4_2++;
}

void Mandar_Informacion(void)
{
    for(i = 0; i < 255; i++)                           //sizeof(tx_buffer)
    {
        while (!(UCA1IFG & UCTXIFG));               // Espera mientras envía un byte, es decir, si hay pendiente algo por enviar
        UCA1TXBUF = tx_buffer[i];                   // Envía un byte a la vez
    }
    for(i = 1; i < 204; i++)                           //sizeof(tx_buffer)
    {
        while (!(UCA1IFG & UCTXIFG));               // Espera mientras envía un byte, es decir, si hay pendiente algo por enviar
        UCA1TXBUF = tx_buffer[i+255];                   // Envía un byte a la vez
    }

    while (!(UCA1IFG & UCTXIFG));
    UCA1TXBUF = 0x0A;
}

void Selector_Mux(void)
{
    switch(contador_timer3)
        {
        case 0:
            P4OUT &= ~BIT3;
            P4OUT &= ~BIT0;
            P3OUT &= ~BIT7;
            break;
        case 1:
            P4OUT &= ~BIT3;
            P4OUT &= ~BIT0;
            P3OUT |= BIT7;
            break;

        case 2:
            P4OUT &= ~BIT3;
            P4OUT |= BIT0;
            P3OUT &= ~BIT7;
            break;

        case 3:
            P4OUT &= ~BIT3;
            P4OUT |= BIT0;
            P3OUT |= BIT7;
            break;

        case 4:
            P4OUT |= BIT3;
            P4OUT &= ~BIT0;
            P3OUT &= ~BIT7;
            break;

        case 5:
            P4OUT |= ~BIT3;
            P4OUT &= ~BIT0;
            P3OUT |= BIT7;
            break;

        case 6:
            P4OUT |= BIT3;
            P4OUT |= BIT0;
            P3OUT &= ~BIT7;
            break;

        case 7:
            P4OUT |= BIT3;
            P4OUT |= BIT0;
            P3OUT |= BIT7;
            break;
        }
}

int main(void)
{

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    Configurar_GPIO();
    Configurar_TimerA();
    Configurar_ADC();
    Configurar_UART();

    __enable_interrupt();                   // Activa las interrupciones globales

    while(1)
    {
        if(bandera_inicializar==0x01)       //Si se manda la activación desde la computadora, se manda la información
        {
            if(banderaADC_Activar==0x01)
            {
                Adquirir_ADC_Canal0();
                Adquirir_ADC_Canal1();
                Adquirir_ADC_Canal2();
                Adquirir_ADC_Canal3();

                Transformar_Informacion_ADC1();
                Transformar_Informacion_ADC2();
                Transformar_Informacion_ADC3();
                Transformar_Informacion_ADC4();

                banderaADC_Activar=0x00;
                banderaTimer_Activar = 0x01;

                contador_general++;

                if(contador_general==136)
                {
                    bandera_inicializar=0x00;
                    Mandar_Informacion();
                    contador_arreglo1 = 0;
                    contador_arreglo2 = 8;
                    contador_arreglo3 = 16;
                    contador_arreglo4 = 24;

                    contador_arreglo1_2 = 0;
                    contador_arreglo2_2 = 0;
                    contador_arreglo3_2 = 0;
                    contador_arreglo4_2 = 0;
                }
            }
        }
    }

}

// Rutina de interrupción del Timer 0
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    if(bandera_inicializar==0x01)                       //Si se manda la activación desde la computadora, se manda la información del timer
    {
        if(banderaTimer_Activar == 0x01)
        {
            P1OUT ^= 0x01;                                  // XOR al P1.0, este funcionará como un indicador de funcionamiento. Solo bajaremos la velocidad del Timer cuando lo requiramos

            contador_timer++;

            if(contador_timer==1)                           //Pulso principal a 1ms el cual inicia el barrido de Shift Register
            {
                P1OUT |= BIT5;                              //SH_CP del shift register se manda a 1 (ver Proteus)
                if(contador_timer2==0)                      //Si es la primera vez que se acciona el shift register
                {
                    P1OUT |= BIT4;                              //DS del del shift register se manda a 1 (ver proteus)
                }
                else                                        //Se desactiva el DS del shift register para que deje de mandar nuevos pulsos del barrido
                {
                    P1OUT &= ~BIT4;                         //DS del del shift register se manda a 0 (ver proteus)
                }
                P1OUT &= ~BIT3;                             //ST_CP del shift register se manda a 0 (ver proteus)
            }
            else if(contador_timer==2)
            {
                contador_timer=0;               //Se resetea el contador_timer para que se reinicie la maquina de estados
                contador_timer2++;

                P1OUT &= ~BIT5;                 ////SH_CP del shift register se manda a 0 (ver Proteus)
                if(contador_timer2==17)         //Cuando el pulso del shift register llega a la fila 17
                {
                    P1OUT |= BIT4;              //DS del del shift register se manda a 1 (ver proteus)
                    contador_timer2=0;          //Se resetea el contador_timer2 para que vuelva a mandar el pulso de activiación al shift register
                    contador_timer3++;          //Aumena la cuenta para cambiar la selector del MUX (ver proteus)
                    if(contador_timer3==8)      //Cuando sea la última opción del selector: "111"
                    {
                        contador_timer3=0;      //Re resetean los selectores del mux a "000"
                    }
                    Selector_Mux();             //Función para la selección de los multiplexores
                }
                else
                {
                    P1OUT &= ~BIT4;             ////DS del del shift register se manda a 0 (ver proteus)
                }
                P1OUT |= BIT3;                  ////ST_CP del shift register se manda a 0 (ver proteus)

                banderaADC_Activar=0x01;        //Bandera que desactiva al ADC
                banderaTimer_Activar = 0x00;    //Bandera que desactiva al Timer
            }

        }
    }
}

// Rutina de interrupción de UART
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
  switch(__even_in_range(UCA1IV,4))
  {
  case 0:break;                             // Vector 0 - No hay interrupción
  case 2:                                   // Vector 2 - RXIFG
    while(!(UCA1IFG & UCTXIFG));            // Espera mientras envía un byte, es decir, si hay pendiente algo por enviar
    if(UCA1RXBUF=='I')                      //Si adquiere una A, comenzará a mandar información y el Timer se activará
    {
        bandera_inicializar=0x01;           //Bandera que permite el trabajo del ADC y Timer
        banderaTimer_Activar = 0x01;        //Bandera que activa al Timer
    }
    else if(UCA1RXBUF=='D')
    {
        bandera_inicializar=0x00;           //Bandera que desactiva al ADC y Timer
    }
    break;
  case 4:break;                             // Vector 4 - TXIFG
  default: break;
  }
}
