/* Jogo da Sequência, por Emanoel Rodrigues, 2020
github.com/emrodrigues
biblioteca LiquidCrystal_I2C baixada de https://blogmasterwalkershop.com.br/arquivos/libs/NewliquidCrystal.zip
Fique a vontade para editar e aprimorar este algoritmo*/

//bibliotecas
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  //NÃO acompanha a IDE do Arduino
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3, POSITIVE); //LCD 16x2

//leds - para um bom funcionamento, sempre mantenha os LEDs em sequência. Porém não importa qual a primeira porta.
#define LED_G 8
#define LED_Y 9
#define LED_R 10
#define LED_B 11
#define QUANT_LEDS 4 //altere caso adicione ou remova leds.

//botões - neste caso a sequência não é necessária.
#define PUSH_G 2
#define PUSH_Y 3
#define PUSH_R 4
#define PUSH_B 5

//configurações do jogo
#define SEQ_MAX 50          //não é possível deixar o jogo infinito ou muito grande por limitações de memória.
#define MULTIPLICADOR 1.3   //define o quanto a pontuação aumenta a cada rodada.
#define PONT_INICIAL 10     //pontuação da primeira rodada.

//outros
#define TEMPO_ON 500        //tempo LED aceso (em milissegundos)
#define TEMPO_OFF 250       //tempo LED apagado
#define INDEFINIDO -1
#define BUZZER 13

//variáveis globais
enum Estados{PRONTO_PROX_RODADA, PLAYER_RESPONDENDO, JOGO_FINALIZADO};
byte luzes[SEQ_MAX];
byte rodada = 0;            //altere de byte para int se o número de rodadas for maior que 255.
byte leds_respondidos = 0;  //mesma coisa do de cima
int pontos = 0;
int newPontos = PONT_INICIAL; //salva a próxima pontuação de cada rodada.
int record;
bool recordRodou = false;
bool pMaxRodou = false;

void setup()
{
  //descomente abaixo para apagar o recorde
  //EEPROM.write(0,0); EEPROM.write(1,0);
  iniciaPortas();
  iniciaJogo();
}

int estadoAtual()
{
  if(rodada <= SEQ_MAX)
  {
    if (leds_respondidos == rodada) return PRONTO_PROX_RODADA;
    else return PLAYER_RESPONDENDO;
  }

  else if (rodada > SEQ_MAX) return JOGO_FINALIZADO;
}

void loop()
{
  switch(estadoAtual()){
    case PRONTO_PROX_RODADA: preparaNovaRodada();
      break;

    case PLAYER_RESPONDENDO: processaResposta();
      break;

    case JOGO_FINALIZADO: fimDeJogo();
      break;
  }
}

void preparaNovaRodada()
{
  rodada++;
  pontos = pontos + newPontos;
  newPontos = newPontos*MULTIPLICADOR;

  if(rodada == 1){pontos = 0; newPontos = PONT_INICIAL;}
  if(rodada == 2) pontos = PONT_INICIAL;
  
  leds_respondidos = 0;
  
  updateTela(); 
  
  if (rodada <= SEQ_MAX) ledsRodada();
}

void processaResposta()
{
  int resposta = checaBotao();
  if (resposta == INDEFINIDO) return;

  if (resposta == luzes[leds_respondidos]) leds_respondidos++;
  else rodada = SEQ_MAX + 10;
}

void ledsRodada()
{
for (int i = 0; i < rodada; i++) piscaLed(luzes[i]);
}

int checaBotao()
{
  if (digitalRead(PUSH_G) == LOW) return piscaLed(LED_G);
  if (digitalRead(PUSH_Y) == LOW) return piscaLed(LED_Y);
  if (digitalRead(PUSH_R) == LOW) return piscaLed(LED_R);
  if (digitalRead(PUSH_B) == LOW) return piscaLed(LED_B);

  return INDEFINIDO;
}

