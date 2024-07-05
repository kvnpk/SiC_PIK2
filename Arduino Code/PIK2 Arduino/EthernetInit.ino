void EthernetInit() {
  Ethernet.init(53);
  delay(100);
  while (Ethernet.linkStatus() == LinkOFF) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Ethernet cable is not connected.");
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
  digitalWrite(LED_BUILTIN, LOW);
  if (Ethernet.begin(mac)) {
    Serial.print("server is at: ");
    Serial.println(Ethernet.localIP());
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
