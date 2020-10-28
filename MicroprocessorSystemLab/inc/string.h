#ifndef __STRING__
#define __STRING__

int atoi(char *str)
{
	int i=0;
	int ans = 0;
	while(str[i]!='\0')
	{
		ans = ans *10;
		ans = ans + (str[i++]-'0');
	}
	return ans;
}

void strcpy(char *dest, char *src)
{
	int i=0;
	while(src[i]!='\0')
	{
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
}

int strcmp(char *s1, char *s2)
{
	int i=0;
	while (s1[i]!=0 || s2[i]!=0)
	{
		if (s1[i] > s2[i]) return 1;
		else if (s1[i] <s2 [i]) return 2;
		i++;
	}
	return 0;
}

void itoa(char *str, int n)
{
	int tmp = n;
	int digit = 0;
	int p = 1;
	while ( (tmp/p) != 0 )
	{
		p = p*10;
		digit++;
	}
	p = p / 10;
	if (p == 0)  // when quotient is zero
		p = 1;
	int it = 0;
	if (n < 0)
	{
		str[it++] = '-';
		n = -n;
	}
	while (p!=0)
	{
		str[it++] = (n/p)+'0';
		int t = n/p;
		n = n - (t*p);
		p = p / 10;
	}
	str[it] = '\0';
}

int strlen(char *str)
{
	int i = 0;
	while (str[i]!='\0') i++;
	return i;
}

void str_add_float(char* str, float tmp)
{

}

#endif
