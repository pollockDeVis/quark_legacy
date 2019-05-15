#define RM50 55   //D
#define RM20 21   //F
#define RM10 10   //C
#define RM5 5     //B
#define RM1 1     //@

extern bool successfulTxn;

void tokenizer(int tokens)
{
  while (tokens > 0)
  {
    char* TOKEN_SYMBOL = NULL;
    
    if(tokens >= RM50)
    {
      TOKEN_SYMBOL = "D";
      tokens = tokens - 55;
    }
    else if (tokens >= RM20)
    {
      TOKEN_SYMBOL = "F";
      //Serial.println("F");
       tokens = tokens - 21;
    }
    else if (tokens >= RM10)
    {
      TOKEN_SYMBOL = "C";
      //Serial.println("C");
       tokens = tokens - 10;
    }
    else if (tokens >= RM5)
    {
      TOKEN_SYMBOL = "B";
      //Serial.println("B");
       tokens = tokens - 5;
    }
    else if (tokens >= RM1)
    {
      TOKEN_SYMBOL = "@";
      //Serial.println("@");
       tokens = tokens - 1;
    }else
    {
      TOKEN_SYMBOL = NULL;
    }
    
    Serial.println(TOKEN_SYMBOL);
    successfulTxn = true;
    
  }
  return;
}

int detokenizer(char Cashsignal)
{
  if (Cashsignal == '@')
  {
    return 1;
  }
  else if (Cashsignal == 'B')
  {
    return 5;
  }
  else if (Cashsignal == 'C')
  {
    return 10;
  }
  else if (Cashsignal == 'F')
  {
    return 20;
  }
  else if (Cashsignal == 'D')
  {
    return 50;
  }
  else {
    return 0; // Garbage values
  }
}
