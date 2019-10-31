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

void DebugLn(char* msg)
{
  Debug(msg);
  Debug("\r\n");
}

void DebugDecLn(int val)
{
  DebugDec(val);
  Debug("\r\n");
}
