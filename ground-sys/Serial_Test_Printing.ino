#include <string>
using namespace std;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}
void loop() {
  
 
  Serial.println("0.123 PT1: 150, PT2: 210, PT3: 300, PT4: 400, PT5: 500, PT6: 600, LC1: 0.1, LC2: 0.2, TC1: 60, TC2: 70");
  delay(200);  //50, 200, or 250
}
