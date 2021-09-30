package com.adjust.sdk;

public enum TrackingState {
  OPTED_OUT(1);

  private int value;

  TrackingState(int value) {
    this.value = value;
  }

  public int getValue() {
    return this.value;
  }
}
