#define RM50 55   //D
#define RM20 21   //F
#define RM10 10   //C
#define RM5 5     //B
#define RM1 1     //@
#define MAIN_SERIAL Serial1
extern bool successfulTxn;
const int INF PROGMEM= 100000;


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
char* internal_tokenizer(int _val)
{
  char* TOKEN_SYMBOL = NULL;
  
  if(_val == RM50)
    {
      TOKEN_SYMBOL = "D";
    }
    else if (_val == RM20)
    {
      TOKEN_SYMBOL = "F";
    }
    else if (_val == RM10)
    {
      TOKEN_SYMBOL = "C";
    }
    else if (_val == RM5)
    {
      TOKEN_SYMBOL = "B";
    }
    else if (_val == RM1)
    {
      TOKEN_SYMBOL = "@";
    }else
    {
      TOKEN_SYMBOL = NULL;
    }
    
  return TOKEN_SYMBOL;
}
int coin_change_algorithm(int d[], int n, int k) {
  
  int M[n+1];
  M[0] = 0;

  int S[n+1];
  S[0] = 0;

  int i, j;
  for(j=1; j<=n; j++) {
    int minimum = INF;
    int coin=0;

    for(i=1; i<=k; i++) {
      if(j >= d[i]) {
        if((1+M[j-d[i]]) < minimum) {
          minimum = 1+M[j-d[i]];
          coin = i;
        }
      }
    }
    M[j] = minimum;
    S[j] = coin;
  }

  int l = n;

//  for (int ii = 0; ii <= n; ++ii )
//  {
    Serial.print("l = ");
    //char* TOKEN_SYMBOL = internal_tokenizer(d[S[ii]]) ;
    Serial.println(l);
   // ii = ii + d[S[ii]];
//  }
Serial.print("M[n] = ");
  Serial.println(M[n]);
  char* inverse_array[M[n]];
  int array_counter = M[n]-1;
  while(l>0) {
  
  
   //Serial.printf("%d\n",d[S[l]]);
   char* TOKEN_SYMBOL = internal_tokenizer(d[S[l]]) ;
   inverse_array[array_counter] = TOKEN_SYMBOL;
   array_counter--;

    l = l-d[S[l]];
  }
  for(int j = 0; j< M[n]; j++)
  {
     //Replicating the actual Signal
     MAIN_SERIAL.println(inverse_array[j]);
  }

  return M[n];
}

void tokenizer(int tokens)
{
  int d[] = {0, 1, 5, 10, 21, 55};
  int returnVal = coin_change_algorithm(d, tokens, 5);
  successfulTxn = true;
}
