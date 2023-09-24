extern char *strerror(int);

__swi(0) void writec(int);


static void puts(char *s)
{   int ch;
    for (ch = *s;  ch != 0;  ch = *++s) writec(ch);
}

int main()
{
    puts("\nstrerror(42) returns \"");
    puts(strerror(42));
    puts("\"\n\r\n");

    return 0;
}
