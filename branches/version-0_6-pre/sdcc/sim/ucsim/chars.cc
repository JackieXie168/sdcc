/*@1@*/

//#include <stdio.h>
#include <string.h>

#include "charscl.h"


chars::chars(void)
{
  //printf("%p()\n", this);
  chars_string= 0;
  chars_length= 0;
  //printf("%p=\"%s\"\n", this, chars_string);
}

chars::chars(char *s)
{
  //printf("%p(\"%s\")\n", this, s);
  chars_string= 0;
  chars_length= 0;
  allocate_string(s);
  //printf("%p=\"%s\"\n", this, chars_string);
}

chars::chars(const chars &cs)
{
  //printf("%p(%p/%s)\n", this, &cs, (char*)cs);
  chars_string= 0;
  chars_length= 0;
  allocate_string((char*)cs);
  //printf("%p=\"%s\"\n", this, chars_string);
}

chars::~chars(void)
{
  //printf("~%p\n", this);
  deallocate_string();
}


void
chars::allocate_string(char *s)
{
  //printf("%p.alloc(%p\"%s\")\n", this, s, s);
  deallocate_string();
  if (s)
    {
      //printf(" do allocation s=%p\"%s\"\n", s, s);
      chars_length= strlen(s);
      //printf(" len=%d\n", chars_length);
      chars_string= new char[chars_length+1];
      //printf(" mem=%p (s=%s)\n", chars_string, s);
      strcpy(chars_string, s);
      //printf(" string=\"%s\"\n", chars_string);
    }
  //printf("allocated=\"%s\"\n", chars_string);
}

void
chars::deallocate_string(void)
{
  if (chars_string)
    delete [] chars_string;
  chars_string= 0;
  chars_length= 0;
}


chars &
chars::append(char *s)
{
  //printf("%p.append(\"%s\")\n", this, s);
  if (!s)
    return(*this);

  char *temp= new char[chars_length + strlen(s) + 1];
  if (chars_string)
    strcpy(temp, chars_string);
  else
    temp[0]= '\0';
  strcat(temp, s);
  allocate_string(temp);
  delete [] temp;
  //printf("appended=\"%s\"\n", chars_string);

  return *this;
}


// Assignment operators
chars &
chars::operator=(char *s)
{
  //printf("%p=(\"%s\")\n", this, s);
  allocate_string(s);
  return(*this);
}

chars &
chars::operator=(const chars &cs)
{
  //printf("%p=(%p/%s)\n", this, &cs, (char*)cs);
  allocate_string((char*)cs);
  return(*this);
}


// Arithmetic operators
chars
chars::operator+(char *s)
{
  //printf("%p+(\"%s\")\n", this, s);
  chars temp(chars_string);
  //printf("%p+ temp=%p/%s\n", this, &temp, (char*)temp);
  return(temp.append(s));
}

chars
chars::operator+(const chars &cs)
{
  //printf("%p+(%p/%s)\n", this, &cs, (char*)cs);
  chars temp(chars_string);
  //printf("%p+ temp=%p/%s\n", this, &temp, (char*)temp);
  return(temp.append(cs));
}

chars
operator+(char *s, const chars &cs)
{
  //printf("+(\"%s\",%p/%s)\n", s, &cs, (char*)cs);
  chars temp(s);
  //printf("+ temp=%p/%s\n", &temp, (char*)temp);
  return(temp.append((char*)cs));
};


// Boolean operators
bool
chars::operator==(char *s)
{
  if ((chars_string &&
       !s) ||
      (!chars_string &&
       s))
    return(0);
  if (!chars_string &&
      !s)
    return(1);
  return(strcmp(chars_string, s) == 0);
}

bool
chars::operator==(chars &cs)
{
  return(*this == (char*)cs);
}

bool
operator==(char *s, const chars &cs)
{
  return(cs == s);
}

bool
chars::operator!=(char *s)
{
  if ((chars_string &&
       !s) ||
      (!chars_string &&
       s))
    return(1);
  if (!chars_string &&
      !s)
    return(0);
  return(strcmp(chars_string, s) != 0);
}

bool
chars::operator!=(chars &cs)
{
  return(*this != (char*)cs);
}

bool
operator!=(char *s, const chars &cs)
{
  return(cs != s);
}


/* End of chars.cc */
