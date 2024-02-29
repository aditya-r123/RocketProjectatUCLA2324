#include <string>
using namespace std;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}
void loop() {
  
  int y = random(300);
  //char y = x + '0';
  //Serial.println("0.1233 PT1:" y, PT2: 200, PT3: 300, PT4: 400, PT5: 500, LC1: 0.1, LC2: 0.2");
  
  Serial.print("0.123 PT1: ");
  Serial.print(y);
  Serial.print(", PT2: 200, PT3: 300, PT4: 400, PT5: 500, LC1: 0.1, LC2: 0.2, TC1: 60, TC2: 70\n");
  // );
  delay(200);  //tr/files-pri/T01827RH821-F06LW3NL7SR/image.pngy 50, 200, or 250
}
