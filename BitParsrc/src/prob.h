/*******************************************************************/
/*      File: prob.h                                               */
/*    Author: Helmut Schmid                                        */
/*   Purpose: Logarithmic representation of probabilities to avoid */
/*            underflow problems                                   */
/*   Created: Tue Oct 29 10:01:36 2002                             */
/*  Modified: Thu Jun 14 10:24:39 2012 (schmid)                    */
/* Copyright: Institut fuer maschinelle Sprachverarbeitung         */
/*            Universitaet Stuttgart                               */
/*******************************************************************/

#ifndef PROB_H
#define PROB_H

#include <float.h>
#include <math.h>
#include <assert.h>

static const double LOG_ZERO = -DBL_MAX;
static const double MaxLogDiff = 45;


class Prob {

  double logprob;

public:
  inline Prob( const double p ) {
    assert(p >= 0.0);
    if (p == 0.0)
      logprob = LOG_ZERO;
    else if (p == 1.0)
      logprob = 0.0;
    else
      logprob = log(p);
  }
  inline Prob( const Prob &p ) {
    logprob = p.logprob;
  }
  inline Prob() {
    logprob = LOG_ZERO;
  }
  inline Prob operator+(const Prob x) const;
  inline Prob operator-(const Prob x) const;
  inline Prob operator*(const Prob x) const;
  inline Prob operator*(const double x) const;
  inline Prob operator/(const Prob x) const;
  inline Prob operator+=(const Prob x);
  inline Prob operator-=(const Prob x);
  inline Prob operator*=(const Prob x);
  inline Prob operator*=(const double x);
  inline int  operator==(const Prob x) const;
  inline int  operator>(const Prob x) const;
  inline int  operator>=(const Prob x) const;
  inline int  operator<(const Prob x) const;
  inline int  operator<=(const Prob x) const;
  inline int  operator>(double p) const;
  inline int  operator>=(double p) const;
  inline int  operator<(double p) const;
  inline int  operator<=(double p) const;
  double log_val() const { return logprob / log(10); };
  operator double() const { 
    return exp(logprob);
  }
};


inline Prob Prob::operator*(const Prob x) const
{
  Prob result; // constructor setzt result auf 0

  if (logprob != LOG_ZERO && x.logprob != LOG_ZERO)
    result.logprob = logprob + x.logprob;
  return result;
}

inline Prob Prob::operator*(const double x) const
{
  Prob result; // constructor setzt result auf 0

  if (logprob != LOG_ZERO && x != 0.0)
    result.logprob = logprob + log(x);
  return result;
}

inline Prob Prob::operator/(const Prob x) const
{
  Prob result;

  if (x.logprob == LOG_ZERO) {
    assert(0);
    throw("division by zero");
  }
  else if (logprob == LOG_ZERO)
    result.logprob = LOG_ZERO;
  else
    result.logprob = logprob - x.logprob;
  return result;
}

inline Prob Prob::operator+(const Prob x) const
{
  double base;
  Prob result;

  if (logprob == LOG_ZERO)
    result.logprob = x.logprob;
  else if (x.logprob == LOG_ZERO)
    result.logprob = logprob;
  else if (logprob < x.logprob - MaxLogDiff)
    result = x;
  else if (x.logprob < logprob - MaxLogDiff)
    result = *this;
  /* else if (logprob < x.logprob ) { */
  /*   result.logprob = logprob + log(1 + exp(x.logprob-logprob)); */
  /* } */
  /* else { */
  /*   result.logprob = x.logprob + log(1 + exp(logprob-x.logprob)); */
  /* } */
  else {
    base = (logprob < x.logprob) ? logprob : x.logprob;
    result.logprob = base + log(exp(logprob-base) + exp(x.logprob-base));
  }
  return result;
}

inline Prob Prob::operator-(const Prob x) const
{
  double base;
  Prob result;

  if (x.logprob == LOG_ZERO)
    result.logprob = logprob;
  else if (logprob < x.logprob) {
    assert(0);
    throw("negative result of Prob subtraction");
  }
  else if (logprob - MaxLogDiff > x.logprob)
    result = *this;
  else {
    base = (logprob < x.logprob) ? logprob : x.logprob;
    result.logprob = base + log(exp(logprob-base) - exp(x.logprob-base));
  }
  return result;
}

inline Prob Prob::operator+=(const Prob x)
{
  return (*this = *this + x);
}

inline Prob Prob::operator-=(const Prob x)
{
  return (*this = *this - x);
}

inline Prob Prob::operator*=(const Prob x)
{
  if (logprob == LOG_ZERO)
    ;
  else if (x.logprob == LOG_ZERO)
    logprob = LOG_ZERO;
  else
    logprob += x.logprob;
  return *this;
}

inline Prob Prob::operator*=(const double x)
{
  if (logprob == LOG_ZERO)
    ;
  else if (x == 0.0)
    logprob = LOG_ZERO;
  else
    logprob += log(x);
  return *this;
}

inline int Prob::operator==(Prob x) const
{
  return (logprob == x.logprob);
}

inline int Prob::operator>(Prob x) const
{
  return (logprob > x.logprob);
}

inline int Prob::operator>=(Prob x) const
{
  return (logprob >= x.logprob);
}

inline int Prob::operator<(Prob x) const
{
  return (logprob < x.logprob);
}

inline int Prob::operator<=(Prob x) const
{
  return (logprob <= x.logprob);
}


inline int Prob::operator>(double p) const
{
  if (p == 0.0)
    return logprob > LOG_ZERO;
  return (logprob > log(p));
}

inline int Prob::operator>=(double p) const
{
  if (p == 0.0)
    return true;
  return (logprob >= log(p));
}

inline int Prob::operator<(double p) const
{
  if (p == 0.0)
    return false;
  return (logprob < log(p));
}

inline int Prob::operator<=(double p) const
{
  if (p == 0.0)
    return logprob == LOG_ZERO;
  return (logprob <= log(p));
}


inline Prob operator+(const double f, const Prob x)
{
  return (Prob)f + x;
}

inline Prob operator*(const double f, const Prob x)
{
  return (Prob)f * x;
}

inline Prob operator/(const double f, const Prob x)
{
  return (Prob)f / x;
}

inline Prob operator==(const double f, const Prob x)
{
  return ((Prob)f == x);
}

inline Prob operator>(const double f, const Prob x)
{
  return ((Prob)f > x);
}

inline Prob operator<(const double f, const Prob x)
{
  return ((Prob)f < x);
}


inline double log(const Prob x)
{
  return x.log_val();
}

#endif //PROB_H
