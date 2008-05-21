/*@1@*/

#include <string.h>

#include "charscl.h"


chars::chars(void)
{
  chars_string= 0;
  chars_length= 0;
}

chars::chars(char *s)
{
  chars_string= 0;
  chars_length= 0;
  allocate_string(s);
}

chars::chars(const chars &cs)
{
  chars_string= 0;
  chars_length= 0;
  allocate_string((char*)cs);
}

chars::~chars(void)
{
  deallocate_string();
}


void
chars::allocate_string(char *s)
{
  deallocate_string();
  if (s)
    {
      chars_length= strlen(s);
      chars_string= new char[chars_length+1];
      strcpy(chars_string, s);
    }
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

  return *this;
}


// Assignment operators
chars &
chars::operator=(char *s)
{
  allocate_string(s);
  return(*this);
}

chars &
chars::operator=(const chars &cs)
{
  allocate_string((char*)cs);
  return(*this);
}


// Arithmetic operators
chars
chars::operator+(char *s)
{
  chars temp(chars_string);
  return(temp.append(s));
}

chars
chars::operator+(const chars &cs)
{
  chars temp(chars_string);
  return(temp.append(cs));
}

chars
operator+(char *s, const chars &cs)
{
  chars temp(s);
  return(temp.append((char*)cs));
};


// Boolean operators
bool
chars::equal(char *s)
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
chars::operator==(char *s)
{
  return(equal(s));
}

bool
chars::operator==(const char *s)
{
  return(equal((char*)s));
}

bool
chars::operator==(chars &cs)
{
  return(*this == (char*)cs);
}

bool
operator==(char *s, const chars &cs)
{
  return((chars(cs)).equal(s));
}

bool
operator==(const char *s, const chars &cs)
{
  return((chars(cs)).equal((char*)s));
}

bool
chars::operator!=(char *s)
{
  return(!equal(s));
}

bool
chars::operator!=(const char *s)
{
  return(!equal((char*)s));
}

bool
chars::operator!=(chars &cs)
{
  return(*this != (char*)cs);
}

bool
operator!=(char *s, const chars &cs)
{
  return(!(chars(cs)).equal(s));
}

bool
operator!=(const char *s, const chars &cs)
{
  return(!(chars(cs)).equal((char*)s));
}


/* End of chars.cc */
