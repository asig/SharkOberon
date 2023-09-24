typedef struct two_ch_struct
{ char ch1;
  char ch2;
} two_ch;
 
two_ch max( two_ch a, two_ch b )
{
  return (a.ch1>b.ch1) ? a : b;
}
