typedef struct half_words_struct
{ unsigned field1:16;
  unsigned field2:16;
} half_words;

half_words max( half_words a, half_words b )
{ half_words x;
  x= (a.field1>b.field1) ? a : b;
  return x;
}
