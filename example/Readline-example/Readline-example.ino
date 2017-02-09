
#include "Readline.h"

void setup()
{
  Serial.begin(115200);

  while (!Serial) ;

  Serial.println("Starting...");
}


void loop()
{
  String line;

  line = Readline("prompt> ");

  Serial.println(line);


}