void fimDeJogo()
{
  int POSIC1 = tamanhoPontos(pontos);
  int POSIC2 = tamanhoPontos(record);

  if((pontos > record) && (recordRodou == false)) recorde(), POSIC2 = tamanhoPontos(record);//caso tenha batido recorde.
    
  if((rodada == SEQ_MAX+1) && (pMaxRodou == false)) pMax();//atingiu a pontuação máxima

  limpaTela();

  lcd.setCursor(3,0);
  lcd.print("FIM DE JOGO");
  lcd.setCursor(0,1);
  lcd.print("P. Final:");
  lcd.setCursor(POSIC1,1);
  lcd.print(pontos);

  piscaLed(LED_G);
  piscaLed(LED_Y);
  piscaLed(LED_R);
  piscaLed(LED_B);
    
  limpaTela();

  lcd.setCursor(0,0);
  lcd.print("Record");
  lcd.setCursor(POSIC2,1);
  lcd.print(record);

  piscaLed(LED_B);
  piscaLed(LED_R);
  piscaLed(LED_Y);
  piscaLed(LED_G);
}

int piscaLed(int portaLed)
{
  digitalWrite(portaLed,HIGH);
  tone(BUZZER, portaLed*50);
  delay(TEMPO_ON);

  digitalWrite(portaLed,LOW);
  noTone(BUZZER);
  delay(TEMPO_OFF);

  return portaLed;
}

void iniciaPortas()
{
  lcd.begin(16,2);
  lcd.setBacklight(HIGH);

  pinMode(LED_G,OUTPUT);
  pinMode(LED_Y,OUTPUT);
  pinMode(LED_R,OUTPUT);
  pinMode(LED_B,OUTPUT);

  pinMode(PUSH_G,INPUT_PULLUP);
  pinMode(PUSH_Y,INPUT_PULLUP);
  pinMode(PUSH_R,INPUT_PULLUP);
  pinMode(PUSH_B,INPUT_PULLUP);

  pinMode(BUZZER, OUTPUT);
}

void iniciaJogo()
{
  byte lowByte = EEPROM.read(0);
  byte highByte = EEPROM.read(1);
  record = ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);

  lcd.setCursor(3,0);
  lcd.print("BEM-VINDO");
  lcd.setCursor(0,1);
  lcd.print("teste de memoria");
  delay(3000);
  limpaTela();

  int jogo = analogRead(A0);
  randomSeed(jogo);
  for (int i = 0; i < SEQ_MAX; i++) luzes[i] = random(LED_G, LED_G + QUANT_LEDS);
}

void recorde()
{
  byte lowByte = ((pontos >> 0) & 0xFF);
  byte highByte = ((pontos >> 8) & 0xFF);
  EEPROM.write(0,lowByte);
  EEPROM.write(1,highByte);
        
  limpaTela();

  lcd.setCursor(0,0);
  lcd.print("!!NOVO RECORDE!!");
  lcd.setCursor(0,1);
  lcd.print("!!!!PARABENS!!!!");

  delay(100);
    
  for(int i = 0; i < 7; i++)
  {
    lcd.setBacklight(LOW);
    for(int i = LED_G; i < LED_G + QUANT_LEDS; i++) digitalWrite(i,LOW);
    delay(250);

    lcd.setBacklight(HIGH);
    for(int i = LED_G; i < LED_G + QUANT_LEDS; i++) digitalWrite(i,HIGH);
    delay(500);
  }
    
  for(int i = LED_G; i < LED_G + QUANT_LEDS; i++) digitalWrite(i,LOW);
    
  recordRodou = true;
  record = pontos;
}

void pMax()
{
  limpaTela();

  lcd.setCursor(0,0);
  lcd.print("PONTUACAO MAXIMA!");
  lcd.setCursor(0,1);
  lcd.print("PARABENS!!!");

  piscaLed(LED_G);
  piscaLed(LED_Y);
  piscaLed(LED_R);
  piscaLed(LED_B);

  pMaxRodou = true;
}

void limpaTela()
{
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("                ");
}

void updateTela()
{
  int POSIC2 = tamanhoPontos(pontos);
  int POSIC1 = tamanhoPontos(rodada);

  limpaTela();

  lcd.setCursor(0,0);
  lcd.print("Rodada:");
  lcd.setCursor(POSIC1,0);
  lcd.print(rodada);

  lcd.setCursor(0,1);
  lcd.print("Pontos:");
  lcd.setCursor(POSIC2,1);
  lcd.print(pontos);
}

//determina a posição do número para que sempre fique a direita da tela.
int tamanhoPontos(int check)
{
  String pontos = String(check);
  return 16 - pontos.length();
}
