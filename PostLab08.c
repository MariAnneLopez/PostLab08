/* 
 * File:   PostLab08.c
 * Author: Marian Lopez
 *
 * Created on 20 de abril de 2022, 07:40 PM
 */

// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>
#define _XTAL_FREQ 4000000 // Asigna frecuencia 4MHz a función delay

// PROTOTIPOS DE FUNCIONES
void setup (void);
void display7seg(void);


// VARIABLES GOBLALES
char tab7seg[10]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90};
char var_temp;      
char unidades = 0;  
char decenas = 0;
char residuo = 0;
char centenas = 0;
char unidad_d = 0;
char decena_d = 0;
char centena_d = 0;
char cont = 0;

// INTERRUPCIONES
void __interrupt() isr (void){    
    if(PIR1bits.ADIF){ // Si fue int. ADC entra
        if(ADCON0bits.CHS == 0) // Si fue int. de AN0
            PORTD = ADRESH; // 8 bits mas significativos
        else if (ADCON0bits.CHS == 1) // Si fue int. de AN1
            var_temp = ADRESH; // 8 bits mas significativos
        PIR1bits.ADIF = 0; // Limpia bandera de interrupción
    }
    
    if(INTCONbits.T0IF){ // Si fue int. TMR0 entra
        PORTE = 0b111;
        if (cont == 0){
            PORTC = centena_d;
            PORTEbits.RE0 = 0;
            cont = 1;
        }
        else if(cont == 1){
            PORTC = decena_d;
            PORTEbits.RE1 = 0;
            cont = 2;
        }
        else if(cont == 2){
            PORTC = unidad_d;
            PORTEbits.RE2 = 0;
            cont = 0;
        }
        INTCONbits.T0IF = 0; // Apaga bandera TMR0
        TMR0 = 217; // Resetea TMR0
    }
    return;
}

// CICLO PRINCIPAL
void main (void) {
    setup();
    while(1){
        display7seg(); // Conversión binario a decimal
        
        if(ADCON0bits.GO == 0){ // Si no hay proceso de conversión
            if(ADCON0bits.CHS == 0) // Cambia a canal 1
                ADCON0bits.CHS = 1;
            else if(ADCON0bits.CHS == 1) // Cambia a canal 0
                ADCON0bits.CHS = 0;
            
            __delay_us(50); // Estabilización del capacitor           
            ADCON0bits.GO = 1;// Inicia la conversión
        }
    }
    return;
}

// CONFIGURACIONES
void setup (void){
    ANSEL = 0b00000011; // AN0 y AN1 como analógico
    ANSELH = 0;
    
    TRISA = 0b00000011; //AN0 y AN1 como entrada
    TRISBbits.TRISB7 = 0;
    TRISC = 0;
    TRISD = 0;
    TRISE = 0;
    
    PORTA = 0;
    PORTBbits.RB7 = 1;
    PORTC = 0;
    PORTD = 0;
    TRISE = 6;
    
    // Configuraciones de reloj
    OSCCONbits.IRCF = 0b0110; // 4MHz
    OSCCONbits.SCS = 1; // Oscilador interno
    
    // Configuraciones ADC
    ADCON0bits.ADCS = 1; // Fosc/8
    ADCON0bits.CHS = 0b0000; // Seleccionar AN0
    ADCON0bits.ADON = 1; // Habilitar modulo ADC
    ADCON1bits.VCFG0 = 0; // VDD *Referencias internas
    ADCON1bits.VCFG1 = 0; // VSS
    ADCON1bits.ADFM = 0; // Justificado a la izquierda
    __delay_us(50); // Delay de 40 us
    
    // Configuraciones de interrupciones
    PIR1bits.ADIF = 0; // Limpiar bandera de int. ADC
    PIE1bits.ADIE = 1; // Habilitar int. ADC
    INTCONbits.PEIE = 1; // Interrupción de periféricos
    INTCONbits.GIE = 1; // Interrupciones globales
    INTCONbits.T0IE = 1; // Interrupción de TMR0
    INTCONbits.T0IF = 0; // Bandera int. TMR0 apagada
    
    // Configuraciones de TMR0
    OPTION_REG = 0b11010111; // T0CS = 0, PSA = 0, PS = 256
    TMR0 = 217; // N de TMR0 para 10 ms
    INTCONbits.T0IF = 0;
    
    return;
}

// CONVERSIÓN DECIMAL A BINARIO
void display7seg(){   
    centenas = var_temp/100; // Obtener centenas
    residuo = var_temp%100; // Almacenar el residuo
    decenas = residuo/10; // Obtener decenas
    unidades = var_temp%10; // Obtener unidades 
    unidad_d = tab7seg[unidades]; //Conversión de tabla 
    decena_d = tab7seg[decenas];
    centena_d = tab7seg[centenas];
    return;
}
