/*
 * Copyright (c) 2015 by Thomas Trojer <thomas@trojer.net>
 * Decawave DW1000 library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 * Use this to test two-way communication functionality with two
 * DW1000. Both Arduinos use this sketch, but one node configured
 * as initiator/sender of the (first) ping message and the other 
 * being configured as receiver/answerer of the (first) ping message.
 *
 * Configure each node by setting their "trxToggle" attribute to either
 * "SENDER" or "RECEIVER".
 */

#include <SPI.h>
#include <DW1000.h>

// toggle state
#define SENDER true
#define RECEIVER false
// toggle and message RX/TX
// NOTE: the other Arduino needs to be configured with RECEIVER
volatile boolean trxToggle = SENDER;
volatile boolean trxAck = false;
String msg;
// reset line to the chip
int RST = 9;

void setup() {
  // DEBUG monitoring
  Serial.begin(9600);
  Serial.println("### DW1000-arduino-ping-pong-test ###");
  // initialize the driver
  DW1000.begin(SS, RST, 0);
  Serial.println("DW1000 initialized ...");
  // general configuration
  DW1000.newConfiguration();
  DW1000.setDefaults();
  DW1000.setDeviceAddress(1);
  DW1000.setNetworkId(10);
  DW1000.setFrameFilter(false);
  DW1000.commitConfiguration();
  Serial.println("Committed configuration ...");
  // DEBUG chip info and registers pretty printed
  Serial.print("Device ID: "); Serial.println(DW1000.getPrintableDeviceIdentifier());
  Serial.print("Unique ID: "); Serial.println(DW1000.getPrintableExtendedUniqueIdentifier());
  Serial.print("Network ID & Device Address: "); Serial.println(DW1000.getPrintableNetworkIdAndShortAddress());
  // attach callback for (successfully) sent and received messages
  DW1000.attachSentHandler(handleSent);
  DW1000.attachReceivedHandler(handleReceived);
  // anchor starts by transmitting a POLL message
  if(trxToggle == SENDER) {
    msg = "Ping";
    receiver();
    transmit();
  } else {
    msg = "Pong";
    receiver();
  }
}

void handleSent() {
  // status change on sent success
  trxAck = true;
}

void handleReceived() {
  // status change on received success
  trxAck = true;
}

void transmit() {
  DW1000.newTransmit();
  DW1000.setDefaults();
  DW1000.setData(msg);
  DW1000.startTransmit();
}

void receiver() {
  DW1000.newReceive();
  DW1000.setDefaults();
  // so we don't need to restart the receiver manually
  DW1000.receivePermanently(true);
  DW1000.startReceive();
}

void loop() {
  if(!trxAck) {
    return;
  }
  // continue on any success confirmation
  trxAck = false; 
  trxToggle = !trxToggle;
  if(trxToggle == SENDER) {
    // fomerly in receiving mode
    String rxMsg; 
    DW1000.getData(rxMsg);
    Serial.print("Received | "); Serial.println(rxMsg);
    transmit();
  } else {
    Serial.print("Transmitted | "); Serial.println(msg);
  }
}

