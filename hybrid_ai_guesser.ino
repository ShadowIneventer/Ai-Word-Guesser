#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define BTN_YES   D5
#define BTN_NO    D6
#define BTN_MAYBE D7
#define BUZZER    D8

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Wi-Fi credentials
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// API endpoint
const char* apiUrl = "https://api.deva-ai.guesser.example/guess";  // Replace with working endpoint

const int maxQuestions = 30;
int currentQuestion = 0;
int questionCount = 10;
String answerLog[maxQuestions];
bool wentOnline = false;

const char* questions[] = {
      "Is it edible?", "Is it alive?", "Is it man-made?", "Is it electronic?",
        "Is it big?", "Is it used daily?", "Is it found outside?",
          "Is it a pet?", "Is it soft?", "Can it move?"
          };

          struct Object {
              const char* name;
                bool traits[10];
                };

                Object localObjects[] = {
                      { "Apple", { 1, 0, 0, 0, 0, 1, 0, 0, 1, 0 } },
                        { "Fan",   { 0, 0, 1, 1, 1, 1, 0, 0, 0, 1 } },
                          { "Dog",   { 0, 1, 0, 0, 1, 1, 1, 1, 1, 1 } },
                            { "Rock",  { 0, 0, 0, 0, 1, 0, 1, 0, 0, 0 } },
                              { "Chair", { 0, 0, 1, 0, 0, 1, 0, 0, 0, 0 } },
                                { "Ball",  { 0, 0, 1, 0, 0, 1, 1, 0, 1, 1 } }
                                };

                                const int numObjects = sizeof(localObjects) / sizeof(Object);
                                bool isCandidate[numObjects];

                                void setup() {
                                      Serial.begin(115200);
                                        lcd.init();
                                          lcd.backlight();
                                            pinMode(BTN_YES, INPUT_PULLUP);
                                              pinMode(BTN_NO, INPUT_PULLUP);
                                                pinMode(BTN_MAYBE, INPUT_PULLUP);
                                                  pinMode(BUZZER, OUTPUT);

                                                    for (int i = 0; i < numObjects; i++) isCandidate[i] = true;

                                                      connectWiFi();

                                                        lcd.setCursor(0, 0);
                                                          lcd.print("Think of a word");
                                                            delay(3000);
                                                              askQuestion();
                                                              }

                                                              void loop() {
                                                                  if (digitalRead(BTN_YES) == LOW) {
                                                                        processAnswer("yes");
                                                                            delay(300);
                                                                  }
                                                                    if (digitalRead(BTN_NO) == LOW) {
                                                                            processAnswer("no");
                                                                                delay(300);
                                                                    }
                                                                      if (digitalRead(BTN_MAYBE) == LOW) {
                                                                            processAnswer("maybe");
                                                                                delay(300);
                                                                      }
                                                                      }

                                                                      void askQuestion() {
                                                                          lcd.clear();
                                                                            if (currentQuestion >= questionCount) {
                                                                                    makeLocalGuess();
                                                                                     
                                                                                   return;
                                                                            }

                                                                              lcd.setCursor(0, 0);
                                                                                lcd.print("Q");
                                                                                  lcd.print(currentQuestion + 1);
                                                                                    lcd.setCursor(0, 1);
                                                                                      lcd.print(questions[currentQuestion]);
                                                                                      }

                                                                                      void processAnswer(String ans) {
                                                                                          answerLog[currentQuestion] = ans;
                                                                                            for (int i = 0; i < numObjects; i++) {
                                                                                                    if (!isCandidate[i]) continue;
                                                                                                        bool expected = localObjects[i].traits[currentQuestion];
                                                                                                            if ((ans == "yes" && !expected) || (ans == "no" && expected)) {
                                                                                                                      isCandidate[i] = false;
                                                                                                            }
                                                                                                        }
                                                                                                          currentQuestion++;
                                                                                                            askQuestion();
                                                                                                            }

                                                                                                            void makeLocalGuess() {
                                                                                                                  int count = 0, index = -1;
                                                                                                                    for (int i = 0; i < numObjects; i++) {
                                                                                                                            if (isCandidate[i]) {
                                                                                                                                      count++;
                                                                                                                                            index = i;
                                                                                                                            }
                                                                                                                        }

                                                                                                                          lcd.clear();
                                                                                                                            if (count == 1) {
                                                                                                                                    lcd.setCursor(0, 0);
                                                                                                                                        lcd.print("Is it a:");
                                                                                                                                            lcd.setCursor(0, 1);
                                                                                                                                                lcd.print(localObjects[index].name);
                                                                                                                                                    tone(BUZZER, 1000, 400);
                                                                                                                            } else {
                                                                                                                                    lcd.setCursor(0, 0);
                                                                                                                                        lcd.print("Guess failed!");
                                                                                                                                            lcd.setCursor(0, 1);
                                                                                                                                                lcd.print("Trying online...");
                                                                                                                                                    tone(BUZZER, 300, 400);
                                                                                                                                                        delay(2000);
                                                                                                                                                            wentOnline = true;
                                                                                                                                                                queryOnlineAPI();
                                                                                                                            }
                                                                                                                            }

                                                                                                                            void connectWiFi() {
                                                                                                                                  WiFi.begin(ssid, password);
                                                                                                                                    lcd.clear();
                                                                                                                                      lcd.setCursor(0, 0);
                                                                                                                                        lcd.print("Connecting WiFi");

                                                                                                                                          int tries = 0;
                                                                                                                                            while (WiFi.status() != WL_CONNECTED && tries < 10) {
                                                                                                                                                    delay(1000);
                                                                                                                                                        lcd.print(".");
                                                                                                                                                            tries++;
                                                                                                                                            }

                                                                                                                                              lcd.clear();
                                                                                                                                                if (WiFi.status() == WL_CONNECTED) {
                                                                                                                                                        lcd.setCursor(0, 0);
                                                                                                                                                            lcd.print("WiFi Connected!");
                                                                                                                                                } else {
                                                                                                                                                        lcd.setCursor(0, 0);
                                                                                                                                                            lcd.print("WiFi Failed!");
                                                                                                                                                }

                                                                                                                                                  delay(2000);
                                                                                                                                                  }

                                                                                                                                                  void queryOnlineAPI() {
                                                                                                                                                      if (WiFi.status() != WL_CONNECTED) return;

                                                                                                                                                        HTTPClient http;
                                                                                                                                                          http.begin(apiUrl);
                                                                                                                                                            http.addHeader("Content-Type", "application/json");

                                                                                                                                                              // Prepare JSON body
                                                                                                                                                                String json = "{"answers":[";
                                                                                                                                                              for (int i = 0; i < currentQuestion; i++) {
                                                                                                                                                                    json += """ + answerLog[i] + """;
                                                                                                                                                                        i(i < currentQuestion - 1) json += ",";
                                                                                                                                                              }
                                                                                                                                                                json += "]}";

                                                                                                                                                                  int httpCode = http.POST(json);

                                                                                                                                                                    if (httpCode > 0) {
                                                                                                                                                                            String payload = http.getString();
                                                                                                                                                                                StaticJsonDocument<256> doc;
                                                                                                                                                                                    DeserializationError err = deserializeJson(doc, payload);
                                                                                                                                                                                        if (err) {
                                                                                                                                                                                                  lcd.clear();
                                                                                                                                                                                                        lcd.print("JSON Parse Err");
                                                                                                                                                                                                              return;
                                                                                                                                                                                        }

                                                                                                                                                                                            const char* type = doc["type"];
                                                                                                                                                                                                if (strcmp(type, "guess") == 0) {
                                                                                                                                                                                                          const char* guess = doc["guess"];
                                                                                                                                                                                                                lcd.clear();
                                                                                                                                                                                                                      lcd.setCursor(0, 0);
                                                                                                                                                                                                                            lcd.print("Is it a...");
                                                                                                                                                                                                                                  lcd.setCursor(0, 1);
                                                                                                                                                                                                                                        lcd.print(guess);
                                                                                                                                                                                                  tone(BUZZER, 800, 500);
                                                                                                                                                                                                } else if (strcmp(type, "question") == 0) {
                                                                                                                                                                                                          const char* question = doc["question"];
                                                                                                                                                                                                                lcd.clear();
                                                                                                                                                                                                                      lcd.setCursor(0, 0);
                                                                                                                                                                                                                            lcd.print("Q:");
                                                                                                                                                                                                                                  lcd.setCursor(0, 1);
                                                                                                                                                                                                                                        lcd.print(question);
                                                                                                                                                                                                } else {
                                                                                                                                                                                                          lcd.clear();
                                                                                                                                                                                                                lcd.print("Unknown reply");
                                                                                                                                                                                                }
                                                                                                                                                                                            } else {
                                                                                                                                                                                                    lcd.clear();
                                                                                                                                                                                                        lcd.print("HTTP error:");
                                                                                                                                                                                                            lcd.setCursor(0, 1);
                                                                                                                                                                                                                lcd.print(httpCode);
                                                                                                                                                                                            }

                                                                                                                                                                                              http.end();
                                                                                                                                                                                              }
                                                                                                                                                                                              
                                                                                                                                                                                  