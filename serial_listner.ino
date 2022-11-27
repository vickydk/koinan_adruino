/*  
 *              1000 = 1 pulse 
 *              500  = 2 pulse 
 *              200  = 3 pulse
 *              100  = 4 pulse
 *     ___             ___             ___
 *    |   |           |   |           |   |
 *    |   |           |   |           |   |
 * ___|   |___________|   |___________|   |__________________________
 * 
 * ___|___|___|___|___|___|___|___|___|___|__________________________
 *     30  30  30  30  30  30  30  30  30  
 *--------|---------------|---------------|--------------------------
 *        ^     30x4      ^     30x4      ^
 *        
 */ 


unsigned long next_state_change = 0;
int pattern_timing_list[]       = {0, 0, 0, 0, 0, 0, 0, 0};
byte array_length               = (sizeof(pattern_timing_list) / sizeof(pattern_timing_list[0]));
byte next_array_index           = 0;
boolean next_gpio_status        = LOW;

void simulate_puleses() {
  /*
    Serial.print(next_array_index); Serial.print("|");
    Serial.print(array_length); Serial.print("|");
    Serial.print(millis()); Serial.print("|");
    Serial.print(next_state_change); Serial.print("|");
    Serial.print(next_gpio_status); Serial.println("|");
  */
  if (next_array_index < array_length) {
    if (millis() >= next_state_change) {
      if (next_array_index == 0) {
        next_gpio_status = HIGH;
      }
      digitalWrite(COIN_PULSE_OUT_PIN, next_gpio_status);
      next_state_change = millis() + pattern_timing_list[next_array_index];
      next_gpio_status = !next_gpio_status;
      next_array_index = next_array_index + 1;
      if (pattern_timing_list[next_array_index] == 0) {
        next_gpio_status = LOW;
      }
    }
  }
}

void serial_command_listner() {
  String serial_cmd = "";
  int finish = 0;
  if (Serial.available()) {
    while (Serial.available() || finish == 0) {
      char nw = (char) Serial.read();
      if (nw != '\r') { //check for line ending character
        //Serial.println(serial_cmd);
        serial_cmd += nw;
      } else {
        //serial_cmd += nw;
        finish = 1;
        exit;
      }
      delay(1); //wait for the next byte
    }
  }
  serial_cmd.trim(); //trim the string
  if (serial_cmd.length() >= 1) { // validate the received data
    //Serial.println(serial_cmd);
    serial_cmd.replace("\n", "");  // remove the new line charactor
    if (SERIAL_DEBUG) {
      Serial.print(F("CMD:"));
      Serial.println(serial_cmd);
    }
    switch (serial_cmd.toInt()) {
      case 1000:
        /*
          digitalWrite(COIN_PULSE_OUT_PIN, HIGH);
          delay(PULSE_HIGH_WIDTH);
          digitalWrite(COIN_PULSE_OUT_PIN, LOW);
        */
        pattern_timing_list[0] = PHW;
        pattern_timing_list[1] = 0;
        pattern_timing_list[2] = 0;
        pattern_timing_list[3] = 0;
        pattern_timing_list[4] = 0;
        pattern_timing_list[5] = 0;
        pattern_timing_list[6] = 0;
        pattern_timing_list[7] = 0;
        next_array_index = 0;
        break;
      case 500:
        /*
          digitalWrite(COIN_PULSE_OUT_PIN, HIGH);
          delay(PULSE_HIGH_WIDTH);
          digitalWrite(COIN_PULSE_OUT_PIN, LOW);
          delay(PULSE_LOW_WIDTH);
          digitalWrite(COIN_PULSE_OUT_PIN, HIGH);
          delay(PULSE_HIGH_WIDTH);
          digitalWrite(COIN_PULSE_OUT_PIN, LOW);
        */
        pattern_timing_list[0] = PHW;
        pattern_timing_list[1] = PLW;
        pattern_timing_list[2] = PHW;
        pattern_timing_list[3] = 0;
        pattern_timing_list[4] = 0;
        pattern_timing_list[5] = 0;
        pattern_timing_list[6] = 0;
        pattern_timing_list[7] = 0;
        next_array_index = 0;
        break;
      case 200:
        /*
          digitalWrite(COIN_PULSE_OUT_PIN, HIGH);
          delay(PULSE_HIGH_WIDTH);
          digitalWrite(COIN_PULSE_OUT_PIN, LOW);
          delay(PULSE_LOW_WIDTH);
          digitalWrite(COIN_PULSE_OUT_PIN, HIGH);
          delay(PULSE_HIGH_WIDTH);
          digitalWrite(COIN_PULSE_OUT_PIN, LOW);
          delay(PULSE_LOW_WIDTH);
          digitalWrite(COIN_PULSE_OUT_PIN, HIGH);
          delay(PULSE_HIGH_WIDTH);
          digitalWrite(COIN_PULSE_OUT_PIN, LOW);
        */
        pattern_timing_list[0] = PHW;
        pattern_timing_list[1] = PLW;
        pattern_timing_list[2] = PHW;
        pattern_timing_list[3] = PLW;
        pattern_timing_list[4] = PHW;
        pattern_timing_list[5] = 0;
        pattern_timing_list[6] = 0;
        pattern_timing_list[7] = 0;
        next_array_index = 0;
        break;
      case 100:
        /*
          digitalWrite(COIN_PULSE_OUT_PIN, HIGH);
          delay(PULSE_HIGH_WIDTH);
          digitalWrite(COIN_PULSE_OUT_PIN, LOW);
          delay(PULSE_LOW_WIDTH);
          digitalWrite(COIN_PULSE_OUT_PIN, HIGH);
          delay(PULSE_HIGH_WIDTH);
          digitalWrite(COIN_PULSE_OUT_PIN, LOW);
          delay(PULSE_LOW_WIDTH);
          digitalWrite(COIN_PULSE_OUT_PIN, HIGH);
          delay(PULSE_HIGH_WIDTH);
          digitalWrite(COIN_PULSE_OUT_PIN, LOW);
          delay(PULSE_LOW_WIDTH);
          digitalWrite(COIN_PULSE_OUT_PIN, HIGH);
          delay(PULSE_HIGH_WIDTH);
          digitalWrite(COIN_PULSE_OUT_PIN, LOW);
        */
        pattern_timing_list[0] = PHW;
        pattern_timing_list[1] = PLW;
        pattern_timing_list[2] = PHW;
        pattern_timing_list[3] = PLW;
        pattern_timing_list[4] = PHW;
        pattern_timing_list[5] = PLW;
        pattern_timing_list[6] = PHW;
        pattern_timing_list[7] = 0;
        next_array_index = 0;
        break;
      default:
        Serial.println("Invalid coin");
        break;
    }
  }
}
