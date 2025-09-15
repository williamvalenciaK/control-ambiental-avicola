#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

// Configuración OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display1(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // 0x3C
Adafruit_SSD1306 display2(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // 0x3D

// Configuración DHT
#define DHTPIN 2     
#define DHTTYPE DHT22   // O usa DHT11 si es tu caso
DHT dht(DHTPIN, DHTTYPE);

// Relés
#define RELE_HUMIDIFICADOR 46
#define RELE_DESHUMIDIFICADOR 48
#define RELE_CALOR 50
#define RELE_FRIO 52

// Valores objetivo (setpoints)
float tempSet = 25.0;   // °C
float humSet  = 60.0;   // % HR

// Variables globales para acciones
String accionh = "Inactivo"; // humedad
String acciont = "Inactivo"; // temperatura

void setup() {
  Serial.begin(9600);

  // Iniciar DHT
  Serial.println("Iniciando DHT...");
  dht.begin();

  // Iniciar pantallas
  Serial.println("Iniciando Pantallas...");
  if (!display1.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("No se encontró pantalla 1 (0x3C)"));
    for(;;);
  }
  if (!display2.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
    Serial.println(F("No se encontró pantalla 2 (0x3D)"));
    for(;;);
  }

  // Configurar pines
  Serial.println("Configurando pines...");
  pinMode(RELE_HUMIDIFICADOR, OUTPUT);
  pinMode(RELE_DESHUMIDIFICADOR, OUTPUT);
  pinMode(RELE_CALOR, OUTPUT);
  pinMode(RELE_FRIO, OUTPUT);

  // Apagar todo al inicio
  digitalWrite(RELE_HUMIDIFICADOR, LOW);
  digitalWrite(RELE_DESHUMIDIFICADOR, LOW);
  digitalWrite(RELE_CALOR, LOW);
  digitalWrite(RELE_FRIO, LOW);

  display1.clearDisplay();
  display1.setTextSize(1);
  display1.setTextColor(SSD1306_WHITE);
  display1.setCursor(0,0);
  display1.println("Iniciando DHT22...");
  display1.display();

  display2.clearDisplay();
  display2.setTextSize(1);
  display2.setTextColor(SSD1306_WHITE);
  display2.setCursor(0,0);
  display2.println("Iniciando control...");
  display2.display();

  Serial.println("Iniciando...");
  delay(1000);
}

void loop() {
  delay(1000); // el DHT necesita tiempo entre lecturas

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Validar lectura
  if (isnan(h) || isnan(t)) {
    Serial.println("Error leyendo del DHT!");
    return;
  }

  // Inicializar
  accionh = "Inactivo";
  acciont = "Inactivo";

  // --------- Mostrar en OLED principal (0x3C) ---------
  display1.clearDisplay();
  display1.setTextSize(1);
  display1.setCursor(0,0);
  display1.println("Lecturas actuales");

  display1.setTextSize(2.5);
  display1.setCursor(0,15);
  display1.print("T: "); display1.print(t); display1.println(" C");

  display1.setCursor(0,30);
  display1.print("\nH: "); display1.print(h); display1.println(" %");

  display1.display();

  // ---------------- Control de Humedad ----------------
  if (h < humSet - 5) {
    activarHumidificador(true);
    activarDeshumidificador(false);
    accionh = "Humidificando";
  } 
  else if (h > humSet + 5) {
    activarHumidificador(false);
    activarDeshumidificador(true);
    accionh = "Deshumidificando";
  } 
  else {
    activarHumidificador(false);
    activarDeshumidificador(false);
  }

  // ---------------- Control de Temperatura ----------------
  if (t < tempSet - 1) {
    activarVentilacion(1); // calentar
    acciont = "Calentando";
  } 
  else if (t > tempSet + 1) {
    activarVentilacion(-1); // enfriar
    acciont = "Enfriando";
  } 
  else {
    activarVentilacion(0);
  }

  // --------- Mostrar en segunda OLED (0x3D) ---------
  display2.clearDisplay();
  display2.setTextSize(1);
  display2.setCursor(0,0);
  display2.println("Parametros");

  display2.setCursor(0,15);
  display2.print("Set T: "); display2.print(tempSet); display2.println(" C");

  display2.setCursor(0,30);
  display2.print("Set H: "); display2.print(humSet); display2.println(" %");

  display2.setCursor(0,45);
  display2.print("Act H: "); display2.println(accionh);

  display2.setCursor(0,55);
  display2.print("Act T: "); display2.println(acciont);

  display2.display();
}

// ---------------- Funciones ----------------
void activarHumidificador(bool estado) {
  if (estado) {
    digitalWrite(RELE_HUMIDIFICADOR, HIGH);
  } else {
    digitalWrite(RELE_HUMIDIFICADOR, LOW);  // relé apagado
  }
}

void activarDeshumidificador(bool estado) {
  if (estado) {
    digitalWrite(RELE_DESHUMIDIFICADOR, HIGH);
  } else {
    digitalWrite(RELE_DESHUMIDIFICADOR, LOW);
  }
}

void activarVentilacion(int modo) {
  if (modo == 1) {
    // Calefacción
    digitalWrite(RELE_CALOR, HIGH);
    digitalWrite(RELE_FRIO, LOW);
  } 
  else if (modo == -1) {
    // Refrigeración
    digitalWrite(RELE_CALOR, LOW);
    digitalWrite(RELE_FRIO, HIGH);
  } 
  else {
    // Inactivo → apagar ambos relés y LEDs
    digitalWrite(RELE_CALOR, LOW);
    digitalWrite(RELE_FRIO, LOW);
  }
}
