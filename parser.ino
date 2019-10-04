// parser

char* GetNextItem(char* dict)
{
  char ch;
  //Serial.print("In GetNextItem");
  while (ch = *dict++) {
    if (ch == ',') break; // Advance beyond ',' to next <item>:<val> pair
    if (('}' == ch) || ('{' == ch)) {
      //Serial.println(", found start or end of dict");
      return dict-1;  // Stop at start or end of dict
    }
  }
  //Serial.println(", found comma");
  return dict;  // Now pointing to next <item>
}

bool CmpItem(char* target, char* dict)
{
  char quoteType;
  //Serial.print("CmpItem:"); Serial.print(target);
  while ((*dict != '\'') && (*dict != '\"')) dict++; // Get opening quote
  quoteType = *dict++;  // Note quote type (single or double)
  //Serial.print(" in "); Serial.println(dict);
  while (*target++ == *dict++) ;  // Keep advancing until we have a mismatch
  return ((*--target == '\0') && (*--dict == quoteType));  // Return true if the target is finished and the source dict string is also finished, thus a perfect match
  /*if ((*--target == '\0') && (*--dict == '\'')) {
    Serial.println("Match");
    return true;
  } else {
    Serial.print("No match with ");
    Serial.println(*dict);
    return false;
  }*/
}

char* FindItem(char* dict, char* item)
{
  Serial.print("FindItem:"); Serial.println(item);
  if ('{' == *dict) { // First char of dict is opening curly brace, so skip it
    while ('}' != *dict) {  // Exit if we reach the end of the dict
      dict = GetNextItem(dict); // Advance dict to start of <item> (after comma), or to close curly brace
      if ('}' == *dict) break;  // Stop at end of dict
      if (CmpItem(item, dict)) {
        //Serial.println("Got match!");
        while (*dict++ != ':') ; // Advance beyond ':'
        while ((*dict != '\'') && (*dict != '\"')) dict++; // Get opening quote
        return dict;  // Now pointing to <val> associated with <item>, including opening quote
      } //else Serial.println("Advance to next item...");
      dict++; // Advance beyond start of this item in order to get to next one...
    }
  }
  Serial.println("Not a dict!");
  return 0; // Indicate we've not found the item
}

void GetDictVal(char* dict, char* item, char* val)
{
  char* answer;
  char quoteType;
  answer = FindItem(dict, item);  // Try and find item in dict.  Result is pointer to beginning of <val>, immediately after opening quote
  if (answer) {
    quoteType = *answer++;  // Note quote type (single or double)
    while (*answer != quoteType) *val++ = *answer++; // Copy each character from <val> until we hit the correct terminating quote
    *val = '\0';  // Terminate result by replacing close quote with \0
  } else {
    strcpy("N/A", val);
  }
}
