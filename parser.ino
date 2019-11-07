// parser

char* GetNextItem(char* dict)
{
  char ch;
  while (ch = *dict++) {
    if (ch == ',') break; // Advance beyond ',' to next <item>:<val> pair
    if (('}' == ch) || ('{' == ch)) {
      return dict-1;  // Stop at start or end of dict
    }
  }
  return dict;  // Now pointing to next <item>
}

bool CmpItem(char* target, char* dict)
{
  char quoteType;
  while ((*dict != '\'') && (*dict != '\"')) dict++; // Get opening quote
  quoteType = *dict++;  // Note quote type (single or double)
  while (*target++ == *dict++) ;  // Keep advancing until we have a mismatch
  return ((*--target == '\0') && (*--dict == quoteType));  // Return true if the target is finished and the source dict string is also finished, thus a perfect match
}

char* FindItem(char* dict, char* item)
{
  //Debug("FindItem:"); DebugLn(item);
  if ('{' == *dict) { // First char of dict is opening curly brace, so skip it
    while ('}' != *dict) {  // Exit if we reach the end of the dict
      dict = GetNextItem(dict); // Advance dict to start of <item> (after comma), or to close curly brace
      if ('}' == *dict) break;  // Stop at end of dict
      if (CmpItem(item, dict)) {
        //DebugLn("Got match!");
        while (*dict++ != ':') ; // Advance beyond ':'
        while ((*dict != '\'') && (*dict != '\"')) dict++; // Get opening quote
        return dict;  // Now pointing to <val> associated with <item>, including opening quote
      } //else DebugLn("Advance to next item...");
      dict++; // Advance beyond start of this item in order to get to next one...
    }
  }
  RenderSadFace("Bad report");
  DebugLn("Not a dict!");
  return 0; // Indicate we've not found the item
}

bool GetDictVal(char* dict, char* item, char* val)
{
  char* answer;
  char quoteType;
  answer = FindItem(dict, item);  // Try and find item in dict.  Result is pointer to beginning of <val>, immediately after opening quote
  if (answer) {
    quoteType = *answer++;  // Note quote type (single or double)
    while (*answer != quoteType) *val++ = *answer++; // Copy each character from <val> until we hit the correct terminating quote
    *val = '\0';  // Terminate result by replacing close quote with \0
  }
  return (bool)answer;
}

bool CmpDictVal(char* dict, char* item, char* oldVal) // Returns false if the oldVal is the same as the new value from the dict
{
  char dictVal[64];
  if (GetDictVal(dict, item, dictVal)) {
    return (bool)strcmp(dictVal, oldVal);  // strcmp returns 0 if the same, so make that false by casting
  } else return false;
}
