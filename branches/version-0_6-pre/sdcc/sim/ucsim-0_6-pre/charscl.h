/*@1@*/

#ifndef CHARSCL_HEADER
#define CHARSCL_HEADER


class chars
{
private:
  char *chars_string;	// stores the value
  int chars_length;	// track of string length
public:
  chars(void);
  chars(char *s);
  chars(const chars &cs);
  virtual ~chars(void);
private:
  void allocate_string(char *s);
  void deallocate_string(void);

public:
  chars &append(char *s);

public:
  // Operators

  // Cast
  *operator char(void) { return(chars_string); };
  *operator char(void) const { return(chars_string); };
  // Assignment
  chars &operator=(char *s);
  chars &operator=(const chars &cs);
  // Arithmetic
  chars operator+(char *s);
  chars operator+(const chars &cs);
  chars &operator+=(char *s) { return(append(s)); }
  chars &operator+=(const chars &cs) { return(append((char*)cs)); }
  // Boolean
  bool equal(char *);
  bool operator==(char *s);
  bool operator==(const char *s);
  bool operator==(chars &cs);
  bool operator!=(char *s);
  bool operator!=(const char *s);
  bool operator!=(chars &cs);
};

extern chars operator+(char *s, const chars &cs);
extern bool operator==(char *s, const chars &cs);
extern bool operator==(const char *s, const chars &cs);
extern bool operator!=(char *s, const chars &cs);
extern bool operator!=(const char *s, const chars &cs);


#endif

/* End of charscl.h */
