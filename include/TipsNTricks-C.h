/*
To print the MAC address into a string
*/
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);

/* How to use printf with Serial */
  Serial.printf("Board ID %u: %u bytes\n", incomingReadings.master_id, len);
  Serial.printf("t value: %4.2f \n", incomingReadings.floatvalue1);

/*Convert an int into char[] */
	int message_id = 12;
	char msg_id[12];
	itoa(message_id, msg_id, 10);
