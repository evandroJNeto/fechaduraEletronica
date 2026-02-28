#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Keypad.h>
#include <Servo.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x20, 16, 2); // Inicialização padrão do PCF8574
int senhaPadrao;
Servo servo; // Inicializando o objeto servo
const int pinoServo = 9;
const int ledVerde = 10;
const int ledVermelho = 11; // Definindo os pinos utilizados e conectados para seus respectivos atuadores para cada variável
bool querMudar = false;
String senhaDigitada = ""; // 	Variaveis necessárias para a lógica da mudança de senha e da string digitada

const byte linhas = 4;
const byte colunas = 4;
char teclas[linhas][colunas] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};// Definindo as teclas como um vetor para facilitar o processo de identificação de qual numero está sendo digitado
byte pinosLinhas[] = {2, 3, 4, 5};
byte pinosColunas[] = {6, 7, 8, A0};

Keypad teclado = Keypad(makeKeymap(teclas), pinosLinhas, pinosColunas, linhas, colunas);
// inicializando o objeto teclado e mapeando para caracteres correspondentes
void setup() {
  EEPROM.get(0, senhaPadrao); // pegando da mémoria EEPROM a senhaPadrao atual, útil para caso a senhaPadrao seja alterada, na inicialização ela já volta a original
  if(senhaPadrao == -1 || senhaPadrao > 9999){
    senhaPadrao = 0;
    EEPROM.put(0, senhaPadrao);
  }// E colocando ela como padrão 0000
  
  Serial.begin(9600); // inicialização da comunicação serial em 9600 bits por segundo
  servo.attach(pinoServo); 
  servo.write(0); //Atribuindo o servo a seu respectivo pino e escrevendo o estado inicial(deitado), 0
  pinMode(ledVermelho, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  // configurando os leds 
  lcd.init();                    
  lcd.backlight();               
  lcd.setCursor(0,0); // iniciando o lcd e sua backlight padrões, o cursor fica setado em 0,0 para começar da primeira linha
  digitalWrite(ledVermelho, HIGH);
  digitalWrite(ledVerde, LOW);
  lcd.print("Bloqueado");// o LED vermelho é ativado enquanto o LED verde é desligado, o LCD exibe a mensagem bloqueado
}

void loop() {
  char tecla = teclado.getKey();

  if (tecla) {
    senhaDigitada += tecla;
    lcd.setCursor(0, 1);
    lcd.print(senhaDigitada);
  }// a tecla pressionada é adcionada a variável senhaDigitada para depois ser comparada com a senhaPadrao
  
  if (senhaDigitada.length() >= 4) {
    int senhaInt = senhaDigitada.toInt();
//senhaDigitada que é uma String é transformada em Int para fins de comparação
    if (senhaInt == senhaPadrao) {
      servo.write(90);
      digitalWrite(ledVermelho, LOW);
      digitalWrite(ledVerde, HIGH);
      // Servo é movido para 90 graus simbolizando a destranca
      lcd.clear(); // Limpa a mensagem do LCD para exibir outra 
      lcd.print("Desbloqueado");
      Serial.println("\nAcesso ok! Mudar senha? (Y/N)(3s)");
//O buffer pede para mudar a senha
      long tempoInicio = millis();
      querMudar = false;
      // tempo em 3 segundos para mudar a senha depois é bloqueado novamente
      while(millis() - tempoInicio < 3000) {
        if (Serial.available() > 0) {
          char resposta = Serial.read(); 
          if(resposta == 'Y' || resposta == 'y'){ 
            querMudar = true;
            break;
          }
        }
      } 
      
      if(querMudar) { 
        Serial.println("Digite a nova senha (ate 4 digitos):"); 
        
        while(Serial.available() > 0) { Serial.read(); }
        while(Serial.available() == 0); 
        
        int senhaNova = Serial.parseInt(); 
        senhaPadrao = senhaNova; 
        EEPROM.put(0, senhaPadrao); 
        Serial.println("Senha Alterada!"); // Coloca byte por byte na EEPROM a nova senha, util para quebrar o limite de um BYTE de outra função da EEPROM, EEPROM.write()
      }
      
      delay(2000); 
      servo.write(0); 
      digitalWrite(ledVerde, LOW); 
      digitalWrite(ledVermelho, HIGH); 
      lcd.clear(); 
      lcd.print("Bloqueado");
      // Liga o Led Vermelho e bloqueia de novo
    } else { 
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Senha Errada!   "); 
      delay(2000); 
      lcd.clear(); 
      lcd.print("Bloqueado"); 
    }// senha errada continua a bloquear
    // reseta a senha digitada no final
    senhaDigitada = "";
  }
}