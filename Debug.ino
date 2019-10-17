// Debug

void Debug(char* msg)
{
  Serial.print(msg);
}

void DebugDec(int val)
{
  char strVal[12];
  Debug(itoa(val, strVal, 10)); // Show value in decimal
}

void DebugLn(void)
{
  Debug("\r\n");
}
