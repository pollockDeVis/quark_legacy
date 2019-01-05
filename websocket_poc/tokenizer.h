#define RM50 55
#define RM20 21
#define RM10 10
#define RM5 5
#define RM1 1

void tokenizer(int tokens)
{
  while (tokens > 0)
  {
    
    if(tokens >= RM50)
    {
      Serial.println("D");
      tokens = tokens - 55;
    }
    else if (tokens >= RM20)
    {
      Serial.println("F");
       tokens = tokens - 21;
    }
    else if (tokens >= RM10)
    {
      Serial.println("C");
       tokens = tokens - 10;
    }
    else if (tokens >= RM5)
    {
      Serial.println("B");
       tokens = tokens - 5;
    }
    else if (tokens >= RM1)
    {
      Serial.println("@");
       tokens = tokens - 1;
    }
    
    
  }
  return;
}
