



///////////////////////////////////////////////////////////////////////////////////////////////////
// Specific sensor reading functions
///////////////////////////////////////////////////////////////////////////////////////////////////

//








////////////////////////////////////////////////////////////////////////////////////
// string/json parsing functions 
////////////////////////////////////////////////////////////////////////////////////


const char gIgnoreChars[] = " \n\r\t:,{}";  // set of chars we ignore as white space

bool isIgnoredChar(char c) {
  bool isIgnored = false;
  for (const char *ignore = gIgnoreChars; *ignore != '\0'; ignore++) {
    if (c == *ignore) {
      isIgnored = true;
      break;
    }
  }
  return isIgnored;
}

// Find location of next char which isn't "white space"
char* eatWhiteSpace(char *buf) {
  char c = *buf;

  //always end on a null char
  while (c != '\0') {
    if (!isIgnoredChar(c)) return buf;
    // this current c counts as white space, so move on to the next buf position
    c = *++buf;
  }

  return buf;
}

// return the start and end of the next token, after ignoring white space
char* tokenize(char *buf, char **tokenEnd)
{
  char *start = eatWhiteSpace(buf);
  char *s = start;  // iterator for buf
  char c = *s;

  // iterate through s until we find a characer which is not in gIgnoredChars
  while (c != '\0' && !isIgnoredChar(c)) {
    c = *++s;
  }

  // found the end of the token, so write it if we were given a location to store it
  if (tokenEnd) *tokenEnd = s;
  return start;
}

/*
{ setEntry: { index:0, label:'ceiling temp', pins: [4,13] }}

*/


#if 0
// (code thoughts in progress)
void parse(char *buf)
{
  // chars which go on the stack: '"[
  const int MAXSTACK = 20;
  int ixTop = 0;
  char stack[MAXSTACK];
  
  struct DeviceEntry {char name[16], sensorID[16];} devices[4];
  int ixDevice = 0;
  
  const int MAX_FIELD_NAME = 16;
  char fieldName[MAX_FIELD_NAME];
  bool curStringIsFieldName = true;
  
  char *tokenStart, *tokenEnd;
  char *s;
  char c = *buf;

  tokenStart = tokenize(buf, tokenEnd);
  c = *tokenStart;
  
  if (c == '[') {
    
  }
  
  if (c == '\'' || c == '\"') {  // start of string
    char *s = tokenEnd;
    tokenStart++;  // move past quote mark
    //find end string quote mark
    while (c != *s) {
      if (*s == '\0') {
        //error string didn't end
        return;
      }
      s++;
    }
     
    strncpy(getStringDest(curStringIsFieldName, devices[ixDevice]), tokenStart, s-1-tokenStart);
  }
  
  if (stack[ixTop] == '\'' || stack[ixTop] == '\"') {

    while (*s != stack[ixTop]) s++;  // this allows nulls in string...
    
    if (curStringIsFieldName) strncpy(fieldName, tokenStart, s-tokenStart);
      strncpy(tokenStart, fieldNameToLocation(
}
#endif 




